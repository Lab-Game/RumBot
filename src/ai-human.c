#include <stdio.h>
#include <string.h>

#include "game.h"
#include "ai.h"

Turn *AI_human(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;
    char cardsTaken[16];

    assert(!game->isOver);
    assert(game->currentPlayer == player);

    Game_printForPlayer(game);

    printf("YOUR TURN HUMAN!\n");
    printf("(t)ake or (d)raw?\n");

    char turnType[16];
    do {
        fgets(turnType, sizeof(turnType), stdin);
    } while (turnType[0] != 't' && turnType[0] != 'd');

    if (turnType[0] == 't') {
        Cards takeUpTo = 0;
        do {
            printf("Take up to what card?\n");
            fgets(cardsTaken, sizeof(cardsTaken), stdin);
        } while (!Pile_has(&game->discardPile, takeUpTo));

        do {
            Cards taken = Player_take(player, 1);
        } while (Pile_has(&game->turn.taken, takeUpTo));
    } else {
        ai->drawn = Player_draw(player);
        printf("You drew: ");
        Cards_print(drawn);
        printf("\n");
    }

    Game_printForPlayer(game);

    printf("Enter run crards:\n");
    char runInput[128];
    fgets(runInput, sizeof(runInput), stdin);
    Cards runCards = Cards_fromString(runInput);
    if (runCards != 0) {
        Player_playRun(player, runCards);
    }

    printf("Enter set crards:\n");
    char setInput[128];
    fgets(setInput, sizeof(setInput), stdin);
    Cards setCards = Cards_fromString(setInput);
    if (setCards != 0) {
        Player_playSet(player, setCards);
    }

    if (Cards_size(player->hand) == 0) {
        printf("No discard (you went out!)\n");
    } else {
        printf("Enter discard card:\n");
        char discardInput[16];
        fgets(discardInput, sizeof(discardInput), stdin);
        Cards discardCard = Cards_fromString(discardInput);
        Player_discard(player, discardCard);
    }
}