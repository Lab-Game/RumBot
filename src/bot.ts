import { Card, Meld } from "./card";
import { Game } from "./game";
import { Player } from "./player";
import {  Hand } from "./hand";

class Play {
    taken: Card[];
    drawn: Card | null;
    melds: Meld[];
    noMelds: Set<Card>;
    discard: Card | null;
    game: Game | null;

    private constructor() {}

    static create(): Play {
        const play = new Play();
        play.taken = [];
        play.drawn = null;
        play.melds = [];
        play.noMelds = new Set();
        play.discard = null;
        play.game = null;
        return play;
    }

    static clone(other: Play): Play {
        const play = new Play();
        play.taken = [...other.taken];
        play.drawn = other.drawn;
        play.melds = [...other.melds];
        play.noMelds = new Set(other.noMelds);
        play.discard = other.discard;
        play.game = other.game;
        return play;
    }

    reset() {
        this.taken.length = 0;
        this.drawn = null;
        this.melds.length = 0;
        this.noMelds.clear();
        this.discard = null;
        this.game = null;
    }
}

export class Bot {

    game: Game;
    player: Player;
    play: Play = Play.create();
    // plays: Play[] = [];

    takeTurn(game: Game) {
        this.game = game;
        this.player = game.currentPlayer();

        // Get a variant for the current game
        const variant = Game.variant(this.game, this.player);

        // Generate all possible Plays and resulting Games,
        // with games involving a draw grouped.
        let takePlays = this.generateTakePlays();
        // let drawPlays = this.generateDrawPlays();
    }

    generateTakePlays() {
        let plays: Play[] = [];
        this.play.reset();
        for (let numFromDiscard = 1;
            numFromDiscard < this.game.discardPile.length;
            numFromDiscard++) {

            // Could have the Player take from the discard pile as
            // a single operation.
            this.play.taken = this.game.takeFromDiscard(numFromDiscard);
            this.player.hand.addCards(this.play.taken);

            this.play.game = this.game.clone();

            this.generateMelds();

            // ...and have the player return to the discard pile.
            this.player.hand.removeCards(this.play.taken);
            this.play.game.returnToDiscard(this.play.taken);
        }
    }

    generateMelds() {
        let melds = this.game.findMelds(this.player.hand).filter(meld => !this.play.noMelds.has(meld));
        if (melds.length == 0) {
            this.generateDiscard();
        } else {
            for (const meld of melds) {
                // Consider two cases.  First case: play the meld.
                this.player.hand.removeCards(meld.cards);
                this.game.playMeld(meld);
                this.play.melds.push(meld);

                this.generateeMelds();

                this.play.melds.pop();
                this.game.unplayMeld(meld);
                this.player.hand.addCards(meld.cards);

                // Second case:  do not play the meld.
                this.play.noMelds.add(meld);
                this.generateMelds();
                this.play.noMelds.delete(meld);
            }
        }
    }

    generateDiscard() {
        let choices = [...this.player.hand.cards];
        for (const card of choices) {
            this.player.hand.removeCard(card);
            this.game.discardPile.push(card);
            this.build.discard = card;

            this.build.hand = this.player.hand;
            console.log(this.build.toString());
            this.plays.push(this.build.clone());
            
            this.game.discardPile.pop();
            this.player.hand.addCard(card);
        }
    }
};