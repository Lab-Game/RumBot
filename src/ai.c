#include <stdlib.h>
#include <stdio.h>

#include "ai.h"
#include "game.h"
#include "ai-shallow.h"
#include "ai-deep.h"
#include "meldlist.h"

void AI_init(AI *ai, int mode) {
    ai->mode = mode;
    ai->simMode = 0;
    ai->totalScore = 0;
    ai->game = NULL;
    ai->player = NULL;
    AI_resetForGo(ai);
}

// Clear fields used in selecting a turn.
void AI_resetForGo(AI *ai) {
    Turn_init(&ai->bestTakeTurn);
    ai->possibleDraws = 0;
    for (int i = 0; i < 64; ++i) {
        Turn_init(&ai->bestDrawTurn[i]);
    }
    ai->averageDrawEval = 0;
    MeldList_init(&ai->meldList);
}

void AI_joinGame(AI *ai, Game *game, Player *player) {
    ai->game = game;
    ai->player = player;
}

void AI_exitGame(AI *ai) {
    ai->game = NULL;
    ai->player = NULL;
}

Turn *AI_go(AI *ai) {
    AI_resetForGo(ai);

    if (ai->mode == 2) {
        // Consider all possible turns and evaluate each by simulating
        // many random completions of the game.
        return AI_goDeep(ai);
    } else {
        // Consider all possible turns and evaluate each by a heuristic.
        return AI_goShallow(ai);
    }
}

void AI_print(AI *ai) {
    printf("AI mode %d, total score %d\n", ai->mode, ai->totalScore);
    printf("  Best take turn: ");
    Turn_print(&ai->bestTakeTurn);
    printf("  Best draw turns:\n");
    for (Cards c = Cards_first(ai->possibleDraws); c != 0; c = Cards_next(ai->possibleDraws, c)) {
        Card card = Cards_toCard(c);
        printf("    ");
        Turn_print(&ai->bestDrawTurn[card]);
    }
    printf("  Average draw eval: %d\n", ai->averageDrawEval);
}
