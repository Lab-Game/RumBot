import { Arena } from "./arena";
import { Bot } from "./bot";

for (let j = 0; j < 10; ++j) {
    const bots = [ new Bot(0), new Bot(0), new Bot(1) ];
    for (let i = 0; i < 1000; i++) {
        const arena = new Arena(bots, false);
        arena.runGame();
    }
    console.log("-----");
    let sortedBots = [...bots].sort((a, b) => b.totalPoints - a.totalPoints);
    for (const bot of sortedBots) {
        console.log(bot.strategy, bot.totalPoints, bot.wins);
    }
}