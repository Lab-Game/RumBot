import { Card } from './card';
import { Meld } from './meld';

export class Table {
    join: Meld.Join[] = new Array(52).fill(Meld.Join.NONE);

    constructor() {}

    canPlay(meld: Meld): boolean {
        if (meld.cards.length == 1) {
            let card = meld.cards[0];
            if (meld.join === Meld.Join.RUN) {
                let prev = card.prevInSuit();
                let next = card.nextInSuit();
                return ((!prev.isAce() && this.join[prev.index] == Meld.Join.RUN) ||
                        (!next.isAce() && this.join[next.index] == Meld.Join.RUN));
            } else if (meld.join === Meld.Join.SET) {
                return this.join[card.nextWithValue().index] == Meld.Join.SET;
            }
        }
        return true;
    }

    play(meld: Meld) {
        for (const card of meld.cards) {
            this.join[card.index] = meld.join;
        }
    }

    toString(): string {
        return this.join.map((join, index) => (join == Meld.Join.NONE ? "" : (".rs"[join] + Card.deck[index]))).join(" ");
    }
}
