import { Card, Meld } from "./card";
import { Player } from "./player";
import { Table } from "./table";

export class Game {
    drawPile : Card[];
    discardPile : Card[];
    undiscards : Set<Card>;  // Discards taken into a player's hand
    numPlayers : number;
    currentPlayerIndex : number;
    players : Player[];
    melds : Set<Meld>;

    private constructor() {
        // Constructor is empty, because object creation is handled
        // by static functions.  We can either create a game from
        // scratch or create a variant where cards whose location is
        // unknown to a specified player are randomly reordered.
    }

    // Set up a new game with deck shuffled and cards dealt for a
    // specified number of players.
    static newGame(numPlayers : number) : Game {
        const game = new Game();

        // Create the draw pile
        game.drawPile = [...Card.deck];
        Card.shuffleDeck(game.drawPile);

        // Create players and deal hands
        game.numPlayers = numPlayers;
        game.players = Array.from({length:game.numPlayers}).map(() => new Player(game));
        game.currentPlayerIndex = 0;
        for (let i = 0; i < 7; ++i) {
            for (const player of game.players) {
                player.hand.addCard(game.drawPile.pop()!);
            }
        }

        // Create discard pile with one card
        game.discardPile = [ game.drawPile.pop()! ];

        // Create melds set, initially empty
        game.melds = new Set<Meld>();

        return game;
    }

    static variantGame(original: Game, forPlayer: Player) : Game {
        // Randomly permute all cards in the game whose identity forPlayer can
        // not determine.  This includes all cards in the draw pile together
        // with all cards in the hands of other players that were
        // not drawn from the discard pile.
        const game = new Game();
        game.discardPile = [...original.discardPile];
        game.undiscards = new Set(original.undiscards);
        game.numPlayers = original.numPlayers;
        game.players = Array.from({length:game.numPlayers}).map(() => new Player(game));
        for (const player of game.players) {
            let originalPlayer = original.players[player.index];
            player.points = originalPlayer.points;
        }
        game.melds = new Set(original.melds);

        // Build a list of all cards whose identity forPlayer can not determine
        let mysteryCards: Card[] = [...original.drawPile];
        for (const player of original.players) {
            if (player.index !== forPlayer.index) {
                for (const card of player.hand.cards) {
                    if (!game.undiscards.has(card)) {
                        mysteryCards.push(card);
                    }
                }
            }
        }

        // Shuffle the mysterious cards
        Card.shuffleDeck(mysteryCards);

        // Replace the cards in the draw pile with an equal number of random
        // cards removed from then end of mysteryCards.
        game.drawPile = mysteryCards.splice(-game.drawPile.length);

        // Each player's hand retains undiscarded cards from the original hand.
        // The remaining cards from the original hand are replaced by random cards
        // drawn from the mystery cards.
        for (const player of game.players) {
            let originalPlayer = original.players[player.index];
            for (let card of originalPlayer.hand.cards) {
                if (player.index == forPlayer.index || game.undiscards.has(card)) {
                    player.hand.cards.add(card);
                } else {
                    player.hand.cards.add(mysteryCards.pop()!);
                }
            }
        }

        return game;
    }

    nextTurn() {
        this.currentPlayerIndex = (this.currentPlayerIndex + 1) % this.players.length;
    }

    currentPlayer() : Player {
        return this.players[this.currentPlayerIndex];
    }

    addToDiscard(card: Card) {
        this.discardPile.push(card);
    }

    takeFromDiscard(numCards: number): Card[] {
        // TODO:  Shouldn't these cards become undiscarded?
        // Check the callers...
        return this.discardPile.splice(-numCards);
    }

    toString() : string {
        let playersString = this.players.map(player => String(player)).join("");
        let discardString = `discard: ${Card.cardsToString(this.discardPile)}\n`;
        // display Melds also
        return playersString + discardString;
    }
}