#include <stdio.h>
#include <stdlib.h>

#include "playgen.h"

PlayList playList;

static int depth = 0;

static void generateRec(Table *table, Cards hand, Plays *accepted, Plays *rejected) {
    int numAccepted = Plays_count(accepted);
    int numRejected = Plays_count(rejected);

    Plays plays;
    Cards lowHand = Cards_addLowAces(hand);

    // Find all possible runs, sets, and extensions.
    plays.runCenters = hand & (lowHand << 1) & (hand >> 1);
    plays.setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    plays.runExtensions = ((table->runs << 1) | (table->runs >> 1)) & lowHand;
    plays.setExtensions = ((table->sets << 16) | (table->sets >> 16)) & lowHand;

    // Exlude melds that were rejected at a higher level in the recursive search.
    plays.runCenters &= ~rejected->runCenters;
    plays.runExtensions &= ~rejected->runExtensions;
    plays.setCenters &= ~rejected->setCenters;
    plays.setExtensions &= ~rejected->setExtensions;

    // To avoid counting the same run in multiple ways:
    //   - Never add a 3-card-run immediately after a 1-card extension
    //   - Never add a 1-card extension immediately before a 3-card-run
    //   - Never add 3 consecutive 1-card extensions
    plays.runExtensions &= ~(accepted->runCenters >> 2);
    plays.runExtensions &= ~(accepted->runExtensions << 2);
    plays.runCenters &= ~(accepted->runExtensions << 2);

    // For sets, I want to avoid counting a four-of-a-kind
    // in four different ways.  So the only legal way so to do so is
    // AC + AD AH AS.
    plays.setExtensions &= ~(((accepted->setCenters >> 32) | (accepted->setCenters << 32)) & 0xFFFFFFFFFFFF0000ULL);

    // Consider each possible run.
    for (Cards cs = Cards_first(plays.runCenters); cs != 0; cs = Cards_next(plays.runCenters, cs)) {
        Cards run = Plays_runCenterToCards(cs);
        Cards raised = Cards_raiseAces(run);

        // Try playing this run, unless we've rejected lots of plays already.
        if (numAccepted < MAXMIN || numRejected <= MAXMIN) {
            accepted->runCenters |= cs;
            Table_addRun(table, run);
            Cards_remove(&hand, raised);
            depth += 1;
            generatePlaysRec(table, hand, accepted, rejected);
            depth -= 1;
            Cards_add(&hand, raised);
            Table_removeRun(table, run);
            accepted->runCenters &= ~cs;
        }

        // Try rejecting this run, unless we've accepted lots of plays already.
        if (numRejected < MAXMIN || numAccepted <= MAXMIN) {
            rejected->runCenters |= cs;
            depth += 1;
            generatePlaysRec(table, hand, accepted, rejected);
            depth -= 1;
            rejected->runCenters &= ~cs;
        }

        return;
    }

    // Consider each possible set.
    for (Cards cs = Cards_first(plays.setCenters); cs != 0; cs = Cards_next(plays.setCenters, cs)) {
        Cards set = Plays_setCenterToCards(cs);

        // Try playing this set, unless we've rejected lots of plays already.
        if (numAccepted < MAXMIN || numRejected <= MAXMIN) {
            accepted->setCenters |= cs;
            Table_addSet(table, set);
            Cards_remove(&hand, set);
            depth += 1;
            generatePlaysRec(table, hand, accepted, rejected);
            depth -= 1;
            Cards_add(&hand, set);
            Table_removeSet(table, set);
            accepted->setCenters &= ~cs;
        }

        // Try rejecting this set, unless we've accepted lots of plays already.
        if (numRejected < MAXMIN || numAccepted <= MAXMIN) {
            rejected->setCenters |= cs;
            depth += 1;
            generatePlaysRec(table, hand, accepted, rejected);
            depth -= 1;
            rejected->setCenters &= ~cs;
        }

        return;
    }

    // Consider each possible run extension.
    for (Cards cs = Cards_first(plays.runExtensions); cs != 0; cs = Cards_next(plays.runExtensions, cs)) {
        Cards run = cs;
        Cards raised = Cards_raiseAces(run);

        // Try playing this run extension, unless we've rejected lots of plays already.
        if (numAccepted < MAXMIN || numRejected <= MAXMIN) {
            accepted->runExtensions |= cs;
            Table_addRun(table, run);
            Cards_remove(&hand, raised);
            depth += 1;
            generatePlaysRec(table, hand, accepted, rejected);
            depth -= 1;
            Cards_add(&hand, raised);
            Table_removeRun(table, run);
            accepted->runExtensions &= ~cs;
        }

        // Try rejecting this run extension, unless we've accepted lots of plays already.
        if (numRejected < MAXMIN || numAccepted <= MAXMIN) {
            rejected->runExtensions |= cs;
            depth += 1;
            generatePlaysRec(table, hand, accepted, rejected);
            depth -= 1;
            rejected->runExtensions &= ~cs;
        }

        return;
    }

    // Consider each possible set extension.
    for (Cards cs = Cards_first(plays.setExtensions); cs != 0; cs = Cards_next(plays.setExtensions, cs)) {
        Cards set = cs;

        // Try playing this set extension, unless we've rejected lots of plays already.
        if (numAccepted < MAXMIN || numRejected <= MAXMIN) {
            accepted->setExtensions |= cs;
            Table_addSet(table, set);
            Cards_remove(&hand, set);
            depth += 1;
            generatePlaysRec(table, hand, accepted, rejected);
            depth -= 1;
            Cards_add(&hand, set);
            Table_removeSet(table, set);
            accepted->setExtensions &= ~cs;
        }

        // Try rejecting this set extension, unless we've accepted lots of plays already.
        if (numRejected < MAXMIN || numAccepted <= MAXMIN) {
            rejected->setExtensions |= cs;
            depth += 1;
            generatePlaysRec(table, hand, accepted, rejected);
            depth -= 1;
            rejected->setExtensions &= ~cs;
        }

        return;
    }

    // No more possible plays.  Print the accepted plays.
    for (int i = 0; i < depth; ++i) {
        printf(" ");
    }
    Plays_print(accepted);
    printf("     /     ");
    Plays_print(rejected);
    printf("\n");
}

void PlayGen_generate(Cards hand, Table *table) {
    printf("Generating plays for table:\n");
    Table_print(table);
    printf("from hand: ");
    Cards_print(hand);
    printf("\n\n");

    Plays accepted, rejected;
    Plays_init(&accepted);
    Plays_init(&rejected);

    depth = 0;
    generateRec(table, hand, &accepted, &rejected);
}
