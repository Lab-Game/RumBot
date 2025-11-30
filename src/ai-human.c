#include <stdio.h>
#include <string.h>

#include "game.h"
#include "player.h"
#include "ai.h"

void AI_goHuman(AI *ai, Turn *turn) {
    Game *game = ai->game;
    Player *player = ai->player;

    assert(!game->isOver);
    assert(game->currentPlayer == player);

    // Show the game from the player's perspective.
    Game_printForPlayer(game);

    printf("YOUR TURN HUMAN!\n");
    printf("(t)ake or (d)raw?\n");

    // Determine whether this is a draw or take.
    char turnType[16];
    while(1) {
        fgets(turnType, sizeof(turnType), stdin);
        if (turnType[0] == 't' || turnType[0] == 'd') {
            break;
        }
        printf("Faulty human input REJECTED! (t)ake or (d)raw?\n");
    }
    
    if (turnType[0] == 't') {
        char cardsTaken[8];
        Cards takeDownTo = 0;

        while(1) {
            printf("Take down to what card?\n");
            fgets(cardsTaken, sizeof(cardsTaken), stdin);
            takeDownTo = Cards_fromString(cardsTaken);

            if (takeDownTo == kSpecialCard || Cards_size(takeDownTo) != 1) {
                printf("Invalid card input. Try harder.\n");
                continue;
            }

            if (!Pile_has(&game->discardPile, takeDownTo)) {
                printf("Card not in discard pile.  Do better.\n");
                continue;
            }

            while (Pile_has(&game->turn.taken, takeDownTo)) {
                Player_take(player, 1);
            }

            if (Pile_size(&game->turn.taken) > 1 &&
                !Cards_has(Meld_playableCards(&game->meld, player->hand), takeDownTo)) {
                printf("Must meld deepest card taken.  You failed.\n");
                Player_undoTake(player);
                continue;
            }

            printf("Took cards: ");
            Cards_print(game->turn.taken.allCards);
            printf("\n");
            break;
        }
    } else {
        Cards drawn = Player_draw(player);
        printf("You drew: ");
        Cards_print(drawn);
        printf("\n");
    }

    Game_printForPlayer(game);

    printf("Enter run cards:\n");
    char runInput[128];
    fgets(runInput, sizeof(runInput), stdin);
    Cards runCards = Cards_fromString(runInput);

    // Determine whether these cards can be played as a run.
    // 

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