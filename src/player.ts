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
    }

    unputMeld(meld: Meld) {
        this.hand.addCards(meld.cards);
        this.game.melds.delete(meld);
    }

    discard(card: Card) : boolean {
        // Discarding a card exposes it to public view.  For the remainder of the game,
        // everyone will know where this card is.  This matters, because when we
        // consider possible locations of cards, these are no longer "mystery" cards.
        // A challenge is that, when exploring possible moves, we might discard a
        // card (which may or may not have been previously discarded!) and then take
        // it back into the hand.  When we make a temporary discard, the card
        // temporarily becomes non-mysterious.  But if we undo the discard, should
        // the card regain its mysterious status?  So we return this bit and give
        // the caller responsibility for remembering this bit of state.
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