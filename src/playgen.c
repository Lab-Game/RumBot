#include <stdio.h>
#include <stdlib.h>

#include "playgen.h"

PlayList playList;

static int depth = 0;

// Given a table and hand, we'll recursively explore possible plays of runs,
// sets, and extensions from the hand onto the table.  As we consider each
// possible play, we either accept it (adding it to 'accepted') or reject it
// (adding it to 'rejected').  We try to keep the smaller of the number
// of accepted and rejected plays below a certain threshold, to avoid
// combinatorial explosion.
// When we reach a point where there are no more possible plays, we
// print the accepted plays as one possible play sequence.
static void generateRec(Table *table, Cards hand, Plays *accepted, Plays *rejected) {
    depth += 1;

    if (Plays_count(accepted) > MAXMIN && Plays_count(rejected) > MAXMIN) {
        // If we've both accepted and rejected lots of possible plays,
        // then we're heading toward a combinatorial explosion.  Stop exploring.
        return;
    }

    Plays plays;
    Cards lowHand = Cards_addLowAces(hand);

    // Find all possible runs, sets, and extensions, given the current table and hand.
    plays.runCenters = hand & (lowHand << 1) & (hand >> 1);
    plays.setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    plays.runExtensions = ((table->runs << 1) | (table->runs >> 1)) & lowHand;
    plays.setExtensions = ((table->sets << 16) | (table->sets >> 16)) & lowHand;

    // Exclude melds that were rejected at a higher level in the recursive search.
    plays.runCenters &= ~rejected->runCenters;
    plays.runExtensions &= ~rejected->runExtensions;
    plays.setCenters &= ~rejected->setCenters;
    plays.setExtensions &= ~rejected->setExtensions;

    // We must avoid tiling the same N-card run with different combinations
    // of 3-card runs and 1-card extensions.  The canonical tiling consists
    // of as many 3-card runs as possible, followed by 0, 1, or 2 single-card
    // extensions.  To enforce this, we apply the following rules:
    //   - Never add a 3-card-run immediately after a 1-card extension
    //   - Never add a 1-card extension immediately before a 3-card-run
    //   - Never add 3 consecutive 1-card extensions
    plays.runExtensions &= ~(accepted->runCenters >> 2);
    plays.runCenters &= ~(accepted->runExtensions << 2);
    plays.runExtensions &= ~((accepted->runExtensions << 1) & (accepted->runExtensions << 2));

    // Similarly, we must avoid tiling a four-of-a-kind in four
    // different ways.  The only permitted tiling is AD AH AS + AC.
    // When looking at a specific card as a possible set extension, we
    // check to see if the "opposite" card of the same value is the center
    // of an accepted set.  If so, then this specific card can only be
    // an extension if it is clubs.
    plays.setExtensions &= ~(((accepted->setCenters >> 32) | (accepted->setCenters << 32)) & 0xFFFFFFFFFFFF0000ULL);

    // We'll now find the first possible play (run, set, or extension) and
    // consider both accepting and rejecting it, recursively exploring
    // each choice.  Note that we only consider accepting and rejecting
    // one play per involcation of this function.

    // Consider each possible run.
    Cards c;

    if ((c = Cards_first(plays.runCenters))) {
        Cards run = Plays_runCenterToCards(c);
        Cards raised = Cards_raiseAces(run);

        // Accept this run
        accepted->runCenters |= c;
        Table_addRun(table, run);
        Cards_remove(&hand, raised);
        generateRec(table, hand, accepted, rejected);
        Cards_add(&hand, raised);
        Table_removeRun(table, run);
        accepted->runCenters &= ~c;
        
        // Reject this run
        rejected->runCenters |= c;
        generateRec(table, hand, accepted, rejected);
        rejected->runCenters &= ~c;
    } else if ((c = Cards_first(plays.setCenters))) {
        Cards set = Plays_setCenterToCards(c);

        // Accept this set
        accepted->setCenters |= c;
        Table_addSet(table, set);
        Cards_remove(&hand, set);
        generateRec(table, hand, accepted, rejected);
        Cards_add(&hand, set);
        Table_removeSet(table, set);
        accepted->setCenters &= ~c;

        // Reject this set
        rejected->setCenters |= c;
        generateRec(table, hand, accepted, rejected);
        rejected->setCenters &= ~c;
    } else if ((c = Cards_first(plays.runExtensions))) {
        Cards run = c;
        Cards raised = Cards_raiseAces(run);

        // Accept this run extension
        accepted->runExtensions |= c;
        Table_addRun(table, run);
        Cards_remove(&hand, raised);
        generateRec(table, hand, accepted, rejected);
        Cards_add(&hand, raised);
        Table_removeRun(table, run);
        accepted->runExtensions &= ~c;

        // Reject this run extension
        rejected->runExtensions |= c;
        generateRec(table, hand, accepted, rejected);
        rejected->runExtensions &= ~c;
    } else if ((c = Cards_first(plays.setExtensions))) {
        Cards set = c;

        // Accept this set extension
        accepted->setExtensions |= c;
        Table_addSet(table, set);
        Cards_remove(&hand, set);
        generateRec(table, hand, accepted, rejected);
        Cards_add(&hand, set);
        Table_removeSet(table, set);
        accepted->setExtensions &= ~c;

        // Reject this set extension
        rejected->setExtensions |= c;
        generateRec(table, hand, accepted, rejected);
        rejected->setExtensions &= ~c;
    } else {

        playList.plays[playList.size++] = *accepted;

        // No more possible plays.  Print the accepted plays.
        for (int i = 0; i < depth; ++i) {
            printf(" ");
        }
        Plays_print(accepted);
        printf("     /     ");
        Plays_print(rejected);
        printf("\n");
    }

    depth -= 1;
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

    playList.size = 0;

    depth = 0;
    generateRec(table, hand, &accepted, &rejected);
}
