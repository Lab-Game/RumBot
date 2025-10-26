#ifndef TABLE_H
#define TABLE_H

#include "cards.h"

// A Table holds the current melds (runs and sets) played on the table.

typedef struct TableStruct {
    Cards runs;
    Cards sets;
} Table;

// Initialize the Table to be empty.
static inline Table *Table_init(Table *table) {
    table->runs = 0;
    table->sets = 0;
    return table;
}

// Add a run of cards to the Table.  Could be a new run or an extension.
static inline void Table_addRun(Table *table, Cards meld) {
    table->runs |= meld;
}

// Remove a run of cards from the Table.
static inline void Table_removeRun(Table *table, Cards meld) {
    table->runs &= ~meld;
}

// Add a set of cards to the Table.  Could be a new set or an extension.
static inline void Table_addSet(Table *table, Cards meld) {
    table->sets |= meld;
}

// Remove a set of cards from the Table.
static inline void Table_removeSet(Table *table, Cards meld) {
    table->sets &= ~meld;
}

// Print the runs and sets currently on the Table.
void Table_print(Table *table);

#endif // TABLE_H