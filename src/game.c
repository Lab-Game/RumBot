#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cards.h"
#include "pile.h"
#include "game.h"

void Game_init(Game *game) {
    game->numPlayers = NUM_PLAYERS;
    game->currentPlayer = 0;
    for (int i = 0; i < game->numPlayers; ++i) {
        Player_init(&game->players[i], game, i);
    }
    Pile_fullDeck(&game->drawPile);
    Pile_init(&game->discardPile);
    Table_init(&game->table);
    game->discarded = 0;

    // Shuffle the draw pile
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
}

Player *Game_player(Game *game, int num) {
    assert(num >= 0 && num < game->numPlayers);
    return &(game->players[num]);
}

Player *Game_currentPlayer(Game *game) {
    return &(game->players[game->currentPlayer]);
}

void Game_nextTurn(Game *game) {
    game->currentPlayer = (game->currentPlayer + 1) % game->numPlayers;
}

void Game_print(Game *game) {
    printf("Player %d/%d\n", game->currentPlayer, game->numPlayers);
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        Player_print(player);
    }
    printf("Draw pile: ");
    Pile_print(&game->drawPile);
    printf("\nDiscard pile: ");
    Pile_print(&game->discardPile);
    printf("\n");
    Table_print(&game->table);
}

void Player_init(Player *player, Game *game, int id) {
    player->game = game;
    player->id = id;
    player->score = 0;
    player->hand = 0;
}

Cards Player_draw(Player *player) {
    Cards card = Pile_pop(&player->game->drawPile);
    Cards_add(&player->hand, card);
    return card;
}

void Player_undoDraw(Player *player, Cards card) {
    Pile_push(&player->game->drawPile, card);
    Cards_remove(&player->hand, card);
}

void Player_discard(Player *player, Cards card) {
    Cards_remove(&player->hand, card);
    Pile_push(&player->game->discardPile, card);
}

void Player_undoDiscard(Player *player) {
    Cards card = Pile_pop(&player->game->discardPile);
    Cards_add(&player->hand, card);
}

void Player_playRun(Player *player, Cards meld) {
    assert(Cards_size(meld) >= 3);
    assert(Cards_isLegal(meld));
    assert(Cards_has(player->hand, meld));
    Table_addRun(&player->game->table, meld);
    Cards_remove(&player->hand, meld);
}

void Player_print(Player *player) {
    printf("Player %d: score=%d hand=", player->id, player->score);
    Cards_print(player->hand);
    printf("\n");
}