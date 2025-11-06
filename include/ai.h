#ifndef AI_H
#define AI_H

#include "game.h"
#include "plays.h"

#define MAX_MELDS 100
#define MAXMIN 3

typedef struct AIStruct {
    int mode;
    Game *game;
    Player *player;
    Turn bestTakeTurn;
    Turn bestDrawTurn[64];
    int numMelds;
    Meld melds[MAX_MELDS];
} AI;

// Given a Game, determine the best Turn for the current player
// and update the game state accordingly.  Evaluations are in
// centipoints (1/100 of a point).
void AI_init(AI *ai, int mode, Game *game, Player *player);
void AI_go(AI *ai);
double AI_findBestTakeTurn(AI *ai);
double AI_findBestDrawTurns(AI *ai);
void AI_bestMeldAndDiscard(AI *ai, Turn *bestTurn);
int AI_evaluate(AI *ai);
void AI_generateMelds(AI *ai);
void AI_generateMeldsRec(AI *ai, Plays *accepted, Plays *rejected);
void AI_printMelds(AI *ai);

// Static function to evaluate hand quality given a meld.
int AI_evaluateHand(Cards hand, Meld *meld, Cards drawable);

#endif // AI_H
