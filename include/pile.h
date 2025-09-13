#ifndef PILE_H
#define PILE_H

#include <assert.h>
#include <stdbool.h>
#include "cards.h"

typedef struct {
    Cards cards[52];
    int size;
} Pile;

static inline int Pile_size(Pile *pile) {
    return pile->size;
}

static inline void Pile_push(Pile *pile, Cards card) {
    assert(pile->size < 52);
    pile->cards[pile->size++] = card;
}

static inline Cards Pile_pop(Pile *pile) {
    assert(pile->size >= 1);
    return pile->cards[--pile->size];
}

static inline void Pile_clear(Pile *pile) {
    pile->size = 0;
}

void Pile_fullDeck(Pile *pile);
void Pile_shuffle(Pile *pile);
void Pile_print(Pile *pile);

#endif // PILE_H