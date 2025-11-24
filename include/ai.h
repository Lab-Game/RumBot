#ifndef AI_H
#define AI_H

#include "rumbot.h"
#include "game.h"
#include "meldlist.h"
#include "scoreboard.h"

#define MAX_MELDS 100
#define MAXMIN 3

typedef struct AIStruct {
    int mode;
    int subMode;  // for AIs used to simulate games
    int totalScore;
    Game *game;
    Player *player;
    Turn bestTakeTurn;
    Cards possibleDraws;
    Turn bestDrawTurn[64];
    int averageDrawEval;
    MeldList meldList;
} AI;

void AI_init(AI *ai, int mode);
void AI_resetForTurn(AI *ai);
void AI_joinGame(AI *ai, Game *game, Player *player);
void AI_exitGame(AI *ai);
Turn *AI_goShallow(AI *ai);
Turn *AI_goDeep(AI *ai);
Turn *AI_go(AI *ai);
void AI_findBestTakeTurn(AI *ai);
void AI_findBestDrawTurns(AI *ai);
void AI_findBestMeld(AI *ai, Turn *bestTurn);
void AI_findBestDiscard(AI *ai, Turn *bestTurn);
int AI_evaluateGame(AI *ai);
int AI_evaluateHand(Cards hand, Meld *meld, Cards drawable);
int AI_evaluateHandPlayability(Cards hand, Meld *meld, Cards drawable);
Cards AI_playableCards(Cards hand, Meld *meld);

// For this game, consider all possible takes and draws followed by
// all possible melds and discards.  In each scenario, play out the
// game and record the results in ScoreboardScore structures.
void AI_simulate(AI *ai, Scoreboard *scoreboard);
void AI_simulateTakes(AI *ai, ScoreboardTake *take);
void AI_simulateDraws(AI *ai, ScoreboardDraw *draws);
void AI_simulateMelds(AI *ai, ScoreboardMeld *melds);
void AI_simulateDiscards(AI *ai, ScoreboardDiscard *discards);
void AI_simulateGame(AI *ai, ScoreboardScore *score);

// Helper functions to find the best path through the scoreboard tree
ScoreboardDiscard *AI_findBestScoreboardDiscard(ScoreboardDiscard *discards, int playerId);
ScoreboardDiscard *AI_findBestScoreboardMeldPath(ScoreboardMeld *melds, int playerId);
ScoreboardDiscard *AI_findBestScoreboardTakePath(ScoreboardTake *takes, int playerId);
ScoreboardDiscard *AI_findBestScoreboardDrawPath(ScoreboardDraw *draws, int playerId);

void AI_print(AI *ai);

#endif // AI_H
