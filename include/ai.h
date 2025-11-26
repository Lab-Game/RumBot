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
    int simMode;  // for AIs used to simulate games
    int totalScore;
    Game *game;
    Player *player;
    Turn bestTakeTurn;
    Cards possibleDraws;
    Turn bestDrawTurn[64];
    int averageDrawEval;
    MeldList meldList;
} AI;

// Completely clear an AI structure.
void AI_init(AI *ai, int mode);

// Add the AI to the game, operating the given player.
void AI_joinGame(AI *ai, Game *game, Player *player);

// Remove the AI from the game.
void AI_exitGame(AI *ai);

Turn *AI_goDeep(AI *ai);
Turn *AI_goShallow(AI *ai);
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
void AI_simulate(Game *game, Scoreboard *scoreboard);
void AI_simulateTakes(Game *game, ScoreboardTake *take);
void AI_simulateDraws(Game *game, ScoreboardDraw *draws);
void AI_simulateMelds(Game *game, ScoreboardMeld *melds);
void AI_simulateDiscards(Game *game, ScoreboardDiscard *discards);
void AI_simulateGame(Game *game, ScoreboardScore *score);

void AI_print(AI *ai);

#endif // AI_H
