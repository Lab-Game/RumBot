#include <stdlib.h>
#include <stdio.h>

#include "log.h"

void Log_gameStart(const Game *game) {
    // Print the draw pile
    Pile_print(&game->drawPile);
    printf("\n");

    // Print the discard pile
    Pile_print(&game->discardPile);
    printf("\n");

    // Print each player's hand
    for (int i = 0; i < game->numPlayers; ++i) {
        Cards_print(game->players[i].hand);
        printf("\n");
    }
}

void Log_turn(const Game *game, const Turn *turn) {
    // First indicate whether this is a take or draw.
    // If this is a take, write "T <# of careds>".
    // If this is a draw, wrinte "D <card drawn>".
    if (turn->taken.size > 0) {
        printf("T %d\n", turn->taken.size);
    } else {
        printf("D ");
        Card_print(Cards_toCard(turn->drawn));
        printf("\n");
    }

    // Next print the melded cards.  Run cards are in
    // angle brackets.  Set cards are in curly braces.
    if (turn->meld.runs != 0) {
        printf("< ");
        Cards_print(turn->meld.runs);
        printf(">\n");
    }
    if (turn->meld.sets != 0) {
        printf("{");
        Cards_print(turn->meld.sets);
        printf(" }\n");
    }

    // Finally, print the discard.  If there is no discard, print "#".
    if (turn->discard != 0) {
        Cards_print(turn->discard);
        printf("\n");
    } else {
        printf("#\n");
    }
}