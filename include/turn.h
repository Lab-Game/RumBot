#ifndef TURN_H
#define TURN_H

// A "Turn" represents a player's actions during their turn.  While searching for
// the best turn, we keep track of the current "scratch" turn and the best turn.
// At a leaf of the search tree, we evaluate the turn and update the best turn if
// the scratch turn is better.

#include "cards.h"
#include "pile.h"
#include "meld.h"

typedef struct TurnStruct {
    Pile taken;      // cards taken from discard pile
    Cards draw;      // card drawn (0 if none)
    Cards discard;   // card discarded (0 if none)
    Meld meld;       // cards melded
    int eval;        // evaluation of resulting position
} Turn;

void Turn_init(Turn *play);
int Turn_max(Turn *best, Turn *scratch);
void Turn_print(Turn *play);

#endif // TURN_H