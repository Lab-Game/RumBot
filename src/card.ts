
export class Card {

    static Names = [
        "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "TC", "JC", "QC", "KC", "AC", 
        "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "TD", "JD", "QD", "KD", "AD",
        "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "TH", "JH", "QH", "KH", "AH",
        "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "TS", "JS", "QS", "KS", "AS",
    ];
    static Points = [ 5, 5, 5, 5, 5, 5, 5, 5, 10, 10, 10, 10, 15 ];

    static deck: Card[] = Card.Names.map((name, index) => new Card(name, index));
    static byName: Map<string, Card> = new Map(Card.Names.map((name, index) => [name, Card.deck[index]]));

    name: string;
    index: number;
    tripleSet: Meld;
    singleSet: Meld;
    tripleRun?: Meld;
    singleRun: Meld;
    singleRunLow?: Meld;

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
        return this.value() == Card.Value.Ace;
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

    oppositeOfValue(): Card {
        return Card.deck[(this.index + 26) % 52];
    }
}

export namespace Card {
    export enum Value {
        Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace
    }
}

export class Meld {

    cards: Card[];
    points: number;
    addOns: Meld[] = [];

    constructor(cards: Card[]) {
        this.cards = cards;
        this.points = cards.reduce((sum, card) => sum + card.points(), 0);
        this.addOns = [];
    }

    toString(): string {
        return `[${Card.cardsToString(this.cards)}: ${this.points} pts]`;
    }

    toLongString(): string {
        return `${this.toString()} + ${this.addOns.map(meld => meld.toString()).join(", ")}`;
    }
}

// Attach melds to cards
for (const card of Card.deck) {
    card.tripleSet = new Meld([card.prevOfValue(), card, card.nextOfValue()]);
    card.singleSet = new Meld([card]);
    card.singleRun = new Meld([card]);
    if (card.isAce()) {
        card.singleRunLow = new Meld([card]);
        card.singleRunLow.points -= 10;
    } else {
        card.tripleRun = new Meld([card.prevInSuit(), card, card.nextInSuit()]);
        if (card.prevInSuit().isAce()) {
            card.tripleRun.points -= 10;
        }
    }
}

// Record the only way to extend a three-card set.
for (const card of Card.deck) {
    if (!card.isAce()) {
        card.tripleSet.addOns.push(card.oppositeOfValue().singleSet);
    }
}

// Record ways to add a card to a single-card run.
for (const card of Card.deck) {
    if (!card.isAce()) {
        if (card.value() >= Card.Value.Four) {
            card.singleRun.addOns.push(card.nextInSuit().singleRun);
        }
        if (card.value() <= Card.Value.Jack) {
            card.singleRun.addOns.push(card.prevInSuit().singleRunLow ?? card.prevInSuit().singleRun);
        }
    }
}

// Record ways to add a card to a three-card run.
for (const card of Card.deck) {
    if (card.tripleRun) {
        const firstCard = card.tripleRun.cards[0];
        const lastCard = card.tripleRun.cards[2];
        if (!firstCard.isAce()) {
            card.tripleRun.addOns.push(firstCard.prevInSuit().singleRunLow ?? firstCard.prevInSuit().singleRun);
        }
        if (!lastCard.isAce()) {
            card.tripleRun.addOns.push(lastCard.nextInSuit().singleRun);
        }
    }
}

/*
for (const card of Card.deck) {
    console.log("card:", card.toString());
    console.log("  tripleset:", card.tripleSet.toString());
    console.log("  singleset:", card.singleSet.toString());
    console.log("  triplerun:", card.tripleRun?.toString());
    console.log("  singleRun:", card.singleRun.toString());
    console.log("  singleRunLow:", card.singleRunLow?.toString());
}
*/

