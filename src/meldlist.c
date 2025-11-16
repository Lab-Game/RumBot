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

void MeldList_fill(MeldList *list, Cards hand, Meld *table, Cards mustMeld) {
    printf("Filling MeldList with hand: ");
    Cards_print(hand);
    printf("\nTable meld: ");
    Meld_printCompact(table);
    printf("\nMust meld: ");
    Cards_print(mustMeld);
    printf("\n");
    MeldList_init(list);
    MeldList_fillRec(list, hand, 0, table, mustMeld);
}


// I need to move unplayed cards from the hand to a separate Cards set, which
// I'll try again to play as a set.
void MeldList_fillRec(MeldList *list, Cards hand, Cards kept, Meld *tableMeld, Cards mustMeld) {
    // Ideally, I'd handle the mustMeld constraint right away, but that's pretty complicated.  :-/
    // I might be able to handle the common case (only one way to meld the mustMeld cards) efficiently.
    // So, for now, I'll handle that at the leaf of the recursion.

    if (hand == 0) {
        // No more cards to play.  Check if mustMeld is satisfied.
        // Add this meld to the list.
        Meld_printCompact(tableMeld);
        printf("   [");
        Cards_print(kept);
        printf("] ");
        printf(" %d ", Cards_size(kept | tableMeld->runs | tableMeld->sets));
        if (mustMeld & ~(tableMeld->runs | tableMeld->sets)) {
            printf("  Cannot satisfy mustMeld!\n");
            return; // can't satisfy mustMeld
        }
        printf("  Adding meld to list.\n");
        MeldList_add(list, tableMeld);
        return;
    }

    if (mustMeld & ~(hand | tableMeld->runs | tableMeld->sets)) {
        return; // can't satisfy mustMeld
    }

    // Get the first card in the hand.
    Cards card = Cards_first(hand);
    Cards prev = card >> 1;
    Cards next = card << 1;
    Cards nextNext = card << 2;

    // Option 1:  Do not play this card.
    MeldList_fillRec(list, hand & ~card, kept | card, tableMeld, mustMeld);

    // Option 2:  Play exactly one card.  Possible if the prev or next card is on the table.
    // Put this card on the table, remove the next card from the hand, and recurse.
    if ((next | prev) & tableMeld->runs) {
        tableMeld->runs ^= card;
        MeldList_fillRec(list, hand & ~card & ~next, kept | (hand & next), tableMeld, mustMeld);
        tableMeld->runs ^= card;
    }

    // Option 3:  Play exactly two cards.  Possible if the next card is in
    // the hand and the prev or next-next card is on the table.  Put this card
    // and the next on the table, remove card, next, and next-next from the hand, and recurse.
    if ((next & hand) && ((prev | nextNext) & tableMeld->runs)) {
        Cards played = card | next;
        tableMeld->runs ^= played;
        MeldList_fillRec(list, hand & ~played & ~nextNext, kept | (hand & nextNext), tableMeld, mustMeld);
        tableMeld->runs ^= played;
    }

    // Option 4:  Play three or more cards.  Possible if the next two cards are in the hand.
    // Remove all three from the hand, put them on the table, and recurse.
    if ((next & hand) && (nextNext & hand)) {
        Cards played = card | next | nextNext;
        tableMeld->runs ^= played;
        MeldList_fillRec(list, hand & ~played, kept, tableMeld, mustMeld);
        tableMeld->runs ^= played;
    }
}