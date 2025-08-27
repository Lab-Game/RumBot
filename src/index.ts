import { Arena } from "./arena";
import { Bot } from "./bot";
import { Card } from "./card.ts";

const bots = [ new Bot(), new Bot(), new Bot() ];
const arena = new Arena(bots);
arena.runGame();


