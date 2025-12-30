#ifndef AI_HUMAN_H
#define AI_HUMAN_H

#include "ai.h"

void HumanAI_beginTurn(AI *ai);
bool HumanAI_takeTurn(AI *ai, Turn *turn);
void HumanAI_drawTurn(AI *ai, Cards drawCard, Turn *turn);
void HumanAI_endTurn(AI *ai);

void HumanAI_meldAndDiscard(AI *ai, Cards mustMeld);

#endif // AI_HUMAN_H