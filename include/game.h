#ifndef GAME_H
#define GAME_H

#include "rumbot.h"
#include "turn.h"
#include "cards.h"
#include "pile.h"
#include "meld.h"
#include "player.h"

// Most fields are self-explanatory.  EverDiscarded is a bitset
// of all cards that have ever been discarded during the game,
// and is used to track which cards are known to all players.
// "turn" records the current player's actions during their turn,
// which allows undoing actions within the turn.
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

// Set up a Game and all Players.  All cards are in the draw pile in order.
void Game_init(Game *game);

// Shuffle cards in the draw pile.
void Game_shuffle(Game *game);

// Deal 7 cards to every player and 1 card to the discard pile.
void Game_deal(Game *game);

// Returns a pointer to the num-th Player in the game.
Player *Game_player(Game *game, int num);

// Deep copy a game state, including all players.
void Game_copy(Game *original, Game *copy);

// Swap two cards that are both currentlly in a player's
// hand, the draw pile, or the discard pile.
void Game_swap(Game *game, Cards card1, Cards card2);

// Advance to the next player's turn.  If the current player
// has gone out or the draw pile is empty, mark the game as over
// and return true.
bool Game_nextTurn(Game *game);

// Permute all unknown cards to the current player.
void Game_permute(Game *game);
void Game_print(Game *game);
void Game_printForPlayer(Game *game);

#endif // GAME_H
