import streamDeck, {
  action,
  KeyDownEvent,
  KeyUpEvent,
  SingletonAction,
} from "@elgato/streamdeck";

const BASE_URL = process.env.BASE_URL;
const USERNAME = process.env.USERNAME;
const PASSWORD = process.env.PASSWORD;

const AUTH = `Basic ${Buffer.from(`${USERNAME}:${PASSWORD}`).toString(
  "base64",
)}`;
const REPEAT_INTERVAL_MS = 120; // kako pogosto pošiljamo +5/-5 dokler je gumb pritisnjen

type PremikDrziSettings = {
  dir?: "up" | "down";
};

/**
 * Akcija za kontinuiran premik stojala (drži gumb = premika se, spustiš = ustavi).
 * Ker /move?dir=X poveča/zmanjša zeljena_poz samo za fiksnih 5 enot na klic,
 * plugin ponavlja klic v intervalu, dokler je gumb pritisnjen.
 */
@action({ UUID: "com.sven.mikrofonsko-stojalo-pridobivanje-imen.premik-drzi" })
export class PremikDrzi extends SingletonAction<PremikDrziSettings> {
  private intervals = new Map<string, ReturnType<typeof setInterval>>();

  override async onKeyDown(
    ev: KeyDownEvent<PremikDrziSettings>,
  ): Promise<void> {
    const dir = ev.payload.settings.dir ?? "up";
    const context = ev.action.id;

    this.clearExistingInterval(context);

    const interval = setInterval(() => {
      void this.moveOnce(dir);
    }, REPEAT_INTERVAL_MS);

    this.intervals.set(context, interval);
    await this.moveOnce(dir);
    ev.action.showOk();
  }

  override async onKeyUp(ev: KeyUpEvent<PremikDrziSettings>): Promise<void> {
    const context = ev.action.id;
    this.clearExistingInterval(context);
    try {
      const res = await fetch(`${BASE_URL}/stop`, {
        headers: { Authorization: AUTH },
      });
      if (!res.ok) {
        streamDeck.logger.error(`Napaka pri ustavitvi: ${res.status}`);
      }
    } catch (err) {
      streamDeck.logger.error(`Ni povezave z ESP32: ${err}`);
    }
  }

  private clearExistingInterval(context: string): void {
    const existing = this.intervals.get(context);
    if (existing) {
      clearInterval(existing);
      this.intervals.delete(context);
    }
  }

  private async moveOnce(dir: string): Promise<void> {
    try {
      const res = await fetch(`${BASE_URL}/move?dir=${dir}`, {
        headers: { Authorization: AUTH },
      });
      if (!res.ok) {
        streamDeck.logger.error(
          `Napaka pri premiku (dir=${dir}): ${res.status}`,
        );
      }

      for (const button of streamDeck.actions) {
        if (
          button.isKey() &&
          button.manifestId ===
            "com.sven.mikrofonsko-stojalo-pridobivanje-imen.oseba"
        ) {
          await button.setImage();
        }
      }
    } catch (err) {
      streamDeck.logger.error(`Ni povezave z ESP32: ${err}`);
    }
  }
}
