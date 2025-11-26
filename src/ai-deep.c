#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ai.h"
#include "ai-deep.h"
#include "scoreboard.h"

Turn *AI_goDeep(AI *ai) {
    printf("AI_goDeep: Simulating deep lookahead...\n");

    // Build a scoreboard from the current game state
    Scoreboard *scoreboard = Scoreboard_fromGame(ai->game);

    // Run lots of simulations on every leaf of the scoreboard tree
    // to determine the best turn.
    Game permuted;
    for (int i = 0; i < 20; ++i) {
        Game_copy(ai->game, &permuted);
        Game_permute(&permuted);
        AI_simulate(&permuted, scoreboard);
    }
    Scoreboard_print(scoreboard);

    // Pick out the best lines of play from the scoreboard.
    // ScoreboardTake *bestTake = NULL;

    printf("AI_goDeep: Done.\n");
    exit(0);

    Scoreboard_free(scoreboard);
    return NULL;
}

void AI_simulate(Game *game, Scoreboard *scoreboard) {
    // The caller is responsible for supplying games to simulate,
    // probably by calling Game_permute() on the current game.

    AI_simulateTakes(game, scoreboard->takes);
    AI_simulateDraws(game, scoreboard->draws);
}

void AI_simulateTakes(Game *game, ScoreboardTake *take) {
    Player *player = game->currentPlayer;

    while (take) {
        Player_take(player, take->numTaken);
        AI_simulateMelds(game, take->melds);
        Player_undoTake(player);
        take = take->next;
    }
}

void AI_simulateDraws(Game *game, ScoreboardDraw *draws) {
    Player *player = game->currentPlayer;

    while (draws) {
        Game_swap(game, draws->drawn, Pile_peek(&game->drawPile));
        Player_draw(player);
        AI_simulateMelds(game, draws->melds);
        Player_undoDraw(player);
        Game_swap(game, draws->drawn, Pile_peek(&game->drawPile));
        draws = draws->next;
    }
}

void AI_simulateMelds(Game *game, ScoreboardMeld *meld) {
    Player *player = game->currentPlayer;

    while (meld) {
        Player_meld(player, &meld->meld);
        AI_simulateDiscards(game, meld->discards);
        Player_undoMeld(player, &meld->meld);
        meld = meld->next;
    }
}

void AI_simulateDiscards(Game *game, ScoreboardDiscard *discards) {
    Player *player = game->currentPlayer;
    assert(player->game == game);

    while (discards) {
        Player_discard(player, discards->discard);
        // This could happen, and I haven't figure out what to do.
        assert(Cards_size(discards->discard) == 1);
        // At this point, the game could be over.  So no more simulation
        // is possible...
        AI_simulateGame(game, &discards->score);
        Player_undoDiscard(player);
        discards = discards->next;
    }
}

void AI_simulateGame(Game *game, ScoreboardScore *score) {
    // Play out the rest of the game from the current state.
    // Update the ScoreboardScore structure with the results.

    // Make a copy of the game, which we'll use for simulation.
    Game simGame;
    Game_copy(game, &simGame);

    Game_nextTurn(&simGame);

    // Set up AIs to play the simulated game.
    AI simAIs[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_init(&simAIs[i], 1);  // Should be the ai submode, but whatever
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
