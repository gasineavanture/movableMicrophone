import streamDeck, {
  action,
  KeyDownEvent,
  SingletonAction,
  WillAppearEvent,
  WillDisappearEvent,
} from "@elgato/streamdeck";

const BASE_URL = process.env.BASE_URL;
const USERNAME = process.env.USERNAME;
const PASSWORD = process.env.PASSWORD;

const AUTH = `Basic ${Buffer.from(`${USERNAME}:${PASSWORD}`).toString(
  "base64",
)}`;
const POLL_INTERVAL_MS = 5000;

type OsebaSettings = {
  id?: number;
};

/**
 * Akcija za en gumb "osebe" na Stream Decku. Vsaka instanca ima svoj `id` (0-9),
 * ki pove, katero osebo iz ESP32 PSRAM-a predstavlja.
 */
@action({ UUID: "com.sven.mikrofonsko-stojalo-pridobivanje-imen.oseba" })
export class Oseba extends SingletonAction<OsebaSettings> {
  private readonly pollers = new Map<string, ReturnType<typeof setInterval>>();

  override async onWillAppear(
    ev: WillAppearEvent<OsebaSettings>,
  ): Promise<void> {
    await this.refreshTitle(ev.action);

    const previousPoller = this.pollers.get(ev.action.id);
    if (previousPoller) clearInterval(previousPoller);

    const poller = setInterval(() => {
      void this.refreshTitle(ev.action);
    }, POLL_INTERVAL_MS);
    this.pollers.set(ev.action.id, poller);
  }

  override onWillDisappear(ev: WillDisappearEvent<OsebaSettings>): void {
    const poller = this.pollers.get(ev.action.id);
    if (poller) {
      clearInterval(poller);
      this.pollers.delete(ev.action.id);
    }
  }

  /**
   * Ob pritisku gumba pošlji ukaz za premik stojala na izbrano osebo.
   */
  override async onKeyDown(ev: KeyDownEvent<OsebaSettings>): Promise<void> {
    const id = Number(ev.payload.settings.id ?? 0);
    const svg = `<svg xmlns="http://www.w3.org/2000/svg"
        width="144" height="144">
        <rect width="144" height="144" fill="#0080ff"/>
      </svg>`;

    const selectedImage = `data:image/svg+xml;base64,${Buffer.from(svg).toString("base64")}`;

    for (const button of this.actions) {
      if (!button.isKey()) continue;

      if (button.id === ev.action.id) {
        await button.setImage(selectedImage);
      } else {
        await button.setImage();
      }
    }

    try {
      const res = await fetch(`${BASE_URL}/person?id=${id}`, {
        headers: { Authorization: AUTH },
      });
      if (!res.ok) {
        streamDeck.logger.error(`Napaka pri premiku (id=${id}): ${res.status}`);
        await ev.action.setImage();
      }
    } catch (err) {
      streamDeck.logger.error(`Ni povezave z ESP32: ${err}`);
    }
  }

  private async refreshTitle(
    action: WillAppearEvent<OsebaSettings>["action"],
  ): Promise<void> {
    try {
      const settings = await action.getSettings<OsebaSettings>();
      const id = Number(settings.id ?? 0);
      const res = await fetch(`${BASE_URL}/people`, {
        headers: { Authorization: AUTH },
      });
      if (!res.ok) {
        streamDeck.logger.error(`Napaka pri branju /people: ${res.status}`);
        return;
      }

      const people = (await res.json()) as { name: string }[];
      const name = people[id]?.name?.trim();
      await action.setTitle(name && name.length > 0 ? name : `Oseba ${id + 1}`);
    } catch (err) {
      streamDeck.logger.error(`Napaka pri branju imen: ${err}`);
    }
  }
}
