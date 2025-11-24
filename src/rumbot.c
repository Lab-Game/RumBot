#include <stdlib.h>
#include <stdio.h>

#include "rumbot.h"
#include "game.h"
#include "ai.h"
#include "log.h"

int DEB = 2;  // Debug level (0=none, 1=some, 2=more, 3=lots)

void playGame(AI *ais[], Game *game, FILE *log_file) {
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_joinGame(ais[i], game, Game_player(game, i));
    }

    if (DEB >= 1) {
        printf("\n=== NEW GAME ===\n");
    }

    if (log_file) {
        Log_writeGame(game, log_file);
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
        if (log_file) {
            Log_writeTurn(turn, log_file);
        }

        if (DEB >= 2) {
            printf("  ");
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
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai;
    shuffled_ais[1] = ai + 2;
    shuffled_ais[2] = ai + 1;
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 1;
    shuffled_ais[1] = ai;
    shuffled_ais[2] = ai + 2;
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 1;
    shuffled_ais[1] = ai + 2;
    shuffled_ais[2] = ai;
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 2;
    shuffled_ais[1] = ai;
    shuffled_ais[2] = ai + 1;
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 2;
    shuffled_ais[1] = ai + 1;
    shuffled_ais[2] = ai;
    playGame(shuffled_ais, &copy, NULL);
}

void readGameFromLog() {
    Game game;
    Game_init(&game);

    FILE *log_file = NULL;
    log_file = fopen("logs/game.log", "r");
    if (!log_file) {
        fprintf(stderr, "Error: could not open log file for reading\n");
        exit(1);
    }

    Log_readGame(&game, log_file);
    printf("Done reading game from log file. Current game state:\n");
    Game_print(&game);
    printf("exiting...\n");
    fclose(log_file);
    exit(0);
}  

int main(int argc, char *argv[]) {
    FILE *log_file = NULL;
    if (argc > 1) {
        log_file = fopen(argv[1], "w");
        if (!log_file) {
            fprintf(stderr, "Error: could not open log file %s for writing\n", argv[1]);
            return 1;
        }
    }

    AI ais[NUM_PLAYERS];
    AI *ai_ptrs[NUM_PLAYERS];

    AI_init(&ais[0], 1);
    AI_init(&ais[1], 1);
    AI_init(&ais[2], 1);
    
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        ai_ptrs[i] = &ais[i];
    }

    const int numGames = 1;

    Game game;
    for (int i = 0; i < numGames; ++i) {
        Game_init(&game);
        Game_deal(&game);
        playGame(ai_ptrs, &game, log_file);
    }

    // Print final AI scores
    printf("\n=== FINAL SCORES AFTER %d GAMES ===\n", numGames);
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        printf("AI %d (mode %d): %d points\n", i, ais[i].mode, ais[i].totalScore);
    }

    if (log_file) {
        fclose(log_file);
    }

    return 0;
}