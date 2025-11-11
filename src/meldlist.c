#include "meldlist.h"
#include "plays.h"

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

void MeldList_fill(MeldList *list, Cards hand, Meld *tableMeld, Cards mustMeld) {
    Plays accepted, rejected;
    Plays_init(&accepted);
    Plays_init(&rejected);

    MeldList_init(list);
    MeldList_fillRec(list, hand, tableMeld, &accepted, &rejected, mustMeld);
}

void MeldList_fillRec(MeldList *list, Cards hand, Meld *tableMeld, Plays *accepted, Plays *rejected, Cards mustMeld) {
    if (list->size >= MELDLIST_MAX_SIZE) {
        // No space to hold more meld possibilities.
        return;
    }

    if (Plays_count(accepted) > MELDLIST_MAXMIN && Plays_count(rejected) > MELDLIST_MAXMIN) {
        // We've both accepted and rejected lots of possible plays.
        // We're heading toward a combinatorial explosion.  Stop exploring.
        return;
    }
    
    Plays plays;
    Plays_findAll(&plays, tableMeld, hand, accepted, rejected);

    Cards c;
    if ((c = Cards_first(plays.runCenters))) {
        Cards run = Plays_runCenterToCards(c);

        // Accept this run
        accepted->runCenters |= c;
        Meld_addRun(tableMeld, run);
        MeldList_fillRec(list, hand & ~run, tableMeld, accepted, rejected, mustMeld);
        Meld_removeRun(tableMeld, run);
        accepted->runCenters &= ~c;

        // Reject this run
        rejected->runCenters |= c;
        MeldList_fillRec(list, hand, tableMeld, accepted, rejected, mustMeld);
        rejected->runCenters &= ~c;
    } else if ((c = Cards_first(plays.setCenters))) {
        Cards set = Plays_setCenterToCards(c);

        // Accept this set
        accepted->setCenters |= c;
        Meld_addSet(tableMeld, set);
        MeldList_fillRec(list, hand & ~set, tableMeld, accepted, rejected, mustMeld);
        Meld_removeSet(tableMeld, set);
        accepted->setCenters &= ~c;

        // Reject this set
        rejected->setCenters |= c;
        MeldList_fillRec(list, hand, tableMeld, accepted, rejected, mustMeld);
        rejected->setCenters &= ~c;
    } else if ((c = Cards_first(plays.runExtensions))) {
        // Accept this run extension
        accepted->runExtensions |= c;
        Meld_addRun(tableMeld, c);
        MeldList_fillRec(list, hand & ~c, tableMeld, accepted, rejected, mustMeld);
        Meld_removeRun(tableMeld, c);
        accepted->runExtensions &= ~c;

        // Reject this run extension
        rejected->runExtensions |= c;
        MeldList_fillRec(list, hand, tableMeld, accepted, rejected, mustMeld);
        rejected->runExtensions &= ~c;
    } else if ((c = Cards_first(plays.setExtensions))) {
        // Accept this set extension
        accepted->setExtensions |= c;
        Meld_addSet(tableMeld, c);
        MeldList_fillRec(list, hand & ~c, tableMeld, accepted, rejected, mustMeld);
        Meld_removeSet(tableMeld, c);
        accepted->setExtensions &= ~c;
        // Reject this set extension
        rejected->setExtensions |= c;
        MeldList_fillRec(list, hand, tableMeld, accepted, rejected, mustMeld);
        rejected->setExtensions &= ~c;
    } else {
        // No more plays possible.  Store the current meld.
        Meld meld;
        Plays_toMeld(accepted, &meld);
        if (Cards_has(meld.runs | meld.sets, mustMeld)) {
            // The meld includes all required cards.
            MeldList_add(list, &meld);
        }
    }
}