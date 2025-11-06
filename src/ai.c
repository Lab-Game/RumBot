#include <stdio.h>

#include "ai.h"
#include "game.h"

int AI_evaluate(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    int score = player->score * 100;

    if (Cards_isEmpty(player->hand)) {
        // Each other player loses maybe 700 centipoints per card in hand
        int negativePoints = 0;
        for (int i = 0; i < game->numPlayers; ++i) {
            negativePoints += Cards_size(Game_player(game, i)->hand) * 700;
        }
        return score + negativePoints / (game->numPlayers - 1);        
    } else {
        int handScore = AI_evaluateHand(player->hand, &game->meld, Player_couldDraw(player));
        return score + handScore / 2;
    }
}

int AI_evaluateHand(Cards hand, Meld *meld, Cards drawable) {
    // Evaluate the quality of a hand given the table meld and
    // cards that could possibly be drawn.  In a good hand, there
    // are many possible draws that enable many playable cards.
    // So, for each drawable card, we'll estimate the number of
    // points that can be played from the hand into the meld.
    // Then we'll average over all playable cards.
    int totalPoints = 0;
    for (Cards c = Cards_first(drawable); c != 0; c = Cards_next(drawable, c)) {
        // Simulate adding this card to the hand
        Cards simulatedHand = hand | c;
        Cards playable = Plays_findPlayableCards(simulatedHand, meld);
        int points = Cards_points(playable);
        
        totalPoints += points;
    }

    // Return the average points per playable card
    int numPlayable = Cards_size(drawable);
    return numPlayable > 0 ? 100 * totalPoints / numPlayable : 0;
}

void AI_init(AI *ai, int mode, Game *game, Player *player) {
    ai->mode = mode;
    ai->game = game;
    ai->player = player;
}

void AI_go(AI *ai) {
    Game_print(ai->game);

    int averageDrawEval = AI_findBestDrawTurns(ai);
    printf("Draw avg:  %d\n", averageDrawEval);
    int bestTakeEval = AI_findBestTakeTurn(ai);
    printf("Take best: %d\n", bestTakeEval);

    printf("=> ");
    if (bestTakeEval > averageDrawEval) {
        // Execute the best take turn
        Turn_print(&ai->bestTakeTurn);
        Player_takeNum(ai->player, ai->bestTakeTurn.taken.size);
        Player_meld(ai->player, &ai->bestTakeTurn.meld);
        if (!Cards_isEmpty(ai->bestTakeTurn.discard)) {
            Player_discard(ai->player, ai->bestTakeTurn.discard);
        }

    } else {
        // Execute the best draw turn
        Cards drawnCard = Player_draw(ai->player);
        Card drawnCardIndex = Cards_toCard(drawnCard);
        Turn *bestDrawTurn = &ai->bestDrawTurn[drawnCardIndex];
        Turn_print(bestDrawTurn);
        Player_meld(ai->player, &bestDrawTurn->meld);
        if (!Cards_isEmpty(bestDrawTurn->discard)) {
            Player_discard(ai->player, bestDrawTurn->discard);
        }
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

    Cards drawable = Player_couldDraw(player);

    int totalEval = 0;
    for (Cards c = Cards_first(drawable); c != 0; c = Cards_next(drawable, c)) {
        Card card = Cards_toCard(c);
        Turn_init(&ai->bestDrawTurn[card]);

        // Here, we'll crudely simulate drawing this card by simply adding
        // a copy to the top of the deck.  This won't work if we simulate
        // subsequent turns, because there are two copies of the card in play.
        Pile_push(&game->drawPile, c);
        Player_draw(player);
        AI_bestMeldAndDiscard(ai, &ai->bestDrawTurn[card]);
        totalEval += ai->bestDrawTurn[card].eval;
        Player_undoDraw(player);
        Pile_pop(&game->drawPile);
    }

    // Return the average evaluation across all possible draws.
    return totalEval / Cards_size(drawable);
}

void AI_bestMeldAndDiscard(AI *ai, Turn *bestTurn) {
    Player *player = ai->player;
    AI_generateMelds(ai);
    for (int i = 0; i < ai->numMelds; ++i) {
        Player_meld(player, &ai->melds[i]);

        if (Cards_size(player->hand) == 0) {
            // No discard possible
            player->turn.eval = AI_evaluate(ai);
            Turn_max(bestTurn, &player->turn);
        } else {
            // Try each possible discard
            for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
                Player_discard(player, c);
                player->turn.eval = AI_evaluate(ai);
                Turn_max(bestTurn, &player->turn);
                Player_undoDiscard(player);
            }
        }

        Player_undoMeld(player, &ai->melds[i]);
    }
    // Turn_print(bestTurn );
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