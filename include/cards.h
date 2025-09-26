#ifndef CARDS_H
#define CARDS_H

// A "Card" is a single playing card, represented as a number from 0 to 63.
// The low 4 bits are the value (0=low Ace, 1=2, ..., 12=King, 13=high Ace).
// The high 4 bits are the suit (0=Clubs, 1=Diamonds, 2=Hearts, 3=Spades).
// Values 14-15, 30-31, 46-47, and 62-63, and 64-255 are illegal.
//
// A "Cards" is a bitmask of cards, with bit 0 representing card 0, bit 1
// representing card 1, and so forth.  This allows a "Cards" to represent
// both individual cards and sets of cards.

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t Card;

extern const char *kCardName[64];

static inline bool Card_isLegal(Card i) {
    return (1ULL << i) & 0x3FFF3FFF3FFF3FFF;
}

const char *Card_name(Card i);

typedef uint64_t Cards;

#define FULL_DECK 0x3FFE3FFE3FFE3FFEULL

static inline bool Cards_isLegal(Cards cards) {
    return (cards & ~FULL_DECK) == 0;
}

static inline bool Cards_has(Cards cards, Cards c) {
    return (cards & c) == c;
}

static inline void Cards_add(Cards *cards, Cards c) {
    *cards |= c;
}

static inline void Cards_remove(Cards *cards, Cards c) {
    *cards &= ~c;
}

static inline int Cards_size(Cards cards) {
    return __builtin_popcountll(cards);
}

static inline Cards Cards_addLowAces(Cards cards) {
    const uint64_t kHighAceMask = 0x0001000100010001ULL;
    return cards | ((cards & kHighAceMask) >> 13);
}

static inline Card Cards_toCard(Cards cards) {
    assert(cards != 0);
    return __builtin_ctzll(cards); // count trailing zeros
}

static inline int Cards_points(Cards cards) {
    uint64_t five = cards & 0x21FF21FF21FF21FFULL;
    uint64_t ten = cards & 0x3E003E003E003E00ULL;
    return 5 * (__builtin_popcountll(five) + (__builtin_popcountll(ten) << 1));
}

// Iterate over the cards in a Cards set:
//   for (Cards c = Cards_low(cards); c != 0; c = Cards_next(cards, c)) { ... } 
static inline Cards Cards_low(Cards cards) {
    return cards & -cards;
}

static inline Cards Cards_next(Cards cards, Cards card) {
    cards &= ~(((card << 1) - 1));
    return cards & -cards;
}

Cards Cards_fromString(const char *str);
void Cards_print(Cards cards);

#endif // CARDS_H