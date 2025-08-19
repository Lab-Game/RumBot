import { Meld } from "./meld";

export class Card {

    static Names = [
        "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "TC", "JC", "QC", "KC"," AC", 
        "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "TD", "JD", "QD", "KD", "AD",
        "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "TH", "JH", "QH", "KH", "AH",
        "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "TS", "JS", "QS", "KS", "AS",
    ];
    static Points = [ 5, 5, 5, 5, 5, 5, 5, 5, 10, 10, 10, 10, 15 ];

    static TWO_VALUE = 0;
    static FOUR_VALUE = 2;
    static JACK_VALUE = 9;
    static ACE_VALUE = 12;

    static deck: Card[] = Card.Names.map((name, index) => new Card(name, index));
    static byName: Map<string, Card> = new Map(Card.Names.map((name, index) => [name, Card.deck[index]]));

    name: string;
    index: number;

    private constructor(name: string, index: number) {
        this.name = name;
        this.index = index;
    }

    toString(): string {
        return this.name;
    }

    static cardsToString(cards: Card[]): string {
        return cards.map(card => card.toString()).join(" ");
    }

    static fromString(name: string): Card {
        let card = Card.byName.get(name);
        if (card === undefined) {
            throw new Error(`Card ${name} not found`);
        }
        return card;
    }

    suit(): number {
        return Math.floor(this.index / 13) * 13;
    }

    value(): number {
        return this.index % 13;
    }

    points(): number {
        return Card.Points[this.value()];
    }

    isAce(): boolean {
        return this.value() == Card.ACE_VALUE;
    }

    prevInSuit(): Card {
        return Card.deck[(this.index + 12) % 13 + this.suit()];
    }

    nextInSuit(): Card {
        return Card.deck[(this.index + 1) % 13 + this.suit()];
    }

    prevOfValue(): Card {
        return Card.deck[(this.index + 39) % 52];
    }

    nextOfValue(): Card {
        return Card.deck[(this.index + 13) % 52];
    }
}
