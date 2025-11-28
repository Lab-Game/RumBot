#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ai.h"
#include "ai-deep.h"
#include "scoreboard.h"

void AI_goDeep(AI *ai) {
    printf("AI_goDeep: Simulating deep lookahead...\n");

    // Build a scoreboard from the current game state
    Scoreboard *scoreboard = Scoreboard_fromGame(ai->game);

    // Run lots of simulations on every leaf of the scoreboard tree
    // to determine the best turn.
    Game permuted;
    for (int i = 0; i < 100; ++i) {
        Game_copy(ai->game, &permuted);
        Game_permute(&permuted);
        AI_simulate(ai, &permuted, scoreboard);
    }

    AI_extractBestTakeTurn(ai, scoreboard);
    AI_extractBestDrawTurns(ai, scoreboard);

    Scoreboard_free(scoreboard);
}

void AI_extractBestTakeTurn(AI *ai, Scoreboard *scoreboard) {
    Turn *bestTakeTurn = &ai->bestTakeTurn;
    Turn_init(bestTakeTurn);

    for (ScoreboardTake *t = scoreboard->takes; t != NULL; t = t->next) {
        for (ScoreboardMeld *m = t->melds; m != NULL; m = m->next) {
            for (ScoreboardDiscard *d = m->discards; d != NULL; d = d->next) {
                ScoreboardScore *score = d->score;
                assert(score->numGames > 0);
                Turn_max(bestTakeTurn, &score->turn);
            }
        }
    }
}

void AI_extractBestDrawTurns(AI *ai, Scoreboard *scoreboard) {
    for (int i = 0; i < 64; ++i) {
        Turn_init(&ai->bestDrawTurn[i]);
    }

    int sumEval = 0;
    int numEval = 0;
    for (ScoreboardDraw *t = scoreboard->draws; t != NULL; t = t->next) {
        Card drawCard = Cards_toCard(t->drawn);
        for (ScoreboardMeld *m = t->melds; m != NULL; m = m->next) {
            for (ScoreboardDiscard *d = m->discards; d != NULL; d = d->next) {
                ScoreboardScore *score = d->score;
                assert(score->numGames > 0);
                Turn_max(&ai->bestDrawTurn[drawCard], &score->turn);
            }
        }
        sumEval += ai->bestDrawTurn[drawCard].eval;
        numEval += 1;
    }
    assert(numEval > 0);
    ai->averageDrawEval = sumEval / numEval;
}

void AI_simulate(AI *ai, Game *game, Scoreboard *scoreboard) {
    // The caller is responsible for supplying games to simulate,
    // probably by calling Game_permute() on the current game.

    AI_simulateTakes(ai, game, scoreboard->takes);
    AI_simulateDraws(ai, game, scoreboard->draws);
}

void AI_simulateTakes(AI *ai, Game *game, ScoreboardTake *take) {
    Player *player = game->currentPlayer;

    while (take) {
        Player_take(player, take->numTaken);
        AI_simulateMelds(ai, game, take->melds);
        Player_undoTake(player);
        take = take->next;
    }
}

void AI_simulateDraws(AI *ai, Game *game, ScoreboardDraw *draws) {
    Player *player = game->currentPlayer;

    while (draws) {
        Game_swap(game, draws->drawn, Pile_peek(&game->drawPile));
        Player_draw(player);
        AI_simulateMelds(ai, game, draws->melds);
        Player_undoDraw(player);
        Game_swap(game, draws->drawn, Pile_peek(&game->drawPile));
        draws = draws->next;
    }
}

void AI_simulateMelds(AI *ai, Game *game, ScoreboardMeld *meld) {
    Player *player = game->currentPlayer;

    while (meld) {
        Player_meld(player, &meld->meld);
        AI_simulateDiscards(ai, game, meld->discards);
        Player_undoMeld(player, &meld->meld);
        meld = meld->next;
    }
}

void AI_simulateDiscards(AI *ai, Game *game, ScoreboardDiscard *discards) {
    Player *player = game->currentPlayer;
    assert(player->game == game);

    while (discards) {
        if (discards->discard) {
            Player_discard(player, discards->discard);
            AI_simulateGame(ai, game, discards->score);
            Player_undoDiscard(player);
        } else {
            // No discard (should only happen if the game is over)
            AI_simulateGame(ai, game, discards->score);
        }
        discards = discards->next;
    }
}

void AI_simulateGame(AI *ai, Game *game, ScoreboardScore *score) {
    // Play out the rest of the game from the current state.
    // Update the ScoreboardScore structure with the results.

    // Make a copy of the game, which we'll use for simulation.
    Game simGame;
    Game_copy(game, &simGame);

    Game_nextTurn(&simGame);
    if (game->isOver) {
        // The game ended immediately after the last discard.
        // No need for simulation.
        score->numGames += 1;
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            score->totalScore[i] += simGame.players[i].score;
        }
        return;
    }

    // Set up AIs to play the simulated game to conclusion.
    AI simAIs[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_init(&simAIs[i], ai->simMode, 0);
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

    // Update the turn's eval to the difference between the
    // AI player's score and the average opponent score.
    int sumEval = 0;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        if (i == ai->player->id) {
            sumEval += 100 * score->totalScore[i];
        } else {
            sumEval -= 100 * score->totalScore[i] / (NUM_PLAYERS - 1);
        }
    }
    score->turn.eval = sumEval / score->numGames;

    // Clean up the AIs.
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_exitGame(&simAIs[i]);
    }
}
