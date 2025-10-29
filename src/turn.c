#include <assert.h>
#include <stdio.h>
#include "turn.h"

void Turn_init(Turn *turn) {
    Pile_init(&turn->taken);
    turn->draw = 0;
    turn->discard = 0;
    Meld_init(&turn->meld);
    turn->eval = -9999;
}

int Turn_max(Turn *best, Turn *scratch) {
    if (scratch->eval > best->eval) {
        *best = *scratch;
    }
    return best->eval;
}

void Turn_print(Turn *turn) {
    printf("Taken: ");
    Pile_print(&turn->taken);
    printf("\nDraw: ");
    Cards_print(turn->draw);
    printf("\nDiscard: ");
    Cards_print(turn->discard);
    printf("\n");
    Meld_print(&turn->meld);
    printf("Eval: %d\n", turn->eval);
}
