#include <stdio.h>

#include "cards.h"
#include "pile.h"
#include "table.h"
#include "game.h"
#include "turn.h"

void Search_meld(Game *game, Turn *scratch, Turn *bestTurn) {

}

void Search_take(Game *game, Turn *scratch, Turn *bestTurn) {
    Player *player = Game_currentPlayer(game);
    Pile *discardPile = &game->discardPile;

    // Try taking each number of cards in the discard pile, up to all of them.
    while (Pile_size(discardPile) > 0) {
        Player_take(player);
        Search_meld(game, scratch, bestTurn);
    }
    Player_undoTakes(player);
}

int main(void) {
    Game game;
    Game_init(&game);
    Game_print(&game);

    Turn scratch, bestTurn;
    Turn_init(&scratch);
    Turn_init(&bestTurn);

    Search_take(&game, &scratch, &bestTurn);

    return 0;
}