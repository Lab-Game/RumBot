#include <stdio.h>

#include "cards.h"

int main(void) {
    Cards cards = Cards_fromString("aC TC 5D 6D 2H JH 6S KS AS");
    Cards_print(cards);
    printf("\n");
    return 0;
}