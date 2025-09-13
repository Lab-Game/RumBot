#include <assert.h>
#include <stdio.h>

#include "play.h"
#include "pile.h"
#include "game.h"
#include "table.h"

void Play_init(Play *play) {
    play->numTaken = 0;
    play->deepest = 0;
    play->draw = 0;
    play->discard = 0;
    Table_init(&play->meld);
    play->eval = -1; // unevaluated
}

int Play_max(Play *best, Play *scratch) {
    if (scratch->eval > best->eval) {
        *best = *scratch;
    }
    return best->eval;
}

void Play_print(Play *play) {\
    printf("Num Taken: %d\n", play->numTaken);
    printf("Deepest: ");
    Cards_print(play->deepest);
    printf("\nDraw: ");
    Cards_print(play->draw);
    printf("\nDiscard: ");
    Cards_print(play->discard);
    Table_print(&play->meld);
    printf("Eval: %d\n", play->eval);
}
