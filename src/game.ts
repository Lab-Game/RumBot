import { Card } from "./card";
import { Table } from "./table";
import { Player } from "./player";

export class Game {

    deck : Card[];
    discardPile : Card[] = [];
    players : Player[] = [];
    currentPlayerIndex : number;
    table : Table;
    unseenCards : Set<Card> = new Set<Card>(Card.deck);

    constructor(numPlayers : number) {
        this.currentPlayerIndex = 0;
        for (let i = 0; i < numPlayers; i++) {
            new Player(this);
        }

        this.deck = [...Card.deck];

        // Shuffle the deck
        for (let i = this.deck.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [this.deck[i], this.deck[j]] = [this.deck[j], this.deck[i]];
        }        

        // deal cards
        for (let i = 0; i < 7; ++i) {
            for (const player of this.players) {
                let card = this.deck.pop();
                if (card !== undefined) {
                    player.hand.addCard(card);
                    this.unseenCards.delete(card);
                } else {
                    throw new Error("Ran out of cards during deal.");
                }
            }
        }

        let discard = this.deck.pop();
        if (discard !== undefined) {
            this.discardPile.push(discard);
        } else {
            throw new Error("Ran out of cards when creating discard pile.");
        }
        this.unseenCards.delete(discard)

        this.table = new Table();
    }

    next() {
        this.currentPlayerIndex = (this.currentPlayerIndex + 1) % this.players.length;
    }

    currentPlayer() : Player {
        return this.players[this.currentPlayerIndex];
    }

    addToDiscard(card: Card) {
        this.discardPile.push(card);
    }

    removeFromDiscard(numCards: number): Card[] {
        return this.discardPile.splice(-numCards);
    }

    returnToDiscard(cards: Card[]) {
        this.discardPile.push(...cards);
    }

    toString() : string {
        let playersString = this.players.map(player => String(player)).join("");
        let discardString = `discard: ${Card.cardsToString(this.discardPile)}\n`;
        let tableString = `table: ${this.table.toString()}\n`;
        return playersString + discardString + tableString;
    }
}