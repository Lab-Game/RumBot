#include <stdio.h>
#include "playlist.h"

void generatePlaysRec(Table *table, Cards hand, Plays *accepted, Plays *rejected);

void generatePlays(Table *table, Cards hand) {
    Plays accepted, rejected;
    Plays_init(&accepted);
    Plays_init(&rejected);

    generatePlaysRec(table, hand, &accepted, &rejected);
}

void generatePlaysRec(Table *table, Cards hand, Plays *accepted, Plays *rejected) {
    // Find all possible plays given the current table and hand.
    // Drop plays that were previously rejected.
    Plays plays;
    Plays_find(table, hand, &plays);
    Plays_exclude(&plays, rejected);

    // Consider each possible run.
    for (Cards cs = Cards_first(plays.runCenters); cs != 0; cs = Cards_next(plays.runCenters, cs)) {
        Cards run = Plays_runCenterToCards(cs);
        
        accepted->runCenters |= cs;
        Table_addRun(table, run);
        Cards_remove(&hand, run);
        generatePlaysRec(table, hand, accepted, rejected);
        Cards_add(&hand, run);
        Table_removeRun(table, run);
        accepted->runCenters &= ~cs;

        rejected->runCenters |= cs;
    }

    // Consider each possible set.
    for (Cards cs = Cards_first(plays.setCenters); cs != 0; cs = Cards_next(plays.setCenters, cs)) {
        Cards set = Plays_setCenterToCards(cs);

        accepted->setCenters |= cs;
        Table_addSet(table, set);
        Cards_remove(&hand, set);
        generatePlaysRec(table, hand, accepted, rejected);
        Cards_add(&hand, set);
        Table_removeSet(table, set);
        accepted->setCenters &= ~cs;

        rejected->setCenters |= cs;
    }

    printf("Accepted Plays:\n");
    Plays_print(accepted);
    printf("Rejected Plays:\n");
    Plays_print(rejected);
    printf("Hand after plays:\n");
    Cards_print(hand);
    printf("\nTable after plays:\n");
    Table_print(table);
    printf("\n");
}
