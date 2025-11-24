#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cards.h"

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