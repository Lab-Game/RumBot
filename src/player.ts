import { Hand } from "./hand";
import { Game } from "./game";
import { Card } from "./card";
import { Meld } from "./meld";

export class Player {

    game : Game;
    index : number;
    isBot : boolean;
    hand : Hand;
    points : number;

    constructor(game : Game) {
        this.game = game;
        this.index = this.game.players.length;
        this.game.players.push(this);
        this.isBot = false;
        this.hand = new Hand();
        this.points = 0;
    }

    melds(): Meld[] {
        let candidates = new Set<Meld>();
        for (const card of this.hand.knownCards) {
            for (const meld of card.meldList) {
                candidates.add(meld);
            }
        }

        let melds: Meld[] = [];
        for (const meld of candidates) {
            if (this.hand.hasCards(meld.cards) && this.game.table.canPlay(meld)) {
                melds.push(meld);
            }
        }
        return melds;
    }

    playMeld(meld: Meld) {
        if (this.hand.hasCards(meld.cards) && this.game.table.canPlay(meld)) {
            this.points += this.game.table.play(meld);
            this.hand.removeCards(meld.cards);
        } else {
            throw new Error(`Cannot play meld: ${meld}`);
        }
    }

    toString() : string {
        return `Player ${this.index} ${this.isBot ? "(Bot)" : ""}: ${this.hand.toString()} (${this.points}pts)\n`;
    }
}