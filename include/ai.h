#ifndef AI_H
#define AI_H

#include "game.h"
#include "plays.h"

#define MAX_PLAYS 100
#define MAXMIN 3

typedef struct AIStruct {
    Game *game;
    Player *player;
    Turn *bestTurn;
    int numPlays;
    Plays plays[MAX_PLAYS];
} AI;

// Given a Game, determine the best Turn for the current player
// and update the game state accordingly.
void AI_init(AI *ai, Game *game, Player *player);
void AI_go(AI *ai);
void AI_generatePlays(AI *ai);
void AI_generatePlaysRec(AI *ai, Plays *accepted, Plays *rejected);
void AI_printPlays(AI *ai);

#endif // AI_H
