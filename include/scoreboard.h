#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "game.h"

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
    Turn turn;
    int numGames;
    int totalScore[NUM_PLAYERS];
} ScoreboardDiscard;

Scoreboard *Scoreboard_fromGame(Game *game);
void Scoreboard_free(Scoreboard *scoreboard);
ScoreboardTake *Scoreboard_takes(Game *game);
ScoreboardDraw *Scoreboard_draws(Game *game);  // This is an array of size 64, indexed by the drawn card.
ScoreboardMeld *Scoreboard_melds(Game *game);
ScoreboardDiscard *Scoreboard_discards(Game *game);

void Scoreboard_print(Scoreboard *scoreboard);
void ScoreboardTake_print(ScoreboardTake *take);
void ScoreboardDraw_print(ScoreboardDraw *draw);
void ScoreboardMeld_print(ScoreboardMeld *meld);
void ScoreboardDiscard_print(ScoreboardDiscard *discard);

void Scoreboard_free(Scoreboard *scoreboard);

#endif // SCOREBOARD_H