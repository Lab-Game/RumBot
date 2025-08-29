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
        console.log("initial game", game.toString());

        while (true) {
            console.log("-----");
            this.bots[game.currentPlayerIndex].takeTurn(game);
            console.log("game after turn", game.toString());
            if (game.currentPlayer().hand.size() == 0 || game.drawPile.length == 0) {
                break;
            } else {
                game.nextTurn();
            }
        }

        for (const player of game.players) {
            console.log(`Player ${player.index} got ${player.points} points -> ${this.bots[player.index].totalPoints + player.points}`);
            this.bots[player.index].totalPoints += player.points;
        }
    }
}