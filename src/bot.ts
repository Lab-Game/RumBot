import { Card } from "./card";
import { Game } from "./game";
import { Meld, } from "./meld";

export class Bot {

    game : Game;

    constructor() {

        process.stdin.on("data", data => {
            console.log("Message:", data.toString());
            let fields = data.toString().trim().split(" ");
            let messageType = fields[0];

            if (messageType == "start") {
                let numPlayers = parseInt(fields[1]);
                let botIndex = parseInt(fields[2]);
                let discard = Card.fromString(fields[3]);
                this.game = new Game(numPlayers, botIndex, discard);
                let numCards = fields.length - 4;

                for(let player of this.game.players) {
                    if (player.isBot) {
                        for (let cardName of fields.slice(4)) {
                            player.hand.addCard(Card.fromString(cardName));
                        }
                    } else {
                        player.hand.addUnknownCards(numCards);
                    }
                }
            } else if (messageType == "player") {
                this.game.currentPlayerIndex = parseInt(fields[1]);
                if (this.game.currentPlayer().isBot) {
                    // Call to Bot AI logic goes here
                    console.log("Bot's turn logic not implemented yet.");
                }
            } else if (messageType == "take") {
                let numCards = parseInt(fields[1]);
                if (numCards > 0) {
                    let cards = this.game.removeFromDiscard(numCards);
                    this.game.currentPlayer().hand.addCards(cards);
                } else {
                    this.game.currentPlayer().hand.addUnknownCards(1);
                }
            } else if (messageType == "run") {
                let cards = fields.slice(1).map(cardName => Card.fromString(cardName));
                let meld = Meld.find(Meld.Join.RUN, cards);
                this.game.table.play(meld);
                this.game.currentPlayer().hand.removeCards(cards);
            } else if (messageType == "set") {
                let cards = fields.slice(1).map(cardName => Card.fromString(cardName));
                let meld = Meld.find(Meld.Join.SET, cards);
                this.game.table.play(meld);
                this.game.currentPlayer().hand.removeCards(cards);
            } else if (messageType == "discard") {
                let card = Card.fromString(fields[1]);
                this.game.currentPlayer().hand.removeCard(card);
                this.game.addToDiscard(card);
            } else if (messageType == "goodbye") {
                process.exit(0);
            }

            console.log("Game state:", String(this.game));
        });
    }
};