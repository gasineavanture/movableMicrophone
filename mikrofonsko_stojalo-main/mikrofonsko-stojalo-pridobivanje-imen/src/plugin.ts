import streamDeck from "@elgato/streamdeck";

import { Oseba } from "./actions/oseba";
import { PremikFiksno } from "./actions/premik-fiksno";
import { PremikDrzi } from "./actions/premik-drzi";
import { NastaviHitrost } from "./actions/hitrost";
import { Referenca } from "./actions/referenca";
import { Reset } from "./actions/reset";

// We can enable "trace" logging so that all messages between the Stream Deck, and the plugin are recorded. When storing sensitive information
streamDeck.logger.setLevel("trace");

// Register the oseba action.
streamDeck.actions.registerAction(new Oseba());
streamDeck.actions.registerAction(new PremikFiksno());
streamDeck.actions.registerAction(new PremikDrzi());
streamDeck.actions.registerAction(new NastaviHitrost());
streamDeck.actions.registerAction(new Referenca());
streamDeck.actions.registerAction(new Reset());

// Finally, connect to the Stream Deck.
streamDeck.connect();
