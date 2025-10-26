#include <stdio.h>

#include "plays.h"

void Plays_findAll(Table *table, Cards hand, Plays *plays) {
    // Add a low ace for every high ace so we can discover a-2-3 runs.
    Cards lowHand = Cards_addLowAces(hand);
    plays->runCenters = hand & (lowHand << 1) & (hand >> 1);
    plays->setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    plays->runExtensions = ((table->runs << 1) | (table->runs >> 1)) & lowHand;
    plays->setExtensions = ((table->sets << 16) | (table->sets >> 16)) & lowHand;
}

void Plays_findAcceptable(Table *table, Cards hand, Plays *accepted, Plays *rejected, Plays *plays) {
    Plays_findAll(table, hand, plays);

    // Exclude previously rejected melds.
    Plays_exclude(plays, rejected);

    // For runs, I want to avoid counting the same run in multiple ways.
    // For example, I want to avoid:
    //  - Adding a 3 after a 1
    //  - Adding a 1 before a 3
    //  - Adding three consecutive 1's
    // Fortunately, this logic runs really fast.
    plays->runExtensions &= ~(accepted->runCenters >> 2);
    plays->runExtensions &= ~(accepted->runExtensions << 2);
    plays->runCenters &= ~(accepted->runExtensions << 2);

    // For sets, I want to avoid counting a four-of-a-kind
    // in four different ways.  So the only legal way so to do so is
    // AC + AD AH AS.
    plays->setExtensions &= ~(((accepted->setCenters >> 32) | (accepted->setCenters << 32)) & 0xFFFFFFFFFFFF0000ULL);
}

void Plays_print(Plays *plays) {
    // Print the cards in each run or run extension bracketed with angle brackets.
    // Print the cards in each set or set extension bracketed with square brackets.
    // Keep everything compact on one line.

    for (Cards cs = Cards_first(plays->runCenters); cs != 0; cs = Cards_next(plays->runCenters, cs)) {
        Cards run = Plays_runCenterToCards(cs);
        printf(" <");
        Cards_print(run);
        printf(">");
    }

    for (Cards cs = Cards_first(plays->runExtensions); cs != 0; cs = Cards_next(plays->runExtensions, cs)) {
        Cards run = cs;
        printf(" <");
        Cards_print(run);
        printf(">");
    }

    for (Cards cs = Cards_first(plays->setCenters); cs != 0; cs = Cards_next(plays->setCenters, cs)) {
        Cards set = Plays_setCenterToCards(cs);
        printf(" [");
        Cards_print(set);
        printf("]");
    }

    for (Cards cs = Cards_first(plays->setExtensions); cs != 0; cs = Cards_next(plays->setExtensions, cs)) {
        Cards set = cs;
        printf(" [");
        Cards_print(set);
        printf("]");
    }
}