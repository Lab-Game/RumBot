import { Card, Meld } from "./card";

// This represents the actions that a play can take during one
// turn:
//  - Take some discards or draw one card.
//  - Play some melds
//  - Discard a card
// We can also associate a "evaluation" score with this Play to
// help a bot determine which Play to pursue.
// Fields are filled in incrementally.

export class Play {

    discardsTaken: number;
    newCards: Card[];
    melds: Meld[];
    nonMelds: Set<Meld>
    discard: Card | null;
    evaluation: number;

    constructor(play: Play | null = null) {
        if (play) {
            this.discardsTaken = play.discardsTaken;
            this.newCards = [...play.newCards];
            this.melds = [...play.melds];
            this.discard = play.discard;
            this.evaluation = play.evaluation;
        } else {
            // Build a full structure
            this.discardsTaken = 0;
            this.newCards = [];
            this.melds = [];
            this.nonMelds = new Set<Meld>();
            this.discard = null;
            this.evaluation = 0;
        }
    }
}