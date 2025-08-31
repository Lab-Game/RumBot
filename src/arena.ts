import { Bot } from "./bot";
import { Game } from "./game";

export class Arena {

    bots: Bot[];
    verbose: boolean;

    constructor(bots : Bot[], verbose?: boolean) {
        if (bots.length < 2) {
            throw new Error("At least two bots are required.");
        }
        this.bots = bots;
        this.verbose = verbose ?? false;
    }

    runGame() {
        // Shuffle the array of bots
        for (let i = this.bots.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [this.bots[i], this.bots[j]] = [this.bots[j], this.bots[i]];
        }

        const game = Game.create(this.bots.length, this.verbose);

        console.log(game.toString());

        while (true) {
            if (this.verbose) {
                let index = game.currentPlayerIndex;
                console.log(`\n--- Player ${index}, Strategy = ${this.bots[index].strategy} ---`);
                console.log(game.toString());
            }
            this.bots[game.currentPlayerIndex].takeTurn(game);

            if (game.player().hand.size() == 0 || game.drawPile.length == 0) {
                break;
            } else {
                game.nextTurn();
            }
        }

        for (const player of game.players) {
            this.bots[player.index].totalPoints += player.points - player.hand.points();
        }

        // Determine the winner(s)
        let maxPoints = Math.max(...this.bots.map(bot => bot.totalPoints));
        let winners = this.bots.filter(bot => bot.totalPoints === maxPoints);
        for (const winner of winners) {
            winner.wins += 1.0 / winners.length;
        }
    }
}