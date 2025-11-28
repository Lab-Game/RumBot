#include <stdlib.h>
#include <stdio.h>

#include "ai.h"
#include "game.h"
#include "ai-shallow.h"
#include "ai-deep.h"
#include "meldlist.h"

void AI_init(AI *ai, int mode, int simMode) {
    ai->mode = mode;
    ai->simMode = simMode;
    ai->totalScore = 0;
    ai->game = NULL;
    ai->player = NULL;
    AI_resetForGo(ai);
}

// Clear fields used in selecting a turn.
void AI_resetForGo(AI *ai) {
    Turn_init(&ai->bestTakeTurn);
    for (int i = 0; i < 64; ++i) {
        Turn_init(&ai->bestDrawTurn[i]);
    }
    ai->averageDrawEval = 0;
    MeldList_init(&ai->meldList);
}

void AI_joinGame(AI *ai, Game *game, Player *player) {
    assert(ai->game == NULL);
    assert(ai->player == NULL);

    ai->game = game;
    ai->player = player;
}

void AI_exitGame(AI *ai) {
    assert(ai->game != NULL);
    assert(ai->player != NULL);

    ai->game = NULL;
    ai->player = NULL;
}

void AI_go(AI *ai, Turn *turn) {
    AI_resetForGo(ai);

    if (ai->mode == -1) {
        // Human player: just return an empty turn for now.
        AI_goHuman(ai, turn);
    } else if (ai->mode >= 10) {
        // Consider all possible turns and evaluate each by simulating
        // many random completions of the game.
        AI_goDeep(ai);
        AI_print(ai);
    } else {
        // Consider all possible turns and evaluate each by a heuristic.
        AI_goShallow(ai);
    }

    if (ai->bestTakeTurn.eval > ai->averageDrawEval) {
        *turn = ai->bestTakeTurn;
    } else {
        // To simplify the code, we'll let the AI peek at the draw card
        // so we can return the best Turn for that draw as opposed to
        // the best Turn for every possible draw, which makes for more
        // complex code.  This is a problem if the drawn card is supplied
        // externally, but I'll deal with that later.  In principle,
        // I just need to return take vs. draw now and then can return
        // the rest of the Turn after the draw card is known.
        Card drawCard = Cards_toCard(Pile_peek(&ai->game->drawPile));
        *turn = ai->bestDrawTurn[drawCard];
    }
}

void AI_print(AI *ai) {
    printf("AI mode %d, total score %d\n", ai->mode, ai->totalScore);
    printf("  Best take turn: ");
    Turn_print(&ai->bestTakeTurn);
    printf("  Best draw turns:\n");
    for (Cards c = Cards_first(FULL_DECK); c != 0; c = Cards_next(FULL_DECK, c)) {
        Card card = Cards_toCard(c);
        Turn *turn = &ai->bestDrawTurn[card];
        if (turn->drawn == c) {
            printf("    ");
            Turn_print(turn);
        }
    }
    printf("  Average draw eval: %d\n", ai->averageDrawEval);
}
