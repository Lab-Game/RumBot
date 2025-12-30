#ifndef AI_DEEP_H
#define AI_DEEP_H

#include "ai.h"

// API
void DeepAI_beginTurn(AI *ai);
bool DeepAI_takeTurn(AI *ai, Turn *turn);
void DeepAI_drawTurn(AI *ai, Cards drawCard, Turn *turn);
void DeepAI_endTurn(AI *ai);

void DeepAI_extractBestTakeTurn(AI *ai, Scoreboard *scoreboard);
void DeepAI_extractBestDrawTurns(AI *ai, Scoreboard *scoreboard);
void DeepAI_simulate(AI *ai, Game *game, Scoreboard *scoreboard);
void DeepAI_simulateTakes(AI *ai, Game *game, ScoreboardTake *take);
void DeepAI_simulateDraws(AI *ai, Game *game, ScoreboardDraw *draws);
void DeepAI_simulateMelds(AI *ai, Game *game, ScoreboardMeld *melds);
void DeepAI_simulateDiscards(AI *ai, Game *game, ScoreboardDiscard *discards);
void DeepAI_simulateGame(AI *ai, Game *game, ScoreboardScore *score);

#endif // AI_DEEP_H