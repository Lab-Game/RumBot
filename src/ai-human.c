#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "game.h"
#include "player.h"
#include "ai.h"
#include "ai-human.h"

void HumanAI_beginTurn(AI *ai) {
    (void) ai;
}

void HumanAI_endTurn(AI *ai) {
    (void) ai;
}

bool HumanAI_takeTurn(AI *ai, Turn *turn) {
    Game *game = ai->game;
    Player *player = ai->player;

    assert(!game->isOver);
    assert(game->currentPlayer == player);

    // Determine whether this is a draw or take.
    char turnTypeInput[8];
    while(1) {
        printf("(t)ake or (d)raw? ");
        fgets(turnTypeInput, sizeof(turnTypeInput), stdin);
        if (turnTypeInput[0] == 't' || turnTypeInput[0] == 'd') {
            break;
        }
    }

    // Handle a take turn
    Cards mustMeld = 0;
    if (turnTypeInput[0] == 't') {
        char takeInput[8];

        while(1) {
            printf("take how many (1-%d)? ", Pile_size(&game->discardPile));
            fgets(takeInput, sizeof(takeInput), stdin);
            int numTaken = atoi(takeInput);
            if (numTaken < 1 || numTaken > Pile_size(&game->discardPile)) {
                continue;
            }

            // If more than one card is taken, the player must play the deepest card.
            if (numTaken > 1) {
                mustMeld = game->discardPile.cards[game->discardPile.size - numTaken];
            }

            Player_take(player, numTaken);

            // Ensure that the deepest card can be played.
            if (!Cards_has(Meld_playableCards(&game->meld, player->hand), mustMeld)) {
                printf("can't play deepest card\n");
                Player_undoTake(player);
                continue;
            }

            // Complete the meld and discard.
            HumanAI_meldAndDiscard(ai, mustMeld);

            // Record the turn.
            *turn = game->turn;

            // Unwind the player's actions so the main loop can reapply them.
            Player_undoDiscard(player);
            Player_undoMeld(player, &turn->meld);
            Player_undoTake(player);
            
            return true;
        }
    } else {
        return false;
    }
}

void HumanAI_drawTurn(AI *ai, Cards drawCard, Turn *turn) {
    Game *game = ai->game;
    Player *player = ai->player;

    assert(!game->isOver);
    assert(game->currentPlayer == player);

    Cards drawn = Player_draw(player);
    assert (drawn == drawCard);
    printf("drew ");
    Cards_print(drawn);
    printf("\n");

    // Complete the meld and discard.
    HumanAI_meldAndDiscard(ai, 0);

    // Record the turn.
    *turn = game->turn;

    // Unwind the player's actions so the main loop can reapply them.
    Player_undoDiscard(player);
    Player_undoMeld(player, &turn->meld);
    Player_undoDraw(player);
}

void HumanAI_meldAndDiscard(AI *ai, Cards mustMeld) {
    Game *game = ai->game;
    Player *player = ai->player;

    assert(!game->isOver);
    assert(game->currentPlayer == player);

    // Show the game again after drawing/taking.
    Game_printForPlayer(game);

    //  Handle playing runs.
    Cards playableInRun = Meld_playableInRun(game->meld.runs, player->hand);
    while(1) {
        printf("run meld? ");
        char runInput[128];
        fgets(runInput, sizeof(runInput), stdin);
        Cards runCards = Cards_fromString(runInput);
        if (!Cards_has(player->hand, runCards) || !Cards_has(playableInRun, runCards)) {
            continue;
        }
        Player_playRun(player, runCards);

        // If the mustMeld card is in the player's hand and is not playable after this meld, undo the meld.
        if (Cards_has(player->hand, mustMeld) &&
            !Cards_has(Meld_playableCards(&game->meld, mustMeld), mustMeld)) {
            printf("must play deepest card\n");
            Player_undoRun(player, runCards);
            continue;
        }
    }

    // Handle playing sets.
    Cards playableInSet = Meld_playableInSet(game->meld.sets, player->hand);
    while(1) {
        printf("set meld? ");
        char setInput[128];
        fgets(setInput, sizeof(setInput), stdin);
        Cards setCards = Cards_fromString(setInput);
        if (!Cards_has(player->hand, setCards) || !Cards_has(playableInSet, setCards)) {
            continue;
        }
        Player_playSet(player, setCards);
        if (Cards_has(player->hand, mustMeld)) {
            printf("must play deepest card\n");
            Player_undoSet(player, setCards);
            continue;
        }
    }

    // Handle discarding.
    if (Cards_size(player->hand) == 0) {
        printf("no discard\n");
    } else {
        while(1) {
            printf("discard? ");
            char discardInput[8];
            fgets(discardInput, sizeof(discardInput), stdin);
            Cards discardCard = Cards_fromString(discardInput);
            if (!Cards_has(player->hand, discardCard) || Cards_size(discardCard) != 1) {
                printf("invalid discard\n");
                continue;
            }
            Player_discard(player, discardCard);
            break;
        }
    }
}