#include <stdlib.h>
#include <stdio.h>

#include "rumbot.h"
#include "game.h"
#include "ai.h"

int DEB = 2;  // Debug level (0=none, 1=some, 2=more, 3=lots)

void playGame(AI *ais[], Game *game) {
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_joinGame(ais[i], game, Game_player(game, i));
    }

    if (DEB >= 1) {
        printf("\n=== NEW GAME ===\n");
    }

    while (!game->isOver) {
        if (DEB >= 1) {
            printf("\n=== Player %d ===\n", game->currentPlayer->id);
            Game_print(game);
        }

        AI *ai = ais[game->currentPlayerId];
        Turn *turn = AI_go(ai);

        if (DEB >= 1) {
            printf("AI: ");
            Turn_print(turn);
        }

        Player_play(game->currentPlayer, turn);

        if (DEB >= 2) {
            Player_print(game->currentPlayer);
        }

        Game_nextTurn(game);

        if (DEB >= 1) {
            printf("\n");
        }
    }

    if (DEB >= 1) {
        printf("\n=== Game Over ===\n");
        Game_print(game);
    }

    // Add scores to AI total scores
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        ais[i]->totalScore += ais[i]->player->score;
    }
}

void playAllOrders(AI *ai, Game *game) {
    Game copy;
    AI *shuffled_ais[NUM_PLAYERS];

    Game_copy(game, &copy);
    shuffled_ais[0] = ai;
    shuffled_ais[1] = ai + 1;
    shuffled_ais[2] = ai + 2;
    playGame(shuffled_ais, &copy);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai;
    shuffled_ais[1] = ai + 2;
    shuffled_ais[2] = ai + 1;
    playGame(shuffled_ais, &copy);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 1;
    shuffled_ais[1] = ai;
    shuffled_ais[2] = ai + 2;
    playGame(shuffled_ais, &copy);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 1;
    shuffled_ais[1] = ai + 2;
    shuffled_ais[2] = ai;
    playGame(shuffled_ais, &copy);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 2;
    shuffled_ais[1] = ai;
    shuffled_ais[2] = ai + 1;
    playGame(shuffled_ais, &copy);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 2;
    shuffled_ais[1] = ai + 1;
    shuffled_ais[2] = ai;
    playGame(shuffled_ais, &copy);
}

int main(void) {
    AI ais[NUM_PLAYERS];
    AI *ai_ptrs[NUM_PLAYERS];

    AI_init(&ais[0], 1);
    AI_init(&ais[1], 1);
    AI_init(&ais[2], 1);
    
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        ai_ptrs[i] = &ais[i];
    }

    const int numGames = 100;

    Game game;
    for (int i = 0; i < numGames; ++i) {
        Game_init(&game);

        playGame(ai_ptrs, &game);
    }

    // Print final AI scores
    printf("\n=== FINAL SCORES AFTER %d GAMES ===\n", numGames);
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        printf("AI %d (mode %d): %d points\n", i, ais[i].mode, ais[i].totalScore);
    }

    return 0;
}