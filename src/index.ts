import { Arena } from "./arena";
import { Bot } from "./bot";

// npx vite-node src/index.ts

for (let j = 0; j < 1; ++j) {
    const bots = [ new Bot(0), new Bot(0), new Bot(5) ];
    const arena = new Arena(bots, true);
    for (let i = 0; i < 1; i++) {
        arena.runGame();
    }

    console.log("-----");

    let sortedBots = [...bots].sort((a, b) => b.totalPoints - a.totalPoints);
    for (const bot of sortedBots) {
        console.log(bot.strategy, bot.totalPoints, bot.wins);
    }
}