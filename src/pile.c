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

void Pile_swapToTop(Pile *pile, int index) {
    assert(index >= 0 && index < pile->size);
    Cards temp = pile->cards[index];
    pile->cards[index] = pile->cards[pile->size - 1];
    pile->cards[pile->size - 1] = temp;
}

void Pile_printToFile(Pile *pile, FILE *file) {
    bool printed = false;
    for (int i = 0; i < pile->size; ++i) {
        Cards card = pile->cards[i];
        assert(Cards_size(card) == 1);
        if (printed) {
            fprintf(file, " ");
        }
        Cards_printToFile(card, file);
        printed = true;
    }
}

void Pile_print(Pile *pile) {
    Pile_printToFile(pile, stdout);
}
