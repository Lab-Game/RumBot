#include <stdio.h>
#include <stdlib.h>
#include "playlist.h"

void generatePlaysRec(Table *table, Cards hand, Plays *accepted, Plays *rejected);

// The number of accepted plays and the number of rejected plays can not BOTH
// exceed this threshold.
const int maxmin = 1;

static int depth = 0;

void generatePlays(Table *table, Cards hand) {
    printf("Generating plays for table:\n");
    Table_print(table);
    printf("from hand: ");
    Cards_print(hand);
    printf("\n\n");

    Plays accepted, rejected;
    Plays_init(&accepted);
    Plays_init(&rejected);

    depth = 0;
    generatePlaysRec(table, hand, &accepted, &rejected);
}

void generatePlaysRec(Table *table, Cards hand, Plays *accepted, Plays *rejected) {
    // Count the number of accepted and rejected plays.  I want to keep the
    // smaller of these two numbers below a threshold.  So, for example, suppose
    // the threshold is 2.  If I've accepted 5 and rejected 1, then I can reject at
    // most one more play, but I can accept any number of additional plays.
    int numAccepted = Plays_count(accepted);
    int numRejected = Plays_count(rejected);

    // Find all possible plays given the current table and hand.
    // Drop plays that were previously rejected.
    // TODO:  We may also implicitly reject some plays based on previous accepted plays.
    Plays plays;
    Plays_findAcceptable(table, hand, accepted, rejected, &plays);

    // Consider each possible run.
    for (Cards cs = Cards_first(plays.runCenters); cs != 0; cs = Cards_next(plays.runCenters, cs)) {
        Cards run = Plays_runCenterToCards(cs);
        Cards raised = Cards_raiseAces(run);

        // Try playing this run, unless we've rejected lots of plays already.
        if (numAccepted < maxmin || numRejected <= maxmin) {
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
        if (numRejected < maxmin || numAccepted <= maxmin) {
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
        if (numAccepted < maxmin || numRejected <= maxmin) {
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
        if (numRejected < maxmin || numAccepted <= maxmin) {
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
        if (numAccepted < maxmin || numRejected <= maxmin) {
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
        if (numRejected < maxmin || numAccepted <= maxmin) {
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
        if (numAccepted < maxmin || numRejected <= maxmin) {
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
        if (numRejected < maxmin || numAccepted <= maxmin) {
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