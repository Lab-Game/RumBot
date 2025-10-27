// We'll walk thorugh each card in the hand.
// We'll sort each card into one of four piles:
//   - Unplayable, given the state of the table and cards remaining in the hand.
//   - Playable, but we prefer not to play it.
//   - Playable as part of a run.
//   - Playable as part of a set.
// When classifying a card, we may also classify some subsequent cards, e.g. cards
// later in the same run or set.

#include <stdio.h>
#include <stdbool.h>

#include "cards.h"
#include "table.h"

// I can pass down:
//   Cards in the hand
//   Cards in rows on the table
//   Cards in sets on the table
//   Played cards (mostly just the number)
//   Kept cards (so I don't lose track of the hand)
//   Unplayable cards

void generateRec(Cards hand, Cards runs, Cards sets, Cards played, Cards kept, Cards unplayable) {
    if (Cards_isEmpty(hand)) {
        // No more cards to classify.  Print the results.
        printf("Played: ");
        Cards_print(played);
        printf("  Kept: ");
        Cards_print(kept);
        printf("  Unplayable: ");
        Cards_print(unplayable);
        printf("  Runs on table: ");
        Cards_print(runs);
        printf("  Sets on table: ");
        Cards_print(sets);
        printf("\n");
        return;
    }

    if (Cards_size(kept) > 3 && Cards_size(played) > 3) {
        // We're keeping a bunch of cards and playing a bunch of cards,
        // which risks combinatorial exposion.  So just stop here.
        return;
    }

    // Get the first card in the hand.
    Cards c = Cards_first(hand);

    printf("Condsidering card ");
    Cards_print(c);
    printf("\n");

    // If we do not play the card, we'll need to classify it as unplayable or kept.
    bool playable = false;

    // Count how many cards with the same value have been been played in sets.
    Cards cSet = c;
    cSet |= (c << 32) | (c >> 32);
    cSet |= (cSet << 16) | (cSet >> 16);

    if (Cards_size(cSet & sets) == 3) {
        // There is already a set of three cards with this value on the table.
        // Consider playing this card as the fourth.
        playable = true;
        generateRec(hand & ~c, runs, sets | c, played | c, kept, unplayable);
    } else {
        Cards cSetInHand = hand & cSet;
        Cards cSetInHandSize = Cards_size(cSetInHand);
        if (cSetInHandSize >= 3) {
            // There are three or four cards with this value in hand.
            // Consider playing all of them as a set.
            playable = true;
            generateRec(hand & ~cSetInHand, runs, sets | cSetInHand, played | cSetInHand, kept, unplayable);
        }
        
        if (cSetInHandSize == 4) {
            // We have all four cards with this value in hand.
            // Consider pairing this card with two others as a set,
            // moving the last card to the kept pile.
            playable = true;
            Cards others = cSetInHand & ~c;
            for (Cards extra = Cards_first(others); extra != 0; extra = Cards_next(others, extra)) {
                Cards set = cSet & ~extra;
                generateRec(hand & ~cSet, runs, sets | set, played | set, kept | extra, unplayable);
            }
        }
    }

    // Low aces are another special case.
    // These can extend to the next card or the next, next card to form a run.
    // Or these can form a new run with the next two cards.
    // So that's a lot of new cases to consider.  :-(

    // Determine whether this card can be the start of a run.
    if (runs & (c >> 1)) {
        // There is a run on the table that this card can extend.
        playable = true;
        generateRec(hand & ~c, runs | c, sets, played | c, kept, unplayable);
    }

    if ((hand & (c << 1)) && (runs & (c << 2))) {
        // This card and the next one in the same suit are in hand,
        // and the next-next card is in a run on the table.
        // We can play both cards from our hand to extend the run.
        Cards run = c | (c << 1);
        playable = true;
        generateRec(hand & ~run, (runs | run), sets, played | run, kept, unplayable);
    }

    if (Cards_has(hand, (c << 1) | (c << 2))) {
        // This card and the next two cards in the same suit are in hand.
        // We can play all three cards as a new run.
        Cards run = c | (c << 1) | (c << 2);
        playable = true;
        generateRec(hand & ~run, (runs | run), sets, played | run, kept, unplayable);
    }

    // The card has not been played.  Classify it as kept or unplayable.
    if (playable) {
        // The card is playable, but we may choose to keep it in our hand.
        generateRec(hand & ~c, runs, sets, played, kept | c, unplayable);
    } else {
        // The card is not playable, so we must classify it as unplayable.
        generateRec(hand & ~c, runs, sets, played, kept, unplayable | c);
    }
}

void generate(Table *table, Cards hand) {
    // Print table and hand
    Table_print(table);
    printf("Hand: ");
    Cards_print(hand);
    printf("\n\n");
    generateRec(hand, table->runs, table->sets, 0, 0, 0);
}