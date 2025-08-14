import { Card } from "./card";

export class Meld {

    static meldList: Meld[] = [];

    static init() {
        for (const card of Card.deck) {
            card.addMeld(new Meld(Meld.Join.SET, [ card ]));
            card.addMeld(new Meld(Meld.Join.RUN, [ card ]));
            card.addMeld(new Meld(Meld.Join.SET, [ card.prevWithValue(), card, card.nextWithValue() ]));
            if (!card.isAce()) {
                card.addMeld(new Meld(Meld.Join.RUN, [ card.prevInSuit(), card, card.nextInSuit() ]));
            }
        }
    }

    static find(join: Meld.Join, cards: Card[]): Meld {
        for (const meld of Meld.meldList) {
            if (meld.join === join && meld.cards.length === cards.length &&
                meld.cards.every(card => cards.includes(card))) {
                return meld;
            }
        }
        throw new Error(`Meld not found for join ${join} of ${Card.cardsToString(cards)}`);
    }

    join: Meld.Join;
    cards: Card[];
    points: number;

    constructor(join: Meld.Join, cards: Card[]) {
        this.join = join;
        this.cards = cards;
        this.points = cards.reduce((sum, card) => sum + card.points(), 0);
        if (this.join === Meld.Join.RUN && this.cards[0].isAce()) {
            this.points -= 10;
        }
        Meld.meldList.push(this);
    }

    toString(): string {
        return `Join ${this.join} of ${Card.cardsToString(this.cards)}`;
    }
}

export namespace Meld {
    export enum Join { NONE, SET, RUN };
}