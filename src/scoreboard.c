
#include <stdlib.h>
#include <stdio.h>

#include "scoreboard.h"
#include "meldlist.h"

// I want to generate an outline of all possible Turns from the current game state.
// For now, I'll just print turns, rather than allocating a structure.
// Later, I'll build a structure to hold all possible game states after the
// player's turn.  I'll be able to simulate a lot of games starting at
// any point in this tree of possibilities, stashing the results so I can
// pick the best path.

Scoreboard *Scoreboard_fromGame(Game *game) {
    Scoreboard *scoreboard = malloc(sizeof(Scoreboard));
    scoreboard->takes = Scoreboard_takes(game);
    scoreboard->draws = Scoreboard_draws(game);
    return scoreboard;
}
    
ScoreboardTake *Scoreboard_takes(Game *game) {
    ScoreboardTake *takes = NULL;

    Player *player = game->currentPlayer;
    while (Pile_size(&game->discardPile) > 0) {
        Player_take(player);
        ScoreboardTake *take = malloc(sizeof(ScoreboardTake));
        take->next = takes;
        take->numTaken = Pile_size(&player->turn.taken);
        take->melds = Scoreboard_melds(game);
        takes = take;
    }
    Player_undoTakes(player);

    // Return a linked list of all possible takes.
    return takes;
}

ScoreboardDraw *Scoreboard_draws(Game *game) {
    ScoreboardDraw *draws = NULL;

    // Go through all cards in the draw pile.
    // Swap each card to the top of the draw pile,
    // explore the scenario, and swap back.
    Player *player = game->currentPlayer;
    Pile *drawPile = &game->drawPile;
    for (int i = 0; i < Pile_size(drawPile); ++i) {
        Pile_swapToTop(drawPile, i);
        Player_draw(player);

        ScoreboardDraw *draw = malloc(sizeof(ScoreboardDraw));
        draw->next = draws;
        draw->drawn = player->turn.draw;
        draw->melds = Scoreboard_melds(game);
        draws = draw;

        Player_undoDraw(player);
        Pile_swapToTop(drawPile, i);
    }

    // Now go through all other players' hands.
    // Swap each card that was not previously discarded
    // into the draw pile, explore the scenario,
    // and swap back.
    for (int p = 0; p < game->numPlayers; ++p) {
        Player *other = Game_player(game, p);
        if (other != player) {
            Cards drawable = other->hand & ~game->everDiscarded;
            for (Cards c = Cards_first(drawable); c != 0; c = Cards_next(drawable, c)) {
                // Swap this card into the draw pile and draw it.
                Pile_push(drawPile, c);
                Cards_remove(&other->hand, c);
                Player_draw(player);

                ScoreboardDraw *draw = malloc(sizeof(ScoreboardDraw));
                draw->next = draws;
                draw->drawn = player->turn.draw;
                draw->melds = Scoreboard_melds(game);
                draws = draw;

                // Undo the draw and swap the card back.
                Player_undoDraw(player);
                Cards_add(&other->hand, c);
                Pile_pop(drawPile);
            }
        }
    }

    // Return a linked list of all possible draws.
    return draws;
}

ScoreboardMeld *Scoreboard_melds(Game *game) {
    ScoreboardMeld *melds = NULL;

    // Generate all possible melds from the current hand.
    Player *player = game->currentPlayer;
    Cards mustMeld = player->turn.taken.size > 1 ? Pile_peek(&player->turn.taken) : 0;
    MeldList meldList;
    MeldList_fill(&meldList, player->hand, &game->meld, mustMeld);

    // For each meld, consider all possible discards.
    for (int i = 0; i < meldList.size; ++i) {
        Player_meld(player, &meldList.melds[i]);

        ScoreboardMeld *meld = malloc(sizeof(ScoreboardMeld));
        meld->next = melds;
        meld->meld = player->turn.meld;
        meld->discards = Scoreboard_discards(game);
        melds = meld;

        Player_undoMeld(player, &meldList.melds[i]);
    }

    // Return a linked list of all possible melds.
    return melds;
}

ScoreboardDiscard *Scoreboard_discards(Game *game) {
    ScoreboardDiscard *discards = NULL;

    Player *player = game->currentPlayer;
    for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
        if (c == player->turn.taken.allCards && !Meld_cards(&player->turn.meld)) {
            // We can not discard a just-taken card without a meld.
            // This somewhat reduces the odds of an infinite loop.
            continue;
        }

        Player_discard(player, c);

        ScoreboardDiscard *discard = malloc(sizeof(ScoreboardDiscard));
        discard->next = discards;
        discard->discard = player->turn.discard;
        discard->turn = player->turn;
        discard->result = *game;
        discard->numGames = 0;
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            discard->totalScore[i] = 0;
        }
        discards = discard;

        Player_undoDiscard(player);
    }

    // Return a linked list of all possible discards.
    return discards;
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
    Turn_print(&discard->turn);
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
