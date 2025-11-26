#ifndef AI_SHALLOW_H
#define AI_SHALLOW_H

#include "ai.h"

Turn *AI_goShallow(AI *ai);
void AI_findBestTakeTurn(AI *ai);
void AI_findBestDrawTurns(AI *ai);
void AI_findBestMeld(AI *ai, Turn *bestTurn);
void AI_findBestDiscard(AI *ai, Turn *bestTurn);
int AI_evaluateGame(AI *ai);
int AI_evaluateHand(Cards hand, Meld *meld, Cards drawable);
int AI_evaluateHandPlayability(Cards hand, Meld *meld, Cards drawable);
Cards AI_playableCards(Cards hand, Meld *meld);

#endif // AI_SHALLOW_H