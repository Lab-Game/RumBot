#include <stdlib.h>
#include <stdio.h>

#include "ai.h"
#include "game.h"

const int unknownCardCentipoints = 700;

void AI_init(AI *ai, int mode) {
    ai->mode = mode;
    ai->subMode = 0;
    ai->totalScore = 0;
    ai->game = NULL;
    ai->player = NULL;
    AI_resetForTurn(ai);
}

void AI_resetForTurn(AI *ai) {
    Turn_init(&ai->bestTakeTurn);
    ai->possibleDraws = 0;
    for (int i = 0; i < 64; ++i) {
        Turn_init(&ai->bestDrawTurn[i]);
    }
    ai->averageDrawEval = 0;
}

void AI_joinGame(AI *ai, Game *game, Player *player) {
    ai->game = game;
    ai->player = player;
}

void AI_exitGame(AI *ai) {
    ai->game = NULL;
    ai->player = NULL;
}

Turn *AI_go(AI *ai) {
    if (ai->mode == 2) {
        return AI_goDeep(ai);
    } else {
        return AI_goShallow(ai);
    }
}

Turn *AI_goDeep(AI *ai) {
    printf("AI_goDeep: Starting simulations...\n");
    Scoreboard *scoreboard = Scoreboard_fromGame(ai->game);
    Scoreboard_print(scoreboard);

    // Run lots of simulations on every leaf of the scoreboard tree
    // to determine the best turn.
    for (int i = 0; i < 1; ++i) {
        Game permuted;
        Game_permute(ai->game, &permuted);
        AI_simulate(ai, scoreboard);
    }

    Scoreboard_print(scoreboard);

    // TODO:  Pick out the best line of play from the scoreboard.
    // Return best turn as in AI_goShallow().


    Scoreboard_free(scoreboard);

    return NULL;
}


void AI_simulate(AI *ai, Scoreboard *scoreboard) {
    // The caller is responsible for supplying games to simulate,
    // probably by calling Game_permute() on the current game.
    Game_print(ai->game);
    AI_simulateTakes(ai, scoreboard->takes);
    AI_simulateDraws(ai, scoreboard->draws);
}

void AI_simulateTakes(AI *ai, ScoreboardTake *take) {
    Player *player = ai->player;

    while (take) {
        for (int i = 0; i < take->numTaken; ++i) {
            Player_take(player);
        }
        assert(take->numTaken == Pile_size(&player->turn.taken));
        AI_simulateMelds(ai, take->melds);
        take = take->next;
        Player_undoTakes(player);
    }
}

void AI_simulateDraws(AI *ai, ScoreboardDraw *draws) {
    Player *player = ai->player;
    Game *game = ai->game;

    while (draws) {
        Cards swapped = Game_swapToTop(game, draws->drawn);
        Cards drawn = Player_draw(player);
        assert(drawn == draws->drawn);
        AI_simulateMelds(ai, draws->melds);
        Player_undoDraw(player);
        Game_swapToTop(game, swapped);
        draws = draws->next;
    }
}

void AI_simulateMelds(AI *ai, ScoreboardMeld *meld) {
    Player *player = ai->player;

    while (meld) {
        Player_meld(player, &meld->meld);
        AI_simulateDiscards(ai, meld->discards);
        Player_undoMeld(player, &meld->meld);
        meld = meld->next;
    }
}

void AI_simulateDiscards(AI *ai, ScoreboardDiscard *discards) {
    Player *player = ai->player;

    while (discards) {
        Player_discard(player, discards->discard);
        AI_simulateGame(ai, &discards->score);
        Player_undoDiscard(player);
        discards = discards->next;
    }
}

void AI_simulateGame(AI *ai, ScoreboardScore *score) {
    // Play out the rest of the game from the current state.
    // Update the ScoreboardScore structure with the results.

    // Make a copy of the game, which we'll use for simulation.
    Game simGame;
    Game_copy(ai->game, &simGame);

    // Advance to the next player's turn.
    Game_nextTurn(&simGame);

    // Set up AIs to play the simulated game.
    AI simAIs[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_init(&simAIs[i], ai->subMode);
        AI_joinGame(&simAIs[i], &simGame, Game_player(&simGame, i));
    }

    // Play out the game until it's over.
    while (!simGame.isOver) {
        Turn *turn = AI_go(&simAIs[simGame.currentPlayerId]);
        Player_play(simGame.currentPlayer, turn);
        Game_nextTurn(&simGame);
    }

    // Record the outcome in the ScoreboardScore structure.
    score->numGames += 1;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        score->totalScore[i] += simGame.players[i].score;
    }
}

Turn *AI_goShallow(AI *ai) {
    AI_resetForTurn(ai);
    AI_findBestTakeTurn(ai);
    AI_findBestDrawTurns(ai);

    if (ai->bestTakeTurn.eval > ai->averageDrawEval) {
        return &ai->bestTakeTurn;
    } else {
        Card drawCard = Cards_toCard(Pile_peek(&ai->game->drawPile));
        return &ai->bestDrawTurn[drawCard];
    }
}

void AI_findBestTakeTurn(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    while (Pile_size(&game->discardPile) > 0) {
        Player_take(player);
        AI_findBestMeld(ai, &ai->bestTakeTurn);
    }
    Player_undoTakes(player);
}

void AI_findBestDrawTurns(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    int totalEval = 0;
    ai->possibleDraws = Player_couldDraw(player);
    for (Cards c = Cards_first(ai->possibleDraws); c != 0; c = Cards_next(ai->possibleDraws, c)) {
        Turn *turn = &ai->bestDrawTurn[Cards_toCard(c)];

        Cards swapped = Game_swapToTop(game, c);
        Player_draw(player);
        AI_findBestMeld(ai, turn);
        totalEval += turn->eval;
        Player_undoDraw(player);
        Game_swapToTop(game, swapped);
    }

    ai->averageDrawEval = totalEval / Cards_size(ai->possibleDraws);
}

void AI_findBestMeld(AI *ai, Turn *bestTurn) {
    Player *player = ai->player;

    // Generate a list of possible melds
    Cards mustMeld = player->turn.taken.size > 1 ? Pile_peek(&player->turn.taken) : 0;

    MeldList_generate(&ai->meldList, player->hand, &ai->game->meld, mustMeld);

    // Try all of the possible melds.
    for (int i = 0; i < ai->meldList.size; ++i) {
        Player_meld(player, &ai->meldList.melds[i]);
        AI_findBestDiscard(ai, bestTurn);
        Player_undoMeld(player, &ai->meldList.melds[i]);
    }
}

void AI_findBestDiscard(AI *ai, Turn *bestTurn) {
    Player *player = ai->player;

    // If you took 1 card and didn't meld, then you must discard a different card.
    // This rule helps prevent infinite loops, where each player keeps taking
    // and discarding the same card over and over.
    Cards illegalDiscard = 0;
    if (Meld_cards(&player->turn.meld) == 0 && player->turn.taken.size == 1) {
        illegalDiscard = player->turn.taken.allCards;
    }
    
    // Consider all legal discards.
    for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
        if (c != illegalDiscard) {
            Player_discard(player, c);
            player->turn.eval = AI_evaluateGame(ai);
            Turn_max(bestTurn, &player->turn);
            Player_undoDiscard(player);
        }
    }
}

int AI_evaluateGame(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    int base_centipoints = player->score * 100;

    if (player->hand == 0) {
        // Player went out.  Give 700 centipoints per card remaining
        // in opponents' hands.
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
        int handCentipoints = 100 * Cards_points(player->hand);
        int eval = base_centipoints + handPlayability * 0.5 + handCentipoints / 100;

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
    int totalCentipoints = 0;
    for (Cards c = Cards_first(drawable); c != 0; c = Cards_next(drawable, c)) {
        // Simulate adding this card to the hand
        Cards simulatedHand = hand | c;
        Cards playable = AI_playableCards(simulatedHand, meld);
        int centipoints = Cards_points(playable) * 100;  // convert to cpts
        totalCentipoints += centipoints;
    }

    // Return the average centipoints per drawable cards
    int numDrawable = Cards_size(drawable);
    int averageCentipoints = numDrawable > 0 ? totalCentipoints / numDrawable : 0;

    return averageCentipoints;
}

Cards AI_playableCards(Cards hand, Meld *meld) {
    Cards lowHand = Cards_addLowAces(hand);
    Cards runCenters = hand & (lowHand << 1) & (hand >> 1);
    Cards setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    Cards runExtensions = ((meld->runs << 1) | (meld->runs >> 1)) & lowHand;
    Cards extendedMeldsRuns = meld->runs | runExtensions;
    runExtensions = ((extendedMeldsRuns << 1) | (extendedMeldsRuns >> 1)) & lowHand;
    Cards setExtensions = ((meld->sets << 16) | (meld->sets >> 16)) & lowHand;

    // Find all playable cards from these plays

    Cards playable = runCenters | (runCenters << 1) | (runCenters >> 1) | runExtensions |
                     setCenters | (setCenters << 16) | (setCenters >> 16) |
                     (setCenters << 48) | (setCenters >> 48) | setExtensions;

    // Kick out low aces from playable cards if the corresponding high aces are playable.
    return Cards_preferHighAces(playable);
}

void AI_print(AI *ai) {
    printf("AI mode %d, total score %d\n", ai->mode, ai->totalScore);
    printf("  Best take turn: ");
    Turn_print(&ai->bestTakeTurn);
    printf("  Best draw turns:\n");
    for (Cards c = Cards_first(ai->possibleDraws); c != 0; c = Cards_next(ai->possibleDraws, c)) {
        Card card = Cards_toCard(c);
        printf("    ");
        Turn_print(&ai->bestDrawTurn[card]);
    }
    printf("  Average draw eval: %d\n", ai->averageDrawEval);
}
