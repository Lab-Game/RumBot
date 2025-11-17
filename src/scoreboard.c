#include <stdio.h>

#include "scoreboard.h"
#include "meldlist.h"

// I want to generate an outline of all possible Turns from the current game state.
// For now, I'll just print turns, rather than allocating a structure.
// Later, I'll build a structure to hold all possible game states after the
// player's turn.  I'll be able to simulate a lot of games starting at
// any point in this tree of possibilities, stashing the results so I can
// pick the best path.

void Scoreboard_fromGame(Game *game) {
    Scoreboard_take(game);
    Scoreboard_draw(game);
}
    
void Scoreboard_take(Game *game) {
    Player *player = game->currentPlayer;
    while (Pile_size(&game->discardPile) > 0) {
        Player_take(player);
        Scoreboard_meld(game);
    }
    Player_undoTakes(player);
}

void Scoreboard_draw(Game *game) {
    Player *player = game->currentPlayer;

    // Go through all cards in the draw pile.
    // Swap each card to the top of the draw pile,
    // explore the scenario, and swap back.
    Pile *drawPile = &game->drawPile;
    for (int i = 0; i < Pile_size(drawPile); ++i) {
        Pile_swapToTop(drawPile, i);
        Player_draw(player);
        Scoreboard_meld(game);
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
                // Swap this card into the draw pile.
                Pile_push(drawPile, c);
                Cards_remove(&other->hand, c);

                Player_draw(player);
                Scoreboard_meld(game);
                Player_undoDraw(player);

                // Swap the card back.
                Cards_add(&other->hand, c);
                Pile_pop(drawPile);
            }
        }
    }
}

void Scoreboard_meld(Game *game) {
    Player *player = game->currentPlayer;

    // Generate all possible melds from the current hand.
    MeldList meldList;
    Cards mustMeld = player->turn.taken.size > 1 ? Pile_peek(&player->turn.taken) : 0;

    MeldList_fill(&meldList, player->hand, &game->meld, mustMeld);

    for (int i = 0; i < meldList.size; ++i) {
        Player_meld(player, &meldList.melds[i]);
        Scoreboard_discard(game);
        Player_undoMeld(player, &meldList.melds[i]);
    }
}

void Scoreboard_discard(Game *game) {
    Player *player = game->currentPlayer;

    for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
        if (c == player->turn.taken.allCards && !Meld_cards(&player->turn.meld)) {
            // We can not discard a just-taken card without a meld.
            // This somewhat reduces the odds of an infinite loop.
            continue;
        }

        Player_discard(player, c);

        // Here is a complete turn:  the player has taken or drawn,
        // melded, and discarded.
        Turn_print(&player->turn);

        Player_undoDiscard(player);
    }
}
