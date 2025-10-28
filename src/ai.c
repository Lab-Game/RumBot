#include <stdio.h>

#include "ai.h"
#include "game.h"

void AI_init(AI *ai, Game *game, Player *player) {
    ai->game = game;
    ai->player = player;
    ai->bestTurn = &player->turn;
    ai->numPlays = 0;
}

void AI_go(AI *ai) {
    Player *player = ai->player;

    printf("=== AI for Player %d ===\n", player->id);
    Game_print(ai->game);

    Cards draw = Player_draw(player);
    printf("Drew ");
    Cards_print(draw);
    printf("\n");

    printf("---generating plays---\n");
    AI_generatePlays(ai);
    AI_printPlays(ai);

    // Make a play!
    Player_makePlays(player, &ai->plays[0]);
}

void AI_generatePlays(AI *ai) {
    Plays accepted, rejected;
    Plays_init(&accepted);
    Plays_init(&rejected);

    ai->numPlays = 0;
    AI_generatePlaysRec(ai, &accepted, &rejected);
}

void AI_generatePlaysRec(AI *ai, Plays *accepted, Plays *rejected) {
    if (ai->numPlays >= MAX_PLAYS) {
        // We've already found the maximum number of plays we can store.
        return;
    }

    if (Plays_count(accepted) > MAXMIN && Plays_count(rejected) > MAXMIN) {
        // We've both accepted and rejected lots of possible plays.
        // We're heading toward a combinatorial explosion.  Stop exploring.
        return;
    }

    Player *player = ai->player;
    Cards hand = player->hand;
    Cards lowHand = Cards_lowerAces(hand);
    Meld *Meld = &ai->game->Meld;

    // Find all possible runs, sets, and extensions, given the current Meld and hand.
    Plays plays;
    plays.runCenters = hand & (lowHand << 1) & (hand >> 1);
    plays.setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    plays.runExtensions = ((Meld->runs << 1) | (Meld->runs >> 1)) & lowHand;
    plays.setExtensions = ((Meld->sets << 16) | (Meld->sets >> 16)) & lowHand;

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

        // Accept this run
        accepted->runCenters |= c;
        Player_playRun(player, run);
        AI_generatePlaysRec(ai, accepted, rejected);
        Player_undoRun(player, run);
        accepted->runCenters &= ~c;
        
        // Reject this run
        rejected->runCenters |= c;
        AI_generatePlaysRec(ai, accepted, rejected);
        rejected->runCenters &= ~c;
    } else if ((c = Cards_first(plays.setCenters))) {
        Cards set = Plays_setCenterToCards(c);

        // Accept this set
        accepted->setCenters |= c;
        Player_playSet(player, set);
        AI_generatePlaysRec(ai, accepted, rejected);
        Player_undoSet(player, set);
        accepted->setCenters &= ~c;

        // Reject this set
        rejected->setCenters |= c;
        AI_generatePlaysRec(ai, accepted, rejected);
        rejected->setCenters &= ~c;
    } else if ((c = Cards_first(plays.runExtensions))) {
        // Accept this run extension
        accepted->runExtensions |= c;
        Player_playRun(player, c);
        AI_generatePlaysRec(ai, accepted, rejected);
        Player_undoRun(player, c);
        accepted->runExtensions &= ~c;

        // Reject this run extension
        rejected->runExtensions |= c;
        AI_generatePlaysRec(ai, accepted, rejected);
        rejected->runExtensions &= ~c;
    } else if ((c = Cards_first(plays.setExtensions))) {
        // Accept this set extension
        accepted->setExtensions |= c;
        Player_playSet(player, c);
        AI_generatePlaysRec(ai, accepted, rejected);
        Player_undoSet(player, c);
        accepted->setExtensions &= ~c;

        // Reject this set extension
        rejected->setExtensions |= c;
        AI_generatePlaysRec(ai, accepted, rejected);
        rejected->setExtensions &= ~c;
    } else {
        // No more cards can be played.  Store the accepted plays.

        // Maybe I should be storing a Meld containing the cards
        // in the accepted plays.  If nothing else, that's half the size.
        // I could call those "melds" instead of "plays" to be more precise.
        // And where are point values going again?
        //
        // Do I end up with three structures?
        // Meld:  Stores cards in sets and runs.  Certainly needed to track the game
        //         state.  Maybe useful to store combinations of cards that the
        //         player can put down on the Meld during a turn.
        // Plays:  Stores an encoding of possible sets and runs, given a hand and Meld.
        //         This should really only be used for the recursive search, I think.
        // Melds:  Stores the cards in sets and runs put down by a player during a turn.
        //         Identical to a Meld, unfortunately.  I guess I could call both
        //         Melds.  Includes low aces in runs.
        //
        // After I generate all possible melds, I'm going to have to evaluate them.
        // Where am I going to store that evaluation?
        //
        // There is also a Turn, which records everything a player has done during
        // a turn so that I can explore and rewind various possibilities.

        ai->plays[ai->numPlays++] = *accepted;
    }
}

void AI_printPlays(AI *ai) {
    printf("\nGenerated %d possible play sequences:\n", ai->numPlays);
    for (int i = 0; i < ai->numPlays; ++i) {
        printf("Play %d: ", i + 1);
        Plays_print(&ai->plays[i]);
        printf("\n");
    }
}