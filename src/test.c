#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cards.h"
#include "pile.h"
#include "meld.h"
#include "meldlist.h"
#include "game.h"
#include "ai.h"
#include "ai-shallow.h"
#include "ai-deep.h"
#include "player.h"

int DEB = 2;  // Debug level (0=none, 1=some, 2=more, 3=lots)
int POV = 0;  // Print debug from current player's point of view

void Card_test(void) {
    printf("Testing Card... ");

    int numLegal = 0;
    int numSuit[4] = { 0, 0, 0, 0 };
    int numValue[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int numRed = 0;
    int numBlack = 0;
    for (int i = 0; i < 255; ++i) {
        Card c = (Card) i;
        if (Card_isLegal(c)) {
            numLegal++;

            int value = Card_value(c);
            int suit = Card_suit(c);

            assert(value >= 0 && value <= 13);
            assert(suit >= 0 && suit <= 3);

            numValue[value]++;
            numSuit[suit]++;

            Card d = Card_fromValueSuit(value, suit);
            assert(c == d);

            const char *name = Card_name((Card) i);
            assert(name[0] == kCardValues[value]);
            assert(name[1] == kCardSuits[suit]);

            if (Card_isRed(c)) {
                numRed++;
                assert(suit == kDiamondSuit || suit == kHeartSuit);
                assert(name[1] == 'D' || name[1] == 'H');
            }
            if (Card_isBlack(c)) {
                numBlack++;
                assert(suit == kClubSuit || suit == kSpadeSuit);
                assert(name[1] == 'C' || name[1] == 'S');
            }
        }
    }

    // There are 56 legal cards (including low aces).
    assert(numLegal == 56);

    // There are 14 cards of each suit.
    for (int i = 0; i < 4; ++i) {
        assert(numSuit[i] == 14);
    }

    // There are 4 cards of each value.
    for (int i = 0; i < 14; ++i) {
        assert(numValue[i] == 4);
    }

    // There are 28 cards of each color.
    assert(numRed == 28);
    assert(numBlack == 28);

    printf("passed.\n");
}

void Cards_test(void) {
    puts("\nTesting Cards...");
    Cards cards = Cards_fromString("aC TC 5D 6D 6S KS AS 2H JH");
    printf("Expected:  aC TC 5D 6D 6S KS AS AH JH\n");
    printf("Actual:    ");
    Cards_print(cards);
    printf("\n");
    assert(Cards_points(cards) == 70);

    // Remove a card and check points again.
    Cards_remove(&cards, Cards_fromString("aC")); // remove aC
    printf("Expected:  TC 5D 6D 6S KS AS 2H JH\n");
    printf("Actual:    ");
    Cards_print(cards);
    printf("\n");
    assert(Cards_points(cards) == 65);

    // Add some cards and check points again.
    Cards_add(&cards, Cards_fromString("JC QC")); // add JC QC
    printf("Expected:  TC JC QC 5D 6D 6S KS AS 2H JH\n");
    printf("Actual:    ");
    Cards_print(cards);
    printf("\n");
    assert(Cards_points(cards) == 85);

    // Test adding low aces.
    Cards handWithHighAces = Cards_fromString("AC AD AH AS 5H 6H");
    Cards handWithLowAces = Cards_addLowAces(handWithHighAces);
    printf("Expected:  aC AC aD AD aS AS aH 5H 6H AH\n");
    printf("Actual:    ");
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
    Pile_push(&pile, kSpecialCard);
    assert(Pile_size(&pile) == 1);
    assert(pile.allCards == kSpecialCard);
    assert(Cards_has(pile.allCards, kSpecialCard));

    Pile_init(&pile);
    assert(Pile_size(&pile) == 0);
    assert(pile.allCards == 0);

    Pile_fullDeck(&pile);
    assert(Pile_size(&pile) == 52);
    assert(pile.allCards == kFullDeck);

    Pile_shuffle(&pile);
    assert(Pile_size(&pile) == 52);
    assert(pile.allCards == kFullDeck);

    assert(pile.size == 52);
    for (int i = 0; i < 52; ++i) {
        Cards card = Pile_pop(&pile);
        assert(Cards_size(card) == 1);
    }
    assert(Pile_size(&pile) == 0);
    assert(pile.allCards == 0);

    Pile_push(&pile, Cards_fromString("KD"));
    Pile_push(&pile, Cards_fromString("QS"));
    Pile_push(&pile, Cards_fromString("7H"));
    Pile_push(&pile, Cards_fromString("9H"));
    Pile_push(&pile, Cards_fromString("8C"));
    assert(Pile_size(&pile) == 5);
    assert(pile.allCards == Cards_fromString("KD QS 7H 9H 8C"));

    Cards peeked = Pile_peek(&pile);
    assert(peeked == Cards_fromString("8C"));

    Cards popped = Pile_pop(&pile); // remove 8C
    assert(popped == Cards_fromString("8C"));
    assert(Pile_size(&pile) == 4);
    assert(pile.allCards == Cards_fromString("KD QS 7H 9H"));
}

void Meld_test(void) {
    puts("\nTesting Meld...");
    Meld meld;
    Meld_init(&meld);
    assert(meld.runs == 0);
    assert(meld.sets == 0);

    Meld_addRun(&meld, Cards_fromString("TH JH QH"));
    Meld_addRun(&meld, Cards_fromString("4C 5C 6C 7C"));
    Meld_addSet(&meld, Cards_fromString("7H 7C 7D"));
    Meld_addSet(&meld, Cards_fromString("JC JD JH JS"));
    printf("Expected:\nRuns: 4C 5C 6C 7C, TH JH QH\nSets: 7C 7D 7H, JC JD JH JS\nActual:\n");
    Meld_print(&meld);
    assert(meld.runs == Cards_fromString("4C 5C 6C 7C TH JH QH"));
    assert(meld.sets == Cards_fromString("7C 7D 7H JC JD JH JS"));

    Meld_removeRun(&meld, Cards_fromString("TH JH QH"));
    Meld_removeSet(&meld, Cards_fromString("7C 7D 7H"));
    Meld_addSet(&meld, Cards_fromString("QC QH QS"));
    printf("Expected:\nRuns: (no cards)\nSets: JC JD JH JS, QC QH QS\nActual:\n");
    Meld_print(&meld);
    assert(Cards_size(meld.runs) == 4);
    assert(Cards_size(meld.sets) == 7);
}

void MeldList_test(void) {
    puts("\nTesting MeldList...");

    MeldList list;

    Meld tableMeld;
    Meld_init(&tableMeld);
    Meld_addRun(&tableMeld, Cards_fromString("aC 2C 3C"));
    Meld_addSet(&tableMeld, Cards_fromString("TD TC TS"));
    Cards hand = Cards_fromString("4C 5C 7C 8C 9H TH JH QH 8D JD JC JS");
    Cards mustMeld = Cards_fromString("JC");
    MeldList_generate(&list, hand, &tableMeld, mustMeld);
    MeldList_print(&list);
    assert(list.size == MELDLIST_MAX_SIZE || list.size == 33);

    Meld_init(&tableMeld);
    hand = Cards_fromString("2H 3H 4H 5H 6H 7H 8H 9H TH JH QH KH AH");
    mustMeld = Cards_fromString("7H");
    MeldList_generate(&list, hand, &tableMeld, mustMeld);
    assert(list.size == MELDLIST_MAX_SIZE || list.size == 258);
}

void Game_test(void) {
    puts("\nTesting Game...");
    Game game;
    Game_init(&game);
    Game_deal(&game);

    assert(game.numPlayers == NUM_PLAYERS);
    assert(game.currentPlayerId == 0);
    assert(Pile_size(&game.drawPile) == 52 - NUM_PLAYERS * 7 - 1);
    assert(Pile_size(&game.discardPile) == 1);
    assert(game.meld.runs == 0);
    assert(game.meld.sets == 0);
    assert(game.everDiscarded == Pile_peek(&game.discardPile));
    for (int i = 0; i < game.numPlayers; ++i) {
        Player *player = Game_player(&game, i);
        assert(player->game == &game);
        assert(player->id == i);
        assert(player->score == 0);
        assert(Cards_size(player->hand) == 7);
    }
    Game_print(&game);
}

int main(void) {
    Card_test();
    Cards_test();
    Pile_test();
    Meld_test();
    MeldList_test();
    Game_test();
    printf("\nAll tests passed.\n");
    return 0;
}