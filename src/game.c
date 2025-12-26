#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"

void Game_init(Game *game) {
    game->numPlayers = NUM_PLAYERS;
    game->currentPlayerId = 0;
    game->currentPlayer = &game->players[0];
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = &game->players[i];
        Player_init(player);
        player->game = game;
        player->id = i;
    }
    Pile_fullDeck(&game->drawPile);
    Pile_init(&game->discardPile);
    Meld_init(&game->meld);
    game->everDiscarded = 0;
    Turn_init(&game->turn);
    game->isOver = false;
}

void Game_shuffle(Game *game) {
    Pile_shuffle(&game->drawPile);
}

void Game_deal(Game *game) {
    assert(Pile_size(&game->drawPile) >= NUM_PLAYERS * 7 + 1);

    // Deal 7 cards to each player
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        Player *player = Game_player(game, i);
        for (int j = 0; j < 7; ++j) {
            Cards_add(&player->hand, Pile_pop(&game->drawPile));
        }
    }

    // Move the top card from the draw pile to the discard pile,
    // and mark it as having been discarded.
    Pile_push(&game->discardPile, Pile_pop(&game->drawPile));
    game->everDiscarded = Pile_peek(&game->discardPile);
}

void Game_copy(Game *original, Game *copy) {
    *copy = *original;
    for (int i = 0; i < copy->numPlayers; ++i) {
        copy->players[i].game = copy;
    }
    copy->currentPlayer = &copy->players[copy->currentPlayerId];
}

// Replace the extract card with the insert card.  The insert card should
// NOT be present in the game initially.  The extract card MUST be present.
// Returns the card that was extracted.
static Cards Game_replace(Game *game, Cards insert, Cards extract) {
    // Look for the extracted card in the draw pile
    Pile *drawPile = &game->drawPile;
    if (Pile_has(drawPile, extract)) {
        return Pile_swap(&game->drawPile, insert, extract);
    }

    // Look for the extracted card in the discard pile
    Pile *discardPile = &game->discardPile;
    if (Pile_has(discardPile, extract)) {
        return Pile_swap(&game->discardPile, insert, extract);
    }

    // Look in each player's hand
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        if (Cards_has(player->hand, extract)) {
            return Cards_swap(&player->hand, insert, extract);
        }
    }

    assert(false); // Could not find card to extract

    return 0;
}

void Game_swap(Game *game, Cards card1, Cards card2) {
    if (card1 == card2) {
        // Swapping a card with itself has no effect.
        return;
    }

    // Replalce card1 with a special temporary card, then replace card2 with card1,
    // then replace the temporary card with card2.
    Game_replace(game, kSpecialCard, card1);
    Game_replace(game, card1, card2);
    Game_replace(game, card2, kSpecialCard);
}

Player *Game_player(Game *game, int num) {
    assert(num >= 0 && num < game->numPlayers);
    return &(game->players[num]);
}

bool Game_nextTurn(Game *game) {
    assert(!game->isOver);

    Turn_init(&game->turn);

    Player *player = game->currentPlayer;
    if (player->hand == 0) {
        // Player went out.
        // Reduce the scores of other players by the points in their hand
        game->isOver = true;
        for (int i = 0; i < game->numPlayers; ++i) {
            Player *p = Game_player(game, i);
            int handPoints = Cards_points(p->hand);
            p->score -= handPoints;
        }
    } else if (Pile_size(&game->drawPile) == 0) {
        // Draw pile is empty.  Game is over, but no penalties are applied.
        game->isOver = true;
    } else {
        // Advance to next player's turn
        game->currentPlayerId = (game->currentPlayerId + 1) % game->numPlayers;
        game->currentPlayer = &game->players[game->currentPlayerId];
    }

    return game->isOver;
}

void Game_permute(Game *game) {
    assert(!game->isOver);

    // Permute cards unknown to the current player.
    Player *player = game->currentPlayer;

    // Note the size of each player's hand.
    int handSize[NUM_PLAYERS];
    for (int i = 0; i < game->numPlayers; ++i) {
        handSize[i] = Cards_size(game->players[i].hand);
    }

    for (int i = 0; i < game->numPlayers; ++i) {
        if (i != player->id) {
            // Take all cards out of other players' hands that were never discarded
            // and put them in the draw pile.
            Player *other = &game->players[i];
            Cards withdrawn = other->hand & ~game->everDiscarded;
            Cards_remove(&other->hand, withdrawn);
            for (Cards c = Cards_first(withdrawn); c != 0; c = Cards_next(withdrawn, c)) {
                Pile_push(&game->drawPile, c);
            }
        }
    }

    // Shuffle the draw pile
    Pile_shuffle(&game->drawPile);

    // Refill hands from the draw pile.
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *other = &game->players[i];
        int numToDeal = handSize[i] - Cards_size(other->hand);
        for (int j = 0; j < numToDeal; ++j) {
            Cards card = Pile_pop(&game->drawPile);
            Cards_add(&other->hand, card);
        }
    }
}

void Game_print(Game *game) {
    if (Meld_cards(&game->meld)) {
        printf("Meld: ");
        Meld_printCompact(&game->meld);
        printf("\n");
    }

    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        printf(i == game->currentPlayerId ? "-> " : "   ");
        Player_print(player);
    }

    // Print all cards in the discard pile
    printf("Discard: ");
    Pile_print(&game->discardPile);
    printf("\n");

    // Print all cards in the draw pile
    printf("Draw: ");
    Pile_print(&game->drawPile);
    printf("\n");
}

void Game_printForPlayer(Game *game) {
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        if (i == game->currentPlayerId) {
            printf("-> ");
            Player_print(player);
        } else {
            printf("Player %d (%3d pts)  ", player->id, player->score);
            for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
                printf("?? ");
            }
            printf("\n");

            printf("   Player %d: ", player->id);
            Cards_print(player->hand);
            printf(" (cards hidden)\n");
        }
    }

    // Print all cards in the discard pile
    printf("Discard: ");
    Pile_print(&game->discardPile);
    printf("\n");

    // Print all cards in the draw pile
    printf("Draw: (%d cards)\n", Pile_size(&game->drawPile));
}