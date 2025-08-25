import { Card, Meld } from "./card";
import { Game } from "./game";
import { Player } from "./player";
import {  Hand } from "./hand";

class Play {
    numFromDiscard: number;
    drawnCards: Card[];
    melds: Meld[] = [];
    nonMelds: Set<Meld>;
    meldPoints: number = 0;
    maxMeldPoints: number = 0;
    discard: Card;
    hand: Hand;

    clone() {
        const clone = new Play();
        clone.numFromDiscard = this.numFromDiscard;
        clone.drawnCards = [...this.drawnCards];
        clone.melds = [...this.melds];
        clone.meldPoints = this.meldPoints;
        clone.discard = this.discard;
        clone.hand = this.hand.clone();
        return clone;
    }

    toString() {
        return `Play: took ${this.numFromDiscard}: ${Card.cardsToString(this.drawnCards)}, meld: ${this.melds.map(meld => meld.toString()).join(" ")} (${this.meldPoints} pts), discard: ${this.discard.toString()} -> ${this.hand.toString()}`;
    }
}

export class Bot {

    game: Game;
    player: Player;
    score: number = 0;
    build: Play;
    plays: Play[] = [];

    constructor() {
        this.build = new Play();
        this.build.nonMelds = new Set<Meld>();
    }

    beginGame(game: Game) {
        this.game = game;
        this.player = this.game.currentPlayer();
    }

    generatePlays() {
        console.log(`Generating plays for player ${this.player.index}`);
        this.plays.length = 0;
        this.chooseDiscardsTaken();

        for (const play of this.plays) {
            console.log(play.toString());
        }
    }

    chooseDiscardsTaken() {
        this.build.numFromDiscard = 0;
        for (const card of this.game.deck) {
            console.log('Drawing card', card.toString());
            this.build.drawnCards = [card];
            this.player.hand.addCard(card);
            this.chooseMelds();
            this.player.hand.removeCard(card);
        }

        for (this.build.numFromDiscard = 1;
             this.build.numFromDiscard <= this.game.discardPile.length;
             this.build.numFromDiscard++) {
            console.log(`Taking ${this.build.numFromDiscard} discards`);
            this.build.drawnCards = this.game.removeFromDiscard(this.build.numFromDiscard);
            this.player.hand.addCards(this.build.drawnCards);
            this.chooseMelds();
            this.player.hand.removeCards(this.build.drawnCards);
            this.game.returnToDiscard(this.build.drawnCards);
        }
    }

    chooseMelds() {
        console.log("Choosing melds...")
        let melds = this.game.table.findMelds(this.player.hand).filter(meld => !this.build.nonMelds.has(meld));
        console.log("found melds", melds.length);
        console.log("Melds:", melds.map(meld => meld.toString()).join(" "));

        if (melds.length == 0) {
            this.chooseDiscard();
            return;
        }

        for (const meld of melds) {
            this.player.hand.removeCards(meld.cards);
            this.game.table.playMeld(meld);
            this.build.melds.push(meld);
            this.build.meldPoints += meld.points;

            let playPoints = this.chooseMelds();
            
            this.build.meldPoints -= meld.points;
            this.build.melds.pop();
            this.game.table.unplayMeld(meld);
            this.player.hand.addCards(meld.cards);

            this.build.nonMelds.add(meld);
            let noPlayPoints = this.chooseMelds();
            this.build.nonMelds.delete(meld);
        }
    }

    chooseDiscard() {
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

    endGame() {
        this.score += this.player.points - this.player.hand.points();
    }
};