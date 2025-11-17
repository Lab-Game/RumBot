#include <stdio.h>

#include "plays.h"

void Plays_findAll(Plays *plays, Meld *meld, Cards hand, Plays *accepted, Plays *rejected) {
    Cards lowHand = Cards_addLowAces(hand);
    plays->runCenters = hand & (lowHand << 1) & (hand >> 1);
    plays->setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    plays->runExtensions = ((meld->runs << 1) | (meld->runs >> 1)) & lowHand;
    plays->setExtensions = ((meld->sets << 16) | (meld->sets >> 16)) & lowHand;

    // Exclude melds that were previously rejected.
    plays->runCenters &= ~rejected->runCenters;
    plays->runExtensions &= ~rejected->runExtensions;
    plays->setCenters &= ~rejected->setCenters;
    plays->setExtensions &= ~rejected->setExtensions;

    // We must avoid tiling the same N-card run with different combinations
    // of 3-card runs and 1-card extensions.  The canonical tiling consists
    // of as many 3-card runs as possible, followed by 0, 1, or 2 single-card
    // extensions.  To enforce this, we apply the following rules:
    //   - Never add a 3-card-run immediately after a 1-card extension
    //   - Never add a 1-card extension immediately before a 3-card-run
    //   - Never add 3 consecutive 1-card extensions
    plays->runExtensions &= ~(accepted->runCenters >> 2);
    plays->runCenters &= ~(accepted->runExtensions << 2);
    plays->runExtensions &= ~((accepted->runExtensions << 1) & (accepted->runExtensions << 2));

    // Similarly, we must avoid tiling a four-of-a-kind in four
    // different ways.  The only permitted tiling is AD AH AS + AC.
    // When looking at a specific card as a possible set extension, we
    // check to see if the "opposite" card of the same value is the center
    // of an accepted set.  If so, then this specific card can only be
    // an extension if it is clubs.
    plays->setExtensions &= ~(((accepted->setCenters >> 32) | (accepted->setCenters << 32)) & 0xFFFFFFFFFFFF0000ULL);
}

Cards Plays_findPlayableCards(Cards hand, Meld *meld) {
    Cards lowHand = Cards_addLowAces(hand);
    Cards runCenters = hand & (lowHand << 1) & (hand >> 1);
    Cards setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    Cards runExtensions = ((meld->runs << 1) | (meld->runs >> 1)) & lowHand;
    Cards extendedMeldsRuns = meld->runs | runExtensions;
    runExtensions = ((extendedMeldsRuns << 1) | (extendedMeldsRuns >> 1)) & lowHand;
    Cards setExtensions = ((meld->sets << 16) | (meld->sets >> 16)) & lowHand;

    // Find all playable cards from these plays
    Cards playable = (Plays_runCenterToCards(runCenters) | runExtensions |
                      Plays_setCenterToCards(setCenters) | setExtensions);

    // Kick out low aces from playable cards if the corresponding high aces are playable.
    return Cards_preferHighAces(playable);
}

void Plays_print(Plays *plays) {
    // Print the contents of the Plays structure compactly.
    for (Cards c = Cards_first(plays->runCenters); c != 0; c = Cards_next(plays->runCenters, c)) {
        Cards meld = Plays_runCenterToCards(c);
        printf("<");
        Cards_print(meld);
        printf("> ");
    }

    for (Cards c = Cards_first(plays->runExtensions); c != 0; c = Cards_next(plays->runExtensions, c)) {
        printf("<");
        Cards_print(c);
        printf("> ");
    }

    for (Cards c = Cards_first(plays->setCenters); c != 0; c = Cards_next(plays->setCenters, c)) {
        Cards meld = Plays_setCenterToCards(c);
        printf("{");
        Cards_print(meld);
        printf("} ");
    }
 
    for (Cards c = Cards_first(plays->setExtensions); c != 0; c = Cards_next(plays->setExtensions, c)) {
        printf("{");
        Cards_print(c);
        printf("} ");
    }
}