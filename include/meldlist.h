#ifndef MELDLIST_H
#define MELDLIST_H

#include <stdbool.h>
#include "meld.h"

// Suppose the player has a hand of cards, there are some cards already
// in the meld on the table, and the player must meld a particular card
// (because it was the deepest card taken from the discard pile).
// The MeldList_generate function generates a list of possible melds
// that the player could make from their hand.  For efficiency, this
// list is limited to MELDLIST_MAX_SIZE possibilities.

#define MELDLIST_MAX_SIZE 12

typedef struct {
    Meld melds[MELDLIST_MAX_SIZE];
    int size;
} MeldList;

void MeldList_generate(MeldList *list, Cards hand, Meld *tableMeld, Cards mustMeld);
void MeldList_print(MeldList *list);

#endif // MELDLIST_H