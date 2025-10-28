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

    return 0;
}