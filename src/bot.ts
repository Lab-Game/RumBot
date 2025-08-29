import { Card, Meld } from "./card";
import { Game } from "./game";
import { Player } from "./player";
import {  Hand } from "./hand";

class Play {
    start: Game;
    taken: Card[] = [];
    drawn: Card | undefined;
    melds: Meld[] = [];
    nonMelds: Set<Meld> = new Set<Meld>();
    discard: Card | undefined;
    score : number;
    end: Game;

    constructor(start: Game) {
        this.start = start;
    }

    clone() {
        const play = new Play(this.start);
        play.end = this.start.clone();
        play.taken = [...this.taken];
        play.drawn = this.drawn;
        play.melds = [...this.melds];
        // play.nonMelds = new Set(this.nonMelds);
        play.discard = this.discard;
        return play;
    }

    toString() {
        const intakeString = this.drawn ? `draw: ${this.drawn.toString()}\n` : `take: ${this.taken.map(card => card.toString()).join(" ")}\n`;
        const meldString = `melds: ${this.melds.map(meld => meld.toString()).join(" ")}\n`;
        const discardString = this.discard ? `discard: ${this.discard.toString()}\n` : "<none>";
        return intakeString + meldString + discardString;
    }
}

export class Bot {

    strategy: number;
    totalPoints: number = 0;

    constructor(strategy: number) {
        this.strategy = strategy;
        this.totalPoints = 0;
    }

    bestTakePlay(plays: Play[]) {
        let bestPlay: Play | undefined;
        let bestScore = -Infinity;

        for (const play of plays) {
            let score = this.evaluatePoints(play);
            if (score > bestScore) {
                bestScore = score;
                bestPlay = play;
            }
        }

        return bestPlay;
    }   

    bestDrawPlays(plays: Play[]) {

        // Find the best evaluation for every possible drawn card
        const maxEval = new Map<Card, number>();
        for (const drawPlay of drawPlays) {
            let score = this.evaluatePoints(drawPlay.end);
            maxEval.set(drawPlay.drawn!, Math.max(maxEval.get(drawPlay.drawn!) || -Infinity, score));
        }

        // Average best evaluation for every possible drawn card
        let totalScore = 0;
        for (const [card, score] of maxEval) {
            totalScore += score;
        }
        return totalScore / maxEval.size;


        let bestPlays: Play[] = [];
        let bestScore = -Infinity;

        for (const play of plays) {
            let score = this.evaluatePoints(play);
            if (score > bestScore) {
                bestScore = score;
                bestPlays = [play];
            } else if (score === bestScore) {
                bestPlays.push(play);
            }
        }

        return bestPlays;
    }

    takeTurn(game: Game) {
        let player = game.currentPlayer();
        console.log("Taking turn for player", player.index);

        // We'll analyze a variant of the actual game, where all cards unknown to
        // the player are shuffled.  That prevents any cheating by the bot.
        let play = new Play(Game.variant(game, player));

        // There are two types of play:  those starting with a take from the discard
        // pile and those starting with a draw from the draw pile.  Let's first find
        // the best "take" play.
        let takePlays = this.generateTakePlays(play);
        let bestTake = this.bestTakePlay(takePlays);

        // Draw plays are different, because we can't know what card we draw.
        // So we'll consider all possible draws, assuming they're equally likely.
        // Then we'll work out the best response to each draw and average out
        // the resulting scores to get an "average score" for a play beginning
        // with a random draw.
        let drawPlays = this.pickDrawPlay(play);
        let bestDraws = this.bestDrawPlays(drawPlays);


        let averageDrawScore = this.averageOverDraws(drawPlays);

        console.log(`Best take play score: ${bestTakeScore}`);
        console.log(`Average draw play score: ${averageDrawScore}`);

        if (bestTakeScore > averageDrawScore) {
            console.log(`Taking ${bestTakePlay!.taken.length} cards from the discard pile`);
            player.takeCards(bestTakePlay!.taken.length);
            for (const meld of bestTakePlay!.melds) {
                console.log(`Putting meld: ${meld.toString()}`);
                player.putMeld(meld);
            }
            if (bestTakePlay!.discard) {
                console.log(`Discarding: ${bestTakePlay!.discard.toString()}`);
                player.discard(bestTakePlay!.discard);
            }
        } else {
            let drawCard = player.drawCard();
            console.log(`Drew card: ${drawCard.toString()}`);
            // Find the best draw play with this draw card.
            let bestDrawScore = -Infinity;
            let bestDrawPlay: Play;
            for (const play of drawPlays) {
                if (play.drawn == drawCard) {
                    let points = this.evaluatePointy(play.end);
                    if (points > bestDrawScore) {
                        bestDrawScore = points;
                        bestDrawPlay = play;
                    }
                }
            }
            for (const meld of bestDrawPlay!.melds) {
                console.log(`Putting meld: ${meld.toString()}`);
                player.putMeld(meld);
            }
            if (bestDrawPlay!.discard) {
                console.log(`Discarding: ${bestDrawPlay!.discard.toString()}`);
                player.discard(bestDrawPlay!.discard);
            }
        }
    }

    makePlay(play: Play, drawCard: Card | undefined) {
        let game = plays.start;
        let player = game.currentPlayer();

        for (const card of plays.taken) {
            player.takeCards(1);
        }
        if (plays.drawn) {
            player.drawCard();
        }
        for (const meld of plays.melds) {
            player.putMeld(meld);
        }
        if (plays.discard) {
            player.discard(plays.discard);
        }

        plays.end = game;
        return plays;
    }

    averageOverDraws(drawPlays: Play[]) : number {
        // Find the best evaluation for every possible drawn card
        const maxEval = new Map<Card, number>();
        for (const drawPlay of drawPlays) {
            let score = this.evaluatePoints(drawPlay.end);
            maxEval.set(drawPlay.drawn!, Math.max(maxEval.get(drawPlay.drawn!) || -Infinity, score));
        }

        // Average best evaluation for every possible drawn card
        let totalScore = 0;
        for (const [card, score] of maxEval) {
            totalScore += score;
        }
        return totalScore / maxEval.size;
    }

    pickTakePlay(play: Play) : Play | undefined {
        let game = play.start;
        let player = game.currentPlayer();
        let bestScore = -Infinity;
        let bestPlay : Play | undefined;

        for (let numTaken = 1; numTaken <= game.discardPile.length; numTaken++) {
            play.taken = player.takeCards(numTaken);
            let candidate = this.pickMelds(play);
            if (candidate && candidate.score > bestScore) {
                bestScore = candidate.score;
                bestPlay = candidate;
            }
            player.untakeCards(play.taken);
            play.taken.length = 0;
        }
        return bestPlay;
    }

    pickDrawPlay(play: Play) : Play | undefined {
        let game = play.start;
        let player = game.currentPlayer();
        let bestScore = -Infinity;
        let bestPlay : Play | undefined;

        // We need to imagine the player drawing every cards that possibly
        // could be drawn, from that player's perspective.  This includes
        // not only cards in the draw pile, but also cards in the hands of
        // other players (provided they weren't previously drawn from the
        // discard pile).
        let i = game.drawPile.length - 1;
        let topDrawCard = game.drawPile[i];
        let swaps = game.drawPile;
        for (let j = 0; j < swaps.length; j++) {
            [ game.drawPile[i], swaps[j] ] = [ swaps[j], game.drawPile[i] ];
            play.drawn = player.drawCard();
            let candidate = this.pickMelds(play);
            if (candidate && candidate.score > bestScore) {
                bestScore = candidate.score;
                bestPlay = candidate;
            }
            player.undrawCard(play.drawn);
            [ game.drawPile[i], swaps[j] ] = [ swaps[j], game.drawPile[i] ];
            play.drawn = undefined;
        }

        // Now consider cards in the hands of other players
        for (const opponent of game.players) {
            if (opponent.index != player.index) {
                let swaps = [...opponent.hand.cards].filter(card => !game.discarded.has(card));

                for (let j = 0; j < swaps.length; j++) {
                    opponent.hand.removeCard(swaps[j]);
                    opponent.hand.addCard(topDrawCard);
                    game.drawPile[i] = swaps[j];
                    play.drawn = player.drawCard();
                    let candidate = this.pickMelds(play);
                    if (candidate && candidate.score > bestScore) {
                        bestScore = candidate.score;
                        bestPlay = candidate;
                    }
                    player.undrawCard(play.drawn);
                    opponent.hand.removeCard(topDrawCard);
                    opponent.hand.addCard(swaps[j]);
                    game.drawPile[i] = topDrawCard;
                    play.drawn = undefined;
                }
            }
        }

        return bestPlay;
    }

    pickMelds(play: Play) : Play | undefined {
        let game = play.start;
        let player = game.currentPlayer();
        let bestScore = -Infinity;
        let bestPlay : Play | undefined;

        let melds = player.melds().filter(meld => !play.nonMelds.has(meld));
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

    pickDiscard(play: Play) : Play {
        let game = play.start;
        let player = game.currentPlayer();
        let bestPlay = play.clone();

        if (player.hand.size() == 0) {
            
            plays.push(play.clone());
        } else {
            let discardChoices = [...player.hand.cards];
            for (const card of discardChoices) {
                play.discard = card;
                let previouslyDiscarded = player.discard(card);
                let score = this.evaluate(play.start);
                plays.push(play.clone());
                player.undiscard(previouslyDiscarded);
                play.discard = undefined;
            }
        }

        return plays;
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