#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// This is an automatic player for the card game Rummy.
//
// Representations of cards, sets of cards, and lists of cards are central.
//
// One representation of a card is as a uint8_t (typedef'ed to Index),
// where all clubs have the bit pattern 0000xxxx, diamonds 0001xxxx,
// hearts 0010xxxx, and spades 0011xxxx.  The lower four bits (xxxx)
// represent the rank of the card, with 0000 representing a low ace,
// 1100 representing the king, and 1101 representing a high ace.  Bit
// patterns xxxx1110 and xxxx1111 are illegal.  Aces are considered high
// unless played in an ace-two-three meld, in which case they are low.
//
// More commonly, cards are represented by uint64_t's (typedef'ed to
// Heap).  A card with Index = b is encoded as (1ULL << b).  This takes
// more memory than an index and the name is odd, but the Heap type
// is convenient for working with both individual cards and sets
// of cards.  In particular, a set of cards is the bitwise OR of the
// individual card representations.
//
// To represent the draw and discard stacks, we need ordered lists of cards.
// These are structs consisting of an array of 52 cards and a count of the
// actual number in the stack.  Again, this is memory inefficient (unless
// the stack actually contains 52 cards), but avoids dynamic memory
// allocation.

///////////////////////////////////////////////////////////////////////////////
//
//    Index
//

typedef uint8_t Index;

const char *kSuitSymbol[4] = { "♣", "♦", "♥", "♠" };

const char *kIndexName[64] = {
    "a♣", "2♣", "3♣", "4♣", "5♣", "6♣", "7♣", "8♣", "9♣", "10♣", "J♣", "Q♣", "K♣", "A♣", "14", "15",
    "a♦", "2♦", "3♦", "4♦", "5♦", "6♦", "7♦", "8♦", "9♦", "10♦", "J♦", "Q♦", "K♦", "A♦", "30", "31",
    "a♥", "2♥", "3♥", "4♥", "5♥", "6♥", "7♥", "8♥", "9♥", "10♥", "J♥", "Q♥", "K♥", "A♥", "46", "47",
    "a♠", "2♠", "3♠", "4♠", "5♠", "6♠", "7♠", "8♠", "9♠", "10♠", "J♠", "Q♠", "K♠", "A♠", "62", "63"
};

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

Suit_t indexSuit(Index i) {
    return i & 0xF0;
}

Value_t indexValue(Index i) {
    return i & 0x0F;
}

bool indexIsLegal(Index i) {
    return (i & 0x0F) <= VALUE_ACE && (i & 0xF0) <= SUIT_SPADES;
}

const char *indexName(Index i) {
    assert(indexIsLegal(i));
    return kIndexName[i];
}

void indexTest(void) {
    Index twoOfClubs = VALUE_2 + SUIT_CLUBS;
    Index lowAceOfHearts = VALUE_LOW_ACE + SUIT_HEARTS;

    assert(indexSuit(twoOfClubs) == SUIT_CLUBS);
    assert(indexValue(twoOfClubs) == VALUE_2);
    assert(strcmp(indexName(twoOfClubs), "2♣") == 0);
    assert(strcmp(indexName(lowAceOfHearts), "a♥") == 0);
    assert(indexIsLegal(twoOfClubs));
    assert(!indexIsLegal(15));
    assert(!indexIsLegal(200));
}

///////////////////////////////////////////////////////////////////////////////
//
//    Heap 
//

typedef uint64_t Heap;

const Heap kFullDeck = 0x3FFE3FFE3FFE3FFEULL;

bool heapIsLegal(Heap heap) {
    return (heap & ~kFullDeck) == 0;
}

bool heapHas(Heap heap, Heap card) {
    return (heap & card) != 0;
}

void heapAdd(Heap *heap, Heap card) {
    *heap |= card;
}

void heapRemove(Heap *heap, Heap card) {
    *heap &= ~card;
}

int heapSize(Heap heap) {
    return __builtin_popcountll(heap);
}

int heapPoints(Heap heap) {
    uint64_t addFive = heap & 0x21FF21FF21FF21FFULL;
    uint64_t addTen = heap & 0x3E003E003E003E00ULL;
    return 5 * (__builtin_popcountll(addFive) + (__builtin_popcountll(addTen) << 1));
}

// These functions are used to iterate over the cards in a heap:
//
// Heap cards = ...;
// for (Card c = heapLowCard(cards); c != 0; c = heapNextCard(cards, c)) { ... }

Heap heapLowCard(Heap heap) {
    return heap & -heap;
}

Heap heapNextCard(Heap heap, Heap card) {
    heap &= ~(((card << 1) - 1));
    return heap & -heap;
}

// Heap <---> Index conversions

Heap indexToHeap(Index i) {
    return (Heap)(1ULL << i);
}

Index heapToIndex(Heap heap) {
    assert(heap != 0);
    return __builtin_ctzll(heap); // count trailing zeros
}

void heapPrint(Heap heap) {
    bool first = true;
    for (Heap c = heapLowCard(heap); c != 0; c = heapNextCard(heap, c)) {
        if (first) {
            printf("%s", indexName(heapToIndex(c)));
            first = false;
        } else {
            printf(" %s", indexName(heapToIndex(c)));
        }
    }
}

void heapTest(void) {
    Heap sevenOfClubs = indexToHeap(VALUE_7 + SUIT_CLUBS);
    Heap aceOfSpades = indexToHeap(VALUE_ACE + SUIT_SPADES);
    Heap tenOfDiamonds = indexToHeap(VALUE_10 + SUIT_DIAMONDS);
    Heap heap = 0;

    // Test hand points
    assert(heapPoints(heap) == 0);

    // Test adding cards
    heapAdd(&heap, sevenOfClubs);
    heapAdd(&heap, aceOfSpades);
    assert(heapHas(heap, sevenOfClubs));
    assert(heapHas(heap, aceOfSpades));
    assert(!heapHas(heap, tenOfDiamonds));
    assert(heapPoints(heap) == 20);

    // Test iteration over a heap of cards
    for (Heap c = heapLowCard(heap); c != 0; c = heapNextCard(heap, c)) {
        const char *name = indexName(heapToIndex(c));
        assert(strcmp(name, "7♣") == 0 || strcmp(name, "A♠") == 0);
    }

    heapAdd(&heap, tenOfDiamonds);
    assert(heapHas(heap, tenOfDiamonds));
    assert(heapPoints(heap) == 30);

    // Test removing cards
    heapRemove(&heap, sevenOfClubs);
    heapRemove(&heap, aceOfSpades);
    assert(!heapHas(heap, sevenOfClubs));
    assert(!heapHas(heap, aceOfSpades));
    assert(heapHas(heap, tenOfDiamonds));
    assert(heapPoints(heap) == 10);
}

///////////////////////////////////////////////////////////////////////////////
//
//    Pile
//

typedef struct {
    Heap cards[52];
    int size;
} Pile;

void pilePush(Pile *pile, Heap card) {
    assert(pile->size < 52);
    pile->cards[pile->size++] = card;
}

Heap pilePop(Pile *pile) {
    assert(pile->size >= 1);
    return pile->cards[--pile->size];
}

Pile *pileNewEmpty(void) {
    Pile *new = malloc(sizeof(Pile));
    new->size = 0;
    return new;
}

Pile *pileNewFullDeck(void) {
    Pile *new = pileNewEmpty();
    for (Heap c = heapLowCard(kFullDeck); c != 0; c = heapNextCard(kFullDeck, c)) {
        pilePush(new, c);
    }
    return new;
}

void pileFree(Pile *pile) {
    free(pile);
}

Pile *pileShuffle(Pile *pile) {
    for (int i = pile->size - 1; i > 0; --i) {
        int j = arc4random_uniform(i + 1);
        Heap temp = pile->cards[i];
        pile->cards[i] = pile->cards[j];
        pile->cards[j] = temp;
    }
    return pile;
}

void pilePrint(Pile *pile) {
    bool first = true;
    for (int i = 0; i < pile->size; ++i) {
        Heap card = pile->cards[i];
        assert(heapSize(card) == 1);
        if (first) {
            printf("%s", indexName(heapToIndex(card)));
            first = false;
        } else {
            printf(" %s", indexName(heapToIndex(card)));
        }
    }
}

void pileTest(void) {
    Pile *emptyPile = pileNewEmpty();
    assert(emptyPile->size == 0);
    pilePush(emptyPile, indexToHeap(VALUE_7 + SUIT_CLUBS));
    pilePush(emptyPile, indexToHeap(VALUE_JACK + SUIT_HEARTS));
    assert(emptyPile->size == 2);
    assert(pilePop(emptyPile) == indexToHeap(VALUE_JACK + SUIT_HEARTS));
    assert(pilePop(emptyPile) == indexToHeap(VALUE_7 + SUIT_CLUBS));
    pileFree(emptyPile);

    Pile *deckPile = pileNewFullDeck();
    assert(deckPile->size == 52);
    pileShuffle(deckPile);
    assert(deckPile->size == 52);
    pileFree(deckPile);
}

///////////////////////////////////////////////////////////////////////////////
//
//    MeldSet
//

typedef struct MeldSetStruct MeldSet;

struct MeldSetStruct {
    Heap runs;
    Heap sets;
};

MeldSet *meldSetInit(MeldSet *meldSet) {
    meldSet->runs = 0;
    meldSet->sets = 0;
    return meldSet;
}

void meldSetAddRun(MeldSet *meldSet, Heap meld) {
    meldSet->runs |= meld;
}

void meldSetRemoveRun(MeldSet *meldSet, Heap meld) {
    meldSet->runs &= ~meld;
}

void meldSetAddSet(MeldSet *meldSet, Heap meld) {
    meldSet->sets |= meld;
}

void meldSetRemoveSet(MeldSet *meldSet, Heap meld) {
    meldSet->sets &= ~meld;
}

void meldSetPrint(MeldSet *meldSet) {
    printf("  a23456789TJQKA\n");
    for (int suit = SUIT_CLUBS; suit <= SUIT_SPADES; suit += 16) {
        printf("%s ", kSuitSymbol[suit >> 16]);
        for (int value = VALUE_LOW_ACE; value <= VALUE_ACE; ++value) {
            Heap card = indexToHeap(suit + value);
            if (heapHas(meldSet->runs, card)) {
                printf("-");
            } else if (heapHas(meldSet->sets, card)) {
                printf("|");
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}

void meldSetTest(void) {
    // Construct some runs and sets, add them to the MeldSet
    MeldSet meldSet;
    meldSetInit(&meldSet);

    // Construct  with three eights. eightSet = 0;
    Heap eightSet = 0;
    heapAdd(&eightSet, indexToHeap(SUIT_CLUBS + VALUE_8));
    heapAdd(&eightSet, indexToHeap(SUIT_DIAMONDS + VALUE_8));
    heapAdd(&eightSet, indexToHeap(SUIT_HEARTS + VALUE_8));
    meldSetAddSet(&meldSet, eightSet);

    // Construct  with a run of 4-5-6-7 of clubs. clubRun = 0;
    Heap clubRun = 0;
    heapAdd(&clubRun, indexToHeap(SUIT_CLUBS + VALUE_4));
    heapAdd(&clubRun, indexToHeap(SUIT_CLUBS + VALUE_5));
    heapAdd(&clubRun, indexToHeap(SUIT_CLUBS + VALUE_6));
    heapAdd(&clubRun, indexToHeap(SUIT_CLUBS + VALUE_7));
    meldSetAddRun(&meldSet, clubRun);

    // Print the MeldSet
    meldSetPrint(&meldSet);
}

///////////////////////////////////////////////////////////////////////////////
//
//    Player and Game Structs
//

typedef struct PlayerStruct Player;
typedef struct GameStruct Game;

struct PlayerStruct {
    Game *game;
    int index;
    Heap hand;
    int points;
};

struct GameStruct {
    int numPlayers;
    int currentPlayer;
    Player *players;
    Pile drawPile;
    Pile discardPile;
    Heap discarded;
};

///////////////////////////////////////////////////////////////////////////////
//
//    Player Methods
//

void playerInit(Player *player, Game *game, int index) {
    player->game = game;
    player->index = index;
    player->hand = 0;
    player->points = 0;
}

Heap playerDraw(Player *player) {
    Heap card = pilePop(&(player->game->drawPile));
    heapAdd(&(player->hand), card);
    return card;
}

void playerUndoDraw(Player *player, Heap card) {
    pilePush(&(player->game->drawPile), card);
    heapRemove(&(player->hand), card);
}

void playerDiscard(Player *player, Heap card) {
    heapRemove(&(player->hand), card);
    pilePush(&(player->game->discardPile), card);
}

void playerPrint(Player *player) {
    printf("Player %d: ", player->index);
    heapPrint(player->hand);
    printf(" (%d pts)\n", player->points);
}

void playerTest(void) {
    Player player;
    playerInit(&player, NULL, 0);
    player.index = 2;
    heapAdd(&player.hand, VALUE_5 + SUIT_DIAMONDS);
    heapAdd(&player.hand, VALUE_ACE + SUIT_HEARTS);
    player.points = 15;
    playerPrint(&player);
}

#if 0

///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
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
 *hand = &(player->hand);
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

///////////////////////////////////////////////////////////////////////////////
//
//    Play
//

typedef struct PlayStruct Play;

struct PlayStruct {
    CardList cardsTaken;  // I especially care about the deepest card taken.  Can I avoid a list?
    Card cardDrawn;
    MeldSet melds;
    MeldSet rejectedMelds;  // for bookkeeping during search
    Card discard;
    int score;
};

int Play_improve(Play *best, Play *scratch) {
    if (scratch->score > best->score) {
        // Copy the cardsTaken list
        best->cardsTaken.size = scratch->cardsTaken.size;
        for (int i = 0; i < scratch->cardsTaken.size; ++i) {
            best->cardsTaken.cards[i] = scratch->cardsTaken.cards[i];
        }
        best->cardDrawn = scratch->cardDrawn;
        best->melds = scratch->melds;
        best->rejectedMelds = scratch->rejectedMelds;
        best->discard = scratch->discard;
        best->score = scratch->score;
    }
    return best->score;
}

///////////////////////////////////////////////////////////////////////////////
//
//    Search
//

// All search functions return the highest-achieved score.  Parameters
// are the game state, a Play structure recording actions to reach
// that game state, and the best Play found so far, which includes its
// score.

int searchDiscard(Game *game, Play *scratch, Play *best) {
    Player *player = Game_currentPlayer(game); hand = player->hand;

    if (hand == 0) {
        // Hand is empty.  Discard nothing.
        scratch->discard = 0;
        scratch->score = Evaluate(game);
        return Play_improve(best, scratch);
    }

    // Iterate over cards in the hand
    for cs = CardSet_lowestCard(hand); cs != 0; cs = CardSet_nextCard(hand, cs)) {
        scratch->discard = cs;
    
        Player_discard(cs);  // TODO:  track whether this was previously discarded
        scratch->score = Evaluate(game);
        Play_improve(best, scratch);
        Player_undoDiscard(cs);
        player->hand = hand | cs;  // put the card back
    }

    return 0;
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//    Main
//

int main(int argc, char **argv) {
    indexTest();
    heapTest();
    pileTest();
    meldSetTest();

#if 0
    CardSet_test();
    CardList_test();
    MeldSet_test();
    Player_test();
    Game_test();
    Evaluate_test();

    uint64_t x = (1 << 13) | (1 << 21);
    printf("%llu\n", x & -x);
#endif

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
