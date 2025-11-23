#include <stdlib.h>
#include <stdio.h>

#include "log.h"

Log *Log_init(int viewpoint) {
    Log *log = malloc(sizeof(Log));
    log->viewpoint = viewpoint;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        log->initialHand[i] = 0;
    }
    log->head = NULL;
    return log;
}

void Log_setInitialHand(Log *log, int playerId, Cards hand) {
    log->initialHand[playerId] = hand;
}

void Log_free(Log *log) {
    LogEntry *entry = log->head;
    while (entry != NULL) {
        LogEntry *next = entry->next;
        free(entry);
        entry = next;
    }
    free(log);
}

void Log_addEntry(Log *log, Turn *turn) {
    LogEntry *entry = malloc(sizeof(LogEntry));
    entry->numTaken = turn->taken.size;
    entry->drawn = turn->drawn;
    entry->meld = turn->meld;
    entry->discard = turn->discard;
    entry->next = log->head;
    log->head = entry;
}

void Log_applyEntry(Game *game, LogEntry *entry) {
    // Carry out actions in the log entry on the game state.
    Player *player = game->currentPlayer;
    // Handle draw or take
    if (entry->drawn) {
        // Modify the game so that the drawn card is on top of the draw pile,
        // if we know what it is.
        if (entry->drawn != 0) {
            Cards swapped = Game_swapToTop(game, entry->drawn);
        }

        // Draw a card
        Cards drawnCard = Player_draw(player);
    } else {
        // Take one or more cards from discard pile
        for (int i = 0; i < entry->numTaken; ++i) {
            Player_take(player);
        }
    }

    // Handle meld
    Player_meld(player, &entry->meld);

    // Handle discard
    if (entry->discard) {
        Player_discard(player, entry->discard);
    }
}

void Log_print(Log *log) {
    printf("Log (viewpoint: %d):\n", log->viewpoint);
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        printf("  Initial hand for player %d: ", i);
        Cards_print(log->initialHand[i]);
        printf("\n");
    }
    LogEntry *entry = log->head;
    int turnNum = 0;
    while (entry != NULL) {
        printf("  Turn %d: ", turnNum);
        printf("Taken: %d, Drawn: ", entry->numTaken);
        Cards_print(entry->drawn);
        printf(", Meld: ");
        Meld_printCompact(&entry->meld);
        printf(", Discard: ");
        Cards_print(entry->discard);
        printf("\n");
        entry = entry->next;
        turnNum++;
    }
}