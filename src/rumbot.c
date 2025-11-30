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
        Log_writeGame(log_file, game);
    }

    while (!game->isOver) {
        if (DEB >= 1) {
            printf("\n=== Player %d ===\n", game->currentPlayer->id);
            Game_print(game);
        }

        AI *ai = ais[game->currentPlayerId];
        Turn turn;
        AI_go(ai, &turn);

        if (DEB >= 1) {
            printf("AI: ");
            Turn_print(&turn);
        }

        printf("Player %d plays turn: ", game->currentPlayer->id);
        Turn_print(&turn);
        Player_play(game->currentPlayer, &turn);
        if (log_file) {
            Log_writeTurn(log_file, &turn);
        }

        if (DEB >= 2) {
            printf("  ");
            Player_print(game->currentPlayer);
        }

        Game_nextTurn(game);
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

    Log_readGame(log_file, &game);
    printf("Done reading game from log file. Current game state:\n");
    Game_print(&game);
    printf("exiting...\n");
    fclose(log_file);
    exit(0);
}  

int main() {
    FILE *log_file = NULL;

    AI ais[NUM_PLAYERS];
    AI *ai_ptrs[NUM_PLAYERS];

    AI_init(&ais[0], 10, 2);
    AI_init(&ais[1], 2, 0);
    AI_init(&ais[2], 2, 0);
    
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        ai_ptrs[i] = &ais[i];
    }

    const int numGames = 1;

    Game game;
    for (int i = 0; i < numGames; ++i) {
        Game_init(&game);
        Game_shuffle(&game);
        // Pile_fromString(&game.drawPile, "3C AC 4D QH 2S 8D 4C 5S 9S 3H JD JS 5C 9H 2D 6C QS 8C 7D AD 4S AH KC 3S JC 3D 8H 7H 7C KS 6H 7S 9C JH AS TD QC 4H KD 5D TC 5H 2H TH 9D 2C 6S 6D TS QD 8S KH");
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