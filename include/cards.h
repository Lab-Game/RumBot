#ifndef CARDS_H
#define CARDS_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "card.h"

#define FULL_DECK   0x3FFE3FFE3FFE3FFEULL  // Full deck (high aces only)
#define LEGAL_CARDS 0x3FFF3FFF3FFF3FFFULL  // All legal cards (high and low aces)
#define LOW_ACES    0x0001000100010001ULL
#define HIGH_ACES   0x2000200020002000ULL

// The Cards type is a bitmask used to represent a set of playing cards.
// There are bits to represent low aces, but these are used only when
// representings runs, the sole place where low aces are meaningful.
typedef uint64_t Cards;

extern const Cards kSpecialCard;  // Tempororary used when swapping cards

// Confirm that all cards in the Cards set are legal.
static inline bool Cards_isLegal(Cards cards) {
    return (cards & ~FULL_DECK) == 0;
}

// Determine whether Cards set 'cards' includes all cards in the Cards set 'c'.
static inline bool Cards_has(Cards cards, Cards c) {
    return (cards & c) == c;
}

// Add all cards in set c to the Cards set pointed to by cards.
static inline void Cards_add(Cards *cards, Cards c) {
    *cards |= c;
}

// Remove all cards in set c from the Cards set pointed to by cards.
// Cards in c are not required to be present in the hand.
static inline void Cards_remove(Cards *cards, Cards c) {
    *cards &= ~c;
}

// Return the number of cards in the Cards set.
static inline int Cards_size(Cards cards) {
    return __builtin_popcountll(cards);
}

// Swap 8C with PLACEHOLDER in a Pile.
// Swap the 8C with the 7H in a Cards set.
// Swap the 7H with PLACEHOLDER in the Pile.

static inline Cards Cards_swap(Cards *cards, Cards remove, Cards insert) {
    *cards = (*cards & ~remove) | insert;
    return remove;
}

// Return true of the set of cards includes any low aces.
static inline bool Cards_isLowAce(Cards card) {
    return (card & LOW_ACES) != 0;
}

// If there is a low or high ace, add the other ace to the Cards set.
static inline Cards Cards_addAllAces(Cards cards) {
    return (cards | ((cards & LOW_ACES) << 13) | ((cards & HIGH_ACES) >> 13));
}

// Return a Cards set with a low ace added for every high ace present.
static inline Cards Cards_addLowAces(Cards cards) {
    return cards | ((cards & HIGH_ACES) >> 13);
}

// Return a Cards set with all aces moved high.  Low aces are removed.
static inline Cards Cards_raiseAces(Cards cards) {
    return (cards | ((cards & LOW_ACES) << 13)) & FULL_DECK;
}

static inline Cards Cards_preferHighAces(Cards cards) {
    return cards & ~((cards & HIGH_ACES) >> 13);
}

static inline Cards Cards_sameValue(Cards card) {
    assert(card != 0);
    card |= (card << 32) | (card >> 32);
    card |= (card << 16) | (card >> 16);
    return card;
}

// Return the Card corresponding to the lowest card in the Cards set.
static inline Card Cards_toCard(Cards cards) {
    assert(cards != 0);
    return __builtin_ctzll(cards); // count trailing zeros
}

static inline Cards Cards_fromCard(Card c) {
    assert(Card_isLegal(c));
    return 1ULL << c;
}

// Return the point value of the Cards set.  Counts low aces as 5 points
// and high aces as 15 points.
static inline int Cards_points(Cards cards) {
    uint64_t five = cards & 0x21FF21FF21FF21FFULL;
    uint64_t ten = cards & 0x3E003E003E003E00ULL;
    return 5 * (__builtin_popcountll(five) + (__builtin_popcountll(ten) << 1));
}

// Iterate over the cards in a Cards set:
//   for (Cards c = Cards_first(cards); c != 0; c = Cards_next(cards, c)) { ... } 
static inline Cards Cards_first(Cards cards) {
    return cards & -cards;
}

static inline Cards Cards_next(Cards cards, Cards card) {
    cards &= ~(((card << 1) - 1));
    return cards & -cards;
}

// Create a Cards set from a string of space-separated card codes.
Cards Cards_fromString(const char *str);

// Print the Cards set as a space-separated list of card codes.
void Cards_print(Cards cards);
void Cards_printToFile(Cards cards, FILE *file);
void Cards_printExposed(Cards cards, Cards exposed);

#endif // CARDS_H