#include <stdlib.h>
#include <stdio.h>

#include "ai.h"
#include "game.h"

const int unknownCardCentipoints = 700;

int AI_evaluateGame(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    int base_centipoints = player->score * 100;

    if (!player->hand) {
        // Player went out
        // Player went out.  Give 700 centipoints per card remaining
        // in opponents' hands.  TODO:  Use actual points for known cards.
        int penaltyTotal = 0;
        for (int i = 0; i < game->numPlayers; ++i) {
            penaltyTotal += Cards_size(Game_player(game, i)->hand) * unknownCardCentipoints;
        }
        int penaltyAverage = penaltyTotal / (game->numPlayers - 1);
        int eval = base_centipoints + penaltyAverage;
        return eval;
    }

    if (ai->mode == 0) {
        return base_centipoints + Cards_points(player->hand);
    }

    if (ai->mode == 1) {
        Cards drawableCards = Player_couldDraw(player);
        int handPlayability = AI_evaluateHandPlayability(player->hand, &game->meld, drawableCards);
        int handPoints = Cards_points(player->hand);
        int eval = base_centipoints + handPlayability * 0.5 + handPoints;

        if (DEB >= 3) {
            printf("AI_evaluateGame: %d base + handPlayability %d * 0.5 + handPoints %d = eval %d\n",
                     base_centipoints, handPlayability, handPoints, eval);
        }

        return eval;
    }
    
    return base_centipoints;
}

int AI_evaluateHandPlayability(Cards hand, Meld *meld, Cards drawable) {
    // Evaluate the quality of a hand given the table meld and
    // cards that could possibly be drawn.  In a good hand, there
    // are many possible draws that enable many playable cards.
    // So, for each drawable card, we'll estimate the number of
    // centipoints (cpts) that can be played from the hand into the meld.
    // Then we'll average over all playable cards.
    if (DEB >= 3) {
        printf("AI_evaluateHandPlayability: hand = ");
        Cards_print(hand);
        printf(" meld = ");
        Meld_printCompact(meld);
        printf(" drawable = ");
        Cards_print(drawable);
        printf("\n");
    }

    int totalCentipoints = 0;
    for (Cards c = Cards_first(drawable); c != 0; c = Cards_next(drawable, c)) {
        // Simulate adding this card to the hand
        Cards simulatedHand = hand | c;
        Cards playable = Plays_findPlayableCards(simulatedHand, meld);
        int centipoints = Cards_points(playable) * 100;  // convert to cpts

        if (DEB >= 4) {
            if (playable) {
                printf("AI_evaluateHandPlayability:   draw: ");
                Card_print(Cards_toCard(c));
                printf(" -> ");
                Cards_print(playable);
                printf(" (%d cpts)\n", centipoints);
            }
        }

        totalCentipoints += centipoints;
    }

    // Return the average centipoints per drawable cards
    int numDrawable = Cards_size(drawable);
    int averageCentipoints = numDrawable > 0 ? totalCentipoints / numDrawable : 0;

    if (DEB >= 3) {
        printf("AI_evaluateHandPlayability: avg = %d\n", averageCentipoints);
    }

    return averageCentipoints;
}

void AI_init(AI *ai, int mode) {
    ai->mode = mode;
    ai->totalScore = 0;
}

void AI_join(AI *ai, Game *game, Player *player) {
    ai->game = game;
    ai->player = player;
}

Turn *AI_go(AI *ai) {
    Game *game = ai->game;

    int bestTakeEval = AI_findBestTakeTurn(ai);
    int averageDrawEval = AI_findBestDrawTurns(ai);

    if (DEB >= 2) {
        printf("AI_go: ");
        Turn_print(&ai->bestTakeTurn);
        for (Cards c = Cards_first(Player_couldDraw(ai->player)); c != 0; c = Cards_next(Player_couldDraw(ai->player), c)) {
            Card card = Cards_toCard(c);
            printf("AI_go: ");
            Turn_print(&ai->bestDrawTurn[card]);
        }
    }
    
    if (DEB >= 1) {
        printf("AI_go: average draw = %d  best take = %d\n", averageDrawEval, bestTakeEval);
    }

    if (bestTakeEval > averageDrawEval) {
        return &ai->bestTakeTurn;
    } else {
        return &ai->bestDrawTurn[Cards_toCard(Pile_peek(&game->drawPile))];
    }
}

int AI_findBestTakeTurn(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    Turn_init(&ai->bestTakeTurn);
    while (Pile_size(&game->discardPile) > 0) {
        Player_take(player);
        AI_bestMeldAndDiscard(ai, &ai->bestTakeTurn);
    }
    Player_undoTakes(player);

    return ai->bestTakeTurn.eval;
}

int AI_findBestDrawTurns(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    // Go through all cards in the draw pile.
    // Swap each card to the top of the draw pile,
    // evaluate the scenario, and swap back.
    int totalEval = 0;
    int numEvals = 0;
    Pile *drawPile = &game->drawPile;
    for (int i = 0; i < Pile_size(drawPile); ++i) {
        Pile_swapToTop(drawPile, i);
        totalEval += AI_tryDrawTurn(ai);
        numEvals += 1;
        Pile_swapToTop(drawPile, i);
    }

    // Now go through all other players' hands.
    // Swap each card that was not previously discarded
    // into the draw pile, evaluate the scenario,
    // and swap back.
    for (int p = 0; p < game->numPlayers; ++p) {
        Player *otherPlayer = Game_player(game, p);
        if (otherPlayer == player) {
            continue;
        }
        for (Cards c = Cards_first(otherPlayer->hand); c != 0; c = Cards_next(otherPlayer->hand, c)) {
            if (Cards_has(game->everDiscarded, c)) {
                // This card has been discarded before.
                continue;
            }

            // Swap this card into the draw pile.
            Pile_push(drawPile, c);
            Cards_remove(&otherPlayer->hand, c);

            totalEval += AI_tryDrawTurn(ai);
            numEvals += 1;

            // Swap the card back.
            Cards_add(&otherPlayer->hand, c);
            Pile_pop(drawPile);
        }
    }

    // Return the average evaluation across all possible draws.
    return totalEval / numEvals;
}

int AI_tryDrawTurn(AI *ai) {
    // Simulate drawing the top card of the draw pile,
    // find the best meld and discard for that scenario,
    // and return the evaluation of that turn.
    // This is used when the deck has been fixed to place
    // a specific card on top.
    Player *player = ai->player;
    Cards drawCard = Player_draw(player);
    Card card = Cards_toCard(drawCard);
    Turn *bestTurn = &ai->bestDrawTurn[card];
    Turn_init(bestTurn);
    AI_bestMeldAndDiscard(ai, bestTurn);
    Player_undoDraw(player);
    return bestTurn->eval;
}

void AI_bestMeldAndDiscard(AI *ai, Turn *bestTurn) {
    Player *player = ai->player;
    AI_generateMelds(ai);
    for (int i = 0; i < ai->numMelds; ++i) {
        Player_meld(player, &ai->melds[i]);

        if (Cards_size(player->hand) == 0) {
            // No discard possible
            player->turn.eval = AI_evaluateGame(ai);
            Turn_max(bestTurn, &player->turn);
        } else {
            // Try each possible discard
            for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
                if (c == player->turn.taken.allCards && !Meld_cards(&player->turn.meld)) {
                    // We can not discard a just-taken card without a meld.
                    continue;
                }
                
                Player_discard(player, c);
                player->turn.eval = AI_evaluateGame(ai);
                Turn_max(bestTurn, &player->turn);
                Player_undoDiscard(player);
            }
        }

        Player_undoMeld(player, &ai->melds[i]);
    }
}

void AI_generateMelds(AI *ai) {
    Plays accepted, rejected;
    Plays_init(&accepted);
    Plays_init(&rejected);

    ai->numMelds = 0;
    AI_generateMeldsRec(ai, &accepted, &rejected);
}

void AI_generateMeldsRec(AI *ai, Plays *accepted, Plays *rejected) {
    if (ai->numMelds >= MAX_MELDS) {
        // We've already found the maximum number of plays we can store.
        return;
    }

    if (Plays_count(accepted) > MAXMIN && Plays_count(rejected) > MAXMIN) {
        // We've both accepted and rejected lots of possible plays.
        // We're heading toward a combinatorial explosion.  Stop exploring.
        return;
    }

    Player *player = ai->player;
    Cards hand = player->hand;
    Meld *meld = &ai->game->meld;

    // Find all possible runs, sets, and extensions, given the current Meld and hand.
    Plays plays;
    Plays_findAll(&plays, meld, hand, accepted, rejected);

    // We'll now find the first possible play (run, set, or extension) and
    // consider both accepting and rejecting it, recursively exploring
    // each choice.  Note that we only consider accepting and rejecting
    // one play per involcation of this function.

    // Consider each possible run.
    Cards c;
    if ((c = Cards_first(plays.runCenters))) {
        Cards run = Plays_runCenterToCards(c);

        // Accept this run
        accepted->runCenters |= c;
        Player_playRun(player, run);
        AI_generateMeldsRec(ai, accepted, rejected);
        Player_undoRun(player, run);
        accepted->runCenters &= ~c;
        
        // Reject this run
        rejected->runCenters |= c;
        AI_generateMeldsRec(ai, accepted, rejected);
        rejected->runCenters &= ~c;
    } else if ((c = Cards_first(plays.setCenters))) {
        Cards set = Plays_setCenterToCards(c);

        // Accept this set
        accepted->setCenters |= c;
        Player_playSet(player, set);
        AI_generateMeldsRec(ai, accepted, rejected);
        Player_undoSet(player, set);
        accepted->setCenters &= ~c;

        // Reject this set
        rejected->setCenters |= c;
        AI_generateMeldsRec(ai, accepted, rejected);
        rejected->setCenters &= ~c;
    } else if ((c = Cards_first(plays.runExtensions))) {
        // Accept this run extension
        accepted->runExtensions |= c;
        Player_playRun(player, c);
        AI_generateMeldsRec(ai, accepted, rejected);
        Player_undoRun(player, c);
        accepted->runExtensions &= ~c;

        // Reject this run extension
        rejected->runExtensions |= c;
        AI_generateMeldsRec(ai, accepted, rejected);
        rejected->runExtensions &= ~c;
    } else if ((c = Cards_first(plays.setExtensions))) {
        // Accept this set extension
        accepted->setExtensions |= c;
        Player_playSet(player, c);
        AI_generateMeldsRec(ai, accepted, rejected);
        Player_undoSet(player, c);
        accepted->setExtensions &= ~c;

        // Reject this set extension
        rejected->setExtensions |= c;
        AI_generateMeldsRec(ai, accepted, rejected);
        rejected->setExtensions &= ~c;
    } else {
        // A meld is valid only if the player took at most one card or
        // played the deepest taken card.
        if (Pile_size(&player->turn.taken) <= 1 ||
            !Cards_has(player->hand, Pile_peek(&player->turn.taken))) {
            Plays_toMeld(accepted, &ai->melds[ai->numMelds++]);
        }
    }
}

void AI_printMelds(AI *ai) {
    printf("\nGenerated %d possible melds:\n", ai->numMelds);
    for (int i = 0; i < ai->numMelds; ++i) {
        printf("%d: ", i + 1);
        Meld_printCompact(&ai->melds[i]);
        printf("\n");
    }
}