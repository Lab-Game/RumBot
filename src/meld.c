#include <stdio.h>
#include "meld.h"

void Meld_print(Meld *meld) {
    printf("+---------------------------------------------------\n");
    // Print all the runs in the Meld.  Separate cards in the same run by spaces,
    // and separate different runs by commas.
    printf("|  Runs: ");
    Cards prevPrinted = 0;
    for (Cards c = Cards_first(meld->runs); c != 0; c = Cards_next(meld->runs, c)) {
        if (prevPrinted != 0) {
            if ((prevPrinted << 1) == c) {
                // Same run, just print a space
                printf(" ");
            } else {
                // Different run, print a comma
                printf(", ");
            }
        }
        prevPrinted = c;
        Cards_print(c);
    }
    printf("\n");

    // Similarly, print all the sets in the Meld.
    printf("|  Sets: ");
    prevPrinted = 0;
    for (int value = 0; value <= 13; ++value) {
        for (int suit = 0; suit < 4; ++suit) {
            Cards c = Cards_fromCard(value + (suit << 4));
            if (Cards_has(meld->sets, c)) {
                if (prevPrinted != 0) {                        
                    if ((Cards_toCard(prevPrinted) & 0xF) == value) {
                        // Same set, just print a space
                        printf(" ");
                    } else {
                        // Different set, print a comma
                        printf(", ");
                    }
                }
                prevPrinted = c;
                Cards_print(c);
            }
        }
    }
    printf("\n");

    printf("+---------------------------------------------------\n");
}
