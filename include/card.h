#ifndef CARD_H
#define CARD_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

// A "Card" is a single playing card, represented as a number from 0 to 63.
// The low 4 bits are the value (0=low Ace, 1=2, ..., 12=King, 13=high Ace).
// The high 4 bits are the suit (0=Clubs, 1=Diamonds, 2=Hearts, 3=Spades).
// Values 14-15, 30-31, 46-47, and 62-63, and 64-255 are illegal.

typedef uint8_t Card;

extern const Card kIllegalCard;

static inline bool Card_isLegal(Card i) {
    return (i & 0xf) <= 13 && (i >> 4) <= 3;
}

static inline int Card_value(Card c) {
    assert(Card_isLegal(c));
    return c & 0xF;
}

static inline int Card_suit(Card c) {
    assert(Card_isLegal(c));
    return c >> 4;
}

static inline Card Card_fromValueSuit(int value, int suit) {
    assert(value >= 0 && value <= 13);
    assert(suit >= 0 && suit <= 3);
    return (suit << 4) | value;
}

static inline bool Card_isRed(Card c) {
    assert(Card_isLegal(c));
    return Card_suit(c) == 1 || Card_suit(c) == 3;
}

static inline bool Card_isBlack(Card c) {
    assert(Card_isLegal(c));
    return Card_suit(c) == 0 || Card_suit(c) == 2;
}

// Return a null-terminated two-letter code for legal cards, e.g.
// "8C" for the 8 of Clubs.  Low aces are represented as "a", high aces as "A".
const char *Card_name(Card i);

// Escape the two-letter code to add color.
const char *Card_colorName(Card i);

#endif // CARD_H
