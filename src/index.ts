import { Arena } from "./arena";
import { Bot } from "./bot";
import { Card } from "./card.ts";

const bots = [ new Bot(), new Bot(), new Bot() ];
const arena = new Arena(bots);
arena.runGame();

// This is a system for automated playing of the card game Rummy.
// There are three main elements.
//
//   - An Arena, which manages individual rounds of the game and
//     a set of robots that play the game.
//   - A Game, which represents a single instance of the card game
//     in play.
//   - Bots, which are automated players.
//
// The Area creates Games, directs Bots to play them, and stores
// scores associated with the Bots so we can evaluate their
// relative performance.
//
// A Game consists of:
//
//   - Draw pile, a list of cards
//   - Discard pile, a list of cards
//   - A set of Melds played on the table
//   - A collection of players, where each player consists of
//     - A hand, which is a set of cards
//     - Points played by that player, which is a number
//   - The index of the current player
//
// There is a further nuance around Games.  In particular, we
// distinguish between a "full game" (which describes the
// complete state) and a "game view" (which is information
// about the game visible to one player).  In particular, the
// full game records the sequence of cards in the draw
// pile and the complete set of cards in each playe hand.
// In contrast, a game view records the number of cards in
// the draw pile, but which cards are present.  Similarly,
// the game view records the number of cards in each player's
// and and possibly a subset of the actual cards (namely,
// those which the player publics added by drawing from
// the discard pile).
//
// How best to represent the two possible states of the
// draw pile and player hands?  We could add instances of
// an "unknown card", but we'd need many distinct instances
// in player hands, which are Sets.  Alternatively, we could
// eliminate the notion of a "game view".  Instead, when
// a Bot is shown the game state, we randomize the order of
// of the non-public cards.  In other words, a game view
// is a *possible* state of the game, given the information
// available to the Bot.  Finally, we could represent a
// partially-known player hand as a hand size together
// with a subset of known cards.
//
// When do we transition from one Game type (full or view)
// to another?  Typically a Bot wants to consider a
// variety of scenarios:
//
//   - What if I take N cards from the discard pile,
//     play some set of melds, and then discard this card?
//     In this case, we could evaluate the effect of
//     this sequence of actions either by:
//
//     A Tallying points in a simple way after this
//       sequence of actions.
//     B Doing a short simulation, e.g. drawing one more
//       card and counting melds to assess the "playability"
//       of cards remaining in the hand.
//     C Simulating the entire remainder of the game.
//
//    - What happens if I draw 1 card from the draw pile?
//      Here, we could consider each possible draw,
//      evaluate the consequences of the draw, and average
//      the outcomes.
//
// The critical point is that we can't explore exponentially
// many scenarios.  Here is how this could happen:
//
//   - We start with a full game.
//   - We consider many possibilities for the current player's
//     turn:  card intke, meld, and discard.
//   - For each of these possibilities, we try to evaluate
//     the end-state of the game.
//     - We could do this by method A or B above, both of
//       which require a fixed amount of computation.
//     - Alternativey, we could "roll out", letting the
//       bot simulate the actions of the next player.
//     - If we simulate the next player, then we generate
//       a range of possible moves for that next player.
//     - Next, we evaluate the outcome of each of these
//       moves by recursively considering moves for the next
//       player, and so on, branching and branching.
//   - The key is that when we start to roll out, all
//     subsequent decisions have to be made without additional
//     rollouts.  In other words, when a player is deciding
//     among moves during a rollout, that decision can not
//     be BASED on additional rollouts.  After the player
//     decides a move, we can continue the single-path rollout.
//
// So we when evaluating a game state, there are two procedures:
//
//   - Play out the move and then calculation something like:
//
//     points played - points in hand discounted by playability,
//         provided the other players have a bunch of cards left
//
//   - Play out the whole game, where all subsequent player moves
//     use strategy #1.  The valuation is something like this
//     player's final score minus the average of opponent scores.
//
//
// 
//
//
//
//


