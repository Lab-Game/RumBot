#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cards.h"

const Cards kFullDeck = 0x3FFE3FFE3FFE3FFEULL;
const Cards kLegalCards = 0x3FFF3FFF3FFF3FFFULL;
const Cards kLowAces = 0x0001000100010001ULL;
const Cards kHighAces = 0x2000200020002000ULL;
const Cards kJoker = 0x8000000000000000ULL;  // Temp used when swapping cards

Cards Cards_fromString(const char *str) {
    const char *values = "a23456789TJQKA";
    const char *suits = "CDSH";
    Cards cards = 0;

    if (!*str) {
        return cards;
    }

    while (true) {
        if (!*str || !*(str+1)) {
            return kJoker;
        }

        char *valuePtr = strchr(values, *str);
        char *suitPtr = strchr(suits, *(str + 1));

        if (!valuePtr || !suitPtr) {
            return kJoker;
        }

        str += 2;

        int value = valuePtr - values;
        int suit = suitPtr - suits;

        cards |= Cards_fromCard(Card_fromValueSuit(value, suit));

        if (!*str) {
            return cards;
        }
        
        if (*str != ' ') {
            return kJoker;
        }

        str += 1;
    }
}

void Cards_print(Cards cards) {
    bool first = true;

    for (Cards cs = Cards_first(cards); cs != 0; cs = Cards_next(cards, cs)) {
        Card c = Cards_toCard(cs);
        if (first) {
            first = false;
        } else {
            putchar(' ');
        }
        
        if (c == kJoker) {
            printf("<special>");
        } else {
            printf("%s", Card_colorName(c));
        }
    }
}