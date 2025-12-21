#ifndef TURN_H
#define TURN_H

// A "Turn" represents a player's actions during their turn.
//
// Turns are used in several places.  During actual gameplay, a Turn
// structure records the actions taken by the current player during
// their turn.  This is used to allow undoing of actions within the turn.
//
// While running an AI, Turn structures are used to represent possible
// turns that the AI is considering.  These turns are evaluated to
// determine the best turn to take.  The result of this evaluation can
// be stored in the Turn structure, for convenience.
//
// If a card has even been discarded, then its location is known by all
// players until the end of the game.  We need to track these "known" cards.
// During an AI search, we'll run the game forward and backward many times.
// Suppose a card is discarded and then that action is rewound.  We need
// to know whether the card was previously discarded or not, so that we
// can restore the game state correctly.  The discard field records the
// card discarded during this turn (if any) in newDiscards.

#include "rumbot.h"
#include "cards.h"
#include "pile.h"
#include "meld.h"

typedef struct TurnStruct {
    Pile taken;  // cards taken from discard pile
    Cards drawn;  // card drawn (0 if none)
    Cards discard;  // card discarded (0 if none)
    Meld meld;  // cards melded
    Cards newDiscards;  // cards discarded for the first time this turn
    int eval;
} Turn;

void Turn_init(Turn *play);
int Turn_max(Turn *best, Turn *scratch);
void Turn_print(Turn *play);

#endif // TURN_H