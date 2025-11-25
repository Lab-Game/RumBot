#ifndef GAME_H
#define GAME_H

#include "rumbot.h"
#include "turn.h"
#include "cards.h"
#include "pile.h"
#include "meld.h"
#include "player.h"

typedef struct GameStruct {
    int numPlayers;
    int currentPlayerId;
    Player *currentPlayer;
    Player players[NUM_PLAYERS];
    Pile drawPile;
    Pile discardPile;
    Meld meld;
    Cards everDiscarded;
    Turn turn;
    bool isOver;
} Game;

void Game_init(Game *game);
void Game_deal(Game *game);
void Game_copy(Game *original, Game *copy);
Cards Game_swap(Game *game, Cards remove, Cards insert);
Cards Game_swapToTop(Game *game, Cards card);
void Game_permute(Game *game, Game *permuted);
Player *Game_player(Game *game, int num);
void Game_nextTurn(Game *game);
void Game_print(Game *game);
void Game_printExposed(Game *game);

#endif // GAME_H
