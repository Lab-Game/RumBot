#ifndef GAME_H
#define GAME_H

#include "table.h"

#define NUM_PLAYERS 3

typedef struct GameStruct Game;

typedef struct PlayerStruct {
    Game *game;
    int id;
    Cards hand;
    int score;
} Player;

struct GameStruct {
    int numPlayers;
    int currentPlayer;
    Player players[NUM_PLAYERS];
    Pile drawPile;
    Pile discardPile;
    Table table;
    Cards discarded;
};

void Game_init(Game *game);
Player *Game_player(Game *game, int num);
Player *Game_currentPlayer(Game *game);
void Game_nextTurn(Game *game);
void Game_print(Game *game);

void Player_init(Player *player, Game *game, int id);
Cards Player_draw(Player *player);
void Player_undoDraw(Player *player, Cards card);
void Player_discard(Player *player, Cards card);
void Player_undoDiscard(Player *player);
void Player_print(Player *player);

#endif // GAME_H
