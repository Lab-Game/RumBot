#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

#if 0

void Log_writeGame(FILE *log_file, Game *game) {
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

void Log_writeTurn(FILE *log_file, Turn *turn) {
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

Cards Log_readCard(FILE *log_file) {
    // Read a card from the log file.
    // If a card can not be read, return 0.
    // If a unkonwn card ("?") is read, return PLACEHOLDER.
    const char *values = "a23456789TJQKA";
    const char *suits = "CDSH";

    int ch = fgetc(log_file);
    if (ch == '?') {
        return kSpecialCard;
    }
    char *value_ptr = strchr(values, ch);
    if (!value_ptr) {
        ungetc(ch, log_file);
        return 0;
    }
    int value = value_ptr - values;

    ch = fgetc(log_file);
    char *suit_ptr = strchr(suits, ch);
    if (!suit_ptr) {
        ungetc(ch, log_file);
        return 0;
    }
    int suit = suit_ptr - suits;

    return Cards_fromCard(Card_fromValueSuit(value, suit));
}

void Log_readCards(FILE *log_file, Game *game, Cards *cards) {
    *cards = 0;
    while (1) {
        Cards card = Log_readCard(log_file);
        if (card == 0) {
            fprintf(stderr, "Log_readCards: error reading cards\n");
            exit(1);
        }

        if (card == kSpecialCard) {
            // Replace the unknown card with the top card from the draw pile.
            card = Pile_pop(&game->drawPile);
        }

        Cards_add(cards, card);

        int ch = fgetc(log_file);
        if (ch != ' ') {
            ungetc(ch, log_file);
            return;
        }
    }
}

void Log_readPile(FILE *log_file, Game *game, Pile *pile) {
    Pile_init(pile);
    while (1) {
        Cards card = Log_readCard(log_file);
        if (card == 0) {
            fprintf(stderr, "Log_readPile: error reading pile\n");
            exit(1);
        }

        if (card == kSpecialCard) {
            // Replace the unknown card with the top card from the draw pile.
            card = Pile_pop(&game->drawPile);
        }

        Pile_push(pile, card);

        int ch = fgetc(log_file);
        if (ch != ' ') {
            ungetc(ch, log_file);
            return;
        }
    }
}

void Log_readGame(FILE *log_file, Game *game) {
    // Start with all the cards in the draw pile.
    Game_init(game);

    // Read each player's hand
    for (int i = 0; i < game->numPlayers; ++i) {
        Cards *hand = &game->players[i].hand;
        Pile handPile;

        Log_readPile(log_file, game, &handPile);
        *hand = handPile.allCards;
        if (fgetc(log_file) != '\n') {
            fprintf(stderr, "Log_readGame: error reading player %d hand\n", i);
            exit(1);
        }
    }

    // Read the draw pile
    Log_readPile(log_file, game, &game->drawPile);
    if (fgetc(log_file) != '\n') {
        fprintf(stderr, "Log_readGame: error reading draw pile\n");
        exit(1);
    }

    // Read the discard pile
    Log_readPile(log_file, game, &game->discardPile);
    if (fgetc(log_file) != '\n') {
        fprintf(stderr, "Log_readGame: error reading discard pile\n");
        exit(1);
    }

    // Now we'll read a sequence of turns until EOF.
    while (1) {
        int ch = fgetc(log_file);
        if (ch == EOF) {
            // We've finished reading the log.
            return;
        }

        if (ch == 'T') {
            // This is a take turn.
            int takeCount;
            fscanf(log_file, "%d", &takeCount);
            Player_take(game->currentPlayer, takeCount);
        } else if (ch == 'D') {
            // This is a draw turn.
            Cards drawnCard = Log_readCard(log_file);
            if (drawnCard == kSpecialCard) {
                // If the drawn card is unknown, then just draw from the draw pile.
                drawnCard = Pile_pop(&game->drawPile);
            } else {
                // Otherwise, we need to make sure the drawn card is on top of the draw pile.
                Cards topCard = Pile_peek(&game->drawPile);
                if (topCard != drawnCard) {
                    // Swap the special card in place of the drawn card,
                    // wherever it is.
                    Game_swap(game, kSpecialCard, drawnCard);
                    // Now swap the drawn card to the top of the draw pile.
                    Pile_swap(&game->drawPile, drawnCard, topCard);
                    // And put the old top card wherever the drawn card was.
                    Game_swap(game, topCard, kSpecialCard);

                }
            }
            Cards card = Player_draw(game->currentPlayer);
            assert(card == drawnCard);
        } else {
            fprintf(stderr, "Log_readGame: error reading turn type\n");
            exit(1);
        }

        if (fgetc(log_file) != ' ') {
            fprintf(stderr, "Log_readGame: error reading turn meld/discard\n");
            exit(1);
        }

        // Read melds, if any
        Meld meld;
        Meld_init(&meld);
        while (1) {
            int ch = fgetc(log_file);
            if (ch == '<') {
                // Read a run meld
                Cards runCards;
                Log_readCards(log_file, game, &runCards);
                Meld_addRun(&meld, runCards);

                ch = fgetc(log_file);
                if (ch != '>') {
                    fprintf(stderr, "Log_readGame: error reading run meld\n");
                    exit(1);
                }
            } else if (ch == '{') {
                // Read a set meld
                Cards setCards;
                Log_readCards(log_file, game, &setCards);
                Meld_addSet(&meld, setCards);

                ch = fgetc(log_file);
                if (ch != '}') {
                    fprintf(stderr, "Log_readGame: error reading set meld\n");
                    exit(1);
                }
            } else {
                ungetc(ch, log_file);
                break;
            }

            if (fgetc(log_file) != ' ') {
                fprintf(stderr, "Log_readGame: error reading meld/discard separator\n");
                exit(1);
            }
        }
    }
        #if 0
        // Ensure that the player has all the cards for the meld.
        Cards meldCards = Meld_cards(&meld);
        Player *player = game->currentPlayer;
        for (Cards c = Cards_first(meldCards); c != 0; c = Cards_next(meldCards, c)) {
            Log_ensurePlayerHasCard(player, c);
        }
        Player_meld(game->currentPlayer, &meld);

        // Read discard, if any
        ch = fgetc(log_file);
        if (ch == '\n' || ch == EOF) {
            // No discard
            continue;
        }
        Cards discardCard = Log_readCard(log_file);
        if (discardCard == 0 || discardCard == kSpecialCard) {
            fprintf(stderr, "Log_readGame: error reading discard card\n");
            exit(1);
        }
        Log_ensurePlayerHasCard(game->currentPlayer, discardCard);
        Player_discard(game->currentPlayer, discardCard);
#endif
}

#endif