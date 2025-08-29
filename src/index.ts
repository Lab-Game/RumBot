import { Arena } from "./arena";
import { Bot } from "./bot";
import { Card } from "./card.ts";

const bots = [ new Bot(0), new Bot(1), new Bot(0) ];
const arena = new Arena(bots);
for (let i = 0; i < 1000; i++) {
    arena.runGame();
}