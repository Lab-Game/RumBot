#ifndef TURN_H
#define TURN_H

#include "cards.h"
#include "pile.h"
#include "table.h"

typedef struct TurnStruct {
    Pile taken;      // cards taken from discard pile
    Cards draw;      // card drawn (0 if none)
    Cards discard;   // card discarded (0 if none)
    Table meld;      // cards melded
    int eval;        // evaluation of resulting position
} Turn;

void Turn_init(Turn *play);
int Turn_max(Turn *best, Turn *scratch);
void Turn_print(Turn *play);

#endif // TURN_H