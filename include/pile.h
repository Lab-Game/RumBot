#ifndef PILE_H
#define PILE_H

#include <assert.h>
#include <stdbool.h>
#include "cards.h"

// A Pile is a stack of Cards.  A Pile is used to represent the draw and discard piles.

typedef struct {
    Cards cards[52];
    int size;
    Cards allCards;
} Pile;

// Return the number of cards in the pile.
static inline int Pile_size(Pile *pile) {
    return pile->size;
}

void Pile_print(Pile *pile);

// Add a card to the top of the pile.
static inline void Pile_push(Pile *pile, Cards card) {
    assert(!(card & ~FULL_DECK)); // card is legal
    assert(!(pile->allCards & card)); // card not already in pile
    pile->cards[pile->size++] = card;
    pile->allCards |= card;
}

// Remove and return the card from the top of the pile.
static inline Cards Pile_pop(Pile *pile) {
    assert(pile->size >= 1);
    Cards c = pile->cards[--pile->size];
    pile->allCards &= ~c;
    return c;
}

void Pile_swapToTop(Pile *pile, int index);

static inline Cards Pile_peek(Pile *pile) {
    assert(pile->size >= 1);
    return pile->cards[pile->size - 1];
}

// Initialize the pile to be empty.
static inline void Pile_init(Pile *pile) {
    pile->size = 0;
    pile->allCards = 0;
}

// Put a full deck of cards (with only high aces) into the pile in sorted order.
void Pile_fullDeck(Pile *pile);

// Shuffle the cards in the pile.
void Pile_shuffle(Pile *pile);

// Print the cards in the pile as a space-separated list of card codes.
void Pile_print(Pile *pile);
void Pile_printToFile(Pile *pile, FILE *file);

#endif // PILE_H