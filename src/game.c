#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"

void Game_init(Game *game) {
    game->numPlayers = NUM_PLAYERS;
    game->currentPlayerId = 0;
    game->currentPlayer = &game->players[0];
    for (int i = 0; i < game->numPlayers; ++i) {
        Player_init(&game->players[i], game, i);
    }
    Pile_init(&game->discardPile);
    Meld_init(&game->meld);
    game->everDiscarded = 0;

    // Shuffle the draw pile
    Pile_fullDeck(&game->drawPile);
    Pile_shuffle(&game->drawPile);

    // Deal 7 cards to each player
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        for (int j = 0; j < 7; ++j) {
            Player_draw(player);
        }
    }

    // First player draws one more card, which becomes the discard pile.
    Player *firstPlayer = Game_player(game, 0);
    Player_discard(firstPlayer, Player_draw(firstPlayer));

    // Clear the play for the first player
    Turn_init(&firstPlayer->turn);

    game->isOver = false;
}

void Game_copy(Game *original, Game *copy) {
    *copy = *original;
    // Fix up player->game pointers
    for (int i = 0; i < copy->numPlayers; ++i) {
        copy->players[i].game = copy;
    }
    copy->currentPlayer = &copy->players[copy->currentPlayerId];
}

Cards Game_swapToTop(Game *game, Cards card) {
    // Swap the top card in the draw pile with the specified card.
    // Return the card that was previously on top of the draw pile.

    // First we need to find the card, which could be in the
    // draw pile or a player's hand.  First, check the draw pile
    Pile *drawPile = &game->drawPile;
    Cards prevTop = Pile_peek(drawPile);

    if (Cards_has(drawPile->allCards, card)) {
        // The card is in the draw pile
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

    printf("Discard: ");
    Pile_print(&game->discardPile);
    printf("\n");

    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        if (i == game->currentPlayerId) {
            printf("-> ");
        } else {
            printf("   ");
        }
        Player_print(player);
    }

    if (DEB >= 2) {
        // Print all cards in the draw pile
        printf("Draw: ");
        Pile_print(&game->drawPile);
        printf("\n");
    } else {
        // Print only the number of cards in the draw pile
        // and only if there are few cards left.
        int drawSize = Pile_size(&game->drawPile);
        if (drawSize <= 5) {
            printf("Draw: %d cards\n", drawSize);
        }
    }
}

void Player_init(Player *player, Game *game, int id) {
    player->game = game;
    player->id = id;
    player->score = 0;
    player->hand = 0;
    Turn_init(&player->turn);
}

bool Player_isCurrent(Player *player) {
    return player->id == player->game->currentPlayerId;
}

Cards Player_draw(Player *player) {
    Cards card = Pile_pop(&player->game->drawPile);
    Cards_add(&player->hand, card);
    player->turn.drawn = card;
    return card;
}

void Player_undoDraw(Player *player) {
    Cards card = player->turn.drawn;
    Pile_push(&player->game->drawPile, card);
    Cards_remove(&player->hand, card);
    player->turn.drawn = 0;
}

Cards Player_couldDraw(Player *player) {
    Game *game = player->game;
    return FULL_DECK & ~(game->everDiscarded | player->hand | Meld_cards(&game->meld));
}

Cards Player_take(Player *player) {
    assert(Pile_size(&player->game->discardPile) >= 1);
    Cards card = Pile_pop(&player->game->discardPile);
    Pile_push(&player->turn.taken, card);
    Cards_add(&player->hand, card);
    return card;
}

void Player_undoTakes(Player *player) {
    while (Pile_size(&player->turn.taken) > 0) {
        Cards card = Pile_pop(&player->turn.taken);
        Cards_remove(&player->hand, card);
        Pile_push(&player->game->discardPile, card);
    }
}

void Player_discard(Player *player, Cards card) {
    assert(Cards_size(card) == 1);

    Game *game = player->game;
    Turn *turn = &player->turn;

    Cards_remove(&player->hand, card);
    Pile_push(&game->discardPile, card); 

    turn->discard = card;
    turn->newDiscards |= (card & ~game->everDiscarded);
    game->everDiscarded |= turn->newDiscards;
    turn->eval = -9999;
}

void Player_undoDiscard(Player *player) {
    Game *game = player->game;
    Turn *turn = &player->turn;

    Cards card = Pile_pop(&player->game->discardPile);
    Cards_add(&player->hand, card);

    player->turn.discard = 0;

    game->everDiscarded &= ~turn->newDiscards;
    turn->newDiscards = 0;
}

void Player_meld(Player *player, Meld *meld) {
    Player_playRun(player, meld->runs);

    if (!Cards_has(player->hand, meld->sets)) {
        printf("Error:  Player does not have all cards for meld set!\n");
        Meld_printCompact(meld);
        printf("Player hand after playing runs: ");
        Cards_print(player->hand);
        exit(1);
    }

    Player_playSet(player, meld->sets);
}

void Player_undoMeld(Player *player, Meld *meld) {
    Player_undoRun(player, meld->runs);
    Player_undoSet(player, meld->sets);
}

void Player_playRun(Player *player, Cards run) {
    assert(Cards_has(player->hand, Cards_raiseAces(run)));
    Meld_addRun(&player->game->meld, run);
    Meld_addRun(&player->turn.meld, run);
    Cards_remove(&player->hand, Cards_raiseAces(run));
    int points = Cards_points(run);
    player->score += points;
}

void Player_undoRun(Player *player, Cards run) {
    Cards_add(&player->hand, Cards_raiseAces(run));
    Meld_removeRun(&player->game->meld, run);
    Meld_removeRun(&player->turn.meld, run);
    int points = Cards_points(run);
    player->score -= points;
}

void Player_playSet(Player *player, Cards set) {
    assert(Cards_has(player->hand, set));
    Meld_addSet(&player->game->meld, set);
    Meld_addSet(&player->turn.meld, set);
    Cards_remove(&player->hand, set);
    int points = Cards_points(set);
    player->score += points;
}

void Player_undoSet(Player *player, Cards set) {
    Cards_add(&player->hand, set);
    Meld_removeSet(&player->game->meld, set);
    Meld_removeSet(&player->turn.meld, set);
    int points = Cards_points(set);
    player->score -= points;
}

void Player_play(Player *player, Turn *turn) {
    // Handle draw or take
    if (turn->drawn) {
        // Draw a card
        Cards drawnCard = Player_draw(player);
        assert(drawnCard == turn->drawn);
    } else {
        // Take one or more cards from discard pile
        int takeCount = Pile_size(&turn->taken);
        for (int i = 0; i < takeCount; ++i) {
            Player_take(player);
        }
    }

    // Handle meld
    Player_meld(player, &turn->meld);

    // Handle discard
    if (turn->discard) {
        Player_discard(player, turn->discard);
    }
}

void Player_print(Player *player) {
    Game *game = player->game;
    printf("Player %d (%3d pts)  ", player->id, player->score);
    if (DEB >= 1) {
        Cards_print(player->hand);
    } else if (!player->hand) {
        printf("(no cards)");
    } else {
        Cards exposed = player->hand & game->everDiscarded;
        Cards hidden = player->hand & ~game->everDiscarded;
        if (exposed) {
            Cards_print(exposed);
        }
        if (hidden) {
            printf(" +%d", Cards_size(hidden));
        }   
    }
    
    printf("\n");
}