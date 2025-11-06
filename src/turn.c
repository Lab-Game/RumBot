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
    if (Pile_size(&turn->taken) == 0) {
        // This is a draw turn
        printf("Draw: ");
        Cards_print(turn->draw);
    } else {
        // This is a take turn
        printf("Take: ");
        Pile_print(&turn->taken);
    }
    if (Meld_cards(&turn->meld)) {
        printf(" Meld: ");
        Meld_printCompact(&turn->meld);
    }
    printf(" Discard: ");
    Cards_print(turn->discard);
    printf(" Eval: %d\n", turn->eval);
}
