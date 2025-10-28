#include <stdio.h>

#include "game.h"
#include "ai.h"

int main(void) {
    Game game;
    Game_init(&game);
    Game_print(&game);

    AI ais[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_init(&ais[i], &game, Game_player(&game, i));
    }

    for (int j = 0; j < 3; ++j) {
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            printf("\n=========================\n");
            printf("Player %d's turn:\n", i);
            AI_go(&ais[i]);
            Game_nextTurn(&game);
        }
    }

    return 0;
}