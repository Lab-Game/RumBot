#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cards.h"
#include "pile.h"
#include "table.h"
#include "plays.h"
#include "playgen.h"
#include "game.h"
#include "ai.h"

void Cards_test(void) {
    puts("\nTesting Cards...");
    Cards cards = Cards_fromString("aC TC 5D 6D 2H JH 6S KS AS");
    printf("Expected:  aC TC 5D 6D 2H JH 6S KS AS\n");
    printf("Actual:    ");
    Cards_print(cards);
    printf("\n");
    assert(Cards_points(cards) == 70);

    // Remove a card and check points again.
    Cards_remove(&cards, Cards_fromString("aC")); // remove aC
    printf("Expected:  TC 5D 6D 2H JH 6S KS AS\n");
    printf("Actual:    ");
    Cards_print(cards);
    printf("\n");
    assert(Cards_points(cards) == 65);

    // Add some cards and check points again.
    Cards_add(&cards, Cards_fromString("JC QC")); // add JC QC
    printf("Expected:  TC JC QC 5D 6D 2H JH 6S KS AS\n");
    printf("Actual:    ");
    Cards_print(cards);
    printf("\n");
    assert(Cards_points(cards) == 85);

    // Display the full deck.
    Cards fullDeck = FULL_DECK;
    printf("Expected:  2C 3C 4C 5C 6C 7C 8C 9C TC JC QC KC AC 2D 3D 4D 5D 6D 7D 8D 9D TD JD QD KD AD 2H 3H 4H 5H 6H 7H 8H 9H TH JH QH KH AH 2S 3S 4S 5S 6S 7S 8S 9S TS JS QS KS AS\n");
    printf("Actual:    ");
    Cards_print(fullDeck);
    printf("\n");

    // Test adding low aces.
    Cards handWithHighAces = Cards_fromString("AC AD AH AS 5H 6H");
    Cards handWithLowAces = Cards_addLowAces(handWithHighAces);
    printf("Expected with low aces:  aC AC aD AD aH 5H 6H AH aS AS\n");
    printf("Actual with low aces:    ");
    Cards_print(handWithLowAces);
    printf("\n");

    // Test iteration over a Cards set.
    Cards hand = Cards_fromString("TC JC QC 5D 6D 2H JH 6S KS AS");
    int numCards = 0;
    for (Cards c = Cards_first(hand); c != 0; c = Cards_next(hand, c)) {
        assert(Cards_has(hand, c));
        numCards++;
    }
    assert(numCards == Cards_size(hand));
}

void Pile_test(void) {
    puts("\nTesting Pile...");
    Pile pile;
    Pile_init(&pile);
    assert(Pile_size(&pile) == 0);
    Pile_fullDeck(&pile);
    printf("Expected:  2C 3C 4C 5C 6C 7C 8C 9C TC JC QC KC AC 2D 3D 4D 5D 6D 7D 8D 9D TD JD QD KD AD 2H 3H 4H 5H 6H 7H 8H 9H TH JH QH KH AH 2S 3S 4S 5S 6S 7S 8S 9S TS JS QS KS AS\n");
    printf("Actual:    ");
    Pile_print(&pile);
    printf("\n");
    assert(pile.size == 52);
    for (int i = 0; i < 52; ++i) {
        Cards card = Pile_pop(&pile);
        assert(Cards_size(card) == 1);
    }
    assert(Pile_size(&pile) == 0);
    printf("Expected:  (no cards)\n");
    printf("Actual:    ");
    Pile_print(&pile);
    printf("\n");
    Pile_push(&pile, Cards_fromString("KD"));
    Pile_push(&pile, Cards_fromString("QS"));
    Pile_push(&pile, Cards_fromString("7H"));
    Pile_push(&pile, Cards_fromString("9H"));
    Pile_push(&pile, Cards_fromString("8C"));
    printf("Expected:  KD QS 7H 9H 8C\n");
    printf("Actual:    ");
    Pile_print(&pile);
    printf("\n");
    assert(Pile_size(&pile) == 5);
}

void Table_test(void) {
    puts("\nTesting Table...");
    Table table;
    Table_init(&table);
    assert(table.runs == 0);
    assert(table.sets == 0);

    Table_addRun(&table, Cards_fromString("TH JH QH"));
    Table_addRun(&table, Cards_fromString("4C 5C 6C 7C"));
    Table_addSet(&table, Cards_fromString("7H 7C 7D"));
    Table_addSet(&table, Cards_fromString("JC JD JH JS"));
    printf("Expected:\nRuns: 4C 5C 6C 7C, TH JH QH\nSets: 7C 7D 7H, JC JD JH JS\nActual:\n");
    Table_print(&table);
    assert(table.runs == Cards_fromString("4C 5C 6C 7C TH JH QH"));
    assert(table.sets == Cards_fromString("7C 7D 7H JC JD JH JS"));

    Table_removeRun(&table, Cards_fromString("TH JH QH"));
    Table_removeSet(&table, Cards_fromString("7C 7D 7H"));
    Table_addSet(&table, Cards_fromString("QC QH QS"));
    printf("Expected:\nRuns: (no cards)\nSets: JC JD JH JS, QC QH QS\nActual:\n");
    Table_print(&table);
    assert(Cards_size(table.runs) == 4);
    assert(Cards_size(table.sets) == 7);
}

void PlayGen_test(void) {
    puts("\nTesting PlayGen...");
    Table table;
    Table_init(&table);
    Cards hand = Cards_fromString("9H TH JH QC QH QS 5D 6D 7D 8D");
    PlayGen_generate(hand, &table);
}

void Game_test(void) {
    puts("\nTesting Game...");
    Game game;
    Game_init(&game);
    assert(game.numPlayers == NUM_PLAYERS);
    assert(game.currentPlayer == 0);
    assert(Pile_size(&game.drawPile) == 52 - NUM_PLAYERS * 7 - 1);
    assert(Pile_size(&game.discardPile) == 1);
    assert(game.table.runs == 0);
    assert(game.table.sets == 0);
    assert(game.discarded == 0);
    for (int i = 0; i < game.numPlayers; ++i) {
        Player *player = Game_player(&game, i);
        assert(player->game == &game);
        assert(player->id == i);
        assert(player->score == 0);
        assert(Cards_size(player->hand) == 7);
    }
    Game_print(&game);
}

void AI_test(void) {
    puts("\nTesting AI...");
    Game game;
    Game_init(&game);
    AI_go(&game);

    Player *player = Game_currentPlayer(&game);
    Turn_print(&player->turn);
}   

int main(void) {
    Cards_test();
    Pile_test();
    Table_test();
    PlayGen_test();
    Game_test();
    AI_test();
    printf("\nAll tests passed.\n");
    return 0;
}