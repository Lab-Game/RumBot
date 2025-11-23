#include <stdlib.h>
#include <stdio.h>

#include "log.h"

void Log_game(Game *game, FILE *log_file) {
    // Print the draw pile
    Pile_printToFile(&game->drawPile, log_file);
    fprintf(log_file, "\n");

    // Print the discard pile
    Pile_printToFile(&game->discardPile, log_file);
    fprintf(log_file, "\n");

    // Print each player's hand
    for (int i = 0; i < game->numPlayers; ++i) {
        Cards_printToFile(game->players[i].hand, log_file);
        fprintf(log_file, "\n");
    }
}

void Log_turn(Turn *turn, FILE *log_file) {
    // First indicate whether this is a take or draw.
    // If this is a take, write "T <# of careds>".
    // If this is a draw, wrinte "D <card drawn>".
    if (turn->taken.size > 0) {
        fprintf(log_file, "T %d", turn->taken.size);
    } else {
        fprintf(log_file, "D ");
        Cards_printToFile(turn->drawn, log_file);
    }

    // Next print the melded cards.  Run cards are in
    // angle brackets.  Set cards are in curly braces.
    if (turn->meld.runs != 0) {
        fprintf(log_file, " <");
        Cards_printToFile(turn->meld.runs, log_file);
        fprintf(log_file, ">");
    }
    if (turn->meld.sets != 0) {
        fprintf(log_file, " {");
        Cards_printToFile(turn->meld.sets, log_file);
        fprintf(log_file, "}");
    }

    // Finally, print the discard.  If there is no discard, print "#".
    if (turn->discard != 0) {
        fprintf(log_file, " ");
        Cards_printToFile(turn->discard, log_file);
        fprintf(log_file, "\n");
    } else {
        fprintf(log_file, " #\n");
    }
}