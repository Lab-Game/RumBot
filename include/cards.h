#ifndef CARDS_H
#define CARDS_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

// A "Card" is a single playing card, represented as a number from 0 to 63.
// The low 4 bits are the value (0=low Ace, 1=2, ..., 12=King, 13=high Ace).
// The high 4 bits are the suit (0=Clubs, 1=Diamonds, 2=Hearts, 3=Spades).
// Values 14-15, 30-31, 46-47, and 62-63, and 64-255 are illegal.
typedef uint8_t Card;

#define ILLEGAL_CARD 255

static inline bool Card_isLegal(Card i) {
    return (1ULL << i) & 0x3FFF3FFF3FFF3FFF;
}

static inline int Card_value(Card c) {
    assert(Card_isLegal(c));
    return c & 0xF;
}

static inline int Card_suit(Card c) {
    assert(Card_isLegal(c));
    return c >> 4;
}

static inline Card Card_fromSuitValue(int suit, int value) {
    assert(suit >= 0 && suit <= 3);
    assert(value >= 0 && value <= 13);
    return (suit << 4) | value;
}

// Return a two-letter code for legal cards, e.g. "8C" for the 8 of Clubs.
// Low aces are represented as "a", high aces as "A".
// Illegal cards are are represented as the number followed by "?", e.g. "14?".
const char *Card_name(Card i);

// Print the two-letter code for a legal card.
void Card_print(Card c);

// The Cards type is a bitmask used to represent a set of playing cards.
// There are bits to represent low aces, but these are used only when
// representings runs, the sole place where low aces are meaningful.
typedef uint64_t Cards;

// Full deck mask (all legal cards).  Only high aces are included.
#define FULL_DECK 0x3FFE3FFE3FFE3FFEULL
#define RED_CARDS 0x0000FFFF0000FFFFULL
#define BLACK_CARDS 0x3FFF00003FFF0000ULL

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
static inline void Cards_remove(Cards *cards, Cards c) {
    assert(Cards_has(*cards, c));
    *cards &= ~c;
}

// Return the number of cards in the Cards set.
static inline int Cards_size(Cards cards) {
    return __builtin_popcountll(cards);
}

// Return a Cards set with a low ace added for every high ace present.
static inline Cards Cards_lowerAces(Cards cards) {
    const uint64_t kHighAceMask = 0x2000200020002000ULL;
    return cards | ((cards & kHighAceMask) >> 13);
}

// Return a Cards set with a high ace added for every low ace present.
static inline Cards Cards_raiseAces(Cards cards) {
    const uint64_t kLowAceMask = 0x0001000100010001ULL;
    return (cards | ((cards & kLowAceMask) << 13)) & FULL_DECK;
}

static inline Cards Cards_preferHighAces(Cards cards) {
    const uint64_t kHighAceMask = 0x2000200020002000ULL;
    return cards & ~((cards & kHighAceMask) >> 13);
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
void Cards_printExposed(Cards cards, Cards exposed);

#endif // CARDS_H