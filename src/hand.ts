import { Card } from './card';

export class Hand {
    cards: Set<Card> = new Set<Card>();
    publicCards: Set<Card> = new Set<Card>();

    constructor(...cardNames: string[]) {
        this.addCards(cardNames.map(cardName => Card.fromString(cardName)));
    }

    numCards(): number {
        return this.cards.size;
    }

    hasCard(card: Card): boolean {
        return this.cards.has(card);
    }

    hasCards(cards: Card[]): boolean {
        return cards.every(card => this.hasCard(card));
    }

    addCard(card: Card) {
        this.cards.add(card);
    }

    addCards(cards: Card[]) {
        for (const card of cards) {
            this.addCard(card);
        }
    }

    addPublicCard(card: Card) {
        this.cards.add(card);
        this.publicCards.add(card);
    }

    addPublicCards(cards: Card[]) {
        for (const card of cards) {
            this.addPublicCard(card);
        }
    }

    removeCard(card: Card) {
        if (this.cards.has(card)) {
            this.cards.delete(card);
            this.publicCards.delete(card);
        } else {
            throw new Error(`Card ${card} not in hand`);
        }
    }

    removeCards(cards: Card[]) {
        for (const card of cards) {
            this.removeCard(card);
        }
    }

    points() : number {
        return 0;
    }

    toString(): string {
        return `${Card.cardsToString(Array.from(this.cards))} [${Card.cardsToString(Array.from(this.publicCards))}]`;
    }
}