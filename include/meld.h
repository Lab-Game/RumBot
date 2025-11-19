#ifndef MELD_H
#define MELD_H

#include "cards.h"

// A Meld holds the current melds (runs and sets) played on the Meld.

typedef struct MeldStruct {
    Cards runs;
    Cards sets;
} Meld;

// Initialize the Meld to be empty.
static inline Meld *Meld_init(Meld *Meld) {
    Meld->runs = 0;
    Meld->sets = 0;
    return Meld;
}

// Add a run of cards to the Meld.  Could be a new run or an extension.
static inline void Meld_addRun(Meld *Meld, Cards meld) {
    Meld->runs |= meld;
}

// Remove a run of cards from the Meld.
static inline void Meld_removeRun(Meld *Meld, Cards meld) {
    Meld->runs &= ~meld;
}

// Add a set of cards to the Meld.  Could be a new set or an extension.
static inline void Meld_addSet(Meld *Meld, Cards meld) {
    Meld->sets |= meld;
}

// Remove a set of cards from the Meld.
static inline void Meld_removeSet(Meld *Meld, Cards meld) {
    Meld->sets &= ~meld;
}

// Return all the cards in the Meld (both runs and sets).
static inline Cards Meld_cards(Meld *Meld) {
    return Cards_raiseAces(Meld->runs | Meld->sets);
}

// Print the runs and sets currently on the Meld.
void Meld_print(Meld *Meld);
void Meld_printCompact(Meld *Meld);

#endif // MELD_H