import { Card } from "./card";
import { Meld } from "./meld";
import { Bot } from "./bot";

console.log("Initializaing cards...");
Card.init();
console.log("Initializaing melds...");
Meld.init();
console.log("Starting bot...");
let b = new Bot();
