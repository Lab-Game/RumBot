#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
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

void Pile_print(Pile *pile) {
    bool printed = false;
    for (int i = 0; i < pile->size; ++i) {
        Cards card = pile->cards[i];
        assert(Cards_size(card) == 1);
        if (printed) {
            printf(" ");
        }
        Card_print(Cards_toCard(card));
        printed = true;
    }

    if (!printed) {
        printf("(empty pile)");
    }
}
