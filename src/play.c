#include "play.h"

void Play_find(Table *table, Cards hand, Play *play) {
    // Add a low ace for every high ace so we can discover a-2-3 runs.
    Cards lowHand = Cards_addLowAces(hand);

    play->runCenters = hand & (hand << 1) & (hand >> 1);

    // Need to exclude sets of low aces here... TODO
    play->setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    play->runExtensions = ((table->runs << 1) | (table->runs >> 1)) & lowHand;
    play->setExtensions = ((table->sets << 16) | (table->sets >> 16)) & lowHand;
}