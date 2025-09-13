#include "cards.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const char *kCardName[64] = {
    "aC", "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "TC", "JC", "QC", "KC", "AC", "14", "15",
    "aD", "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "TD", "JD", "QD", "KD", "AD", "30", "31",
    "aH", "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "TH", "JH", "QH", "KH", "AH", "46", "47",
    "aS", "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "TS", "JS", "QS", "KS", "AS", "62", "63"
};

const char *Card_name(Card i) {
    if (!Card_isLegal(i)) {
        return "??";
    } else {
        return kCardName[i];
    }
}

Cards Cards_fromString(const char *str) {
    const char *values = "a23456789TJQKA";
    const char *suits = "CDHS";
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

        cards |= (1ULL << (value + (suit << 4)));

        if (!*str) {
            return cards;
        } else {
            assert(*str == ' ');
            str++;
        }
    }
}

void Cards_print(Cards cards) {
    bool first = true;
    for (Cards cs = Cards_low(cards); cs != 0; cs = Cards_next(cards, cs)) {
        Card c = Cards_toCard(cs);
        if (first) {
            printf("%s", Card_name(c));
            first = false;
        } else {
            printf(" %s", Card_name(c));
        }
    }
    if (first) {
        printf("(none)");
    }
}