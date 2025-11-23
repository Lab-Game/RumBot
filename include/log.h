#ifndef LOG_H
#define LOG_H

#include "cards.h"
#include "meld.h"
#include "turn.h"
#include "game.h"

// A log records the sequence of plays made during a game,
// either from the perspective of a single player or from
// an omniscient viewpoint.

typedef struct LogEntryStruct {
    int numTaken;
    Cards drawn;  // 0 if unknown to player
    Meld meld;
    Cards discard;
    struct LogEntryStruct *next;
} LogEntry;

typedef struct LogStruct {
    int viewpoint;  // -1 = omniscient, else player id
    Cards initialHand[NUM_PLAYERS];  // only cards known to viewpoint player
    Cards initialDiscard;
    LogEntry *head;
} Log;

Log *Log_init(int viewpoint);
void Log_setInitialHand(Log *log, int playerId, Cards hand);
void Log_free(Log *log);
void Log_addEntry(Log *log, Turn *Turn);
void Log_applyEntry(Game *game, LogEntry *entry);
void Log_print(Log *log);

// File format is:
// <viewpoint>
//
// If the viewpoint is -1, then hands are provided for each player on separate lines.
// Otherwise, only the viewpoint player's hand is provided.
//
// A hand is a string such as 8H TC AS, etc.
//
// The initial card in the discard pile is listed next on a separate line.
//
// Then, each log entry is on a separate line, either:
//   T <numTaken> <meld> <discard>
//   D <drawn> <meld> <discard>
//
// Here, <meld> is a sequence of runs in angle brackets and sets in curly brackets.
// For example:  <8H 9H TH> {AS AD AC} <3C 4C 5C>
//
// Discard is a single card such as 7D, or "0" for no discard.

void Log_writeToFile(Log *log, const char *filename);
Log *Log_readFromFile(const char *filename);

#endif // LOG_H
