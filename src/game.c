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
        Player_init(player, i);
        player->game = game;
    }
    Pile_fullDeck(&game->drawPile);
    Pile_init(&game->discardPile);
    Meld_init(&game->meld);
    game->everDiscarded = 0;
    game->isOver = false;
}

void Game_deal(Game *game) {
    // Shuffle the deck
    Pile_shuffle(&game->drawPile);

    // Deal 7 cards to each player
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        Player *player = Game_player(game, i);
        for (int j = 0; j < 7; ++j) {
            Cards_add(&player->hand, Pile_pop(&game->drawPile));
        }
    }

    // Move the top card from the draw pile to the discard pile.
    Pile_push(&game->discardPile, Pile_pop(&game->drawPile));
}

void Game_copy(Game *original, Game *copy) {
    *copy = *original;
    // Fix up player->game pointers
    for (int i = 0; i < copy->numPlayers; ++i) {
        copy->players[i].game = copy;
    }
    copy->currentPlayer = &copy->players[copy->currentPlayerId];
}

Cards Game_swap(Game *game, Cards insert, Cards extract) {
    // Swap the specified cards in the game.
    // The removed card could be in the draw pile, a player's hand, or the discard pile.

    // Check the draw pile
    Pile *drawPile = &game->drawPile;
    if (Cards_has(drawPile->allCards, extract)) {
        return Pile_swap(&game->drawPile, insert, extract);
    }
    
    // Check each player's hand
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        if (Cards_has(player->hand, extract)) {
            return Cards_swap(&player->hand, insert, extract);
        }
    }

    // Check the discard pile
    Pile *discardPile = &game->discardPile;
    if (Cards_has(discardPile->allCards, extract)) {
        return Pile_swap(&game->discardPile, insert, extract);
    }

    assert(false); // remove card not found
}

Cards Game_swapToTop(Game *game, Cards card) {
    // Swap the top card in the draw pile with the specified card.
    // Return the card that was previously on top of the draw pile.
    // First we need to find the card, which could be in the
    // draw pile, a player's hand, or the discard pile.
    Pile *drawPile = &game->drawPile;
    Cards prevTop = Pile_peek(drawPile);

    // Check the draw pile
    if (Cards_has(drawPile->allCards, card)) {
        for (int i = 0; i < drawPile->size; ++i) {
            if (drawPile->cards[i] == card) {
                Pile_swapToTop(drawPile, i);
                return prevTop;
            }
        }
    }

    // Check each player's hand
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        if (Cards_has(player->hand, card)) {
            // The card is in this player's hand
            Cards_add(&player->hand, Pile_pop(drawPile));
            Cards_remove(&player->hand, card);
            Pile_push(drawPile, card);
            return prevTop;
        }
    }

    // Check the discard pile
    Pile *discardPile = &game->discardPile;
    if (Cards_has(discardPile->allCards, card)) {
        for (int i = 0; i < discardPile->size; ++i) {
            if (discardPile->cards[i] == card) {
                // The card is in the discard pile
                Pile_swapToTop(discardPile, i);
                Cards drawTop = Pile_pop(drawPile);
                Cards discardTop = Pile_pop(discardPile);
                Pile_push(drawPile, discardTop);
                Pile_push(discardPile, drawTop);
                Pile_swapToTop(discardPile, i);
                return prevTop;
            }
        }
    }

    assert(false); // card not found
    return 0;
}

void Game_permute(Game *game, Game *permuted) {
    // Make a copy of the current game, but with all cards
    // unknown to the current player permuted.
    Game_copy(game, permuted);
    Player *player = permuted->currentPlayer;

    int handSize[NUM_PLAYERS];
    for (int i = 0; i < permuted->numPlayers; ++i) {
        if (i != player->id) {
            // Take all cards out of other players' hands that were never discarded.
            Player *other = &permuted->players[i];
            handSize[i] = Cards_size(other->hand);
            Cards withdrawn = other->hand & ~game->everDiscarded;
            Cards_remove(&other->hand, withdrawn);

            // Put them into the draw pile
            for (Cards c = Cards_first(withdrawn); c != 0; c = Cards_next(withdrawn, c)) {
                Pile_push(&permuted->drawPile, c);
            }
        }
    }

    // Shuffle the draw pile
    Pile_shuffle(&permuted->drawPile);

    // Deal back to other players
    for (int i = 0; i < permuted->numPlayers; ++i) {
        if (i != player->id) {
            Player *other = &permuted->players[i];
            int numToDeal = handSize[i] - Cards_size(other->hand);
            for (int j = 0; j < numToDeal; ++j) {
                Cards card = Pile_pop(&permuted->drawPile);
                Cards_add(&other->hand, card);
            }
        }
    }
}

Player *Game_player(Game *game, int num) {
    assert(num >= 0 && num < game->numPlayers);
    return &(game->players[num]);
}

void Game_nextTurn(Game *game) {
    Player *player = game->currentPlayer;

    if (!player->hand || Pile_size(&game->drawPile) == 0) {
        game->isOver = true;

        // Reduce the score of each player by the points in their hand
        for (int i = 0; i < game->numPlayers; ++i) {
            Player *p = Game_player(game, i);
            int handPoints = Cards_points(p->hand);
            p->score -= handPoints;
        }
    } else {
        // Advance to next player's turn
        game->currentPlayerId = (game->currentPlayerId + 1) % game->numPlayers;
        game->currentPlayer = &game->players[game->currentPlayerId];
        Turn_init(&game->currentPlayer->turn);
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
        if (i == game->currentPlayerId) {
            printf("-> ");
        } else {
            printf("   ");
        }
        Player_print(player);
    }

    // Print all cards in the draw pile
    printf("Draw: ");
    Pile_print(&game->drawPile);
    printf("\n");

    // Print all cards in the discard pile
    printf("Discard: ");
    Pile_print(&game->discardPile);
    printf("\n");
}