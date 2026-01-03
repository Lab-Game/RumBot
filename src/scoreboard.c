
#include <stdlib.h>
#include <stdio.h>

#include "scoreboard.h"
#include "meldlist.h"

Scoreboard *Scoreboard_fromGame(Game *game) {
    Scoreboard *scoreboard = malloc(sizeof(Scoreboard));
    assert(scoreboard != NULL);
    scoreboard->takes = Scoreboard_initTakes(game);
    scoreboard->draws = Scoreboard_initDraws(game);
    return scoreboard;
}
    
ScoreboardTake *Scoreboard_initTakes(Game *game) {
    Player *player = game->currentPlayer;

    // Consider taking each number of cards from the discard pile.
    // Only create an entry in the scoreboard, if the number of
    // cards taken is legal, i.e. the deepest card taken can be played,
    // if multiple cards are taken.
    ScoreboardTake *takes = NULL; // linked list, initially empty
    while (Pile_size(&game->discardPile) > 0) {
        Player_take(player, 1);
        ScoreboardMeld *melds = Scoreboard_initMelds(game);
        if (melds != NULL) {
            ScoreboardTake *take = malloc(sizeof(ScoreboardTake));
            assert(take != NULL);
            take->next = takes;
            take->numTaken = Pile_size(&game->turn.taken);
            take->melds = melds;
            takes = take;
        }
    }
    Player_undoTake(player);

    return takes;
}

ScoreboardDraw *Scoreboard_initDraws(Game *game) {
    Player *player = game->currentPlayer;

    // Make a linked list of all possible draws.  For each cards that could
    // possibly be drawn, we'll swap that to the top of the draw pile,
    // simulate the player drawing the card, and then reverse those steps.
    Cards drawable = Player_couldDraw(player);
    ScoreboardDraw *draws = NULL;
    for (Cards c = Cards_first(drawable); c != 0; c = Cards_next(drawable, c)) {
        Cards swap = Pile_peek(&game->drawPile);
        Game_swap(game, c, swap);
        Player_draw(player);

        ScoreboardDraw *draw = malloc(sizeof(ScoreboardDraw));
        assert(draw != NULL);
        draw->next = draws;
        draw->drawn = game->turn.drawn;
        draw->melds = Scoreboard_initMelds(game);
        draws = draw;

        Player_undoDraw(player);
        Game_swap(game, c, swap);
    }

    return draws;
}

ScoreboardMeld *Scoreboard_initMelds(Game *game) {
    Player *player = game->currentPlayer;

    // Generate possible melds from the current hand.  This list may be
    // truncated for efficiency.
    MeldList meldList;
    Cards mustMeld = game->turn.taken.size > 1 ? Pile_peek(&game->turn.taken) : 0;
    MeldList_generate(&meldList, player->hand, &game->meld, mustMeld);
    ScoreboardMeld *melds = NULL;
    for (int i = 0; i < meldList.size; ++i) {
        Player_meld(player, &meldList.melds[i]);

        ScoreboardMeld *meld = malloc(sizeof(ScoreboardMeld));
        assert(meld != NULL);
        meld->next = melds;
        meld->meld = game->turn.meld;
        meld->discards = Scoreboard_initDiscards(game);
        melds = meld;

        Player_undoMeld(player, &meldList.melds[i]);
    }

    return melds;
}

ScoreboardDiscard *Scoreboard_initDiscards(Game *game) {
    Player *player = game->currentPlayer;

    // Handle the special case of an empty hand.  In this case, no discard is required.
    if (Cards_size(player->hand) == 0) {
        ScoreboardDiscard *discard = malloc(sizeof(ScoreboardDiscard));
        assert(discard != NULL);
        discard->next = NULL;
        discard->discard = 0;
        discard->score = Scoreboard_initScore(game);
        return discard;
    }

    ScoreboardDiscard *discards = NULL;

    // Enforce a special rule to reduce risk of infinite games:  you can not take one
    // card and then discard that same card if you have not melded anything this turn.
    Cards illegalDiscard = 0;
    if (Meld_cards(&player->game->turn.meld) == 0 && player->game->turn.taken.size == 1) {
        illegalDiscard = player->game->turn.taken.allCards;
    }

    for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
        if (c != illegalDiscard) {
            Player_discard(player, c);

            ScoreboardDiscard *discard = malloc(sizeof(ScoreboardDiscard));
            assert(discard != NULL);
            discard->next = discards;
            discard->discard = game->turn.discard;
            discard->score = Scoreboard_initScore(game);
            discards = discard;

            Player_undoDiscard(player);
        }
    }

    return discards;
}

ScoreboardScore *Scoreboard_initScore(Game *game) {
    ScoreboardScore *score = malloc(sizeof(ScoreboardScore));
    assert(score != NULL);

    score->turn = game->turn;
    score->numGames = 0;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        score->totalScore[i] = 0;
    }
    return score;
}

void Scoreboard_print(Scoreboard *scoreboard) {
    printf("Scoreboard:\n");

    ScoreboardTake *take = scoreboard->takes;
    while (take) {
        ScoreboardTake_print(take);
        take = take->next;
    }

    ScoreboardDraw *draw = scoreboard->draws;
    while (draw) {
        ScoreboardDraw_print(draw);
        draw = draw->next;
    }
}

void ScoreboardTake_print(ScoreboardTake *take) {
    printf(" Take %d:\n", take->numTaken);
    ScoreboardMeld *meld = take->melds;
    while (meld) {
        ScoreboardMeld_print(meld);
        meld = meld->next;
    }
}

void ScoreboardDraw_print(ScoreboardDraw *draw) {
    printf(" Draw ");
    Cards_print(draw->drawn);
    printf(":\n");

    ScoreboardMeld *meld = draw->melds;
    while (meld) {
        ScoreboardMeld_print(meld);
        meld = meld->next;
    }
}

void ScoreboardMeld_print(ScoreboardMeld *meld) {
    printf("  Meld ");
    Meld_printCompact(&meld->meld);
    printf(":\n");
    ScoreboardDiscard *discard = meld->discards;
    while (discard) {
        ScoreboardDiscard_print(discard);
        discard = discard->next;
    }
}

void ScoreboardDiscard_print(ScoreboardDiscard *discard) {
    printf("   Discard ");
    Cards_print(discard->discard);
    printf(":  ");
    ScoreboardScore_print(discard->score);
}

void ScoreboardScore_print(ScoreboardScore *score) {
    printf("Games: %d  Scores:", score->numGames);

    // Print the total score for each player followed
    // (in parentheses) by the total score minus the
    // average score of the opponents.
    int sumScores = 0;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        sumScores += score->totalScore[i];
    }
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        printf(" %6d (%6d)", score->totalScore[i], score->totalScore[i] - (sumScores - score->totalScore[i]) / (NUM_PLAYERS - 1));
    }
    printf("\n");
}

void Scoreboard_free(Scoreboard *scoreboard) {
    // Free takes
    ScoreboardTake *take = scoreboard->takes;
    while (take) {
        ScoreboardTake *nextTake = take->next;

        // Free melds
        ScoreboardMeld *meld = take->melds;
        while (meld) {
            ScoreboardMeld *nextMeld = meld->next;

            // Free discards
            ScoreboardDiscard *discard = meld->discards;
            while (discard) {
                ScoreboardDiscard *nextDiscard = discard->next;
                free(discard->score);
                free(discard);
                discard = nextDiscard;
            }

            free(meld);
            meld = nextMeld;
        }

        free(take);
        take = nextTake;
    }

    // Free draws
    ScoreboardDraw *draw = scoreboard->draws;
    while (draw) {
        ScoreboardDraw *nextDraw = draw->next;

        // Free melds
        ScoreboardMeld *meld = draw->melds;
        while (meld) {
            ScoreboardMeld *nextMeld = meld->next;

            // Free discards
            ScoreboardDiscard *discard = meld->discards;
            while (discard) {
                ScoreboardDiscard *nextDiscard = discard->next;
                free(discard->score);
                free(discard);
                discard = nextDiscard;
            }

            free(meld);
            meld = nextMeld;
        }

        free(draw);
        draw = nextDraw;
    }

    // Free scoreboard itself
    free(scoreboard);
}

