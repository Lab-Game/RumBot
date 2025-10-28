#ifndef GAME_H
#define GAME_H

#include "turn.h"
#include "cards.h"
#include "pile.h"
#include "table.h"

#define NUM_PLAYERS 3

typedef struct GameStruct Game;

typedef struct PlayerStruct {
    Game *game;
    int id;
    Cards hand;
    Cards known;
    int score;
    Turn turn;  // Records the player's actions during this turn to enable rewind.
} Player;

// Question:  where should I put the memory needed to store possible plays
// for a player?  Should it go in the Player structure?  Or in the Game structure?
// Or the Turn structure within the Player structure?
// I think it should be in the Player structure, because I might conceivably
// attempt to consider all plays for one player and recursively explore all
// plays for another player.

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
Cards Player_take(Player *player);
void Player_undoTakes(Player *player);
void Player_discard(Player *player, Cards card);
void Player_undoDiscard(Player *player);
void Player_playRun(Player *player, Cards run);
void Player_undoRun(Player *player, Cards run);
void Player_playSet(Player *player, Cards set);
void Player_undoSet(Player *player, Cards set);
void Player_print(Player *player);

#endif // GAME_H
