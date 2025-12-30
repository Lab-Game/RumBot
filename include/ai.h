#ifndef AI_H
#define AI_H

#include <stdbool.h>

#include "rumbot.h"
#include "game.h"
#include "meldlist.h"
#include "scoreboard.h"

#define AI_HUMAN -1  // AI is a human, playing via keyboard

typedef struct AIStruct {
    int totalScore;
    int mode;
    bool deep;
    Game *game;
    Player *player;

    Turn bestTakeTurn;
    Turn bestDrawTurn[64];
    int averageDrawEval;

    // Used only by a deep AI.  May be non-NULL only between AI_beginTurn
    // and AI_endTurn.
    Scoreboard *scoreboard;

    // This is a scratch field used during AI calculations.
    MeldList meldList;
} AI;

// Initialize an AI structure.  Mode -1 means a human player
// provides moves via the keyboard.  All other modes are AI players.
// If "deep" is true, then the AI explores all possible turns and evaluates
// each by simulating many random completions of the game, where all
// subsequent moves are made using the specified AI mode.
void AI_init(AI *ai, int mode, bool deep);

// Add the AI to the game, operating the given player.
void AI_joinGame(AI *ai, Game *game, Player *player);

// Remove the AI from the game.
void AI_exitGame(AI *ai);

// Typical usage of the following functions:
//
// AI_beginTurn(AI);
// if (!AI_takeTurn(AI, &turn)) {
//     AI_drawTurn(AI, drawCard, &turn);
// }
// AI_endTurn(AI);

// Prepare the AI to select a turn, e.g. initialize field,
// allocate a Scoreboard, etc.
void AI_beginTurn(AI *ai);

// Return true if the the AI chooses to take some cards from the discard pile.
// The turn structure is completely populated with the number of cards taken,
// the meld, and discard.  Return false if the AI chooses to draw instead.
bool AI_takeTurn(AI *ai, Turn *turn);

// Should be called only if AI_takeTurn returns false.  The drawCard
// is the top card in the draw pile.  The turn structure is populated
// with the draw, meld, and discard.
void AI_drawTurn(AI *ai, Cards drawCard, Turn *turn);

// Free any resources used by the AI during turn selection, e.g. free
// a Scoreboard, if allocated.
void AI_endTurn(AI *ai);

// Allow the AI to observe another player's turn, updating its internal state.
void AI_observeTurn(AI *ai, Player *player, Turn *turn);

// Print AI state for debugging.
void AI_print(AI *ai); 

#endif // AI_H
