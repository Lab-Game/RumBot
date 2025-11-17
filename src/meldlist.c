#include <stdlib.h>
#include <stdio.h>

#include "meldlist.h"

void MeldList_init(MeldList *list) {
    list->size = 0;
}

bool MeldList_add(MeldList *list, Meld *meld) {
    if (list->size >= MELDLIST_MAX_SIZE) {
        return false;
    } else {
        list->melds[list->size++] = *meld;
        return true;
    }
}

bool MeldList_isFull(MeldList *list) {
    return list->size >= MELDLIST_MAX_SIZE;
};

// This structure holds state used during the recursive meld generation.
typedef struct {
    Cards handExt;
    Cards noRun;
    Cards noSet;
    Meld *oldTable;
    Meld table;
    Meld meld;
    Cards mustMeld;
    MeldList *list;
} MeldData;

void MeldList_fillRec(MeldData *data) {
    if (MeldList_isFull(data->list)) {
        // No space for additional meld options.  Stop recursing.
        return;
    }

    if (data->handExt == 0 ) {
        assert(Cards_has(Meld_cards(&data->table), data->mustMeld));

        if (Cards_has(Meld_cards(&data->table), data->mustMeld)) {
            // The must-be-played card has been played.  Add all cards
            // newly-added to the table to the meld option list.
            data->meld.runs = data->table.runs & ~data->oldTable->runs;
            data->meld.sets = data->table.sets & ~data->oldTable->sets;
            MeldList_add(data->list, &data->meld);
        }
        return;
    }

    // Get the first card in the hand.  This could be a low ace.
    Cards card = Cards_first(data->handExt);

    // Generate nearby cards in the deck.  These may be illegal.
    Cards prev = card >> 1;
    Cards next = card << 1;
    Cards nextNext = card << 2;

    // Handle low aces as a special case.  Can only appear in runs.
    if (Cards_isLowAce(card)) {
        Cards highAce = card << 13;

        Cards run = card | next | nextNext;
        if (Cards_has(data->handExt, run)) {
            Cards runExt = run | highAce;
            data->handExt ^= runExt;
            data->table.runs ^= run;
            MeldList_fillRec(data);
            data->table.runs ^= runExt;
            data->handExt ^= run;
        }

        // Play two cards by prepending to an existing run.
        Cards twoCards = card | next;
        if (Cards_has(data->handExt, twoCards) && Cards_has(data->table.runs, nextNext)) {
            Cards twoCardsExt = twoCards | highAce;
            data->handExt ^= twoCardsExt;
            data->table.runs ^= twoCards;
            MeldList_fillRec(data);
            data->table.runs ^= twoCards;
            data->handExt ^= twoCardsExt;
        }

        // Play one card by prepending to an existing run.
        if (Cards_has(data->table.runs, next)) {
            Cards cardExt = card | highAce;
            data->handExt ^= cardExt;
            data->table.runs ^= card;
            MeldList_fillRec(data);
            data->table.runs ^= card;
            data->handExt ^= cardExt;
        }
    } else {
        // This could be any card other than a low ace.  (The low ace
        // from this suit is now gone from the hand, either played
        // or set aside.)

        // Consider playing this card in a run, unless we previoiusly
        // ruled out that possibility to avoid duplicate melds.
        if (!Cards_has(data->noRun, card)) {

            // Consider playing a three-card run.
            Cards run = card | next | nextNext;
            if (Cards_has(data->handExt, run)) {
                data->handExt ^= run;
                data->table.runs ^= run;
                MeldList_fillRec(data);
                data->table.runs ^= run;
                data->handExt ^= run;
            }

            // Consider playing a two-card sequence.
            Cards twoCards = card | next;
            if (Cards_has(data->handExt, twoCards) &&
                (Cards_has(data->table.runs, prev) || Cards_has(data->table.runs, nextNext))) {
                data->noRun ^= nextNext;
                data->handExt ^= twoCards;
                data->table.runs ^= twoCards;
                MeldList_fillRec(data);
                data->table.runs ^= twoCards;
                data->handExt ^= twoCards;
                data->noRun ^= nextNext;
            }

            // Consider playing a single-card run.
            if (Cards_has(data->table.runs, prev) || Cards_has(data->table.runs, next)) {
                data->noRun ^= next;
                data->handExt ^= card;
                data->table.runs ^= card;
                MeldList_fillRec(data);
                data->table.runs ^= card;
                data->handExt ^= card;
                data->noRun ^= next;
            }
        }

        // Consider playing this card in a set, unless we ruled that out earlier.
        if (!Cards_has(data->noSet, card)) {

            // Form the set of all cards with the same value.
            Cards sameValue = Cards_sameValue(card);

            // Find all cards of this value in hand and on the table.
            Cards sameValueInHand = sameValue & data->handExt;
            Cards sameValueOnTable = sameValue & Meld_cards(&data->table);

            // Count cards of the same value in hand and on table.
            int numSameValueInHand = Cards_size(sameValueInHand);
            int numSameValueOnTable = Cards_size(sameValueOnTable);

            // Play all the cards of this value from the hand, provided that
            // puts at least three cards on the table.  Scenarios are:
            //   - 1 in hand + 3 on table
            //   - 3 in hand (no cards on table)
            //   - 4 in hand (no cards on table)
            if (numSameValueInHand + numSameValueOnTable >= 3) {
                data->handExt ^= sameValueInHand;
                data->table.sets ^= sameValueInHand;
                MeldList_fillRec(data);
                data->table.sets ^= sameValueInHand;
                data->handExt ^= sameValueInHand;
            }

            // If there are four cards of this value in hand, try
            // playing exactly three.  Bar playing the fourth in a set.
            if (numSameValueInHand == 4) {
                for (Cards x = Cards_first(sameValueInHand); x != 0; x = Cards_next(sameValueInHand, x)) {
                    Cards triple = sameValueInHand & ~x;
                    data->handExt ^= triple;
                    data->table.sets ^= triple;
                    data->noSet ^= x;
                    MeldList_fillRec(data);
                    data->noSet ^= x;
                    data->table.sets ^= triple;
                    data->handExt ^= triple;
                }
            }
        }
    }

    // Consider not playing this card, unless it is a mustMeld.  If this is a low ace,
    // then do not enforce the mustMeld restriction, since the high ace may still be played.
    // Also, remove only the low variant from the hand, for the same reason.
    if (!Cards_has(data->mustMeld, card)) {
        data->handExt ^= card;
        MeldList_fillRec(data);
        data->handExt ^= card;
    }
}

void MeldList_fill(MeldList *list, Cards hand, Meld *table, Cards mustMeld) {
    MeldData data;

    /*
    printf("MeldList_fill: hand = ");
    Cards_print(hand);
    printf(" table = ");
    Meld_printCompact(table);
    printf(" mustMeld = ");
    Cards_print(mustMeld);
    printf("\n"); */

    data.handExt = Cards_addLowAces(hand);
    data.noRun = 0;
    data.noSet = 0;
    data.oldTable = table;
    data.table = *table;
    data.mustMeld = mustMeld;
    data.list = list;

    MeldList_init(list);
    MeldList_fillRec(&data);
}