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

        for (const player of game.players) {
            console.log(player.toString());
        }
        console.log(`Discard pile: ${game.discardPile[0]}`);

        for (let i = 0; i < this.bots.length; i++) {
            this.bots[i].beginGame(game);
        }

        while (true) {
            this.bots[game.currentPlayerIndex].generatePlays();
            if (game.currentPlayer().hand.numCards() == 0) {
                break;
            } else {
                game.next();
            }

            break; // for safety during testing
        }

        for (let i = 0; i < this.bots.length; i++) {
            this.bots[i].endGame();
        }
    }
}