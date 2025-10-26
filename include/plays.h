#ifndef PLAYS_H
#define PLAYS_H

#include "cards.h"
#include "game.h"
#include "table.h"

// A Play represents all possible runs and sets that a player can make
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

typedef struct PlaysStruct {
    Cards runCenters;
    Cards runExtensions;
    Cards setCenters;
    Cards setExtensions;
} Plays;

// Clear the Play to have no possible melds.  Useful when starting to
// collect rejected melds.
static inline void Plays_init(Plays *plays) {
    plays->runCenters = 0;
    plays->runExtensions = 0;
    plays->setCenters = 0;
    plays->setExtensions = 0;
}

// Find all possible melds (runs and sets) that can be played given the
// current table state and the player's hand.
void Plays_findAll(Table *table, Cards hand, Plays *plays);

// Find all possible melds that have not been rejected and can be added
// to the accepted set so as to produce a canonical set of plays.
// In particular:
//   - A run extension can not precede a 3-card run.
//   - There can not be three consecutive 1-card run extensions.
//   - A 3-card set can only be extended by adding a club.
void Plays_findAcceptable(Table *table, Cards hand, Plays *accepted, Plays *rejected, Plays *plays);

// Exclude all melds in 'rejected' from the melds in 'play'.  This is
// used while searching for the best meld sequence to play.  In particular,
// this helps avoid considering the same meld multiple times.
static inline void Plays_exclude(Plays *plays, Plays *rejected) {
    plays->runCenters &= ~rejected->runCenters;
    plays->runExtensions &= ~rejected->runExtensions;
    plays->setCenters &= ~rejected->setCenters;
    plays->setExtensions &= ~rejected->setExtensions;
}

// Return true if there are no possible melds in the Plays.
static inline bool Plays_none(Plays *plays) {
    return (plays->runCenters | plays->runExtensions | plays->setCenters | plays->setExtensions) == 0;
}

static inline int Plays_count(Plays *plays) {
    return Cards_size(plays->runCenters) + Cards_size(plays->runExtensions) +
           Cards_size(plays->setCenters) + Cards_size(plays->setExtensions);
}

// Convert a run center card to the full three-card meld.
static inline Cards Plays_runCenterToCards(Cards center) {
    return center | (center << 1) | (center >> 1);
}

// Convert a set center card to the full three-card meld.
static inline Cards Plays_setCenterToCards(Cards center) {
    return center |(center << 16) | (center >> 16) | (center >> 48) | (center << 48);
}

void Plays_print(Plays *plays);

#endif // PLAYS_H
