#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#include "cards.h"
#include "meld.h"
#include "turn.h"
#include "game.h"

void Log_writeGame(FILE *log_file, Game *game);
void Log_writeTurn(FILE *log_file, Turn *turn);
void Log_readGame(FILE *log_file, Game *game);

#endif // LOG_H
