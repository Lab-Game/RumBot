import { Hand } from "./hand";
import { Game } from "./game";
import { Card, Meld } from "./card";

export class Player {

    game : Game;
    index : number;
    hand : Hand;
    points : number;

    static create(game : Game) : Player {
        let player = new Player();
        player.game = game;
        player.index = game.players.length;
        player.hand = new Hand();
        player.points = 0;
        return player;
    }

    clone(game : Game) {
        const player = new Player();
        player.game = game;
        player.index = this.index;
        player.hand = this.hand.clone();
        player.points = this.points;
        return player;
    }

    takeCards(numCards: number) {
        let taken = this.game.discardPile.splice(-numCards);
        this.hand.addCards(taken);
        return taken;
    }

    untakeCards(cards: Card[]) {
        this.hand.removeCards(cards);
        this.game.discardPile.push(...cards);
    }

    drawCard() {
        let card = this.game.drawPile.pop()!;
        this.hand.addCard(card);
        return card;
    }

    undrawCard(card: Card) {
        this.hand.removeCard(card);
        this.game.drawPile.push(card);
    }

    melds() {
        let meldList: Meld[] = [];

        // Find playable triples, both sets and runs
        for (const card of this.hand.cards) {
            if (card.tripleSet.cards.every(c => this.hand.hasCard(c))) {
                meldList.push(card.tripleSet);
            }
            if (card.tripleRun?.cards.every(c => this.hand.hasCard(c))) {
                meldList.push(card.tripleRun);
            }
        }

        // Find extensions to existing melds
        for (const meld of this.game.melds) {
            for (const addition of meld.addOns) {
                if (addition.cards.every(c => this.hand.hasCard(c))) {
                    meldList.push(addition);
                }
            }
        }

        return meldList;
    }

    putMeld(meld: Meld) {
        this.hand.removeCards(meld.cards);
        this.game.melds.add(meld);
        this.points += meld.points;
    }

    unputMeld(meld: Meld) {
        this.hand.addCards(meld.cards);
        this.game.melds.delete(meld);
        this.points -= meld.points;
    }

    discard(card: Card) : boolean {
        this.hand.removeCard(card);
        this.game.discardPile.push(card);
        let previouslyDiscarded = this.game.discarded.has(card);
        this.game.discarded.add(card);
        return previouslyDiscarded
    }

    undiscard(previouslyDiscarded: boolean) {
        let card = this.game.discardPile.pop()!;
        this.hand.addCard(card);
        if (previouslyDiscarded) {
            this.game.discarded.add(card);
        } else {
            this.game.discarded.delete(card);
        }
    }

    toString() : string {
        return `Player ${this.index}: ${this.hand.toString()} (${this.points}pts)\n`;
    }
}