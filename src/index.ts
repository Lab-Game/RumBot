import { Arena } from "./arena";
import { Bot } from "./bot";

const bots = [new Bot(), new Bot(), new Bot()];
const arena = new Arena(bots);
arena.runGame();