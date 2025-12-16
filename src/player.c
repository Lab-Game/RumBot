#include "player.h"
#include "game.h"

void Player_init(Player *player) {
    player->game = NULL;
    player->id = 0;
    player->score = 0;
    player->hand = 0;
}

void Player_play(Player *player, Turn *turn) {
    // Handle draw or take
    if (turn->drawn) {
        Cards drawnCard = Player_draw(player);
        assert(drawnCard == turn->drawn);
    } else {
        assert(turn->taken.size > 0);
        Player_take(player, Pile_size(&turn->taken));
    }

    // Handle meld
    Player_meld(player, &turn->meld);

    // Handle discard
    if (turn->discard) {
        Player_discard(player, turn->discard);
    }
}

Cards Player_couldDraw(Player *player) {
    Game *game = player->game;
    return kFullDeck & ~(game->everDiscarded | player->hand | Meld_cards(&game->meld));
}

void Player_print(Player *player) {
    printf("Player %d (%3d pts)  ", player->id, player->score);
    Cards_print(player->hand);
    printf("\n");
}

Cards Player_draw(Player *player) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn == 0);
    assert(turn->taken.size == 0);
    assert(Meld_isEmpty(&turn->meld));
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);

    Cards card = Pile_pop(&game->drawPile);
    Cards_add(&player->hand, card);
    turn->drawn = card;
    return card;
}

void Player_undoDraw(Player *player) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn != 0);
    assert(turn->taken.size == 0);
    assert(Meld_isEmpty(&turn->meld));
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);

    Cards_remove(&player->hand, turn->drawn);
    Pile_push(&game->drawPile, turn->drawn);
    turn->drawn = 0;
}

void Player_take(Player *player, int num) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn == 0);
    assert(Meld_isEmpty(&turn->meld));
    assert(turn->discard == 0);

    assert(Pile_size(&game->discardPile) >= num);
    for (int i = 0; i < num; ++i) {
        Cards card = Pile_pop(&game->discardPile);
        Pile_push(&turn->taken, card);
        Cards_add(&player->hand, card);
    }
}

void Player_undoTake(Player *player) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn == 0);
    assert(turn->taken.size > 0);
    assert(Meld_isEmpty(&turn->meld));
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);

    while (Pile_size(&turn->taken) > 0) {
        Cards card = Pile_pop(&turn->taken);
        Cards_remove(&player->hand, card);
        Pile_push(&game->discardPile, card);
    }
}

void Player_meld(Player *player, Meld *meld) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn != 0 || turn->taken.size > 0);
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);

    Player_playRun(player, meld->runs);
    Player_playSet(player, meld->sets);
}

void Player_undoMeld(Player *player, Meld *meld) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn != 0 || turn->taken.size > 0);
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);

    Player_undoRun(player, meld->runs);
    Player_undoSet(player, meld->sets);
}

void Player_playRun(Player *player, Cards run) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn != 0 || turn->taken.size > 0);
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);

    Cards_remove(&player->hand, Cards_raiseAces(run));
    Meld_addRun(&game->meld, run);
    Meld_addRun(&turn->meld, run);
    int points = Cards_points(run);
    player->score += points;
}

void Player_undoRun(Player *player, Cards run) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn != 0 || turn->taken.size > 0);
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);

    Cards_add(&player->hand, Cards_raiseAces(run));
    Meld_removeRun(&game->meld, run);
    Meld_removeRun(&turn->meld, run);
    int points = Cards_points(run);
    player->score -= points;
}

void Player_playSet(Player *player, Cards set) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn != 0 || turn->taken.size > 0);
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);

    Meld_addSet(&game->meld, set);
    Meld_addSet(&turn->meld, set);
    Cards_remove(&player->hand, set);
    int points = Cards_points(set);
    player->score += points;
}

void Player_undoSet(Player *player, Cards set) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn != 0 || turn->taken.size > 0);
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);

    Cards_add(&player->hand, set);
    Meld_removeSet(&game->meld, set);
    Meld_removeSet(&turn->meld, set);
    int points = Cards_points(set);
    player->score -= points;
}

void Player_discard(Player *player, Cards card) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    assert(turn->drawn != 0 || turn->taken.size > 0);
    assert(turn->discard == 0);
    assert(turn->newDiscards == 0);
    assert(Cards_size(card) == 1);

    Cards_remove(&player->hand, card);
    Pile_push(&game->discardPile, card);
    turn->discard = card;
    turn->newDiscards = (card & ~game->everDiscarded);
    game->everDiscarded |= turn->newDiscards;
}

void Player_undoDiscard(Player *player) {
    Game *game = player->game;
    Turn *turn = &game->turn;

    Cards card = Pile_pop(&game->discardPile);
    Cards_add(&player->hand, card);

    game->everDiscarded &= ~turn->newDiscards;
    turn->discard = 0;
    turn->newDiscards = 0;
}
