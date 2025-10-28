#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.h"

void Game_init(Game *game) {
    game->numPlayers = NUM_PLAYERS;
    game->currentPlayer = 0;
    for (int i = 0; i < game->numPlayers; ++i) {
        Player_init(&game->players[i], game, i);
    }
    Pile_fullDeck(&game->drawPile);
    Pile_init(&game->discardPile);
    Meld_init(&game->meld);
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

    // Clear the play for the first player
    Turn_init(&firstPlayer->turn);

    game->isOver = false;
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
    Turn_init(&game->players[game->currentPlayer].turn);

    if (Pile_size(&game->drawPile) == 0) {
        game->isOver = true;
    }
}

void Game_print(Game *game) {
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        if (i == game->currentPlayer) {
            printf("-> ");
        } else {
            printf("   ");
        }
        Player_print(player);
    }
    printf("Draw pile: ");
    Pile_print(&game->drawPile);
    printf("\nDiscard pile: ");
    Pile_print(&game->discardPile);
    printf("\n");
    Meld_print(&game->meld);
}

void Player_init(Player *player, Game *game, int id) {
    player->game = game;
    player->id = id;
    player->score = 0;
    player->hand = 0;
    Turn_init(&player->turn);
}

Cards Player_draw(Player *player) {
    Cards card = Pile_pop(&player->game->drawPile);
    Cards_add(&player->hand, card);
    player->turn.draw = card;
    return card;
}

void Player_undoDraw(Player *player, Cards card) {
    Pile_push(&player->game->drawPile, card);
    Cards_remove(&player->hand, card);
    player->turn.draw = 0;
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
    Cards_remove(&player->hand, card);
    Pile_push(&player->game->discardPile, card);
    player->turn.discard = card;
}

void Player_undoDiscard(Player *player) {
    Cards card = Pile_pop(&player->game->discardPile);
    Cards_add(&player->hand, card);
    player->turn.discard = 0;
}

void Player_meld(Player *player, Meld *meld) {
    Player_playRun(player, meld->runs);
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

void Player_print(Player *player) {
    printf("Player %d (%3d pts)  ", player->id, player->score);
    Cards_print(player->hand);
    printf("\n");
}