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

type PremikFiksnoSettings = {
  cm?: string | number;
};

@action({
  UUID: "com.sven.mikrofonsko-stojalo-pridobivanje-imen.premik-fiksno",
})
export class PremikFiksno extends SingletonAction<PremikFiksnoSettings> {
  override async onKeyDown(
    ev: KeyDownEvent<PremikFiksnoSettings>,
  ): Promise<void> {
    const cm = Number(ev.payload.settings.cm ?? 1);
    try {
      const res = await fetch(`${BASE_URL}/move?cm=${cm}`, {
        headers: { Authorization: AUTH },
      });
      if (!res.ok) {
        streamDeck.logger.error(`Napaka pri premiku (cm=${cm}): ${res.status}`);
      }
    } catch (err) {
      streamDeck.logger.error(`Ni povezave z ESP32: ${err}`);
    } finally {
      await ev.action.showOk();
    }
  }
}
