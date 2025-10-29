#ifndef AI_H
#define AI_H

#include "game.h"
#include "plays.h"

#define MAX_MELDS 100
#define MAXMIN 3

typedef struct AIStruct {
    Game *game;
    Player *player;
    Turn bestTurn;
    int numMelds;
    Meld melds[MAX_MELDS];
} AI;

// Given a Game, determine the best Turn for the current player
// and update the game state accordingly.
void AI_init(AI *ai, Game *game, Player *player);
void AI_go(AI *ai);
int AI_evaluate(AI *ai);
void AI_generateMelds(AI *ai);
void AI_generateMeldsRec(AI *ai, Plays *accepted, Plays *rejected);
void AI_printMelds(AI *ai);

#endif // AI_H
