import { Card } from './card';

export class Hand {
    knownCards: Set<Card> = new Set<Card>();
    unknownCards: number = 0;

    constructor() {
    }

    numCards(): number {
        return this.knownCards.size + this.unknownCards;
    }

    hasCard(card: Card): boolean {
        return this.knownCards.has(card);
    }

    addCard(card: Card) {
        this.knownCards.add(card);
    }

    addCards(cards: Card[]) {
        for (const card of cards) {
            this.addCard(card);
        }
    }

    addUnknownCards(num: number) {
        this.unknownCards += num;
    }

    removeCard(card: Card) {
        if (this.knownCards.has(card)) {
            this.knownCards.delete(card);
        } else if (this.unknownCards > 0) {
            this.unknownCards -= 1;
        } else {
            throw new Error(`Card ${card} not in hand`);
        }
    }

    removeCards(cards: Card[]) {
        for (const card of cards) {
            this.removeCard(card);
        }
    }

    toString(): string {
        return `${Card.cardsToString(Array.from(this.knownCards))} ${this.unknownCards}x??`;
    }
}