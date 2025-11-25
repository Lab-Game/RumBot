#ifndef PLAYER_H
#define PLAYER_H

#include "turn.h"
#include "cards.h"

typedef struct {
    struct GameStruct *game;
    int id;
    Cards hand;
    int score;
    Turn turn;
} Player;

void Player_init(Player *player, int id);
bool Player_isCurrent(Player *player);
Cards Player_draw(Player *player);
void Player_undoDraw(Player *player);
Cards Player_couldDraw(Player *player);
Cards Player_take(Player *player);
void Player_undoTakes(Player *player);
void Player_discard(Player *player, Cards card);
void Player_undoDiscard(Player *player);
void Player_meld(Player *player, Meld *meld);
void Player_undoMeld(Player *player, Meld *meld);
void Player_playRun(Player *player, Cards run);
void Player_undoRun(Player *player, Cards run);
void Player_playSet(Player *player, Cards set);
void Player_undoSet(Player *player, Cards set);
void Player_play(Player *player, Turn *turn);
void Player_print(Player *player);

#endif // PLAYER_H
