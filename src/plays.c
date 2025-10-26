#include <stdio.h>

#include "plays.h"

void Plays_find(Table *table, Cards hand, Plays *plays) {
    // Add a low ace for every high ace so we can discover a-2-3 runs.
    Cards lowHand = Cards_addLowAces(hand);
    plays->runCenters = hand & (lowHand << 1) & (hand >> 1);
    plays->setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    plays->runExtensions = ((table->runs << 1) | (table->runs >> 1)) & lowHand;
    plays->setExtensions = ((table->sets << 16) | (table->sets >> 16)) & lowHand;
}

void Plays_print(Plays *plays) {
    if (plays->runCenters) {
        printf("Run Centers:      ");
        Cards_print(plays->runCenters);
        printf("\n");
    }

    if (plays->runExtensions) {
        printf("Run Extensions:   ");
        Cards_print(plays->runExtensions);
        printf("\n");
    }
    if (plays->setCenters) {
        printf("Set Centers:      ");
        Cards_print(plays->setCenters);
        printf("\n");
    }
    if (plays->setExtensions) {
        printf("Set Extensions:   ");
        Cards_print(plays->setExtensions);
        printf("\n");
    }
    printf("\n");
}