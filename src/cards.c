#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cards.h"

static const char *kCardName[64] = {
    "aC", "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "TC", "JC", "QC", "KC", "AC", "14?", "15?",
    "aD", "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "TD", "JD", "QD", "KD", "AD", "30?", "31?",
    "aS", "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "TS", "JS", "QS", "KS", "AS", "62?", "63?",
    "aH", "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "TH", "JH", "QH", "KH", "AH", "46?", "47?",
};

const char *Card_name(Card i) {
    return kCardName[i];
}

void Card_print(Card c) {
    if ((1ULL << c) & RED_CARDS) {
        printf("\x1B[31m%s\x1B[0m", Card_name(c));  // print in red
    } else if ((1ULL << c) & BLACK_CARDS) {
        printf("\x1B[34m%s\x1B[0m", Card_name(c));  // print in blue
    } else {
        printf("%s", Card_name(c));  // illegal card, print normally
    }
}

Cards Cards_fromString(const char *str) {
    const char *values = "a23456789TJQKA";
    const char *suits = "CDSH";
    Cards cards = 0;

    if (!*str) {
        return cards;
    }

    while (true) {
        assert(*str);
        assert(strchr(values, *str));
        assert(*(str+1));
        assert(strchr(suits, *(str + 1)));

        int value = strchr(values, *(str++)) - values;
        int suit = strchr(suits, *(str++)) - suits;

        cards |= Cards_fromCard(Card_fromValueSuit(value, suit));

        if (!*str) {
            return cards;
        } else {
            assert(*str == ' ');
            str++;
        }
    }
}

void Cards_printToFile(Cards cards, FILE *file) {
    bool first = true;
    for (Cards cs = Cards_first(cards); cs != 0; cs = Cards_next(cards, cs)) {
        Card c = Cards_toCard(cs);
        if (first) {
            first = false;
        } else {
            fprintf(file, " ");
        }
        fprintf(file, "%s", Card_name(c));
    }
}

void Cards_print(Cards cards) {
    Cards_printToFile(cards, stdout);
}