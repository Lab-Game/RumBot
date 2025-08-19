import { Hand } from "./hand";
import { Game } from "./game";
import { Card } from "./card";
import { Meld } from "./meld";

export class Player {

    game : Game;
    index : number;
    hand : Hand;
    points : number;

    constructor(game : Game) {
        this.game = game;
        this.index = this.game.players.length;
        this.game.players.push(this);
        this.hand = new Hand();
        this.points = 0;
    }

    melds(): Meld[] {
        let candidates = new Set<Meld>();
        for (const card of this.hand.cards) {
            for (const meld of card.melds) {
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

    oneMeld(): Meld | undefined {
        for (const card of this.hand.cards) {
            for (const meld of card.melds) {
                if (this.hand.hasCards(meld.cards) && this.game.table.canPlay(meld)) {
                    return meld;
                }
            }
        }

        return undefined;
    }

    playMeld(meld: Meld) {
        if (this.hand.hasCards(meld.cards) && this.game.table.canPlay(meld)) {
            this.points += this.game.table.play(meld);
            this.hand.removeCards(meld.cards);
        } else {
            throw new Error(`Cannot play meld: ${meld}`);
        }
    }

    unplayMeld(meld: Meld) {
        
    }

    toString() : string {
        return `Player ${this.index}: ${this.hand.toString()} (${this.points}pts)\n`;
    }
}