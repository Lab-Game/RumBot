#ifndef AI_DEEP_H
#define AI_DEEP_H

#include "ai.h"

void AI_goDeep(AI *ai);
void AI_extractBestTakeTurn(AI *ai, Scoreboard *scoreboard);
void AI_extractBestDrawTurns(AI *ai, Scoreboard *scoreboard);
void AI_simulate(AI *ai, Game *game, Scoreboard *scoreboard);
void AI_simulateTakes(AI *ai, Game *game, ScoreboardTake *take);
void AI_simulateDraws(AI *ai, Game *game, ScoreboardDraw *draws);
void AI_simulateMelds(AI *ai, Game *game, ScoreboardMeld *melds);
void AI_simulateDiscards(AI *ai, Game *game, ScoreboardDiscard *discards);
void AI_simulateGame(AI *ai, Game *game, ScoreboardScore *score);

#endif // AI_DEEP_H