#include <stdio.h>

#include "rumbot.h"
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
        AI *currentAI = &ais[game.currentPlayerId];

        if (DEB >= 1) {
            printf("\n=== Player %d ===\n", currentAI->player->id);
            Game_print(&game);
        }
        
        Turn *turn = AI_go(currentAI);

        if (DEB >= 1) {
            Turn_print(turn);
        }

        Player_play(currentAI->player, turn);

        if (DEB >= 2) {
            Game_print(&game);
        }

        printf("\n");
    }

    printf("\n=== Game Over ===\n");
    Game_print(&game);

    return 0;
}