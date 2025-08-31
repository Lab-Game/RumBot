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
            bot.takeTurn(game);
        } while (game.nextTurn());

        for (let i = 0; i < this.bots.length; ++i) {
            let bot = this.bots[i];
            bot.finalScore = game.players[i].finalScore;
            bot.totalPoints += bot.finalScore;
        }

        // Determine the winner(s)
        let maxPoints = Math.max(...this.bots.map(bot => bot.finalScore));
        let winners = this.bots.filter(bot => bot.finalScore === maxPoints);
        for (const winner of winners) {
            winner.wins += 1.0 / winners.length;
        }
    }
}