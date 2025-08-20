import { Game } from "./game";
import { Player } from "./player";
import { Play } from "./play";
import {  Hand } from "./hand";

export class Bot {

    game: Game;
    player: Player;
    score: number = 0;
    play: Play = new Play();
    plays: Play[] = [];

    constructor() {
    }

    beginGame(game: Game) {
        this.game = game;
        this.player = this.game.currentPlayer();
    }

    generatePlays() {
        this.plays.length = 0;
        this.chooseDiscardsTaken();
    }

    chooseDiscardsTaken() {
        for (let n = 1; n < this.game.discardPile.length; n++) {
            this.play.discardsTaken = n;
            let discards = this.game.removeFromDiscard(n);
            this.game.returnToDiscard(discards);
        }
    }

    endGame() {
        this.score += this.player.points - this.player.hand.points();
    }
};