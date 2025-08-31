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

        do {
            let bot = this.bots[game.currentPlayerIndex];
            if (this.verbose) {
                let index = game.currentPlayerIndex;
                console.log(`\n--- Player ${index}, Strategy = ${this.bots[index].strategy} ---`);
                console.log(game.toString());
            }
            bot.takeTurn(game);
        } while (game.nextTurn());

        // Determine the winner(s)
        let maxPoints = Math.max(...this.bots.map(bot => bot.gamePoints));
        let winners = this.bots.filter(bot => bot.gamePoints === maxPoints);
        for (const winner of winners) {
            winner.wins += 1.0 / winners.length;
        }
    }
}