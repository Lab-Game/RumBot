#ifndef LOG_H
#define LOG_H

#include "cards.h"
#include "meld.h"
#include "turn.h"
#include "game.h"

void Log_gameStart(const Game *game);
void Log_turn(const Game *game, const Turn *turn);

#endif // LOG_H
