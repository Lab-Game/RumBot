#include <stdio.h>
#include "table.h"

void Table_print(Table *table) {
    printf("Runs: ");
    Cards_print(table->runs);
    printf("\nSets: ");
    Cards_print(table->sets);
    printf("\n");
}
