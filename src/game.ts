import { Card, Meld } from "./card";
import { Player } from "./player";
import { Table } from "./table";

export class Game {
    drawPile : Card[];
    discardPile : Card[];
    discarded : Set<Card>;  // Was the card *ever* discarded?
    numPlayers : number;
    currentPlayerIndex : number;
    players : Player[];
    melds : Set<Meld>;

    private constructor() {}

    // Set up a new game with deck shuffled and cards dealt for a
    // specified number of players.
    static create(numPlayers : number) : Game {
        const game = new Game();

        // Create the draw pile
        game.drawPile = [...Card.deck];
        Card.shuffleDeck(game.drawPile);

        // Create players and deal hands
        game.numPlayers = numPlayers;
        game.players = [];
        for (let i = 0; i < game.numPlayers; i++) {
            game.players.push(Player.create(game));
        }
        game.currentPlayerIndex = 0;
        for (let i = 0; i < 7; ++i) {
            for (const player of game.players) {
                player.hand.addCard(game.drawPile.pop()!);
            }
        }

        // Create discard pile with one card
        game.discardPile = [ game.drawPile.pop()! ];
        game.discarded = new Set<Card>(game.discardPile);

        // Create melds set, initially empty
        game.melds = new Set<Meld>();

        return game;
    }

    static variant(original: Game, forPlayer: Player) : Game {
        // Randomly permute all cards in the game whose identity forPlayer can
        // not determine.  This includes all cards in the draw pile together
        // with all cards in the hands of other players that were
        // not drawn from the discard pile.
        const game = new Game();
        game.drawPile = [...original.drawPile];
        game.discardPile = [...original.discardPile];
        game.discarded = new Set(original.discarded);
        game.numPlayers = original.numPlayers;
        game.currentPlayerIndex = original.currentPlayerIndex;
        game.players = [];
        for (const player of original.players) {
            game.players.push(player.clone(game));
        }
        game.melds = new Set(original.melds);

        // Put all cards in opponent hands back into the draw pile,
        // except for those that were drawn from the discard pile.
        for (const player of game.players) {
            if (player.index !== forPlayer.index) {
                for (const card of player.hand.cards) {
                    if (!game.discarded.has(card)) {
                        player.undrawCard(card);
                    }
                }
            }
        }

        // Shuffle the draw pile
        Card.shuffleDeck(game.drawPile);

        // Each player now draws enough cards to replace those they lost
        for (const player of game.players) {
            let originalPlayer = original.players[player.index];
            while (player.hand.size() < originalPlayer.hand.size()) {
                player.drawCard();
            }
        }

        return game;
    }

    clone() {
        const game = new Game();
        game.drawPile = [...this.drawPile];
        game.discardPile = [...this.discardPile];
        game.discarded = new Set(this.discarded);
        game.numPlayers = this.numPlayers;
        game.currentPlayerIndex = this.currentPlayerIndex;
        game.players = this.players.map(player => player.clone(game));
        game.melds = new Set(this.melds);
        return game;
    }

    nextTurn() {
        this.currentPlayerIndex = (this.currentPlayerIndex + 1) % this.players.length;
    }

    player() : Player {
        return this.players[this.currentPlayerIndex];
    }

    toString() : string {
        let drawPileString = `drawPile: ${Card.cardsToString(this.drawPile)}\n`;
        let discardPileString = `discardPile: ${Card.cardsToString(this.discardPile)}\n`;
        let playersString = this.players.map(player => String(player)).join("");
        let meldString = `melds: ${[...this.melds].map(meld => String(meld)).join(" ")}\n`;
        let toPlayString = `player ${this.currentPlayerIndex} is playing\n`;
        return drawPileString + discardPileString + playersString + meldString + toPlayString;
    }
}