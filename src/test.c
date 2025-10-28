#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cards.h"
#include "pile.h"
#include "meld.h"
#include "plays.h"
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
    Cards handWithLowAces = Cards_lowerAces(handWithHighAces);
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

void Meld_test(void) {
    puts("\nTesting Meld...");
    Meld Meld;
    Meld_init(&Meld);
    assert(Meld.runs == 0);
    assert(Meld.sets == 0);

    Meld_addRun(&Meld, Cards_fromString("TH JH QH"));
    Meld_addRun(&Meld, Cards_fromString("4C 5C 6C 7C"));
    Meld_addSet(&Meld, Cards_fromString("7H 7C 7D"));
    Meld_addSet(&Meld, Cards_fromString("JC JD JH JS"));
    printf("Expected:\nRuns: 4C 5C 6C 7C, TH JH QH\nSets: 7C 7D 7H, JC JD JH JS\nActual:\n");
    Meld_print(&Meld);
    assert(Meld.runs == Cards_fromString("4C 5C 6C 7C TH JH QH"));
    assert(Meld.sets == Cards_fromString("7C 7D 7H JC JD JH JS"));

    Meld_removeRun(&Meld, Cards_fromString("TH JH QH"));
    Meld_removeSet(&Meld, Cards_fromString("7C 7D 7H"));
    Meld_addSet(&Meld, Cards_fromString("QC QH QS"));
    printf("Expected:\nRuns: (no cards)\nSets: JC JD JH JS, QC QH QS\nActual:\n");
    Meld_print(&Meld);
    assert(Cards_size(Meld.runs) == 4);
    assert(Cards_size(Meld.sets) == 7);
}

void Game_test(void) {
    puts("\nTesting Game...");
    Game game;
    Game_init(&game);
    assert(game.numPlayers == NUM_PLAYERS);
    assert(game.currentPlayer == 0);
    assert(Pile_size(&game.drawPile) == 52 - NUM_PLAYERS * 7 - 1);
    assert(Pile_size(&game.discardPile) == 1);
    assert(game.Meld.runs == 0);
    assert(game.Meld.sets == 0);
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
    Player *player = Game_currentPlayer(&game);
    player->hand = Cards_fromString("3C QC 8D 9D 2H QS AS 4H KS");

    AI ai;
    AI_init(&ai, &game, Game_currentPlayer(&game));
    AI_generateMelds(&ai);
    assert(ai.numMelds == 2);

    player->hand = Cards_fromString("4C 4D TD AD 6H 7H 2S 3S 7S AS");
    AI_init(&ai, &game, Game_currentPlayer(&game));
    AI_generateMelds(&ai);
    assert(ai.numMelds == 2);

    player->hand = Cards_fromString("2C 9C TC QC AC 4D 3H 6H 9H TH 8S JS KS AS 3C");
    AI_init(&ai, &game, Game_currentPlayer(&game));
    AI_generateMelds(&ai);
    assert(ai.numMelds == 2);
}

int main(void) {
    Cards_test();
    Pile_test();
    Meld_test();
    Game_test();
    AI_test();
    printf("\nAll tests passed.\n");
    return 0;
}