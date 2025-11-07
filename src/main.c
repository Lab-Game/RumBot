#include <stdlib.h>
#include <stdio.h>

#include "rumbot.h"
#include "game.h"
#include "ai.h"

int DEB = 1;  // Debug level (0=none, 1=some, 2=more, 3=lots)
int POV = 1;  // Print debug from current player's point of view

void playGame(AI *ais[]) {
    Game game;
    Game_init(&game);
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_join(ais[i], &game, Game_player(&game, i));
    }

    if (DEB >= 1) {
        printf("\n=== NEW GAME ===\n");
    }

    while (!game.isOver) {
        if (DEB >= 1) {
            printf("\n=== Player %d ===\n", game.currentPlayer->id);
            Game_print(&game);
        }
        
        AI *ai = ais[game.currentPlayerId];

        int temp_DEB = DEB;
        DEB = 2;
        Turn *turn = AI_go(ai);
        DEB = temp_DEB;

        if (DEB >= 1) {
            printf("AI: ");
            Turn_print(turn);
        }

        Player_play(game.currentPlayer, turn);

        if (DEB >= 2) {
            Player_print(game.currentPlayer);
        }

        temp_DEB = DEB;
        DEB = 4;
        AI_evaluate(ai);
        DEB = temp_DEB;

        Game_nextTurn(&game);

        if (DEB >= 1) {
            printf("\n");
        }
    }

    if (DEB >= 1) {
        printf("\n=== Game Over ===\n");
        Game_print(&game);
    }

    // Add scores to AI total scores
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        ais[i]->totalScore += ais[i]->player->score;
    }
}

int main(void) {
    AI ais[NUM_PLAYERS];

    AI_init(&ais[0], 1);
    AI_init(&ais[1], 1);
    AI_init(&ais[2], 1);

    AI *shuffled_ais[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        shuffled_ais[i] = &ais[i];
    }

    for (int i = 0; i < 1; ++i) {
        // Shuffle the ais
        for (int j = NUM_PLAYERS - 1; j > 0; --j) {
            int k = arc4random_uniform(j + 1);
            AI *temp = shuffled_ais[j];
            shuffled_ais[j] = shuffled_ais[k];
            shuffled_ais[k] = temp;
        }
        playGame(shuffled_ais);
    }

    // Print final AI scores
    printf("Final AI scores:\n");
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        printf("AI %d (mode %d): %6d\n", i, ais[i].mode, ais[i].totalScore);
    }

    return 0;
}