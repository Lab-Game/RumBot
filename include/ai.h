#ifndef AI_H
#define AI_H

#include "cards.h"
#include "game.h"
#include "plays.h"

// Given a Game, determine the best Turn for the current player
// and update the game state accordingly.
void AI_go(Game *game);

#endif // AI_H
