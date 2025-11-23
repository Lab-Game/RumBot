#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#include "cards.h"
#include "meld.h"
#include "turn.h"
#include "game.h"

void Log_writeGame(Game *game, FILE *log_file);
void Log_writeTurn(Turn *turn, FILE *log_file);
void Log_readGame(Game *game, FILE *log_file);
int Log_readCard(FILE *log_file, Card *card);

#endif // LOG_H
