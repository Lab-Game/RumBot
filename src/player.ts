import { Hand } from "./hand";
import { Game } from "./game";
import { Card } from "./card";
import { Meld } from "./meld";

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

    toString() : string {
        return `Player ${this.index}: ${this.hand.toString()} (${this.points}pts)\n`;
    }
}