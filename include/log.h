#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#include "cards.h"
#include "meld.h"
#include "turn.h"
#include "game.h"

void Log_game(Game *game, FILE *log_file);
void Log_turn(Turn *turn, FILE *log_file);

#endif // LOG_H
