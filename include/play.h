#ifndef PLAY_H
#define PLAY_H

#include "cards.h"
#include "game.h"
#include "table.h"

// A Play represents all possible melds (runs and sets) that a player can make
// given their hand and the current table state.  A meld can be either a
// three-card run, a three-card set, or a single-card extension to an existing
// run or set.
//
// Playing one meld may unlock more.  So, for example, if a player
// has a four-card run in their hand, then they can play either the lower or
// higher three-card run.  This unlocks an additional "extension" to that run.
//
// Three-card runs and sets that can be played from the player's hand are
// recorded in the runCenters and setCenters fields.  In particular, these
// Cards sets hold the *middle* card in each three-card meld.  For example,
// if the player can play the 5H 6H 7H run, then the bit for 6H will be set
// in runCenters.

typedef struct PlayStruct {
    Cards runCenters;
    Cards runExtensions;
    Cards setCenters;
    Cards setExtensions;
} Play;

// Clear the Play to have no possible melds.  Useful when starting to
// collect rejected melds.
static inline void Play_init(Play *play) {
    play->runCenters = 0;
    play->runExtensions = 0;
    play->setCenters = 0;
    play->setExtensions = 0;
}

// Find all possible melds (runs and sets) that can be played given the
// current table state and the player's hand.
void Play_find(Table *table, Cards hand, Play *play);

// Exclude all melds in 'rejected' from the melds in 'play'.  This is
// used while searching for the best meld sequence to play.  In particular,
// this helps avoid considering the same meld multiple times.
static inline void Play_exclude(Play *play, Play *rejected) {
    play->runCenters &= ~rejected->runCenters;
    play->runExtensions &= ~rejected->runExtensions;
    play->setCenters &= ~rejected->setCenters;
    play->setExtensions &= ~rejected->setExtensions;
}

// Return true if there are no possible melds in the Play.
static inline bool Play_none(Play *play) {
    return (play->runCenters | play->runExtensions | play->setCenters | play->setExtensions) == 0;
}

// Convert a run center card to the full three-card meld.
static inline Cards Play_runCenterToMeld(Cards center) {
    return center | (center << 1) | (center >> 1);
}

// Convert a set center card to the full three-card meld.
static inline Cards Play_setCenterToMeld(Cards center) {
    return center |(center << 16) | (center >> 16) | (center >> 48) | (center << 48);
}

void Play_print(Play *play);

#endif // PLAY_H
