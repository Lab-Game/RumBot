#include <stdio.h>

#include "game.h"
#include "ai.h"

int main(void) {
    Game game;
    Game_init(&game);

    AI ais[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_init(&ais[i], 0, &game, Game_player(&game, i));
    }

    while (!game.isOver) {
        AI_go(&ais[game.currentPlayer]);
        printf("\n");
        //break;
        Game_nextTurn(&game);
    }

    Game_print(&game);

    return 0;
}