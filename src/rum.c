#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
//
//    Card
//

// Cards can be represented as integers (stored in a uint8_t (type Card)
// or as a bit mask (type CardSet).  Clubs are in the range 0-15, diamonds 16-31,
// hearts 32-47, and spades 48-63, though the upper two values are illegal,
// e.g. 14 and 15 of clubs.  Typically, a card represented by integer k is
// also represented by bit mask 1 << k.
//
// Lots of complexity arises from the fact that an ace can be either low or high.
// For an ace in a player's hand, the bit mask representation has BOTH the
// low and high bits set.  When the ace is played in a meld, one of these bits
// is dropped, because the ace is then resolved as either high or low.
// As an integer, an ace can be represented as either the low value (such as 0)
// or the high value (such as 13).
//
// Textually, a "low" ace has a lowercase a, while a "high" ace has an uppercase A.

typedef uint8_t Card;

const char *Card_name_table[64] = {
    "a♣", "2♣", "3♣", "4♣", "5♣", "6♣", "7♣", "8♣", "9♣", "10♣", "J♣", "Q♣", "K♣", "A♣", "14", "15",
    "a♦", "2♦", "3♦", "4♦", "5♦", "6♦", "7♦", "8♦", "9♦", "10♦", "J♦", "Q♦", "K♦", "A♦", "30", "31",
    "a♥", "2♥", "3♥", "4♥", "5♥", "6♥", "7♥", "8♥", "9♥", "10♥", "J♥", "Q♥", "K♥", "A♥", "46", "47",
    "a♠", "2♠", "3♠", "4♠", "5♠", "6♠", "7♠", "8♠", "9♠", "10♠", "J♠", "Q♠", "K♠", "A♠", "62", "63"
};

const int Card_points_table[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, -1, -1 };

typedef enum {
    SUIT_CLUBS = 0,
    SUIT_DIAMONDS = 16,
    SUIT_HEARTS = 32,
    SUIT_SPADES = 48
} Suit_t;

typedef enum {
    VALUE_LOW_ACE = 0,
    VALUE_2,
    VALUE_3,
    VALUE_4,
    VALUE_5,
    VALUE_6,
    VALUE_7,
    VALUE_8,
    VALUE_9,
    VALUE_10,
    VALUE_JACK,
    VALUE_QUEEN,
    VALUE_KING,
    VALUE_ACE
} Value_t;

const char *Card_name(Card card) {
    return Card_name_table[card];
}   

Suit_t Card_suit(Card card) {
    return card & 0x30;
}

Value_t Card_value(Card card) {
    return card & 0x0F;
}

int Card_points(Card card) {
    return Card_points_table[Card_value(card)];
}

bool Card_isLegal(Card card) {
    return Card_value(card) <= VALUE_ACE && Card_suit(card) <= SUIT_SPADES;
}

bool Card_isAce(Card card) {
    Value_t value = Card_value(card);
    return value == VALUE_LOW_ACE || value == VALUE_ACE;
}

Card Card_prevInSuit(Card card) {
    return (Card) (((Card_value(card) + 15) & 0x0F) + Card_suit(card));
}

Card Card_nextInSuit(Card card) {
    return (Card) (((Card_value(card) + 1) & 0x0F) + Card_suit(card));
}

Card Card_prevOfValue(Card card) {
    return (Card) ((card + 48) & 0x3F);
}

Card Card_nextOfValue(Card card) {
    return (Card) ((card + 16) & 0x3F);
}

Card Card_oppositeOfValue(Card card) {
    return (Card) ((card + 32) & 0x3F);
}

void Card_test() {

    Card twoOfClubs = SUIT_CLUBS + VALUE_2;
    assert(strcmp(Card_name(twoOfClubs), "2♣") == 0);
    assert(strcmp(Card_name(twoOfClubs), "a♥") != 0);
    assert(Card_points(twoOfClubs) == 1);
    assert(Card_suit(twoOfClubs) == SUIT_CLUBS);
    assert(Card_value(twoOfClubs) == VALUE_2);
    assert(Card_isLegal(twoOfClubs));
    assert(!Card_isAce(twoOfClubs));
    assert(Card_prevInSuit(twoOfClubs) == (Card) (SUIT_CLUBS + VALUE_LOW_ACE));
    assert(Card_nextInSuit(twoOfClubs) == (Card) (SUIT_CLUBS + VALUE_3));
    assert(Card_prevOfValue(twoOfClubs) == (Card) (SUIT_SPADES + VALUE_2));
    assert(Card_nextOfValue(twoOfClubs) == (Card) (SUIT_DIAMONDS + VALUE_2));
    assert(Card_oppositeOfValue(twoOfClubs) == (Card) (SUIT_HEARTS + VALUE_2));

    Card lowAceOfHearts = SUIT_HEARTS + VALUE_LOW_ACE;
    assert(strcmp(Card_name(lowAceOfHearts), "a♥") == 0);
    assert(strcmp(Card_name(twoOfClubs), "a♥") != 0);
    assert(Card_points(lowAceOfHearts) == 1);
    assert(Card_suit(lowAceOfHearts) == SUIT_HEARTS);
    assert(Card_value(lowAceOfHearts) == VALUE_LOW_ACE);
    assert(Card_isLegal(lowAceOfHearts));
    assert(Card_isAce(lowAceOfHearts));
    assert(!Card_isLegal(Card_prevInSuit(lowAceOfHearts)));
    assert(Card_nextInSuit(lowAceOfHearts) == (Card) (SUIT_HEARTS + VALUE_2));
    assert(Card_prevOfValue(lowAceOfHearts) == (Card) (SUIT_DIAMONDS + VALUE_LOW_ACE));
    assert(Card_nextOfValue(lowAceOfHearts) == (Card) (SUIT_SPADES + VALUE_LOW_ACE));
    assert(Card_oppositeOfValue(lowAceOfHearts) == (Card) (SUIT_CLUBS + VALUE_LOW_ACE));
}

//////////////////////////////////////////////////////////////////////
//
//    CardSet
//

typedef uint64_t CardSet;

bool CardSet_has(CardSet *set, Card card) {
    return (*set & (1ULL << card)) != 0;
}

void CardSet_add(CardSet *set, Card card) {
    *set |= (1ULL << card);
}

void CardSet_remove(CardSet *set, Card card) {
    *set &= ~(1ULL << card);
}

int CardSet_points(CardSet *set) {
    int points = 0;
    for (Card card = 0; card < 52; ++card) {
        if (CardSet_has(set, card)) {
            points += Card_points(card);
        }
    }
    return points;
}

//////////////////////////////////////////////////////////////////////
//
//    CardList
//

typedef struct {
    Card cards[52];
    int size;
} CardList;

CardList *CardList_new() {
    CardList *cardList = malloc(sizeof(CardList));
    cardList->size = 0;
    return cardList;
}

void CardList_free(CardList *cardList) {
    free(cardList);
}

void CardList_push(CardList *cardList, Card card) {
    cardList->cards[cardList->size++] = card;
}

Card CardList_pop(CardList *cardList) {
    return cardList->cards[--cardList->size];
}

void CardList_shuffle(CardList *cardList) {
    for (int i = cardList->size - 1; i > 0; --i) {
        int j = arc4random_uniform(i + 1);
        Card temp = cardList->cards[i];
        cardList->cards[i] = cardList->cards[j];
        cardList->cards[j] = temp;
    }
}

void CardList_print(CardList *cardList) {
    for (int i = 0; i < cardList->size; ++i) {
        printf("%s ", Card_name(cardList->cards[i]));
    }
}

//////////////////////////////////////////////////////////////////////
//
//    Meld
//


typedef struct PlayerStruct Player;
typedef struct GameStruct Game;

struct PlayerStruct {
    Game *game;
    int index;
    CardSet hand;
    int points;
};

struct GameStruct {
    CardList drawPile;
    CardList discardPile;
    CardSet discarded;
    Player players[4];
    int numPlayers;
    int currentPlayer;
};

Game *newGame(int numPlayers) {
    Game *game = malloc(sizeof(Game));
    return NULL;
}

int main(int argc, char **argv) {
    Card_test();


    /*
    CardList *cl = new_CardList();
    for (int i = 0; i < 52; ++i) {
        CardList_push(cl, i);
    }
    CardList_shuffle(cl);
    
    CardList_print(cl);
    printf("\n");

    CardList_free(cl);
    return 0;
    */
}