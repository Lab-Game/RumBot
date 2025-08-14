import { Card } from "./card";
import { Table } from "./table";
import { Player } from "./player";

export class Game {

    players : Player[] = [];
    discardPile : Card[];
    table : Table;
    currentPlayerIndex : number;
    botIndex : number;

    constructor(numPlayers : number, botIndex : number, discard : Card) {
        for (let i = 0; i < numPlayers; i++) {
            new Player(this);
        }
        this.discardPile = [ discard ];
        this.table = new Table();
        this.currentPlayerIndex = 0;
        this.botIndex = botIndex;
        this.bot().isBot = true;
    }

    addToDiscard(card: Card) {
        this.discardPile.push(card);
    }

    removeFromDiscard(numCards: number): Card[] {
        return this.discardPile.splice(-numCards);
    }

    currentPlayer() : Player {
        return this.players[this.currentPlayerIndex];
    }

    bot() : Player {
        return this.players[this.botIndex];
    }

    toString() : string {
        let playersString = this.players.map(player => String(player)).join("");
        let discardString = `discard: ${Card.cardsToString(this.discardPile)}\n`;
        let TableString = `table: ${this.table.toString()}\n`;
        return playersString + discardString + TableString +
            `current player: ${this.currentPlayerIndex} (${this.currentPlayer().isBot ? "Bot" : "Human"})\n`;
    }
}