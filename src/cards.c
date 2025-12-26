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
const Cards kSpecialCard = 0x8000000000000000ULL;  // Temp used when swapping cards

Cards Cards_fromString(const char *str) {
    const char *values = "a23456789TJQKA";
    const char *suits = "CDSH";
    Cards cards = 0;

    if (!*str) {
        return cards;
    }

    while (true) {
        if (!*str || !*(str+1)) {
            return kSpecialCard;
        }

        char *valuePtr = strchr(values, *str);
        char *suitPtr = strchr(suits, *(str + 1));

        if (!valuePtr || !suitPtr) {
            return kSpecialCard;
        }

        str += 2;

        int value = valuePtr - values;
        int suit = suitPtr - suits;

        cards |= Cards_fromCard(Card_fromValueSuit(value, suit));

        if (!*str) {
            return cards;
        }
        
        if (*str != ' ') {
            return kSpecialCard;
        }

        str += 1;
    }
}

void Cards_printToFile(Cards cards, FILE *file) {
    bool first = true;

    if (Cards_has(cards, kSpecialCard)) {
        fprintf(file, "<special>");
        cards &= ~kSpecialCard;
        first = false;
    }

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