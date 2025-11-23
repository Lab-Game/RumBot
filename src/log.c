#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

void Log_writeGame(Game *game, FILE *log_file) {
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

void Log_writeTurn(Turn *turn, FILE *log_file) {
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

    // Finally, print the discard, if there is one.
    if (turn->discard != 0) {
        fprintf(log_file, " ");
        Cards_printToFile(turn->discard, log_file);
    }
    fprintf(log_file, "\n");
}

int Log_readCard(FILE *log_file, Card *card) {
    // After some spaces, if the file contains a legal card, return 1.
    // (And fill in the card via the pointer argument.)
    // If it contains a "?", return 0 (unknown card).
    // If it contains a newline or EOF, return -1 (end of line).
    const char *values = "a23456789TJQKA";
    const char *suits = "CDSH";

    int ch;

    // Skip spaces
    while((ch = fgetc(log_file)) == ' ');

    // Check for newline or EOF
    if (ch == '\n' || ch == EOF) {
        return -1;
    }

    // Check for "?"
    if (ch == '?') {
        return 0;
    }

    // Read the card.  If invalid, exit.
    char *value_ptr = strchr(values, ch);
    if (!value_ptr) {
        fprintf(stderr, "Log_readCard: invalid card value '%c'\n", ch);
        exit(1);
    }
    int value = value_ptr - values;

    ch = fgetc(log_file);
    char *suit_ptr = strchr(suits, ch);
    if (!suit_ptr) {
        fprintf(stderr, "Log_readCard: invalid card suit '%c'\n", ch);
        exit(1);
    }
    int suit = suit_ptr - suits;

    *card = Card_fromValueSuit(value, suit);
    return 1;
}

void Log_readGame(Game *game, FILE *log_file) {
    Game_init(game);

    // Read the draw pile, which should be the same length as the existing draw pile.
    for (int i = 0; i < Pile_size(&game->drawPile); ++i) {
        Card card;
        int result = Log_readCard(log_file, &card);
        if (result != 1) {
            fprintf(stderr, "Log_readGame: expected card in draw pile\n");
            exit(1);
        }
        Cards c = Cards_fromCard(card);
        printf("Trying to move to position %d: ", i);
        Card_print(card);
        printf("\n");

        // Move the card to the top of the draw pile.
        Game_swapToTop(game, c);

        // Swap the card at the top of the draw pile to position i.
        Pile_swapToTop(&game->drawPile, i);
    }

    Game_print(game);
}