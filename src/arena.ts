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
        const game = Game.create(this.bots.length);
        console.log(game.toString());

        while (true) {
            this.bots[game.currentPlayerIndex].takeTurn(game);
            if (game.currentPlayer().hand.numCards() == 0) {
                break;
            } else {
                game.nextTurn();
            }

            break; // for safety during testing
        }
    }
}