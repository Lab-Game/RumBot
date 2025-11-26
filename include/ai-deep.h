#ifndef AI_DEEP_H
#define AI_DEEP_H

#include "ai.h"

Turn *AI_goDeep(AI *ai);
void AI_simulate(Game *game, Scoreboard *scoreboard);
void AI_simulateTakes(Game *game, ScoreboardTake *take);
void AI_simulateDraws(Game *game, ScoreboardDraw *draws);
void AI_simulateMelds(Game *game, ScoreboardMeld *melds);
void AI_simulateDiscards(Game *game, ScoreboardDiscard *discards);
void AI_simulateGame(Game *game, ScoreboardScore *score);

#endif // AI_DEEP_H