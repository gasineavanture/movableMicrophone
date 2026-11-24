import streamDeck, {
  action,
  KeyDownEvent,
  SingletonAction,
} from "@elgato/streamdeck";

const BASE_URL = process.env.BASE_URL;
const USERNAME = process.env.USERNAME;
const PASSWORD = process.env.PASSWORD;

const AUTH = `Basic ${Buffer.from(`${USERNAME}:${PASSWORD}`).toString(
  "base64",
)}`;

type NastaviHitrostSettings = {
  speed?: number;
};

@action({
  UUID: "com.sven.mikrofonsko-stojalo-pridobivanje-imen.hitrost",
})
export class NastaviHitrost extends SingletonAction<NastaviHitrostSettings> {
  override async onKeyDown(
    ev: KeyDownEvent<NastaviHitrostSettings>,
  ): Promise<void> {
    const speed = ev.payload.settings.speed ?? 50;
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
      const res = await fetch(`${BASE_URL}/setSpeed?speed=${speed}`, {
        headers: { Authorization: AUTH },
      });
      if (!res.ok) {
        streamDeck.logger.error(
          `Napaka pri nastavitvi hitrosti (speed=${speed}): ${res.status}`,
        );
        await ev.action.showAlert();
        return;
      }
    } catch (err) {
      streamDeck.logger.error(`Ni povezave z ESP32: ${err}`);
      await ev.action.showAlert();
    }
  }
}
