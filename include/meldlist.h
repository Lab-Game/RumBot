#ifndef MELDLIST_H
#define MELDLIST_H

#include <stdbool.h>
#include "meld.h"
#include "plays.h"

#define MELDLIST_MAX_SIZE 1000
#define MELDLIST_MAXMIN 2

typedef struct {
    Meld melds[MELDLIST_MAX_SIZE];
    int size;
} MeldList;

void MeldList_init(MeldList *list);
bool MeldList_add(MeldList *list, Meld *meld);
bool MeldList_isFull(MeldList *list);
void MeldList_fill(MeldList *list, Cards hand, Meld *tableMeld, Cards mustMeld);

#endif // MELDLIST_H