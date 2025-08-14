import { Hand } from "./hand";
import { Game } from "./game";

export class Player {

    game : Game;
    index : number;
    isBot : boolean;
    hand : Hand;
    points : number;

    constructor(game : Game) {
        this.game = game;
        this.index = this.game.players.length;
        this.game.players.push(this);
        this.isBot = false;
        this.hand = new Hand();
        this.points = 0;
    }

    toString() : string {
        return `Player ${this.index} ${this.isBot ? "(Bot)" : ""}: ${this.hand.toString()} (${this.points}pts)\n`;
    }
}