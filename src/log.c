#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

void Log_writeGame(Game *game, FILE *log_file) {
    // Print each player's hand
    for (int i = 0; i < game->numPlayers; ++i) {
        Cards_printToFile(game->players[i].hand, log_file);
        fprintf(log_file, "\n");
    }

    // Print the draw pile
    Pile_printToFile(&game->drawPile, log_file);
    fprintf(log_file, "\n");

    // Print the discard pile
    Pile_printToFile(&game->discardPile, log_file);
    fprintf(log_file, "\n");
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

int Log_readCards(FILE *log_file, Cards *card) {
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

    *card = Cards_fromCard(Card_fromValueSuit(value, suit));
    return 1;
}

void Log_readGame(Game *game, FILE *log_file) {
    // Start with all the cards in the draw pile.
    Game_init(game);

    // Read each player's hand
    for (int i = 0; i < game->numPlayers; ++i) {
        Cards *hand = &game->players[i].hand;

        while (1) {
            Cards card;
            int result = Log_readCards(log_file, &card);

            if (result == -1) {
                // End of line.  Done with this player's hand.
                break;
            }  else if (result == 1) {
                // If a card is specified, move it to the top of the draw pile.
                Game_swapToTop(game, card);
            }

            // Draw a card and add it to the player's hand.
            Cards_add(hand, Pile_pop(&game->drawPile));
        }
    }

    // Now read the draw pile and arrange cards in the specified order
    int i = 0;
    while (1) {
        Cards card;
        int result = Log_readCards(log_file, &card);

        if (result == -1) {
            // End of line.  Done with the draw pile.
            break;
        } else if (result == 1) {
            // Move the specified card to the top of the draw pile.
            Game_swapToTop(game, card);
        }

        // Move the top card in the draw pile to position i.
        Pile_swapToTop(&game->drawPile, i);

        i += 1;
    }

    // Finally, read the discard pile.
    while (1) {
        Cards card;
        int result = Log_readCards(log_file, &card);

        assert(result != 0); // discard pile cards cannot be unknown

        if (result == -1) {
            // End of line.  Done with the discard pile.
            break;
        } else if (result == 1) {
            // Move the specified card to the top of the draw pile.
            Game_swapToTop(game, card);
        }

        printf("pushing onto discard pile\n");
        Pile_push(&game->discardPile, Pile_pop(&game->drawPile));
    }

    // Now read turns until end of file.  Skip spaces.
    // If we reach and EOF, we're done.  If we see a "T" or "D",
    // then we read a turn.
    while (1) {
        int ch = fgetc(log_file);

        if (ch == EOF) {
            break;
        } else if (ch == ' ') {
            // Skip spaces
            continue;
        }
        
        if (ch != 'T' && ch != 'D') {
            fprintf(stderr, "Log_readGame: expected T or D, got '%c'\n", ch);
            exit(1);
        }   

        Player *player = game->currentPlayer;

        // Handle a draw
        if (ch == 'D') {
            Cards card;
            int result = Log_readCards(log_file, &card);

            if (result == -1) {
                fprintf(stderr, "Log_readGame: expected card after D\n");
                exit(1);
            }

            if (result == 1) {
                // Move the specified card to the top of the draw pile.
                Game_swapToTop(game, card);
            }

            Player_draw(player);
        }

        if (ch == 'T') {
            // Handle a take
            int takeCount;
            if (fscanf(log_file, "%d", &takeCount) != 1) {
                fprintf(stderr, "Log_readGame: expected count after T\n");
                exit(1);
            }

            for (int i = 0; i < takeCount; ++i) {
                Player_take(player);
            }
        }

        // Now look for a meld, including runs of the form <cards> and
        // sets of the form {cards}.
        Meld meld;
        Meld_init(&meld);

        while (1) {
            ch = fgetc(log_file);

            // Skip over spaces
            if (ch == ' ') {
                continue;

            } else if (ch == '<') {
                // Read a run
                Cards run = 0;
                while (1) {
                    Cards card;
                    int result = Log_readCards(log_file, &card);
                    if (result == -1) {
                        fprintf(stderr, "Log_readGame: unexpected end of line in run meld\n");
                        exit(1);
                    } else if (result == 1) {
                        Cards_add(&run, card);
                    }

                    // See if we're done with the run
                    ch = fgetc(log_file);
                    if (ch == '>') {
                        break;
                    } else {
                        ungetc(ch, log_file);
                    }
                }
                Meld_addRun(&meld, run);
            } else if (ch == '{') {
                // Read a set
                Cards set = 0;
                while (1) {
                    Cards card;
                    int result = Log_readCards(log_file, &card);
                    if (result == -1) {
                        fprintf(stderr, "Log_readGame: unexpected end of line in set meld\n");
                        exit(1);
                    } else if (result == 1) {
                        Cards_add(&set, card);
                    }

                    ch = fgetc(log_file);
                    if (ch == '}') {
                        break;
                    } else {
                        ungetc(ch, log_file);
                    }
                }
                Meld_addSet(&meld, set);
            } else {
                // No more melds
                ungetc(ch, log_file);
                break;
            }
        }
    }
}