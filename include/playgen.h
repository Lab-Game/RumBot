#ifndef PLAYGEN_H
#define PLAYGEN_H

#include <assert.h>

#include "cards.h"
#include "table.h"
#include "plays.h"

#define MAX_PLAYS 100
#define MAXMIN 3

typedef struct PlayListStruct {
    int size;
    Plays plays[MAX_PLAYS];
} PlayList;

extern PlayList playList;

void PlayGen_generate(Cards hand, Table *table);
void PlayGen_print(void);

#endif // PLAYGEN_H