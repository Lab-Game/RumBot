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

// This structure holds state used during the recursive meld generation.
typedef struct {
    Cards hand;
    Cards handExt;
    Cards noRun;
    Cards noSet;
    Meld table;
    Cards mustMeld;
    MeldList *list;
} MeldData;

void MeldList_fillRec(MeldData *data) {
    if (data->hand == 0 ) {
        // No more cards in hand.
        Meld_printCompact(&data->table);
        printf("\n");
        return;
    }

    // Get the first card in the hand.
    // If this is an ace, form both high and low variants.
    Cards card = Cards_first(data->extendedHand);
    Cards cardExt = Cards_extendAces(card);

    // Consider playing the card in a run, unless the card is in noRun.
    if (!Cards_has(data->noRun, card)) {

        // Play a standalone, 3-card run.
        Cards run = (cardExt | (cardExt << 1) | (cardExt << 2)) & LEGAL_CARDS;
        if (Cards_has(data->hand, run)) {
            data->hand ^= run;
            data->table.runs ^= run;
            MeldList_fillRec(data);
            data->table.runs ^= run;
            data->hand ^= run;
        }

        // Play exactly two cards by extending an existing run.
        Cards twoCards = (cardExt | (cardExt << 1)) & LEGAL_CARDS;
        if (Cards_has(data->hand, twoCards) &&

        // Name earlier and later cards in the deck.
        Cards prev = card >> 1;
        Cards next = card << 1;
        Cards nextNext = card << 2;

        // Play exactly two cards.  Possible if the next card is also in
        // the hand and the prev or next-next card is on the table.  Put this card
        // and the next on the table, remove card, next, and next-next from the hand, and recurse.
        Cards twoCards = card | next;
        if (Cards_has(data->hand, twoCards) &&
            (Cards_has(data->table.runs, prev) || Cards_has(data->table.runs, nextNext))) {
            Cards noRun = nextNext & data->hand;
            data->hand ^= twoCards | noRun;
            data->table.runs ^= twoCards;
            data->noRun ^= noRun;
            MeldList_fillRec(data);
            data->noRun ^= noRun;
            data->table.runs ^= twoCards;
            data->hand ^= twoCards | noRun;
        }

        // Play only this card in a run.  Possible if the prev or next card is on the table.
        // If the next card is in the hand, move it to noRun.
        if (Cards_has(data->table.runs, prev) || Cards_has(data->table.runs, next)) {
            Cards noRun = next & data->hand;
            data->hand ^= card | noRun;
            data->table.runs ^= card;
            data->noRun ^= noRun;
            MeldList_fillRec(data);
            data->noRun ^= noRun;
            data->table.runs ^= card;
            data->hand ^= card | noRun;
        }
    }

    // Consider playing the card in a set, unless the card is in noSet.
    if (!Cards_has(data->noSet, card)) {
        // Form the set of all cards with the same value.
        Cards sameValue = Cards_sameValue(card);

        // Find all cards of this value in hand and on the table.
        Cards sameValueInHand = sameValue & data->hand;
        Cards sameValueOnTable = sameValue & Meld_cards(&data->table);

        // Count cards of the same value in hand and on table.
        int numSameValueInHand = Cards_size(sameValueInHand);
        int numSameValueOnTable = Cards_size(sameValueOnTable);

        // Option 5:  Play all the cards of this value from the hand,
        // provided that puts at least three on the table.  Scenarios
        // are:
        //   - 1 in hand + 3 on table
        //   - 3 in hand (no cards on table)
        //   - 4 in hand (no cards on table)
        if (numSameValueInHand + numSameValueOnTable >= 3) {
            data->hand ^= sameValueInHand;
            data->table.sets ^= sameValueInHand;
            MeldList_fillRec(data);
            data->table.sets ^= sameValueInHand;
            data->hand ^= sameValueInHand;
        }

        // Option 6:  If there are four cards of this value in hand,
        // play exactly three of them.
        if (numSameValueInHand == 4) {
            for (Cards x = Cards_first(sameValueInHand); x != 0; x = Cards_next(sameValueInHand, x)) {
                Cards threeCards = sameValueInHand & ~x;
                data->hand ^= threeCards;
                data->table.sets ^= threeCards;
                data->noSet ^= x;
                MeldList_fillRec(data);
                data->noSet ^= x;
                data->table.sets ^= threeCards;
                data->hand ^= threeCards;
            }
        }
    }

    // Consider not playing this card.
    data->hand ^= card;
    MeldList_fillRec(data);
    data->hand ^= card;
}

void MeldList_fill(MeldList *list, Cards hand, Meld *table, Cards mustMeld) {
    MeldData data;

    data.hand = hand;  // do I need this??? TODO
    data.handExt = Cards_addLowAces(data.hand);
    data.noRun = 0;
    data.noSet = 0;
    data.table = *table;
    data.mustMeld = mustMeld;
    data.list = list;

    MeldList_init(list);
    MeldList_fillRec(&data);
}