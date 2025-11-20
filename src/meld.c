#include <stdio.h>

#include "meld.h"

bool Meld_isValidPlay(Meld *meld) {
    // The union of runs and sets must be disjoint.
    if ((meld->runs & meld->sets) != 0) {
        return false;
    }

    // An ace must appear high or low, but not both.
    Cards allCards = (meld->runs | meld->sets);
    Cards lowAces = allCards & 0x0001000100010001ULL;
    Cards highAces = allCards & 0x2000200020002000ULL;
    if (((lowAces << 13) & highAces) != 0) {
        return false;
    }

    // A set must contain 0, 1, 3, or 4 cards of the same value.
    for (int value = 0; value <= 13; ++value) {
        Cards sameValue = Cards_sameValue(Cards_fromCard(value));
        int count = Cards_size(meld->sets & sameValue);
        if (!(count == 0 || count == 1 || count == 3 || count == 4)) {
            return false;
        }
    }

    return true;
}

bool Meld_isValidTable(Meld *meld) {
    // The union of runs and sets must be disjoint.
    if ((meld->runs & meld->sets) != 0) {
        return false;
    }

    // An ace must appear high or low, but not both.
    Cards allCards = (meld->runs | meld->sets);
    Cards lowAces = allCards & 0x0001000100010001ULL;
    Cards highAces = allCards & 0x2000200020002000ULL;
    if (((lowAces << 13) & highAces) != 0) {
        return false;
    }

    // Every run card must be adjacent to a card that is itself
    // adjacent to two cards.
    Cards runCenters = meld->runs & (meld->runs << 1) & (meld->runs >> 1);
    if (!(meld->runs == (runCenters | (runCenters << 1) | (runCenters >> 1)))) {
        return false;
    }

    // Every set card card must be adjacent in suit to a card of the same value
    // that is itself adjacent in suit to two cards of the same value.
    Cards setCenters = (meld->sets & ((meld->sets << 16) | (meld->sets >> 48)) &
                        ((meld->sets >> 16) | (meld->sets << 48)));
    if (!(meld->sets == (setCenters | (setCenters << 16) | (setCenters >> 16) |
                         (setCenters << 48) | (setCenters >> 48)))) {
        return false;
    }

    return true;
}

Cards Meld_playableInRun(Cards hand, Cards tableRuns) {
    Cards lowHand = Cards_addLowAces(hand);
    Cards p = tableRuns | (hand & (lowHand << 1) & (hand >> 1));
    p = (p | (p << 1) | (p >> 1)) & lowHand;
    return (p | (p << 1) | (p >> 1)) & lowHand;
}

Cards Meld_playableInSet(Cards hand, Cards tableSets) {
    Cards p = tableSets | (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    return (p | (p << 16) | (p >> 48) | (p >> 16) | (p << 48)) & hand;
}

void Meld_print(Meld *meld) {
    printf("+---------------------------------------------------\n");
    // Print all the runs in the Meld.  Separate cards in the same run by spaces,
    // and separate different runs by commas.
    printf("|  Runs: ");
    Cards prevPrinted = 0;
    for (Cards c = Cards_first(meld->runs); c != 0; c = Cards_next(meld->runs, c)) {
        if (prevPrinted != 0) {
            if ((prevPrinted << 1) == c) {
                // Same run, print a space
                printf(" ");
            } else {
                // Different run, print a comma
                printf(", ");
            }
        }
        prevPrinted = c;
        Cards_print(c);
    }
    printf("\n");

    // Similarly, print all the sets in the Meld.
    printf("|  Sets: ");
    prevPrinted = 0;
    for (int value = 0; value <= 13; ++value) {
        for (int suit = 0; suit < 4; ++suit) {
            Cards c = Cards_fromCard(Card_fromSuitValue(suit, value));
            if (Cards_has(meld->sets, c)) {
                if (prevPrinted != 0) {                        
                    if ((Cards_toCard(prevPrinted) & 0xF) == value) {
                        // Same set, print a space
                        printf(" ");
                    } else {
                        // Different set, print a comma
                        printf(", ");
                    }
                }
                prevPrinted = c;
                Cards_print(c);
            }
        }
    }
    printf("\n");

    printf("+---------------------------------------------------\n");
}

void Meld_printCompact(Meld *meld) {
    Card prev = ILLEGAL_CARD;
    for (Cards c = Cards_first(meld->runs); c != 0; c = Cards_next(meld->runs, c)) {
        Card card = Cards_toCard(c);

        if (prev == ILLEGAL_CARD) {
            printf("<");
        } else if (Card_value(prev) + 1 == Card_value(card) &&
                   Card_suit(prev) == Card_suit(card)) {
            printf(" ");
        } else {
            printf("> <");
        }
        prev = card;

        Card_print(card);
    }
    if (prev != ILLEGAL_CARD) {
        printf("> ");
    }

    for (int value = 0; value <= 13; ++value) {
        Card prev = ILLEGAL_CARD;
        for (int suit = 0; suit < 4; ++suit) {
            Cards c = Cards_fromCard(Card_fromSuitValue(suit, value));
            if (Cards_has(meld->sets, c)) {
                Card card = Cards_toCard(c);

                if (prev == ILLEGAL_CARD) {
                    printf("{");
                    prev = c;
                } else if (Card_value(prev) != Card_value(card)) {
                    printf("} {");
                } else {
                    printf(" ");
                }
                prev = card;

                Card_print(card);
            }
        }
        if (prev != ILLEGAL_CARD) {
            printf("} ");
        }
    }
}
