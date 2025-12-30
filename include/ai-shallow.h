#ifndef AI_SHALLOW_H
#define AI_SHALLOW_H

#include "ai.h"

// API
void ShallowAI_beginTurn(AI *ai);
bool ShallowAI_takeTurn(AI *ai, Turn *turn);
void ShallowAI_drawTurn(AI *ai, Cards drawCard, Turn *turn);
void ShallowAI_endTurn(AI *ai);

// Internal functions
void ShallowAI_findBestTakeTurn(AI *ai);
void ShallowAI_findBestDrawTurns(AI *ai);
void ShallowAI_findBestMeld(AI *ai, Turn *bestTurn);
void ShallowAI_findBestDiscard(AI *ai, Turn *bestTurn);
int ShallowAI_evaluateGame(AI *ai);
int ShallowAI_evaluateHand(Cards hand, Meld *meld, Cards drawable);
int ShallowAI_evaluateHandPlayability(Cards hand, Meld *meld, Cards drawable);
Cards ShallowAI_playableCards(Cards hand, Meld *meld);

#endif // AI_SHALLOW_H