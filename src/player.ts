import { Hand } from "./hand";
import { Game } from "./game";
import { Card, Meld } from "./card";

export class Player {

    game : Game;
    index : number;
    hand : Hand;
    points : number;

    constructor(game : Game) {
        this.game = game;
        this.index = this.game.players.length;
        this.game.players.push(this);
        this.hand = new Hand();
        this.points = 0;
    }

    clone() {
        const player = new Player(this.game);
        player.index = this.index;
        player.hand = this.hand.clone();
        player.points = this.points;
        return player;
    }

    takeCards(numCards: number) {
    }

    untakeCards(cards) {

    }

    drawCard() {

    }

    undrawCard(card: Card) {

    }

    putMeld(meld: Meld) {
        this.hand.removeCards(meld.cards);
        this.game.playMeld(meld);
    }

    unputMeld(meld: Meld) {
        this.hand.addCards(meld.cards);
        this.game.unplayMeld(meld);
    }

    discard(card: Card) {
    }

    undiscard(card: Card) {

    }

    toString() : string {
        return `Player ${this.index}: ${this.hand.toString()} (${this.points}pts)\n`;
    }

/*
    discard(card: Card) {
        this.discardPile.push(card);
    }

    takeFromDiscard(numCards: number): Card[] {
        // TODO:  Shouldn't these cards become undiscarded?
        // Check the callers...
        return this.discardPile.splice(-numCards);
    }

    returnToDiscard(cards: Card[]) {
        // These are no longer undiscarded
        this.discardPile.push(...cards);
    }
*/

}