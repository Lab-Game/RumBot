#include <stdlib.h>
#include <stdio.h>

#include "rumbot.h"
#include "game.h"
#include "ai.h"

int DEB = 0;  // Debug level (0=none, 1=some, 2=more, 3=lots)
int POV = 1;  // Print debug from current player's point of view

void playGame(AI *ais[], Game *game) {
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_join(ais[i], game, Game_player(game, i));
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

int main(void) {
    AI ais[NUM_PLAYERS];
    AI *shuffled_ais[NUM_PLAYERS];

    AI_init(&ais[0], 1);
    AI_init(&ais[1], 1);
    AI_init(&ais[2], 2);
    
    const int numGames = 100000;

    Game game;
    Game copy;
    for (int i = 0; i < numGames; ++i) {
        Game_init(&game);

        // Play a copy of the game with each possible AI ordering
        Game_copy(&game, &copy);
        shuffled_ais[0] = &ais[0];
        shuffled_ais[1] = &ais[1];
        shuffled_ais[2] = &ais[2];
        playGame(shuffled_ais, &copy);

        Game_copy(&game, &copy);
        shuffled_ais[0] = &ais[0];
        shuffled_ais[1] = &ais[2];
        shuffled_ais[2] = &ais[1];
        playGame(shuffled_ais, &copy);

        Game_copy(&game, &copy);
        shuffled_ais[0] = &ais[1];
        shuffled_ais[1] = &ais[0];
        shuffled_ais[2] = &ais[2];
        playGame(shuffled_ais, &copy);

        Game_copy(&game, &copy);
        shuffled_ais[0] = &ais[1];
        shuffled_ais[1] = &ais[2];
        shuffled_ais[2] = &ais[0];
        playGame(shuffled_ais, &copy);

        Game_copy(&game, &copy);
        shuffled_ais[0] = &ais[2];
        shuffled_ais[1] = &ais[0];
        shuffled_ais[2] = &ais[1];
        playGame(shuffled_ais, &copy);

        Game_copy(&game, &copy);
        shuffled_ais[0] = &ais[2];
        shuffled_ais[1] = &ais[1];
        shuffled_ais[2] = &ais[0];
        playGame(shuffled_ais, &copy);

        if (i % 1000 == 0) {
            for (int j = 0; j < NUM_PLAYERS; ++j) {
                printf("AI %d (mode %d): %8d %8.3f\n", j, ais[j].mode, ais[j].totalScore, ais[j].totalScore / (6.0 * (i + 1)));
            }
            printf("\n");
        }
    }

    // Print final AI scores


    return 0;
}