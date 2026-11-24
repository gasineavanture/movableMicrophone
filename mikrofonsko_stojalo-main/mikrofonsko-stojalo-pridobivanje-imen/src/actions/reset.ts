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

@action({ UUID: "com.sven.mikrofonsko-stojalo-pridobivanje-imen.reset" })
export class Reset extends SingletonAction {
  override async onKeyDown(ev: KeyDownEvent): Promise<void> {
    try {
      const res = await fetch(`${BASE_URL}/reset`, {
        headers: { Authorization: AUTH },
      });

      if (!res.ok) {
        streamDeck.logger.error(`Napaka pri resetu: ${res.status}`);
        await ev.action.showAlert();
        return;
      }
    } catch (err) {
      streamDeck.logger.error(`Ni povezave z ESP32: ${err}`);
      await ev.action.showAlert();
    } finally {
      await ev.action.showOk();
    }
  }
}
