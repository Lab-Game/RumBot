#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "pile.h"

void Pile_fullDeck(Pile *pile) {
    Pile_init(pile);
    for (Cards c = Cards_first(kFullDeck); c != 0; c = Cards_next(kFullDeck, c)) {
        Pile_push(pile, c);
    }
}

void Pile_shuffle(Pile *pile) {
    for (int i = pile->size - 1; i > 0; --i) {
        int j = arc4random_uniform(i + 1);
        Cards temp = pile->cards[i];
        pile->cards[i] = pile->cards[j];
        pile->cards[j] = temp;
    }
}

Cards Pile_swap(Pile *pile, Cards insert, Cards extract) {
    for (int i = 0; i < pile->size; ++i) {
        if (pile->cards[i] == extract) {
            pile->cards[i] = insert;
            pile->allCards = (pile->allCards & ~extract) | insert;
            return extract;
        }
    }
    assert(false); // extract card not found
}

void Pile_fromString(Pile *pile, const char *str) {
    Pile_init(pile);

    if (*str == '\0') {
        // Empty string produces empty pile
        return;
    }

    while (1) {
        // Read card value
        char *valuePtr = strchr(kCardValues, *str++);
        assert(valuePtr);
        int value = valuePtr - kCardValues;

        // Read card suit
        char *suitPtr = strchr(kCardSuits, *str++);
        assert(suitPtr);
        int suit = suitPtr - kCardSuits;

        // Add the card to the pile
        Cards card = Cards_fromCard(Card_fromValueSuit(value, suit));
        Pile_push(pile, card);

        // Either end of string or space followed by next card
        if (*str == '\0') {
            return;
        } else {
            assert(*str == ' ');
            str++;
        }
    }
}

void Pile_print(Pile *pile) {
    Pile_printToFile(pile, stdout);
}

void Pile_printToFile(Pile *pile, FILE *file) {
    bool printed = false;
    for (int i = 0; i < pile->size; ++i) {
        Cards card = pile->cards[i];
        if (printed) {
            fprintf(file, " ");
        }
        Cards_printToFile(card, file);
        printed = true;
    }
}

