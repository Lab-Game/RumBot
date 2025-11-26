#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "pile.h"

void Pile_fullDeck(Pile *pile) {
    Pile_init(pile);
    for (Cards c = Cards_first(FULL_DECK); c != 0; c = Cards_next(FULL_DECK, c)) {
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

void Pile_printToFile(Pile *pile, FILE *file) {
    bool printed = false;
    for (int i = 0; i < pile->size; ++i) {
        Cards card = pile->cards[i];
        // assert(Cards_size(card) == 1);
        if (printed) {
            fprintf(file, " ");
        }
        Cards_printToFile(card, file);
        printed = true;
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
    assert(false); // remove card not found
}

void Pile_fromString(Pile *pile, const char *str) {
    Pile_init(pile);

    if (!*str) {
        return;
    }

    while (true) {
        assert(*str);
        assert(strchr(kCardValues, *str));
        assert(*(str + 1));
        assert(strchr(kCardSuits, *(str + 1)));

        int value = strchr(kCardValues, *(str++)) - kCardValues;
        int suit = strchr(kCardSuits, *(str++)) - kCardSuits;

        Cards card = Cards_fromCard(Card_fromValueSuit(value, suit));
        Pile_push(pile, card);

        if (!*str) {
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
