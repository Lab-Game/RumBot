#include <stdlib.h>
#include <stdio.h>

#include "meldlist.h"

/*
 *  A MeldList is a list of possible melds plays by the current player.
 *  This module generates the list of possible melds from the player's
 *  hand, given the current meld on the table, and an optional card that
 *  must be included in the meld (because it was the deepest card taken
 *  from the discard pile).
 * 
 *  There are several complexities:
 *
 *  - When generating possible melds, care must be taken to avoid
 *    generating duplicate melds.  For example, if the hand contains
 *    a long run of hearts, then many different card combinations
 *    can be played on the table, and each must be generated exactly
 *    once for efficiency.  This is implicitly enforced by the
 *    structure of the recursion below.
 * 
 *  - There can be a "combinatorial explosion" of possible melds.
 *    So we cap the length of a meld list to MELDLIST_MAX_SIZE,
 *    and prefer melds that lay down lots of cards.  While perhaps
 *    not optimal play, this reduces the combinations in
 *    subsequent rounds.  That prevents us from wasting too
 *    much time on a rare scenario.
 * 
 *  - Aces in the hand are represented as "high", but aces in a run
 *    can be either "low" or "high".  This transition is an
 *    awkward detail running throughout meld generation.
 * 
 *  - The meld may be required to include a certain card.  This
 *    happens when the player takes more than one card from
 *    the discard pile.  This membership requirement is enforced
 *    somewhat late, potentially causing some wasted work,
 *    to avoid adding still more complexity to the recursive
 *    meld generation.
 */

// This is the workhorse recursive function that generates possible melds.
// The "mayRun" and "maySet" parameters represent cards in the player's hand
// that might be played in runs or sets, respectively.  Aces appear both low
// and high in these sets.
//
// The "newRuns" and "newSets" parameters represent cards in the meld on this
// recursion branch, including both cards played on both this and previous turns.
//
// Consider cards in mayRun and maySet one at a time, in increasing order.
// For each card, consider all possible ways to play it in a run, in a set,
// or not at all, and then recurse to consider the remaining cards.
void MeldList_genRec(MeldList *list, Meld *table, Cards mustMeld,
                     Cards mayRun, Cards maySet,
                     Cards newRuns, Cards newSets) {
    // If the meld list is full, stop recursing.
    if (list->size >= MELDLIST_MAX_SIZE) {
        return;
    }

    // Compute which cards are actually playable in runs and sets.  A card
    // might initially be playable in a set or run, but once other cards are
    // played, it might no longer be playable in this way.  These two sets
    // never grow during recursion.
    mayRun &= Meld_playableInRun(newRuns, mayRun);
    maySet &= Meld_playableInSet(newSets, maySet);

    // If the mustMeld card is neither played nor playable, backtrack.  The mustMeld
    // card can only be high (if it is an ace), but could be played as a low ace.
    if (!Cards_has(Cards_raiseAces(newRuns | mayRun) | newSets | maySet, mustMeld)) {
        return;
    }

    // These are all cards that might be added to the meld.
    Cards mayMeld = mayRun | maySet;

    // If no more cards can be played, we are are at a recursion leaf.
    // Add this meld to the MeldList and backtrack.
    if (mayMeld == 0 ) {
        Meld *meld = &list->melds[list->size++];
        meld->runs = newRuns & ~table->runs;
        meld->sets = newSets & ~table->sets;
        return;
    }

    // Get the next card that might be added to the meld.
    Cards card = Cards_first(mayMeld);

    // Consider playing this card (possibly a low ace!) in a run.
    if (Cards_has(mayRun, card)) {
        // Generate nearby cards in the deck sequence.  These
        // might be illegal and are therefore definitely not in
        // the hand and not on the table, blocking certain plays.
        Cards prev = card >> 1;
        Cards next = card << 1;
        Cards nextNext = card << 2;

        // Consider paying a three-card run.
        Cards run3 = card | next | nextNext;
        if (Cards_has(mayRun, run3)) {
            // Regardless of whether an ace is played low or high, both
            // the low and high versions become nonplayable hereafter.
            Cards played = Cards_addAllAces(run3);
            MeldList_genRec(list, table, mustMeld,
                mayRun & ~played, maySet & ~played, newRuns | run3, newSets);
        }

        // Consider playing a two-card sequence joined to an existing run,
        // either before or after.
        Cards run2 = card | next;
        if (Cards_has(mayRun, run2) &&
            (Cards_has(newRuns, prev) || Cards_has(newRuns, nextNext))) {
            Cards played = Cards_addAllAces(run2);
            MeldList_genRec(list, table, mustMeld,
                mayRun & ~played & ~nextNext, maySet & ~played, newRuns | run2, newSets);
        }

        // Consider a single card joined to an existing run.
        if (Cards_has(newRuns, prev) || Cards_has(newRuns, next)) {
            Cards played = Cards_addAllAces(card);
            MeldList_genRec(list, table, mustMeld,
                mayRun & ~played & ~next, maySet & ~played, newRuns | card, newSets);
        }
    }

    // Consider playing this card in a set.
    if (Cards_has(maySet, card)) {
        // Form the set of all cards with the same value.
        Cards sameValue = Cards_sameValue(card);

        // Find all cards of this value playable from the hand and on the table.
        Cards inHand = maySet & sameValue;
        Cards onTable = newSets & sameValue;

        // Count cards of the same value in hand and on table.
        int numInHand = Cards_size(inHand);
        int numOnTable = Cards_size(onTable);

        // Try playing all the cards of this value from the hand, provided that
        // puts at least three cards on the table.  Scenarios are:
        //   - 1 in hand + 3 on table
        //   - 3 in hand (no cards on table)
        //   - 4 in hand (no cards on table)
        if (numInHand + numOnTable >= 3) {
            Cards played = Cards_addAllAces(inHand);
            MeldList_genRec(list, table, mustMeld,
                mayRun & ~played, maySet & ~played, newRuns, newSets | inHand);
        }

        // If there are four cards of this value in hand, also try playing this
        // card and leaving out one of the others, to form a three-card set.
        // The left-out card can not be played as a set later, because that
        // would form a duplicate meld.
        if (numInHand == 4) {
            Cards others = inHand & ~card;
            for (Cards x = Cards_first(others); x != 0; x = Cards_next(others, x)) {
                Cards triple = card | (others & ~x);
                Cards played = Cards_addAllAces(triple);

                MeldList_genRec(list, table, mustMeld,
                    mayRun & ~played, maySet & ~played & ~x, newRuns, newSets | triple);
            }
        }
    }

    // Consider not playing this card, unless it is a mustMeld.  A decision
    // not to play one ace variant does not rule out playing the other.
    if (!Cards_has(mustMeld, card) ) {
        MeldList_genRec(list, table, mustMeld,
            mayRun & ~card, maySet & ~card, newRuns, newSets);
    }
}

void MeldList_generate(MeldList *list, Cards hand, Meld *table, Cards mustMeld) {
    list->size = 0;
    MeldList_genRec(list, table, mustMeld,
        Cards_addLowAces(hand), hand, table->runs, table->sets);
}

void MeldList_print(MeldList *list) {
    printf("MeldList with %d melds:\n", list->size);
    for (int i = 0; i < list->size; ++i) {
        printf("Meld %d: ", i);
        Meld_printCompact(&list->melds[i]);
        printf("\n");
    }
}
