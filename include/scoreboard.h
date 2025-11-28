#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "game.h"

typedef struct ScoreboardScoreStruct {
    Turn turn;
    int numGames;
    int totalScore[NUM_PLAYERS];
} ScoreboardScore;

typedef struct ScoreboardStruct {
    struct ScoreboardTakeStruct *takes;
    struct ScoreboardDrawStruct *draws;
} Scoreboard;

typedef struct ScoreboardTakeStruct {
    struct ScoreboardTakeStruct *next;
    int numTaken;
    struct ScoreboardMeldStruct *melds;
} ScoreboardTake;

typedef struct ScoreboardDrawStruct {
    struct ScoreboardDrawStruct *next;
    Cards drawn;
    struct ScoreboardMeldStruct *melds;
} ScoreboardDraw;

typedef struct ScoreboardMeldStruct {
    struct ScoreboardMeldStruct *next;
    Meld meld;
    struct ScoreboardDiscardStruct *discards;
} ScoreboardMeld;

typedef struct ScoreboardDiscardStruct {
    struct ScoreboardDiscardStruct *next;
    Cards discard;
    ScoreboardScore *score;
} ScoreboardDiscard;

Scoreboard *Scoreboard_fromGame(Game *game);  // Returns one object
ScoreboardTake *Scoreboard_initTakes(Game *game);  // Returns a linked list
ScoreboardDraw *Scoreboard_initDraws(Game *game);  // Returns a linked list
ScoreboardMeld *Scoreboard_initMelds(Game *game);  // Returns a linked list
ScoreboardDiscard *Scoreboard_initDiscards(Game *game);  // Returns a linked list
ScoreboardScore *Scoreboard_initScore(Game *game);
void Scoreboard_free(Scoreboard *scoreboard);

void Scoreboard_print(Scoreboard *scoreboard);
void ScoreboardTake_print(ScoreboardTake *take);
void ScoreboardDraw_print(ScoreboardDraw *draw);
void ScoreboardMeld_print(ScoreboardMeld *meld);
void ScoreboardDiscard_print(ScoreboardDiscard *discard);
void ScoreboardScore_print(ScoreboardScore *score);

#endif // SCOREBOARD_H