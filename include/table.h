#ifndef TABLE_H
#define TABLE_H

#include "cards.h"

typedef struct TableStruct {
    Cards runs;
    Cards sets;
} Table;

static inline Table *Table_clear(Table *table) {
    table->runs = 0;
    table->sets = 0;
    return table;
}

static inline void Table_addRun(Table *table, Cards meld) {
    table->runs |= meld;
}

static inline void Table_removeRun(Table *table, Cards meld) {
    table->runs &= ~meld;
}

static inline void Table_addSet(Table *table, Cards meld) {
    table->sets |= meld;
}

static inline void Table_removeSet(Table *table, Cards meld) {
    table->sets &= ~meld;
}

void Table_print(Table *table);

#endif // TABLE_H