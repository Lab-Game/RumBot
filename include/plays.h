#ifndef PLAYS_H
#define PLAYS_H

#include "cards.h"

// A Plays objects represents a set of possible runs, sets, and extensions.
// This can be used to represent all available plays, a set of plays
// that can all be made, or a set of plays that have been rejected.
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
