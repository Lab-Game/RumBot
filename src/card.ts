import { Meld } from "./meld";

const cardNames = [
    "AC", "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "TC", "JC", "QC", "KC",
    "AD", "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "TD", "JD", "QD", "KD",
    "AH", "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "TH", "JH", "QH", "KH",
    "AS", "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "TS", "JS", "QS", "KS",
];

const cardPoints = [ 15, 5, 5, 5, 5, 5, 5, 5, 5, 10, 10, 10, 10 ];

export class Card {

    static deck : Card[] = [];
    static byName = new Map<string, Card>();

    static init() {
        for (const cardName of cardNames) {
            let card = new Card(cardName);
        }
    }

    static cardsToString(cards: Card[]): string {
        return cards.map(card => card.toString()).join(" ");
    }

    name: string;
    index: number;
    meldList: Meld[] = [];

    private constructor(name : string) {
        this.name = name;
        Card.byName.set(name, this);
        this.index = Card.deck.length;
        Card.deck.push(this);
    }

    static fromString(name: string): Card {
        const card = this.byName.get(name);
        if (card === undefined) {
            throw new Error(`Card not found: '${name}'`);
        }
        return card;
    }

    toString(): string {
        return cardNames[this.index];
    }

    points(): number {
        return cardPoints[this.index % 13];
    }

    value(): number {
        return this.index % 13;
    }

    suit(): number {
        return Math.floor(this.index / 13) * 13;
    }

    isAce(): boolean {
        return this.value() == 0;
    }

    prevInSuit(): Card {
        return Card.deck[(this.index + 12) % 13 + this.suit()];
    }

    nextInSuit(): Card {
        return Card.deck[(this.index + 1) % 13 + this.suit()];
    }

    prevWithValue(): Card {
        return Card.deck[(this.index + 39) % 52];
    }

    nextWithValue(): Card {
        return Card.deck[(this.index + 13) % 52];
    }

    addMeld(meld: Meld) {
        this.meldList.push(meld);
    }
}



