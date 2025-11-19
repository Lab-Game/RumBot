#ifndef MELDLIST_H
#define MELDLIST_H

#include <stdbool.h>
#include "meld.h"

#define MELDLIST_MAX_SIZE 1000
#define MELDLIST_MAXMIN 2

typedef struct {
    Meld melds[MELDLIST_MAX_SIZE];
    int size;
} MeldList;

void MeldList_generate(MeldList *list, Cards hand, Meld *tableMeld, Cards mustMeld);
void MeldList_genRec(MeldList *list, Meld *table, Cards mustMeld,
                     Cards mayRun, Cards maySet,
                     Cards newRuns, Cards newSets);
void MeldList_print(MeldList *list);

#endif // MELDLIST_H