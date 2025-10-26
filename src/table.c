#include <stdio.h>
#include "table.h"

void Table_print(Table *table) {
    printf("+---------------------------------------------------\n");
    // Print all the runs on the Table.  Separate cards in the same run by spaces,
    // and separate different runs by commas.
    printf("|  Runs: ");
    Cards prevPrinted = 0;
    for (Cards c = Cards_first(table->runs); c != 0; c = Cards_next(table->runs, c)) {
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

    // Similarly, print all the sets on the Table.
    printf("|  Sets: ");
    prevPrinted = 0;
    for (int value = 0; value < 13; ++value) {
        for (int suit = 0; suit < 4; ++suit) {
            Cards c = 1ULL << (value + (suit << 4));
            if (Cards_has(table->sets, c)) {
                if (prevPrinted != 0) {
                    if ((prevPrinted << 16) == c || (prevPrinted >> 48) == c) {
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
