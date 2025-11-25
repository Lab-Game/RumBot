#include "player.h"
#include "game.h"

void Player_init(Player *player, int id) {
    player->game = NULL;
    player->id = id;
    player->score = 0;
    player->hand = 0;
    Turn_init(&player->turn);
}

bool Player_isCurrent(Player *player) {
    return player->id == player->game->currentPlayerId;
}

Cards Player_draw(Player *player) {
    assert(player->turn.drawn == 0);
    assert(player->turn.taken.size == 0);
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
    assert(player->turn.drawn == 0);
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
        if (drawnCard != turn->drawn) {
            printf("Drawn card: ");
            Cards_print(drawnCard);
            printf("\n");
            printf("Expected drawn card: ");
            Cards_print(turn->drawn);
        }
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
    printf("Player %d (%3d pts)  ", player->id, player->score);
    Cards_print(player->hand);
    printf("\n");
}