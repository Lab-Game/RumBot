#include <stdio.h>

#include "play.h"

void Play_find(Table *table, Cards hand, Play *play) {
    // Add a low ace for every high ace so we can discover a-2-3 runs.
    Cards lowHand = Cards_addLowAces(hand);
    play->runCenters = hand & (lowHand << 1) & (hand >> 1);
    play->setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    play->runExtensions = ((table->runs << 1) | (table->runs >> 1)) & lowHand;
    play->setExtensions = ((table->sets << 16) | (table->sets >> 16)) & lowHand;
}

void Play_print(Play *play) {
    printf("Run Centers: ");
    Cards_print(play->runCenters);
    printf("\nRun Extensions: ");
    Cards_print(play->runExtensions);
    printf("\nSet Centers: ");
    Cards_print(play->setCenters);
    printf("\nSet Extensions: ");
    Cards_print(play->setExtensions);
    printf("\n");
}