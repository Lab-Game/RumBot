#include "ai.h"

// Evaluate the state of the game from the perspective of the current AI player.
// A higher return value indicates a state more favorable to the current player.
// Assumes the player has just completed their turn, but Game_nextTurn has not yet been
// called.

typedef struct EvalFeatures {
    int meldedPointsCpts;    // 100 * points melded onto the table
    int handPointsCpts;      // 100 * points in hand
    int playabilityCpts;     // estimated playability of hand
    int discardCpts;         // estimated value of discard pile to opponents
} EvalFeatures;

int Evaluate(AI *ai, EvalFeatures *features);