import { Card, Meld } from "./card";
import { Game } from "./game";
import { Player } from "./player";
import {  Hand } from "./hand";

// Is including the Game in the play a mistake?
// I think including the score is pretty handy.
// But what about nonMelds?  That seems it could be
// a parameter in Meld generation.
//
// Then a Play could be as simple as:
//
//   numCardsTaken (possibly 0)
//   drawCard
//   meldList
//   score
//
// Everywhere, I pass around the game and a play.  In Meld formation,
// I also pass around nonMelds.

class Play {
    taken: number = 0;
    draw?: Card = undefined;
    melds: Meld[] = [];
    discard?: Card = undefined;
    eval: number = -Infinity;

    clone(): Play {
        const clone = new Play();
        clone.taken = this.taken;
        clone.draw = this.draw;
        clone.melds = [...this.melds];
        clone.discard = this.discard;
        clone.eval = this.eval;
        return clone;
    }

    maxWith(other: Play) {
        if (other && other.eval > this.eval) {
            this.taken = other.taken;
            this.draw = other.draw;
            this.melds = [...other.melds];
            this.discard = other.discard;
            this.eval = other.eval;
        }
    }

    static max(p?: Play, q?: Play) : Play | undefined {
        if (p == undefined) {
            return q;
        } else if (q == undefined) {
            return p;
        } else if (p.eval >= q.eval) {
            return p;
        } else {
            return q;
        }
    }

    toString() {
        const intakeString = this.draw ? `draw:${this.draw.toString()}` : `take:${this.taken}`;
        const meldString = `melds: ${this.melds.map(meld => meld.toString()).join(" ")}`;
        const discardString = this.discard ? `discard: ${this.discard.toString()}` : "<none>";
        const evalString = `${this.eval} pts`;
        return `${intakeString} -> ${meldString} -> ${discardString} -> ${evalString}`;
    }
}

export class Bot {

    game: Game;
    strategy: number;
    totalPoints: number = 0;
    wins: number = 0;

    constructor(strategy: number) {
        this.strategy = strategy;
        this.totalPoints = 0;
    }

    takeTurn(game: Game) {
        let variant = Game.variant(game, game.player());
        let bestTake = this.bestTake(variant);
        let bestDraws = this.bestDraws(variant);
        let averageDrawEval = [...bestDraws.values()].reduce((sum, play) => sum + play.eval, 0) / bestDraws.size;

        if (game.verbose) {
            console.log("Take = ", bestTake.eval);
            console.log(bestTake.toString());
            console.log("Draw ~ ", averageDrawEval);
            for (const [card, play] of bestDraws) {
                console.log(play.toString());
            }
        }
        if (averageDrawEval >= bestTake.eval) {
            let draw = game.player().drawCard();
            if (game.verbose) console.log(`-> Draw ${draw}`);
            let bestDraw = bestDraws.get(draw)!;
            for (const meld of bestDraw.melds) {
                if (game.verbose) console.log(`-> Meld: ${meld.toString()}`);
                game.player().putMeld(meld);
            }
            if (bestDraw.discard) {
                if (game.verbose) console.log(`-> Discard: ${bestDraw.discard.toString()}`);
                game.player().discard(bestDraw.discard);
            }
        } else {
            // The player takes bestTake.taken cards from the discard pile.
            // Print these as a string to the console.
            if (game.verbose) console.log(`-> Take ${game.discardPile.slice(-bestTake.taken).map(card => card.toString()).join(" ")}`);
            game.player().takeCards(bestTake.taken);
            for (const meld of bestTake.melds) {
                if (game.verbose) console.log(`-> Meld: ${meld.toString()}`);
                game.player().putMeld(meld);
            }
            if (bestTake.discard) {
                game.player().discard(bestTake.discard);
                if (game.verbose) console.log(`-> Discard: ${bestTake.discard.toString()}`);
            }
        }
    }

    bestTake(game: Game) : Play {
        let play = new Play();
        let bestPlay : Play | undefined = undefined;
        for (play.taken = 1; play.taken <= game.discardPile.length; play.taken++) {
            let cardsTaken = game.player().takeCards(play.taken);
            let candidate = this.bestMelds(game, play, play.taken > 1 ? cardsTaken[cardsTaken.length - 1] : undefined);
            bestPlay = Play.max(bestPlay, candidate);
            game.player().untakeCards(cardsTaken);
        }
        return bestPlay!;
    }

    bestDraws(game: Game) : Map<Card, Play> {
        let play: Play = new Play();
        let bestPlays: Map<Card, Play> = new Map<Card, Play>();

        // Try bringing each card in the draw pile to the top and then doing a draw play.
        let last = game.drawPile.length - 1;
        for (let i = 0; i < game.drawPile.length; i++) {
            // Swap the i-th and last cards in the draw pile
            [ game.drawPile[last], game.drawPile[i] ] = [ game.drawPile[i], game.drawPile[last] ];
            play.draw = game.player().drawCard();
            bestPlays.set(play.draw, this.bestMelds(game, play)!);
            game.player().undrawCard(play.draw);
            [ game.drawPile[last], game.drawPile[i] ] = [ game.drawPile[i], game.drawPile[last] ];
        }

        // Try swapping each card in another player's hand with the top card on the discard pile
        // and then doing a draw play.  Exclude cards in other players' hands that were previously
        // discarded, because the current player knows that those can't be drawn.
        for (const rival of game.players) {
            if (rival !== game.player()) {
                let swaps = [...rival.hand.cards].filter(card => !game.discarded.has(card));
                for (let i = 0; i < swaps.length; ++i) {
                    let card = swaps[i];
                    let top = rival.drawCard();
                    rival.undrawCard(card);
                    play.draw = game.player().drawCard();
                    bestPlays.set(play.draw, this.bestMelds(game, play)!);
                    game.player().undrawCard(play.draw);
                    rival.drawCard();
                    rival.undrawCard(top);
                }
            }
        }

        return bestPlays;
    }

    bestMelds(game: Game, play: Play, mustPlay?: Card, rejectedMelds?: Set<Meld>) : Play | undefined {
        // Called only at the top recursion level.
        if (!rejectedMelds) {
            rejectedMelds = new Set<Meld>();
        }

        // Find melds available to the player that were not previously rejected.
        let melds = game.player().melds().filter(meld => !rejectedMelds.has(meld));

        if (melds.length == 0) {
            if (mustPlay && play.taken > 1 && game.player().hand.hasCard(mustPlay)) {
                // Failed to meld the deepest card in a multi-card take.
                return undefined;
            } else {
                return this.bestDiscard(game, play);
            }
        }

        let bestPlay: Play | undefined = undefined;
        for (const meld of melds) {
            // Suppose this meld is put.
            play.melds.push(meld);
            game.player().putMeld(meld);
            let candidate = this.bestMelds(game, play, mustPlay, rejectedMelds);
            bestPlay = Play.max(bestPlay, candidate);
            game.player().unputMeld(meld);
            play.melds.pop();

            // Now suppose the meld is not put
            rejectedMelds.add(meld);
        }

        // Finally, remove all melds from the rejectedMelds set, so we
        // don't affect parents or siblings in the recursion.
        for (const meld of melds) {
            rejectedMelds.delete(meld);
        }

        return bestPlay;
    }

    bestDiscard(game: Game, play: Play) : Play {
        let bestPlay = new Play();
        if (game.player().hand.size() == 0) {
            play.discard = undefined;
            play.eval = this.evaluate(game);
            bestPlay.maxWith(play);
        } else {
            let discardChoices = [...game.player().hand.cards];
            for (const card of discardChoices) {
                play.discard = card;
                let previouslyDiscarded = game.player().discard(card);
                play.eval = this.evaluate(game);
                bestPlay.maxWith(play);
                game.player().undiscard(previouslyDiscarded);
                play.discard = undefined;
            }
        }
        return bestPlay;
    }

    evaluate(game: Game) : number {
        let player = game.player();
            
        if (this.strategy == 1) {
            return player.points;
        }

        return player.points - player.hand.points();

        /*
        if (this.strategy == 1) {
            // If the player has gone out, award average points in other player's hands.
            if (player.hand.size() == 0) {
                let totalPoints = 0;
                for (const rival of game.players) {
                    if (rival !== player) {
                        totalPoints += rival.hand.points();
                    }
                }
                return player.points + (totalPoints / (game.players.length - 1));
            } else {
                return player.points - player.hand.points();
            }
        */
    }
};