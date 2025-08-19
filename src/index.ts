import { Meld } from "./meld";
import { Hand } from "./hand";
import { Table } from "./table";

console.log("Initializaing melds...");
Meld.init();

let hand = new Hand("2C", "3C", "4C", "5C", "4S", "4D", "4H", "JH");
let table = new Table();
let melds = table.findMelds(hand);
for (const meld of melds) {
    console.log(meld.toString());
}
console.log("Melds found:", melds.length);
