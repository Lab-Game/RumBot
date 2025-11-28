#ifndef AI_H
#define AI_H

#include "rumbot.h"
#include "game.h"
#include "meldlist.h"
#include "scoreboard.h"

typedef struct AIStruct {
    int mode;
    int simMode;  // for AIs used to simulate games
    int totalScore;
    Game *game;
    Player *player;
    Turn bestTakeTurn;
    Turn bestDrawTurn[64];
    int averageDrawEval;
    MeldList meldList;
} AI;

// Completely clear an AI structure.
void AI_init(AI *ai, int mode, int simMode);

// Clear fields used in selecting a turn.
void AI_resetForGo(AI *ai);

// Add the AI to the game, operating the given player.
// Remove the AI from the game.
void AI_joinGame(AI *ai, Game *game, Player *player);
void AI_exitGame(AI *ai);

// Return the best move for the AI's player in the current game state.
void AI_go(AI *ai, Turn *turn);

// Print AI state for debugging.
void AI_print(AI *ai);

#endif // AI_H
