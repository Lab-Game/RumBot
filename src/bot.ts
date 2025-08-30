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

    maxWith(other: Play) {
        if (other.eval > this.eval) {
            this.taken = other.taken
            this.draw = other.draw;
            this.melds = [...other.melds];
            this.discard = other.discard;
            this.eval = other.eval;
        }
    }

    toString() {
        const intakeString = this.draw ? `draw: ${this.draw.toString()}\n` : `take: ${this.taken}\n`;
        const meldString = `melds: ${this.melds.map(meld => meld.toString()).join(" ")}\n`;
        const discardString = this.discard ? `discard: ${this.discard.toString()}\n` : "<none>";
        return intakeString + meldString + discardString;
    }
}

export class Bot {

    game: Game;
    play: Play;
    strategy: number;
    totalPoints: number = 0;

    constructor(strategy: number) {
        this.strategy = strategy;
        this.totalPoints = 0;
    }

    takeTurn(game: Game) {
        let variant = Game.variant(game, game.player());
        this.play = new Play();
        let bestTakePlay = this.bestTakePlay(variant, this.play);
        console.log(bestTakePlay);
    }

    bestTakePlay(game: Game, play: Play) : Play | undefined {
        let bestPlay: Play | undefined;
        for (play.taken = 1; play.taken <= game.discardPile.length; play.taken++) {
            let cardsTaken = game.player().takeCards(play.taken);
            let candidate = this.bestMelds(game, play);
            if (candidate) {
                if (!bestPlay) {
                    bestPlay = candidate;
                } else {
                    bestPlay.maxWith(candidate);
                }
            }
            game.player().untakeCards(cardsTaken);
        }
        return bestPlay;
    }  

    bestMelds(game: Game, play: Play, rejectedMelds?: Set<Meld>) : Play | undefined {
        if (!rejectedMelds) {
            rejectedMelds = new Set<Meld>();
        }

        // Find melds available to the player that were not previously rejected.
        let melds = game.player().melds().filter(meld => !rejectedMelds.has(meld));

        // If there are no melds available, then:
        //   - If the deepest card taken is still in the player's hand, then
        //     this is an illegal play.
        //   - Otherwise, proceed to choose the best discard.
        if (melds.length == 0) {
            if (play.taken.length <= 1 || !player.hand.hasCard(play.taken[0])) {
                // Have to play the deepest card taken if you take more than 1.
                let candidate = this.pickDiscard(play);
                if (candidate.score > bestScore) {
                    bestScore = candidate.score;
                    bestPlay = candidate;
                }
            }
        } else {
            for (const meld of melds) {
                // First, suppose that this meld is put.
                if (!play.nonMelds.has(meld))  {
                    play.melds.push(meld);
                    player.putMeld(meld);
                    let candidate = this.pickMelds(play)
                    if (candidate && candidate.score > bestScore) {
                        bestScore = candidate.score;
                        bestPlay = candidate;
                    }
                    player.unputMeld(meld);
                    play.melds.pop();
                }

                // Otherwise, this meld is forbidding in recursive calls.
                play.nonMelds.add(meld);
            }

            // Finally, remove all melds from the nonMelds set, so we
            // don't affect parents or siblings in the recursion.
            for (const meld of melds) {
                play.nonMelds.delete(meld);
            }
        }
        return bestPlay;
    }

    // Why not allocate a BestPlay with score of -Infinity and "max"
    // that with all cadidate plays?  This BestPlay will alwyas be defined.

    pickDiscard(play: Play) : Play {
        let game = play.game;
        let player = game.currentPlayer();

        if (player.hand.size() == 0) {
            // The player is out of cards.  Discard is not required.
            let score = this.evaluate(game);
            return play.clone();
        } else {
            let bestScore = -Infinity;
            let bestPlay = play.clone();

            let discardChoices = [...player.hand.cards];
            for (const card of discardChoices) {
                play.discard = card;
                let previouslyDiscarded = player.discard(card);
                let score = this.evaluate(play.game);
                if (score > bestScore) {
                    bestScore = score;
                    bestPlay = play.clone();
                }
                player.undiscard(previouslyDiscarded);
                play.discard = undefined;
            }
            return bestPlay;
        }
    }

    evaluate(game: Game) : number {
        // incorporate this.strategy here
        return this.evaluatePoints(game);
    }

    evaluatePoints(game: Game) : number {
        let player = game.currentPlayer();
        // If the player goes "out", then award points for opponent cards
        if (player.hand.size() == 0 && this.strategy >= 1) {
            let totalLoss = 0;
            for (const opponent of game.players) {
                if (opponent.index != player.index) {
                    totalLoss += opponent.hand.points();
                }
            }
            let bonus = totalLoss / (game.players.length - 1);
            return player.points + bonus;
        } else {
            return player.points - player.hand.points();
        }
    }
};