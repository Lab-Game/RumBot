import { Card } from './card';

export class Hand {
    cards: Set<Card> = new Set<Card>();

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

    removeCard(card: Card) {
        this.cards.delete(card);
    }

    removeCards(cards: Card[]) {
        for (const card of cards) {
            this.removeCard(card);
        }
    }

    points() : number {
        return 0;  // TODO
    }

    clone() {
        const clone = new Hand();
        clone.cards = new Set<Card>(this.cards);
        return clone;
    }

    toString(): string {
        return `${Card.cardsToString(Array.from(this.cards))}`;
    }
}