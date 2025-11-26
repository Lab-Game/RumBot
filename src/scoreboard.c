
#include <stdlib.h>
#include <stdio.h>

#include "scoreboard.h"
#include "meldlist.h"

Scoreboard *Scoreboard_fromGame(Game *game) {
    Scoreboard *scoreboard = malloc(sizeof(Scoreboard));
    scoreboard->takes = Scoreboard_initTakes(game);
    scoreboard->draws = Scoreboard_initDraws(game);
    return scoreboard;
}
    
ScoreboardTake *Scoreboard_initTakes(Game *game) {
    ScoreboardTake *takes = NULL;

    Player *player = game->currentPlayer;
    while (Pile_size(&game->discardPile) > 0) {
        Player_take(player, 1);

        ScoreboardTake *take = malloc(sizeof(ScoreboardTake));
        take->next = takes;
        take->numTaken = Pile_size(&game->turn.taken);
        take->melds = Scoreboard_initMelds(game);
        takes = take;
    }
    Player_undoTake(player);

    return takes;
}

ScoreboardDraw *Scoreboard_initDraws(Game *game) {
    ScoreboardDraw *draws = NULL;

    Player *player = game->currentPlayer;
    Cards drawable = Player_couldDraw(player);
    for (Cards c = Cards_first(drawable); c != 0; c = Cards_next(drawable, c)) {
        Cards swapped = Game_swapToTop(game, c);
        Player_draw(player);

        ScoreboardDraw *draw = malloc(sizeof(ScoreboardDraw));
        draw->next = draws;
        draw->drawn = game->turn.drawn;
        draw->melds = Scoreboard_initMelds(game);
        draws = draw;

        Player_undoDraw(player);
        Game_swapToTop(game, swapped);
    }

    return draws;
}

ScoreboardMeld *Scoreboard_initMelds(Game *game) {
    ScoreboardMeld *melds = NULL;

    // Generate possible melds from the current hand.
    Player *player = game->currentPlayer;
    MeldList meldList;
    Cards mustMeld = game->turn.taken.size > 1 ? Pile_peek(&game->turn.taken) : 0;
    MeldList_generate(&meldList, player->hand, &game->meld, mustMeld);

    for (int i = 0; i < meldList.size; ++i) {
        Player_meld(player, &meldList.melds[i]);

        ScoreboardMeld *meld = malloc(sizeof(ScoreboardMeld));
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

    // Handle the special case of an empty hand.
    if (Cards_size(player->hand) == 0) {
        ScoreboardDiscard *discard = malloc(sizeof(ScoreboardDiscard));
        discard->next = NULL;
        discard->discard = 0;
        discard->turn = game->turn;
        Scoreboard_initScore(&discard->score);
        return discard;
    }

    ScoreboardDiscard *discards = NULL;

    // You may not discard the only taken card if you have not melded anything.
    Cards illegalDiscard = 0;
    if (Meld_cards(&player->game->turn.meld) == 0 && player->game->turn.taken.size == 1) {
        illegalDiscard = player->game->turn.taken.allCards;
    }

    for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
        if (c != illegalDiscard) {
            Player_discard(player, c);

            ScoreboardDiscard *discard = malloc(sizeof(ScoreboardDiscard));
            discard->next = discards;
            discard->discard = game->turn.discard;
            discard->turn = game->turn;
            Scoreboard_initScore(&discard->score);
            discards = discard;

            Player_undoDiscard(player);
        }
    }

    // Return a linked list of all possible discards.
    return discards;
}

void Scoreboard_initScore(ScoreboardScore *score) {
    score->numGames = 0;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        score->totalScore[i] = 0;
    }
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
    ScoreboardScore_print(&discard->score);
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

