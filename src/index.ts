import { Hand } from "./hand";
import { Table } from "./table";

let hand = new Hand("2C", "3C", "4C", "5C", "4S", "4D", "4H", "JH", "QH", "KH");
let table = new Table();
let melds = table.findMelds(hand);
for (const meld of melds) {
    console.log(meld.toString());
}
console.log("Melds found:", melds.length);

table.allMelds(hand);