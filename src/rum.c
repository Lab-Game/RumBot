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
// As an integer, an ace can be represented as either the low value (such as 0)
// or the high value (such as 13).
//
// Textually, a "low" ace has a lowercase a, while a "high" ace has an uppercase A.

typedef uint8_t Card;

const char *Card_nameTable[64] = {
    "a♣", "2♣", "3♣", "4♣", "5♣", "6♣", "7♣", "8♣", "9♣", "10♣", "J♣", "Q♣", "K♣", "A♣", "14", "15",
    "a♦", "2♦", "3♦", "4♦", "5♦", "6♦", "7♦", "8♦", "9♦", "10♦", "J♦", "Q♦", "K♦", "A♦", "30", "31",
    "a♥", "2♥", "3♥", "4♥", "5♥", "6♥", "7♥", "8♥", "9♥", "10♥", "J♥", "Q♥", "K♥", "A♥", "46", "47",
    "a♠", "2♠", "3♠", "4♠", "5♠", "6♠", "7♠", "8♠", "9♠", "10♠", "J♠", "Q♠", "K♠", "A♠", "62", "63"
};

const int Card_pointsTable[16] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 10, 10, 10, 10, 15, -1, -1 };

const char *Card_suitSymbol[4] = { "♣", "♦", "♥", "♠" };

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
    return Card_nameTable[card];
}   

Suit_t Card_suit(Card card) {
    return card & 0x30;
}

Value_t Card_value(Card card) {
    return card & 0x0F;
}

int Card_points(Card card) {
    return Card_pointsTable[Card_value(card)];
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
    Card twoOfClubs = VALUE_2 + SUIT_CLUBS;
    assert(strcmp(Card_name(twoOfClubs), "2♣") == 0);
    assert(strcmp(Card_name(twoOfClubs), "a♥") != 0);
    assert(Card_points(twoOfClubs) == 5);
    assert(Card_suit(twoOfClubs) == SUIT_CLUBS);
    assert(Card_value(twoOfClubs) == VALUE_2);
    assert(Card_isLegal(twoOfClubs));
    assert(!Card_isAce(twoOfClubs));
    assert(Card_prevInSuit(twoOfClubs) == (Card) (SUIT_CLUBS + VALUE_LOW_ACE));
    assert(Card_nextInSuit(twoOfClubs) == (Card) (SUIT_CLUBS + VALUE_3));
    assert(Card_prevOfValue(twoOfClubs) == (Card) (SUIT_SPADES + VALUE_2));
    assert(Card_nextOfValue(twoOfClubs) == (Card) (SUIT_DIAMONDS + VALUE_2));
    assert(Card_oppositeOfValue(twoOfClubs) == (Card) (SUIT_HEARTS + VALUE_2));

    Card lowAceOfHearts = VALUE_LOW_ACE + SUIT_HEARTS;
    assert(strcmp(Card_name(lowAceOfHearts), "a♥") == 0);
    assert(strcmp(Card_name(twoOfClubs), "a♥") != 0);
    assert(Card_points(lowAceOfHearts) == 5);
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

const CardSet CardSet_legal = 0x1FFF1FFF1FFF1FFFULL;

bool CardSet_has(CardSet set, Card card) {
    return (set & (1ULL << card)) != 0;
}

void CardSet_add(CardSet *set, Card card) {
    *set |= (1ULL << card);
}

void CardSet_remove(CardSet *set, Card card) {
    *set &= ~(1ULL << card);
}

int CardSet_size(CardSet set) {
    return __builtin_popcountll(set);
}

int CardSet_points(CardSet set) {
    uint64_t addFive = set & 0x21FF21FF21FF21FFULL;
    uint64_t addTen = set & 0x3E003E003E003E00ULL;

    return 5 * (__builtin_popcountll(addFive) + (__builtin_popcountll(addTen) << 1));
}

CardSet CardSet_sets(CardSet set) {
    return set & (((set >> 16) | (set << 48)) & ((set << 16) | (set >> 48)));
}

CardSet CardSet_runs(CardSet set) {
    set &= 0x1FFE;  // mask out aces
    return set & ((set >> 1) & (set << 1));
}

// Iteration
// I'll need to find the lowest bit, which I could then convert into
// a Card (by taking log) or leave as a CardSet.  I'll maintain an
// already-considered set to avoid duplicates.

void iterate(CardSet cs) {
    uint64_t b = cs & -cs;  // Get lowest bit
}

void CardSet_print(CardSet set) {
    bool first = true;
    for (int i = 0; i < 64; ++i) {
        if (CardSet_has(set, i)) {
            if (first) {
                printf("%s", Card_name(i));
                first = false;
            } else {
                printf(" %s", Card_name(i));
            }
        }
    }
}

void CardSet_test() {
    Card sevenOfClubs = VALUE_7 + SUIT_CLUBS;
    Card aceOfSpades = VALUE_ACE + SUIT_SPADES;
    Card tenOfDiamonds = VALUE_10 + SUIT_DIAMONDS;

    CardSet set = 0;

        // Test hand points
    assert(CardSet_points(set) == 0);

    // Test adding cards
    CardSet_add(&set, sevenOfClubs);
    CardSet_add(&set, aceOfSpades);
    assert(CardSet_has(set, sevenOfClubs));
    assert(CardSet_has(set, aceOfSpades));
    assert(!CardSet_has(set, tenOfDiamonds));
    assert(CardSet_points(set) == 20);

    CardSet_add(&set, tenOfDiamonds);
    assert(CardSet_has(set, tenOfDiamonds));
    assert(CardSet_points(set) == 30);

    // Test removing cards
    CardSet_remove(&set, sevenOfClubs);
    CardSet_remove(&set, aceOfSpades);
    assert(!CardSet_has(set, sevenOfClubs));
    assert(!CardSet_has(set, aceOfSpades));
    assert(CardSet_has(set, tenOfDiamonds));
    assert(CardSet_points(set) == 10);
}

//////////////////////////////////////////////////////////////////////
//
//    CardList
//

typedef struct {
    Card cards[52];
    int size;
} CardList;

CardList *CardList_init(CardList *cardList) {
    cardList->size = 0;
    return cardList;
}

CardList *CardList_new() {
    return CardList_init(malloc(sizeof(CardList)));
}

void CardList_free(CardList *cardList) {
    free(cardList);
}

void CardList_push(CardList *cardList, Card card) {
    cardList->cards[cardList->size++] = card;
}

Card CardList_pop(CardList *cardList) {
    assert(cardList->size >= 1);
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
    bool first = true;
    for (int i = 0; i < cardList->size; ++i) {
        if (first) {
            printf("%s", Card_name(cardList->cards[i]));
            first = false;
        } else {
            printf(" %s", Card_name(cardList->cards[i]));
        }
    }
}

void CardList_test() {
    // This is kinda random and wrong, but enough for starters.
    CardList *cl = CardList_new();
    for (int i = 0; i < 52; ++i) {
        CardList_push(cl, i);
    }
    CardList_shuffle(cl);
    CardList_print(cl);
    printf("\n");
    CardList_free(cl);
}

//////////////////////////////////////////////////////////////////////
//
//    MeldSet
//

typedef struct MeldSetStruct MeldSet;

struct MeldSetStruct {
    CardSet runs;
    CardSet sets;
};

MeldSet *MeldSet_init(MeldSet *meldSet) {
    meldSet->runs = 0;
    meldSet->sets = 0;
    return meldSet;
}

void MeldSet_addRun(MeldSet *meldSet, CardSet meld) {
    meldSet->runs |= meld;
}

void MeldSet_removeRun(MeldSet *meldSet, CardSet meld) {
    meldSet->runs &= ~meld;
}

void MeldSet_addSet(MeldSet *meldSet, CardSet meld) {
    meldSet->sets |= meld;
}

void MeldSet_removeSet(MeldSet *meldSet, CardSet meld) {
    meldSet->sets &= ~meld;
}

CardSet MeldSet_setAdds(MeldSet meldSet) {
    CardSet sets = meldSet.sets;
    CardSet adds = ((sets >> 16) | (sets << 16)) & ~sets & CardSet_legal;
    return adds;
}

CardSet MeldSet_runAdds(MeldSet meldSet) {
    CardSet runs = meldSet.runs;
    CardSet adds = ((runs << 1) | (runs >> 1)) & ~runs & CardSet_legal;
    return adds;
}

void MeldSet_print(MeldSet *meldSet) {
    printf("  a23456789TJQKA\n");
    for (int suit = SUIT_CLUBS; suit <= SUIT_SPADES; suit += 16) {
        printf("%s ", Card_suitSymbol[suit / 16]);
        for (int value = VALUE_LOW_ACE; value <= VALUE_ACE; ++value) {
            Card card = suit + value;
            if (CardSet_has(meldSet->runs, card)) {
                printf("-");
            } else if (CardSet_has(meldSet->sets, card)) {
                printf("|");
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}

void MeldSet_test() {
    // Construct some runs and sets, add them to the MeldSet
    MeldSet meldSet;
    MeldSet_init(&meldSet);

    // Construct a CardSet with three eights.
    CardSet eightSet = 0;
    CardSet_add(&eightSet, SUIT_CLUBS + VALUE_8);
    CardSet_add(&eightSet, SUIT_DIAMONDS + VALUE_8);
    CardSet_add(&eightSet, SUIT_HEARTS + VALUE_8);
    MeldSet_addSet(&meldSet, eightSet);

    // Construct a CardSet with a run of 4-5-6-7 of clubs.
    CardSet clubRun = 0;
    CardSet_add(&clubRun, SUIT_CLUBS + VALUE_4);
    CardSet_add(&clubRun, SUIT_CLUBS + VALUE_5);
    CardSet_add(&clubRun, SUIT_CLUBS + VALUE_6);
    CardSet_add(&clubRun, SUIT_CLUBS + VALUE_7);
    MeldSet_addRun(&meldSet, clubRun);

    // Print the MeldSet
    MeldSet_print(&meldSet);
}

//////////////////////////////////////////////////////////////////////
//
//    Player and Game Structs
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
    int numPlayers;
    int currentPlayer;
    Player *players;
    CardList drawPile;
    CardList discardPile;
    CardSet discarded;
};

//////////////////////////////////////////////////////////////////////
//
//    Player Methods
//

void Player_init(Player *player, Game *game, int index) {
    player->game = game;
    player->index = index;
    player->hand = 0;
    player->points = 0;
}

Card Player_drawCard(Player *player) {
    Card card = CardList_pop(&(player->game->drawPile));
    CardSet_add(&(player->hand), card);
    return card;
}

void Player_discardCard(Player *player, Card card) {
    CardSet_remove(&(player->hand), card);
    CardList_push(&(player->game->discardPile), card);
}

void Player_print(Player *player) {
    printf("Player %d: ", player->index);
    CardSet_print(player->hand);
    printf(" (%d pts)\n", player->points);
}

void Player_test() {
    Player player;
    Player_init(&player, NULL, 0);
    player.index = 2;
    CardSet_add(&player.hand, VALUE_5 + SUIT_DIAMONDS);
    CardSet_add(&player.hand, VALUE_ACE + SUIT_HEARTS);
    player.points = 15;
    Player_print(&player);
}

//////////////////////////////////////////////////////////////////////
//
//    Game Methods
//

Player *Game_player(Game *game, int playerIndex) {
    return &(game->players[playerIndex]);
}

Game *Game_new(int numPlayers) {
    Game *game = malloc(sizeof(Game));
    CardList_init(&(game->drawPile));
    CardList_init(&(game->discardPile));
    game->discarded = 0;
    game->numPlayers = numPlayers;
    game->currentPlayer = 0;
    game->players = malloc(numPlayers * sizeof(Player));
    for (int i = 0; i < numPlayers; ++i) {
        Player_init(Game_player(game, i), game, i);
    }

    // Fill and shuffle the draw pile.
    for (int i = 0; i < 64; ++i) {
        if (Card_value(i) >= VALUE_2 && Card_value(i) <= VALUE_ACE) {
            CardList_push(&game->drawPile, i);
        }
    }
    CardList_shuffle(&game->drawPile);

    // Deal cards to each player.
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = Game_player(game, i);
        for (int j = 0; j < 7; ++j) {
            Player_drawCard(player);
        }
    }

    // First player draws one more card, which becomes the discard pile.
    Player *firstPlayer = Game_player(game, 0);
    Player_discardCard(firstPlayer, Player_drawCard(firstPlayer));

    return game;
}

void Game_free(Game *game) {
    if (game) {
        free(game->players);
        free(game);
    }
}

Player *Game_currentPlayer(Game *game) {
    return &game->players[game->currentPlayer];
}

void Game_print(Game *game) {
    printf("Player %d/%d\n", game->currentPlayer, game->numPlayers);
    for (int i = 0; i < game->numPlayers; ++i) {
        Player_print(Game_player(game, i));
    }
    printf("Draw pile: ");
    CardList_print(&game->drawPile);
    printf("\nDiscard pile: ");
    CardList_print(&game->discardPile);
    printf("\n");
}

void Game_test() {
    Game *game = Game_new(3);
    Game_print(game);
    Game_free(game);
}

//////////////////////////////////////////////////////////////////////
//
//    Evaluate
//

int Evaluate(Game *game) {
    Player *player = Game_currentPlayer(game);
    int pointsInHand = CardSet_points(player->hand);
    int pointsFromRivals = 0;
    if (player->hand == 0) {
        for (int p = 0; p < game->numPlayers; ++p) {
            if (p != player->index) {
                pointsFromRivals += CardSet_size(game->players[p].hand);
            }
        }
        pointsFromRivals *= 7;
        pointsFromRivals /= (game->numPlayers - 1);
    }
    return player->points + pointsInHand / 2 + pointsFromRivals;
}

void Evaluate_test() {
    // Create a game
    Game *game = Game_new(3);

    // Set up the hand of player 0.
    Player *player = Game_player(game, 0);
    player->points = 100;

    CardSet *hand = &(player->hand);
    *hand = 0;
    CardSet_add(hand, VALUE_3 + SUIT_HEARTS);
    CardSet_add(hand, VALUE_QUEEN + SUIT_CLUBS);
    CardSet_add(hand, VALUE_ACE + SUIT_CLUBS);

    // Eval is 100 points played + 30 / 2 for points in hand
    int score = Evaluate(game);
    assert(score == 100 + (5 + 10 + 15) / 2);

    // Eval is 100 points played + 7 points per rival card in hand
    *hand = 0;
    int outScore = Evaluate(game);
    assert(outScore == 100 + 7 * 7);

    Game_free(game);
}

//////////////////////////////////////////////////////////////////////
//
//    Play
//

typedef struct PlayStruct Play;

struct PlayStruct {
    CardList cardsTaken;
    Card cardDrawn;
    MeldSet melds;
    MeldSet rejectedMelds;
    Card discard;
    int score;
};

//////////////////////////////////////////////////////////////////////
//
//    Search
//

// All search functions return the highest-achieved score.  Parameters
// are the game state, a Play structure recording actions to reach
// that game state, and the best Play found so far, which includes its
// score.

int searchDiscard(Game *game, Play *scratch, Play *best) {
    Player *player = Game_currentPlayer(game);

    // Iterate over cards in the hand


    // Try discarding each card in hand
    int bestScore = -1;
    Card bestDiscard = 0;

    return 0;
}

//////////////////////////////////////////////////////////////////////
//
//    Main
//

int main(int argc, char **argv) {
    Card_test();
    CardSet_test();
    CardList_test();
    MeldSet_test();
    Player_test();
    Game_test();
    Evaluate_test();

    uint64_t x = (1 << 13) | (1 << 21);
    printf("%llu\n", x & -x);

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