#ifndef PLAY_H
#define PLAY_H

#include "cards.h"
#include "pile.h"
#include "table.h"

typedef struct PlayStruct {
    int numTaken;    // number of cards taken from discard pile
    Cards deepest;   // deepest card taken from discard pile (0 if none)
    Cards draw;      // card drawn (0 if none)
    Cards discard;   // card discarded (0 if none)
    Table meld;      // cards melded
    int eval;        // evaluation of resulting position
} Play;

void Play_init(Play *play);
int Play_max(Play *best, Play *scratch);
void Play_print(Play *play);

#endif // PLAY_H