#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////
//
//    Card
//

typedef uint8_t Card;

char *CardName[52] = {
    "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "TC", "JC", "QC", "KC", "AC", 
    "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "TD", "JD", "QD", "KD", "AD",
    "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "TH", "JH", "QH", "KH", "AH",
    "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "TS", "JS", "QS", "KS", "AS",
};

int Points[14] = { 5, 5, 5, 5, 5, 5, 5, 5, 10, 10, 10, 10, 15 };

typedef enum {
    SUIT_CLUBS = 0,
    SUIT_DIAMONDS = 13,
    SUIT_HEARTS = 26,
    SUIT_SPADES = 39
} Suit_t;

typedef enum {
    VALUE_2 = 0,
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

Suit_t Card_suit(Card card) {
    return card / 13 * 13;
}

Value_t Card_value(Card card) {
    return card % 13;
}

bool Card_isAce(Card card) {
    return Card_value(card) == VALUE_ACE;
}

int Card_points(Card card) {
    return Points[Card_value(card)];
}

Card Card_prevInSuit(Card card) {
    return (Card) ((Card_value(card) + 12) % 13 + Card_suit(card));
}

Card Card_nextInSuit(Card card) {
    return (Card) ((Card_value(card) + 1) % 13 + Card_suit(card));
}   

Card Card_prevOfValue(Card card) {
    return (Card) ((Card_value(card) + 39) % 52);
}

Card Card_nextOfValue(Card card) {
    return (Card) ((Card_value(card) + 13) % 52);
}

Card Card_oppositeColor(Card card) {
    return (Card) ((card + 26) % 52);
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
        printf("%s ", CardName[cardList->cards[i]]);
    }
}

//////////////////////////////////////////////////////////////////////
//
//    Meld
//

// A Meld is a set of cards that can be played together or appended
// to a previously-played meld.  Here are the operations I need:
//
//    - Given a hand of cards (a CardSet) and a set of melds that
//      have already been played, what melds can be played from
//      this hand?
//    - Remove cards from the hand when I play the Meld, and add
//      cards back to the hand when I take them back.
//    - What is the point value of the meld?
//
// In general, there are triple-Melds (both runs and sets), which
// can always be played if the cards are in-hand.
// And there are single-Melds (again, both run and set), which
// depend on some already-played cards.
//
// A basic question is:  what is a Meld?  For example, is it an
// integer or a CardSet?  I think I might use integers, because
// I can then associate a lot of values via an array.  Values of
// interest include:
//
//    - A CardSet of meld members
//    - The point value
//    - Meld # of LeftExtension (for a run)
//    - Meld # of RightExtension (for a run)
//    - Meld # of SetCompletion (for a set)
//
// There are sort of two parts for finding playable Melds:
//    - Given a hand, find the triple-melds with all required cards.
//    - Given a table of already-played meld, find the single-card
//      melds that can be added.
//
// Aces and wraparound are always the hard part!
//
// With bit-twiddling, I guess I can compute:
//    - All single-card set completions.  Values are simple.
//    - All single-card upward run extensions (ace = 15 pts)
//    - All single-card downward run extensions (ace = 5 pts)
//    - All three-card runs (is value tricky?)
//    - All three-cards sets

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
    CardList *cl = new_CardList();
    for (int i = 0; i < 52; ++i) {
        CardList_push(cl, i);
    }
    CardList_shuffle(cl);
    
    CardList_print(cl);
    printf("\n");

    CardList_free(cl);
    return 0;
}