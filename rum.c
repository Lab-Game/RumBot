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
const Heap kHighAceMask = (
    (1ULL << (VALUE_ACE + SUIT_CLUBS)) |
    (1ULL << (VALUE_ACE + SUIT_DIAMONDS)) |
    (1ULL << (VALUE_ACE + SUIT_HEARTS)) |
    (1ULL << (VALUE_ACE + SUIT_SPADES))
);
const Heap kLowAceMask = (
    (1ULL << (VALUE_LOW_ACE + SUIT_CLUBS)) |
    (1ULL << (VALUE_LOW_ACE + SUIT_DIAMONDS)) |
    (1ULL << (VALUE_LOW_ACE + SUIT_HEARTS)) |
    (1ULL << (VALUE_LOW_ACE + SUIT_SPADES))
);

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

void heapAddLowAces(Heap *heap) {
    *heap |= (*heap & kHighAceMask) >> (VALUE_ACE - VALUE_LOW_ACE);
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
//   Heap cards = ...;
//   for (Heap c = heapLowCard(cards); c != 0; c = heapNextCard(cards, c)) { ... }

Heap heapLowCard(Heap heap) {
    return heap & -heap;
}

Heap heapNextCard(Heap heap, Heap card) {
    heap &= ~(((card << 1) - 1));
    return heap & -heap;
}

Heap heapExpand(Heap heap) {   // for each high ace, add the low ace
    return heap | ((heap & kHighAceMask) >> (VALUE_ACE - VALUE_LOW_ACE));
}

Heap heapContract(Heap heap) {  // replace low aces with high aces
    return (heap | ((heap & kLowAceMask) << (VALUE_ACE - VALUE_LOW_ACE))) & ~kLowAceMask;
}

// Heap <---> Index conversions

Heap indexToHeap(Index i) {
    return (Heap)(1ULL << i);
}

Index heapToIndex(Heap heap) {
    assert(heap != 0);
    return __builtin_ctzll(heap); // count trailing zeros
}

Heap heapFromString(const char *str) {
    const char *values = "a23456789TJQKA";
    const char *suits = "CDHS";
    Heap heap = 0;

    if (!*str) {
        return heap;
    }

    while (true) {
        assert(*str);
        assert(strchr(values, *str));
        assert(*(str+1));
        assert(strchr(suits, *(str + 1)));

        int value = strchr(values, *(str++)) - values;
        int suit = strchr(suits, *(str++)) - suits;

        heapAdd(&heap, indexToHeap(value + (suit << 4)));

        if (!*str) {
            return heap;
        } else {
            assert(*str == ' ');
            str++;
        }
    }
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

    printf("Should print: KD QS 7H 9H 8C\n");
    heapPrint(heapFromString("KD QS 7H 9H 8C"));
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

void pileInitEmpty(Pile *pile) {
    pile->size = 0;
}

void pileInitFullDeck(Pile *pile) {
    pile->size = 0;
    for (Heap c = heapLowCard(kFullDeck); c != 0; c = heapNextCard(kFullDeck, c)) {
        pilePush(pile, c);
    }
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
    Pile emptyPile;
    pileInitEmpty(&emptyPile);
    assert(emptyPile.size == 0);
    pilePush(&emptyPile, indexToHeap(VALUE_7 + SUIT_CLUBS));
    pilePush(&emptyPile, indexToHeap(VALUE_JACK + SUIT_HEARTS));
    assert(emptyPile.size == 2);
    assert(pilePop(&emptyPile) == indexToHeap(VALUE_JACK + SUIT_HEARTS));
    assert(pilePop(&emptyPile) == indexToHeap(VALUE_7 + SUIT_CLUBS));

    Pile deckPile;
    pileInitFullDeck(&deckPile);
    assert(deckPile.size == 52);
    pileShuffle(&deckPile);
    assert(deckPile.size == 52);
}

///////////////////////////////////////////////////////////////////////////////
//
//    MeldSet
//

typedef struct MeldSetStruct {
    Heap runs;
    Heap sets;
} MeldSet;

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
        printf("%s ", kSuitSymbol[suit >> 4]);
        for (int value = VALUE_LOW_ACE; value <= VALUE_ACE; ++value) {
            Heap card = indexToHeap(suit + value);
            bool inRun = heapHas(meldSet->runs, card);
            bool inSet = heapHas(meldSet->sets, card);
            if (inRun && inSet) {
                printf("+");  // can happen when considering *possible* melds
            } else if (inRun) {
                printf("-");
            } else if (inSet) {
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
    printf("vvvvv meldSetPrint() Test Output vvvvv\n");
    meldSetPrint(&meldSet);
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
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
    MeldSet melds;
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

void playerUndoDiscard(Player *player) {
    Heap card = pilePop(&(player->game->discardPile));
    heapAdd(&(player->hand), card);
}

void playerPlayRun(Player *player, Heap run) {
    player->points += heapPoints(run);
    meldSetAddRun(&(player->game->melds), run);
    heapRemove(&(player->hand), heapExpand(run));
}

void playerUndoRun(Player *player, Heap run) {
    player->points -= heapPoints(run);
    meldSetRemoveRun(&(player->game->melds), run);
    heapAdd(&(player->hand), heapContract(run));
}

void playerPlaySet(Player *player, Heap set) {
    player->points += heapPoints(set);
    meldSetAddSet(&(player->game->melds), set);
    heapRemove(&(player->hand), heapExpand(set));
}

void playerUndoSet(Player *player, Heap set) {
    player->points -= heapPoints(set);
    meldSetRemoveSet(&(player->game->melds), set);
    heapAdd(&(player->hand), heapContract(set));
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

///////////////////////////////////////////////////////////////////////////////
//
//    Game Methods
//

Player *gamePlayer(Game *game, int playerIndex) {
    return &(game->players[playerIndex]);
}

Player *gameCurrentPlayer(Game *game) {
    return &game->players[game->currentPlayer];
}

Game *gameInit(Game *game, int numPlayers) {
    game->numPlayers = numPlayers;
    game->currentPlayer = 0;
    game->discarded = 0;

    pileInitEmpty(&(game->drawPile));
    pileInitEmpty(&(game->discardPile));

    meldSetInit(&(game->melds));

    game->players = malloc(numPlayers * sizeof(Player));
    for (int i = 0; i < numPlayers; ++i) {
        playerInit(gamePlayer(game, i), game, i);
    }

    // Fill and shuffle the draw pile.
    pileInitFullDeck(&(game->drawPile));
    pileShuffle(&(game->drawPile));

    // Deal cards to each player.
    for (int i = 0; i < game->numPlayers; ++i) {
        Player *player = gamePlayer(game, i);
        for (int j = 0; j < 7; ++j) {
            playerDraw(player);
        }
    }

    // First player draws one more card, which becomes the discard pile.
    Player *firstPlayer = gamePlayer(game, 0);
    playerDiscard(firstPlayer, playerDraw(firstPlayer));

    return game;
}

void gamePrint(Game *game) {
    printf("Player %d/%d\n", game->currentPlayer, game->numPlayers);
    for (int i = 0; i < game->numPlayers; ++i) {
        playerPrint(gamePlayer(game, i));
    }
    printf("Draw pile: ");
    pilePrint(&game->drawPile);
    printf("\nDiscard pile: ");
    pilePrint(&game->discardPile);
    printf("\nMelds:\n");
    meldSetPrint(&game->melds);
}

void gameTest(void) {
    Game game;
    gameInit(&game, 3);
    assert(game.numPlayers == 3);
    assert(game.currentPlayer == 0);
    assert(game.drawPile.size == 52 - 3 * 7 - 1);
    assert(game.discardPile.size == 1);
    for (int i = 0; i < game.numPlayers; ++i) {
        assert(heapSize(game.players[i].hand) == 7);
        assert(game.players[i].points == 0);
    }

    printf("vvvvv gamePrint() Test Output vvvvv\n");
    gamePrint(&game);
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
}

///////////////////////////////////////////////////////////////////////////////
//
//    Evaluate
//

int evaluate(Game *game) {
    Player *player = gameCurrentPlayer(game);
    int pointsInHand = heapPoints(player->hand);
    int pointsFromRivals = 0;
    if (player->hand == 0) {
        for (int p = 0; p < game->numPlayers; ++p) {
            if (p != player->index) {
                pointsFromRivals += heapSize(game->players[p].hand);
            }
        }
        pointsFromRivals *= 7;
        pointsFromRivals /= (game->numPlayers - 1);
    }
    int eval = player->points + pointsInHand / 2 + pointsFromRivals;
    return eval;
}

void evaluateTest() {
    // Create a game
    Game game;
    gameInit(&game, 3);

    Player *player = gamePlayer(&game, 0);
    player->points = 100;
    Heap *hand = &(player->hand);
    *hand = 0;
    heapAdd(hand, indexToHeap(VALUE_3 + SUIT_HEARTS));
    heapAdd(hand, indexToHeap(VALUE_QUEEN + SUIT_CLUBS));
    heapAdd(hand, indexToHeap(VALUE_ACE + SUIT_CLUBS));

    // Eval is 100 points played + 30 / 2 for points in hand
    int score = evaluate(&game);
    assert(score == 100 + (5 + 10 + 15) / 2);

    // Eval is 100 points played + 7 points per rival card in hand
    *hand = 0;
    int outScore = evaluate(&game);
    assert(outScore == 100 + 7 * 7);
}

///////////////////////////////////////////////////////////////////////////////
//
//    Play
//

typedef struct PlayStruct {
    Pile taken;  // Do I need a complete list or just num + deepest card?
    Heap drawn;
    MeldSet melds;
    Heap discard;
    int eval;
} Play;

void playInit(Play *play) {
    pileInitEmpty(&(play->taken));
    play->drawn = 0;;
    meldSetInit(&(play->melds));
    play->discard = 0;
    play->eval = 0;
}

int playMax(Play *best, Play *scratch) {
    if (scratch->eval > best->eval) {
        best->taken = scratch->taken;
        best->drawn = scratch->drawn;
        best->melds = scratch->melds;
        best->discard = scratch->discard;
        best->eval = scratch->eval;
    }
    return best->eval;
}

void playPrint(Play *play) {
    printf("Take: ");
    pilePrint(&(play->taken));
    printf(" Draw:");
    heapPrint(play->drawn);
    printf("\n");
    meldSetPrint(&(play->melds));
    printf("Discard: ");
    heapPrint(play->discard);
    printf(" Eval: %d\n", play->eval);
}

void playTest() {
    Play best;
    playInit(&best);
    Play scratch;
    playInit(&scratch);

    // Test case 1: Basic play
    scratch.eval = 10;
    playMax(&best, &scratch);
    assert(best.eval == 10);

    // Test case 2: Better play
    scratch.eval = 15;
    playMax(&best, &scratch);
    assert(best.eval == 15);

    // Test case 3: Worse play
    scratch.eval = 5;
    playMax(&best, &scratch);
    assert(best.eval == 15);
}

///////////////////////////////////////////////////////////////////////////////
//
//    Opt - Meld Options
//

typedef struct OptStruct {
    Heap runCenters;
    Heap runExtensions;
    Heap setCenters;
    Heap setExtensions;
} Opt;

void optInit(Opt *meldOpt) {
    meldOpt->runCenters = 0;
    meldOpt->runExtensions = 0;
    meldOpt->setCenters = 0;
    meldOpt->setExtensions = 0;
}

void optFind(Game *game, Opt *meldOpt) {
    Heap hand = gameCurrentPlayer(game)->hand;
    MeldSet *melds = &(game->melds);

    // Add a low ace for every high ace
    Heap lowedHand = hand | ((hand & kHighAceMask) >> (VALUE_ACE - VALUE_LOW_ACE));
    meldOpt->runCenters = hand & (hand << 1) & (hand >> 1);
    meldOpt->setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    meldOpt->runExtensions = ((melds->runs << 1) | (melds->runs >> 1)) & hand;
    meldOpt->setExtensions = ((melds->sets << 16) | (melds->sets >> 16)) & hand;
}

void optExclude(Opt *meldOpt, Opt *rejectedOptions) {
    meldOpt->runCenters &= ~rejectedOptions->runCenters;
    meldOpt->runExtensions &= ~rejectedOptions->runExtensions;
    meldOpt->setCenters &= ~rejectedOptions->setCenters;
    meldOpt->setExtensions &= ~rejectedOptions->setExtensions;
}

bool optNone(Opt *meldOpt) {
    return (meldOpt->runCenters == 0 && meldOpt->runExtensions == 0 &&
            meldOpt->setCenters == 0 && meldOpt->setExtensions == 0);
}

Heap optRunCenterToMeld(Heap center) {
    return center | (center << 1) | (center >> 1);
}

Heap optSetCenterToMeld(Heap center) {
    return center |(center << 16) | (center >> 16) | (center >> 48) | (center << 48);
}

void optPrint(Opt *meldOpt) {
    printf("Run Centers: ");
    heapPrint(meldOpt->runCenters);
    printf("\nRun Extensions: ");
    heapPrint(meldOpt->runExtensions);
    printf("\nSet Centers: ");
    heapPrint(meldOpt->setCenters);
    printf("\nSet Extensions: ");
    heapPrint(meldOpt->setExtensions);
    printf("\n");
}

void optTest() {
    Game game;
    gameInit(&game, 3);
    
    // Set hand of the current player
    Heap *hand = &(gameCurrentPlayer(&game)->hand);
    *hand = heapFromString("JC QC KC AC QD QS 7S 9H");

    // Add some already-played melds to the game
    meldSetAddRun(&(game.melds), heapFromString("TH JH QH KH AH"));
    meldSetAddSet(&(game.melds), heapFromString("7H 7C 7D"));

    Opt meldOpt;
    optFind(&game, &meldOpt);
    optPrint(&meldOpt);
}

///////////////////////////////////////////////////////////////////////////////
//
//    Search
//

// TODO:  Check that the deepest card taken from the discard pile was melded.

void searchDraw() {

}

void searchTake() {
    
}

void searchDiscard(Game *game, Play *scratch, Play *best) {
    Player *player = gameCurrentPlayer(game);
    Heap *hand = &(player->hand);

    if (*hand == 0) {
        // Hand is empty.  Discard nothing.
        scratch->discard = 0;
        scratch->eval = evaluate(game);
        playMax(best, scratch);
        printf("\n");
        playPrint(scratch);
    } else {
        // Try discarding each card in the hand
        for (Heap card = heapLowCard(*hand); card != 0; card = heapNextCard(*hand, card)) {
            scratch->discard = card;
            playerDiscard(player, card);
            scratch->eval = evaluate(game);
            printf("\n");
            playPrint(scratch);
            playMax(best, scratch);
            playerUndoDiscard(player);
        }
    }
}

void searchMeldRec(Game *game, Play *scratch, Play *best, Opt *rejectedOptions) {
    Player *player = gameCurrentPlayer(game);
    Heap *hand = &(player->hand);

    Opt meldOptions;
    optFind(game, &meldOptions);
    optExclude(&meldOptions, rejectedOptions);

    if (optNone(&meldOptions)) {
        searchDiscard(game, scratch, best);
        return;
    }

    for (Heap center = heapLowCard(meldOptions.runCenters); center != 0; center = heapNextCard(meldOptions.runCenters, center)) {
        Heap meld = optRunCenterToMeld(center);
        meldSetAddRun(&(scratch->melds), meld);
        playerPlayRun(player, meld);
        searchMeldRec(game, scratch, best, rejectedOptions);
        playerUndoRun(player, meld);
        meldSetRemoveRun(&(scratch->melds), meld);
        heapAdd(&(rejectedOptions->runCenters), center);
    }

    for (Heap center = heapLowCard(meldOptions.setCenters); center != 0; center = heapNextCard(meldOptions.setCenters, center)) {
        Heap meld = optSetCenterToMeld(center);
        meldSetAddSet(&(scratch->melds), meld);
        playerPlaySet(player, meld);
        searchMeldRec(game, scratch, best, rejectedOptions);
        playerUndoSet(player, meld);
        meldSetRemoveSet(&(scratch->melds), meld);
        heapAdd(&(rejectedOptions->setCenters), center);
    }

    for (Heap meld = heapLowCard(meldOptions.runExtensions); meld != 0; meld = heapNextCard(meldOptions.runExtensions, meld)) {
        meldSetAddRun(&(scratch->melds), meld);
        playerPlayRun(player, meld);
        searchMeldRec(game, scratch, best, rejectedOptions);
        playerUndoRun(player, meld);
        meldSetRemoveRun(&(scratch->melds), meld);
        heapAdd(&(rejectedOptions->runExtensions), meld);
    }

    for (Heap meld = heapLowCard(meldOptions.setExtensions); meld != 0; meld = heapNextCard(meldOptions.setExtensions, meld)) {
        meldSetAddSet(&(scratch->melds), meld);
        playerPlaySet(player, meld);
        searchMeldRec(game, scratch, best, rejectedOptions);
        playerUndoSet(player, meld);
        meldSetRemoveSet(&(scratch->melds), meld);
        heapAdd(&(rejectedOptions->setExtensions), meld);
    }

    // All options in this branch were rejected.
    searchMeldRec(game, scratch, best, rejectedOptions);

    heapRemove(&(rejectedOptions->runCenters), meldOptions.runCenters);
    heapRemove(&(rejectedOptions->setCenters), meldOptions.setCenters);
    heapRemove(&(rejectedOptions->runExtensions), meldOptions.runExtensions);
    heapRemove(&(rejectedOptions->setExtensions), meldOptions.setExtensions);
}

void searchMeld(Game *game, Play *scratch, Play *best) {
    Opt rejectedOptions;
    optInit(&rejectedOptions);
    searchMeldRec(game, scratch, best, &rejectedOptions);
}

///////////////////////////////////////////////////////////////////////////////
//
//    Main
//

int main(int argc, char **argv) {
/*
    indexTest();
    heapTest();
    pileTest();
    meldSetTest();
    gameTest();
    evaluateTest();
    */

    Game game;
    gameInit(&game, 3);

    gameCurrentPlayer(&game)->hand = heapFromString("8C 9C TC 2H 2D 2S 4C");

    printf("Starting game position:\n");
    gamePrint(&game);

    Play scratch;
    playInit(&scratch);
    Play best;
    playInit(&best);

    searchMeld(&game, &scratch, &best);
    printf("--- BEST PLAY ---\n");
    playPrint(&best);
}
