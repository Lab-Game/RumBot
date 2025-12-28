#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "game.h"

// A Scoreboard is a tree structure that represents all possible turns
// that the current player could take from the current game state.  Each
// possible turn is represented by a path through the tree.
// The leaves of the tree contain ScoreboardScore structures, which
// record the results of many simulated games played from that state.
// A "deep" AI uses the Scoreboard to evaluate possible turns by
// simulating many random completions of the game from each possible turn.
// The Scoreboard is constructed by simulating the player taking every
// possible action.  So we'll make heavy use of the player action and
// undo functions defined in player.h.

typedef struct ScoreboardStruct {
    struct ScoreboardTakeStruct *takes;  // linked list
    struct ScoreboardDrawStruct *draws;  // linked list
} Scoreboard;

typedef struct ScoreboardTakeStruct {
    struct ScoreboardTakeStruct *next;
    int numTaken;
    struct ScoreboardMeldStruct *melds;  // linked list
} ScoreboardTake;

typedef struct ScoreboardDrawStruct {
    struct ScoreboardDrawStruct *next;
    Cards drawn;
    struct ScoreboardMeldStruct *melds;  // linked list
} ScoreboardDraw;

typedef struct ScoreboardMeldStruct {
    struct ScoreboardMeldStruct *next;
    Meld meld;
    struct ScoreboardDiscardStruct *discards;  // linked list
} ScoreboardMeld;

typedef struct ScoreboardDiscardStruct {
    struct ScoreboardDiscardStruct *next;
    Cards discard;
    struct ScoreboardScoreStruct *score;
} ScoreboardDiscard;

typedef struct ScoreboardScoreStruct {
    Turn turn;
    int numGames;
    int totalScore[NUM_PLAYERS];
} ScoreboardScore;

// Returns a scoreboard tree representing all possible turns
// from the current game state.
Scoreboard *Scoreboard_fromGame(Game *game);

ScoreboardTake *Scoreboard_initTakes(Game *game);  // Returns a linked list
ScoreboardDraw *Scoreboard_initDraws(Game *game);  // Returns a linked list
ScoreboardMeld *Scoreboard_initMelds(Game *game);  // Returns a linked list
ScoreboardDiscard *Scoreboard_initDiscards(Game *game);  // Returns a linked list
ScoreboardScore *Scoreboard_initScore(Game *game);

// Free an entire scoreboard tree.
void Scoreboard_free(Scoreboard *scoreboard);

void Scoreboard_print(Scoreboard *scoreboard);
void ScoreboardTake_print(ScoreboardTake *take);
void ScoreboardDraw_print(ScoreboardDraw *draw);
void ScoreboardMeld_print(ScoreboardMeld *meld);
void ScoreboardDiscard_print(ScoreboardDiscard *discard);
void ScoreboardScore_print(ScoreboardScore *score);

#endif // SCOREBOARD_H