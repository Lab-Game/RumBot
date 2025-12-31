#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ai.h"
#include "ai-deep.h"
#include "scoreboard.h"

const int kNumTakeSimulations = 20;
const int kNumDrawSimulationsLight = 5;
const int kNumDrawSimulationsFull = 50;


void DeepAI_beginTurn(AI *ai) {
    // Initialize the scoreboard for deep AI simulations.
    assert(ai->scoreboard == NULL);
    ai->scoreboard = Scoreboard_fromGame(ai->game);
}

void DeepAI_endTurn(AI *ai) {
    assert(ai->scoreboard != NULL);
    Scoreboard_free(ai->scoreboard);
    ai->scoreboard = NULL;
}

bool DeepAI_takeTurn(AI *ai, Turn *turn) {
    // We'll extensively simulate every possible take turn and
    // more lightly simualate all possible draw turns.  The intuition
    // is that we'll average over all draw turns, giving us a good estimate
    // of the expected value of drawing, which is all that matters here.
    Game permuted;
    for (int i = 0; i < kNumTakeSimulations; ++i) {
        printf("i = %d\n", i);
        Game_copy(ai->game, &permuted);
        Game_permute(&permuted);
        DeepAI_simulateTakes(ai, &permuted, ai->scoreboard->takes);
        if (i < kNumDrawSimulationsLight) {
            DeepAI_simulateDraws(ai, &permuted, ai->scoreboard->draws);
        }
    }

    DeepAI_extractBestTakeTurn(ai, ai->scoreboard);
    DeepAI_extractBestDrawTurns(ai, ai->scoreboard);

    // Decide whether to take or draw based on the evaluations.
    if (ai->bestTakeTurn.eval >= ai->averageDrawEval) {
        *turn = ai->bestTakeTurn;
        return true;
    } else {
        return false;
    }
}

void DeepAI_drawTurn(AI *ai, Cards drawCard, Turn *turn) {
    // Locate the ScoreboardDraw entry for the specified draw card.
    ScoreboardDraw *draw = NULL;
    for (ScoreboardDraw *d = ai->scoreboard->draws; d; d = d->next) {
        if (draw->drawn == Cards_fromCard(drawCard)) {
            draw = d;
            break;
        }
    }
    assert(draw != NULL);

    // Perform additiona simulations for this specific draw card.
    Game permuted;
    for (int i = 0; i < kNumDrawSimulationsFull; ++i) {
        Game_copy(ai->game, &permuted);
        Game_permute(&permuted);

        // Move the specified draw card to the top of the draw pile
        // and simulate the player drawing it.
        Game_swap(&permuted, drawCard, Pile_peek(&permuted.drawPile));
        Player_draw(permuted.currentPlayer);

        // Now consider all possible melds and discards for this draw.
        DeepAI_simulateMelds(ai, &permuted, draw->melds);
    }

    // Recompute the best turn for this draw card.
    Turn *bestTurn = &ai->bestDrawTurn[drawCard];
    Turn_init(bestTurn);
    for (ScoreboardMeld *m = draw->melds; m; m = m->next) {
        for (ScoreboardDiscard *d = m->discards; d != NULL; d = d->next) {
            ScoreboardScore *score = d->score;
            assert(score->numGames > 0);
            Turn_max(bestTurn, &score->turn);
        }
    }

    *turn = *bestTurn;
}

void DeepAI_extractBestTakeTurn(AI *ai, Scoreboard *scoreboard) {
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

void DeepAI_extractBestDrawTurns(AI *ai, Scoreboard *scoreboard) {
    for (int i = 0; i < 64; ++i) {
        Turn_init(&ai->bestDrawTurn[i]);
    }

    int sumEval = 0;
    int numEval = 0;
    for (ScoreboardDraw *d = scoreboard->draws; d != NULL; d = d->next) {
        Card drawCard = Cards_toCard(d->drawn);
        for (ScoreboardMeld *m = d->melds; m != NULL; m = m->next) {
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

void DeepAI_simulate(AI *ai, Game *game, Scoreboard *scoreboard) {
    DeepAI_simulateTakes(ai, game, scoreboard->takes);
    DeepAI_simulateDraws(ai, game, scoreboard->draws);
}

void DeepAI_simulateTakes(AI *ai, Game *game, ScoreboardTake *takes) {
    Player *player = game->currentPlayer;

    for (ScoreboardTake *take = takes; take != NULL; take = take->next) {
        Player_take(player, take->numTaken);
        DeepAI_simulateMelds(ai, game, take->melds);
        Player_undoTake(player);
    }
}

void DeepAI_simulateDraws(AI *ai, Game *game, ScoreboardDraw *draws) {
    Player *player = game->currentPlayer;

    for (ScoreboardDraw *draw = draws; draw; draw = draw->next) {
        Game_swap(game, draw->drawn, Pile_peek(&game->drawPile));
        Player_draw(player);
        DeepAI_simulateMelds(ai, game, draw->melds);
        Player_undoDraw(player);
        Game_swap(game, draw->drawn, Pile_peek(&game->drawPile));
    }
}

void DeepAI_simulateMelds(AI *ai, Game *game, ScoreboardMeld *melds) {
    Player *player = game->currentPlayer;

    for (ScoreboardMeld *meld = melds; meld; meld = meld->next) {
        Player_meld(player, &meld->meld);
        DeepAI_simulateDiscards(ai, game, meld->discards);
        Player_undoMeld(player, &meld->meld);
    }
}

void DeepAI_simulateDiscards(AI *ai, Game *game, ScoreboardDiscard *discards) {
    Player *player = game->currentPlayer;
    assert(player->game == game);

    for (ScoreboardDiscard *discard = discards; discard; discard = discard->next) {
        if (discard->discard) {
            Player_discard(player, discard->discard);
            DeepAI_simulateGame(ai, game, discard->score);
            Player_undoDiscard(player);
        } else {
            // No discard (should only happen if the game is over)
            DeepAI_simulateGame(ai, game, discard->score);
        }
    }
}

void DeepAI_simulateGame(AI *ai, Game *game, ScoreboardScore *score) {
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
        AI_init(&simAIs[i], ai->mode, false);
        AI_joinGame(&simAIs[i], &simGame, Game_player(&simGame, i));
    }

    // Play out the game until it's over.
    int tmp_rounds = 0;
    while (!simGame.isOver) {
        tmp_rounds++;

        AI *ai = &simAIs[simGame.currentPlayerId];

        Turn turn;
        AI_beginTurn(ai);
        if (!AI_takeTurn(ai, &turn)) {
            // Peek at the top card of the draw pile.
            Cards drawCard = Pile_peek(&simGame.drawPile);
            AI_drawTurn(ai, drawCard, &turn);
        }
        if (tmp_rounds > 1000) {
            Turn_print(&turn);
        }
        if (tmp_rounds > 1100) {
            Game_print(&simGame);
            assert(false);
        }
        AI_endTurn(ai);

        Player_play(simGame.currentPlayer, &turn);
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
