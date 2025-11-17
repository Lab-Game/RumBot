#ifndef AI_H
#define AI_H

#include "rumbot.h"
#include "game.h"
#include "meldlist.h"

#define MAX_MELDS 100
#define MAXMIN 3

typedef struct AIStruct {
    int mode;
    int totalScore;
    Game *game;
    Player *player;
    Turn bestTakeTurn;
    Turn bestDrawTurn[64];
    MeldList meldList;
} AI;

// Given a Game, determine the best Turn for the current player
// and update the game state accordingly.  Evaluations are in
// centipoints (1/100 of a point).
void AI_init(AI *ai, int mode);
void AI_joinGame(AI *ai, Game *game, Player *player);
Turn *AI_go(AI *ai);
Turn *AI_goDeep(AI *ai);
int AI_findBestTakeTurn(AI *ai);
int AI_findBestDrawTurns(AI *ai);
int AI_tryDrawTurn(AI *ai);
void AI_bestMeldAndDiscard(AI *ai, Turn *bestTurn);
int AI_evaluate(AI *ai);
int AI_evaluateHandPlayability(Cards hand, Meld *meld, Cards drawable);
int AI_evaluateHand(Cards hand, Meld *meld, Cards drawable);

// Static function to evaluate hand quality given a meld.
int AI_evaluateHand(Cards hand, Meld *meld, Cards drawable);

#endif // AI_H
