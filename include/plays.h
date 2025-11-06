#ifndef PLAYS_H
#define PLAYS_H

#include "cards.h"
#include "meld.h"

// A Plays objects represents a set of possible runs, sets, and extensions.
// Used when selecting the player's meld during their turn.
typedef struct PlaysStruct {
    Cards runCenters;
    Cards runExtensions;
    Cards setCenters;
    Cards setExtensions;
} Plays;

static inline void Plays_init(Plays *plays) {
    plays->runCenters = 0;
    plays->runExtensions = 0;
    plays->setCenters = 0;
    plays->setExtensions = 0;
}

// Find all possible plays (runs, sets, and extensions) from the given hand
// and meld, excluding any plays in the rejected set and avoiding any plays
// that would duplicate previously accepted plays.
void Plays_findAll(Plays *plays, Meld *meld, Cards hand, Plays *accepted, Plays *rejected);

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

static inline void Plays_toMeld(Plays *plays, Meld *meld) {
    meld->runs = Plays_runCenterToCards(plays->runCenters) | plays->runExtensions;
    meld->sets = Plays_setCenterToCards(plays->setCenters) | plays->setExtensions;
}

// Find all cards that can be played from the hand into the meld.
// These cards may not all be simultaneously playable, however.
// If a card is playable as either a low or high ace, only the high ace is returned.
Cards Plays_findPlayableCards(Cards hand, Meld *meld);

void Plays_print(Plays *plays);

#endif // PLAYS_H
