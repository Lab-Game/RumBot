import { Card, Meld } from "./card";
import { Game } from "./game";
import { Player } from "./player";
import {  Hand } from "./hand";

class Play {
    game: Game;
    numTaken: number = 0;
    taken: Card[] = [];
    melds: Meld[] = [];
    nonMelds: Set<Meld> = new Set<Meld>();
    discard: Card | undefined;

    constructor(game: Game) {
        this.game = game;
    }
}

export class Bot {

    game: Game;
    player: Player;

    takeTurn(game: Game) {
        this.game = game;
        this.player = game.currentPlayer();

        // Find all available plays
        const variant = Game.variant(this.game, this.player);
        let play = new Play(this.game);
        let plays = this.generatePlays(play);
        console.log(plays);
    }

    generatePlays(play: Play) : Play[] {
        console.log("generating plays");
        this.generateTakePlays(play);
        return [];
    }

    generateTakePlays(play: Play) : Play[] {
        let plays: Play[] = [];
        for (play.numTaken = 1; play.numTaken <= this.game.discardPile.length; play.numTaken++) {
            console.log("num taken = ", play.numTaken);
            play.taken =this.player.takeCards(play.numTaken);
            this.generateMelds(play);
            this.player.untakeCards(play.taken);
        }
        return plays;
    }

    generateMelds(play: Play) : Play[] {
        let plays: Play[] = [];
        let melds = this.player.melds().filter(meld => !play.nonMelds.has(meld));
        if (melds.length == 0) {
            /*
            if (play.taken[0] && this.player.hand.hasCard(play.taken[0])) {
                return [];  // Invalid play.  The deepest card taken must be melded.
            }
                */
            this.generateDiscard(play);
        } else {
            for (const meld of melds) {
                // Consider two cases.  First case: put the meld.
                play.melds.push(meld);
                this.player.putMeld(meld);
                this.generateMelds(play);
                this.player.unputMeld(meld);
                play.melds.pop();

                // Second case:  do not put the meld.
                play.nonMelds.add(meld);
                this.generateMelds(play);
                play.nonMelds.delete(meld);
            }
        }
        return plays;
    }

    generateDiscard(play: Play) : Play[] {
        let plays: Play[] = [];

        let discardChoices = [...this.player.hand.cards];
        for (const card of discardChoices) {
            play.discard = card;
            let previouslyDiscarded = this.player.discard(card);
            console.log(play.game.toString(), "\n");
            this.player.undiscard(previouslyDiscarded);
            play.discard = undefined;
        }

        return plays;
    }
};