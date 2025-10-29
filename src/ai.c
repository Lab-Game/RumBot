#include <stdio.h>

#include "ai.h"
#include "game.h"

void AI_init(AI *ai, Game *game, Player *player) {
    ai->game = game;
    ai->player = player;
}

void AI_go(AI *ai) {
    Game_print(ai->game);

    Player *player = ai->player;

    Cards draw = Player_draw(player);
    printf("draw: ");
    Cards_print(draw);

    AI_generateMelds(ai);

    Turn_init(&ai->bestTurn);
    for (int i = 0; i < ai->numMelds; ++i) {
        Player_meld(player, &ai->melds[i]);
        // If there are any cards left, consider discarding one.
        for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
            Player_discard(player, c);
            player->turn.eval = AI_evaluate(ai);
            Turn_max(&ai->bestTurn, &player->turn);
            Player_undoDiscard(player);
        }
        Player_undoMeld(player, &ai->melds[i]);
    }
    printf(" meld: ");
    Meld_printCompact(&ai->bestTurn.meld);
    Player_meld(player, &ai->bestTurn.meld);

    Player_discard(player, ai->bestTurn.discard);
    printf("discard: ");
    Cards_print(ai->bestTurn.discard);
    printf("\n");
}

int AI_evaluate(AI *ai) {
    return ai->player->score;
}

void AI_generateMelds(AI *ai) {
    Plays accepted, rejected;
    Plays_init(&accepted);
    Plays_init(&rejected);

    ai->numMelds = 0;
    AI_generateMeldsRec(ai, &accepted, &rejected);
}

void AI_generateMeldsRec(AI *ai, Plays *accepted, Plays *rejected) {
    if (ai->numMelds >= MAX_MELDS) {
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
    Meld *meld = &ai->game->meld;

    // Find all possible runs, sets, and extensions, given the current Meld and hand.
    Plays plays;
    Plays_findAll(&plays, meld, hand, accepted, rejected);

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
        AI_generateMeldsRec(ai, accepted, rejected);
        Player_undoRun(player, run);
        accepted->runCenters &= ~c;
        
        // Reject this run
        rejected->runCenters |= c;
        AI_generateMeldsRec(ai, accepted, rejected);
        rejected->runCenters &= ~c;
    } else if ((c = Cards_first(plays.setCenters))) {
        Cards set = Plays_setCenterToCards(c);

        // Accept this set
        accepted->setCenters |= c;
        Player_playSet(player, set);
        AI_generateMeldsRec(ai, accepted, rejected);
        Player_undoSet(player, set);
        accepted->setCenters &= ~c;

        // Reject this set
        rejected->setCenters |= c;
        AI_generateMeldsRec(ai, accepted, rejected);
        rejected->setCenters &= ~c;
    } else if ((c = Cards_first(plays.runExtensions))) {
        // Accept this run extension
        accepted->runExtensions |= c;
        Player_playRun(player, c);
        AI_generateMeldsRec(ai, accepted, rejected);
        Player_undoRun(player, c);
        accepted->runExtensions &= ~c;

        // Reject this run extension
        rejected->runExtensions |= c;
        AI_generateMeldsRec(ai, accepted, rejected);
        rejected->runExtensions &= ~c;
    } else if ((c = Cards_first(plays.setExtensions))) {
        // Accept this set extension
        accepted->setExtensions |= c;
        Player_playSet(player, c);
        AI_generateMeldsRec(ai, accepted, rejected);
        Player_undoSet(player, c);
        accepted->setExtensions &= ~c;

        // Reject this set extension
        rejected->setExtensions |= c;
        AI_generateMeldsRec(ai, accepted, rejected);
        rejected->setExtensions &= ~c;
    } else {
        // No more possible plays to consider.  Store the accepted plays.
        Plays_toMeld(accepted, &ai->melds[ai->numMelds++]);
    }
}

void AI_printMelds(AI *ai) {
    printf("\nGenerated %d possible melds:\n", ai->numMelds);
    for (int i = 0; i < ai->numMelds; ++i) {
        printf("%d: ", i + 1);
        Meld_printCompact(&ai->melds[i]);
        printf("\n");
    }
}