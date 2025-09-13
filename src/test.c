#include <assert.h>
#include <stdio.h>

#include "cards.h"

void Cards_test(void) {
    Cards cards = Cards_fromString("aC TC 5D 6D 2H JH 6S KS AS");
    printf("Expected:  aC TC 5D 6D 2H JH 6S KS AS\n");
    printf("Actual:    ");
    Cards_print(cards);
    printf("\n");
    assert(Cards_points(cards) == 70);

    // Remove a card and check points again.
    Cards_remove(&cards, Cards_fromString("aC")); // remove aC
    printf("Expected:  TC 5D 6D 2H JH 6S KS AS\n");
    printf("Actual:    ");
    Cards_print(cards);
    printf("\n");
    assert(Cards_points(cards) == 65);

    // Add some cards and check points again.
    Cards_add(&cards, Cards_fromString("JC QC")); // add JC QC
    printf("Expected:  TC JC QC 5D 6D 2H JH 6S KS AS\n");
    printf("Actual:    ");
    Cards_print(cards);
    printf("\n");
    assert(Cards_points(cards) == 85);
}

int main(void) {
    Cards_test();
    return 0;
}