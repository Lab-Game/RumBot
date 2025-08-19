import { Card } from "./card";
import { Table } from "./table";
import { Player } from "./player";

export class Game {

    deck : Card[];
    discardPile : Card[] = [];
    players : Player[] = [];
    playerIndex : number;
    table : Table;
    unseenCards : Set<Card> = new Set<Card>(Card.deck);

    constructor(numPlayers : number) {
        this.playerIndex = 0;
        for (let i = 0; i < numPlayers; i++) {
            new Player(this);
        }

        this.deck = [...Card.deck];
        // shuffle the deck!

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
        this.playerIndex = (this.playerIndex + 1) % this.players.length;
    }

    currentPlayer() : Player {
        return this.players[this.playerIndex];
    }

    addToDiscard(card: Card) {
        this.discardPile.push(card);
    }

    removeFromDiscard(numCards: number): Card[] {
        return this.discardPile.splice(-numCards);
    }

    toString() : string {
        let playersString = this.players.map(player => String(player)).join("");
        let discardString = `discard: ${Card.cardsToString(this.discardPile)}\n`;
        let tableString = `table: ${this.table.toString()}\n`;
        return playersString + discardString + tableString;
    }
}