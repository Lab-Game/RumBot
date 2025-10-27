#include <stdio.h>

#include "plays.h"

void Plays_print(Plays *plays) {
    // Print the contents of the Plays structure compactly.
    for (Cards c = Cards_first(plays->runCenters); c != 0; c = Cards_next(plays->runCenters, c)) {
        Cards meld = Plays_runCenterToCards(c);
        printf("<");
        Cards_print(meld);
        printf("> ");
    }

    for (Cards c = Cards_first(plays->runExtensions); c != 0; c = Cards_next(plays->runExtensions, c)) {
        printf("<");
        Cards_print(c);
        printf("> ");
    }

    for (Cards c = Cards_first(plays->setCenters); c != 0; c = Cards_next(plays->setCenters, c)) {
        Cards meld = Plays_setCenterToCards(c);
        printf("{");
        Cards_print(meld);
        printf("} ");
    }
 
    for (Cards c = Cards_first(plays->setExtensions); c != 0; c = Cards_next(plays->setExtensions, c)) {
        printf("{");
        Cards_print(c);
        printf("} ");
    }
}