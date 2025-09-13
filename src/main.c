#include <stdio.h>

#include "cards.h"
#include "pile.h"
#include "table.h"
#include "game.h"

int main(void) {
    Game game;
    Game_init(&game);
    Game_print(&game);

    return 0;
}