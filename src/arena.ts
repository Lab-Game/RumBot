// An Arena runs rummy games with bot players.

import { Bot } from "./bot";
import { Game } from "./game";

export class Arena {

    bots: Bot[];

    constructor(bots : Bot[]) {
        if (bots.length < 2) {
            throw new Error("At least two bots are required.");
        }
        this.bots = bots;
    }

    runGame() {
        const game = new Game(this.bots.length);
        while (true) {
            this.bots[game.playerIndex].play(game);
            if (game.currentPlayer().hand.numCards() == 0) {
                for (let i = 0; i < game.players.length; i++) {
                    this.bots[i].score += game.players[i].points - game.players[i].hand.points();
                }
                return;
            } else {
                game.next();
            }

            break; // for testing
        }
    }
}