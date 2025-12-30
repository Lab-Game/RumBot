#include <stdlib.h>
#include <stdio.h>

#include "ai.h"
#include "game.h"
#include "ai-shallow.h"
#include "ai-deep.h"
#include "ai-human.h"
#include "meldlist.h"

void AI_init(AI *ai, int mode, bool deep) {
    // Either there is a human player (mode -1) or an AI player (mode >= 0),
    // in which case deep evaluation may be enabled.
    assert((mode == AI_HUMAN && !deep) || (mode >= 0));

    ai->totalScore = 0;
    ai->mode = mode;
    ai->deep = deep;
    ai->game = NULL;
    ai->player = NULL;

    ai->scoreboard = NULL;
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

void AI_beginTurn(AI *ai) {
    if (ai->mode == AI_HUMAN) {
        HumanAI_beginTurn(ai);
    } else if (ai->deep) {
        DeepAI_beginTurn(ai);
    } else {
        ShallowAI_beginTurn(ai);
    }
}

bool AI_takeTurn(AI *ai, Turn *turn) {
    if (ai->mode == AI_HUMAN) {
        return HumanAI_takeTurn(ai, turn);
    } else if (ai->deep) {
        return DeepAI_takeTurn(ai, turn);
    } else {
        return ShallowAI_takeTurn(ai, turn);
    }
}

void AI_drawTurn(AI *ai, Cards drawCard, Turn *turn) {
    if (ai->mode == AI_HUMAN) {
        HumanAI_drawTurn(ai, drawCard, turn);
    } else if (ai->deep) {
        DeepAI_drawTurn(ai, drawCard, turn);
    } else {
        ShallowAI_drawTurn(ai, drawCard, turn);
    }
}

void AI_endTurn(AI *ai) {
    if (ai->mode == AI_HUMAN) {
        HumanAI_endTurn(ai);
    } else if (ai->deep) {
        DeepAI_endTurn(ai);
    } else {
        ShallowAI_endTurn(ai);
    }
}

void AI_print(AI *ai) {
    printf("AI mode %d, total score %d\n", ai->mode, ai->totalScore);
    printf("  Best take turn: ");
    Turn_print(&ai->bestTakeTurn);
    printf("  Best draw turns:\n");
    for (Cards c = Cards_first(kFullDeck); c != 0; c = Cards_next(kFullDeck, c)) {
        Card card = Cards_toCard(c);
        Turn *turn = &ai->bestDrawTurn[card];
        if (turn->drawn == c) {
            printf("    ");
            Turn_print(turn);
        }
    }
    printf("  Average draw eval: %d\n", ai->averageDrawEval);
}
