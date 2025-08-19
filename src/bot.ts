import { Card } from "./card";
import { Game } from "./game";
import { Player } from "./player";
import { Meld, } from "./meld";

export class Bot {

    score: number = 0;
    game: Game;
    player: Player;

    constructor() {
    }

    play(game: Game) {
        this.game = game;
        this.player = this.game.currentPlayer();
        this.evalHand();
    }

    evalHand() {
        // Try adding each unseen card to the hand.
        const unseenCards = this.game.unseenCards;

        const table = this.game.table;
    }
};