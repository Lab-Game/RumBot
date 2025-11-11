#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "game.h"

void Scoreboard_fromGame(Game *game);
void Scoreboard_take(Game *game);
void Scoreboard_draw(Game *game);
void Scoreboard_meld(Game *game);
void Scoreboard_discard(Game *game);

#endif // SCOREBOARD_H