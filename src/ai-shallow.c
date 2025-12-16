#include "ai.h"
#include "ai-shallow.h"

void AI_goShallow(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    assert(!game->isOver);
    assert(game->currentPlayer == player);

    AI_resetForGo(ai);
    AI_findBestTakeTurn(ai);
    AI_findBestDrawTurns(ai);
}

void AI_findBestTakeTurn(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    while (Pile_size(&game->discardPile) > 0) {
        Player_take(player, 1);
        AI_findBestMeld(ai, &ai->bestTakeTurn);
    }
    Player_undoTake(player);
}

void AI_findBestDrawTurns(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    int totalEval = 0;
    Cards drawable = Player_couldDraw(player);
    assert(drawable != 0);
    for (Cards c = Cards_first(drawable); c != 0; c = Cards_next(drawable, c)) {
        Turn *turn = &ai->bestDrawTurn[Cards_toCard(c)];
        Cards topCard = Pile_peek(&game->drawPile);

        Game_swap(game, c, topCard);
        Player_draw(player);
        AI_findBestMeld(ai, turn);
        totalEval += turn->eval;
        Player_undoDraw(player);
        Game_swap(game, c, topCard);
    }

    ai->averageDrawEval = totalEval / Cards_size(drawable);
}

void AI_findBestMeld(AI *ai, Turn *bestTurn) {
    Game *game = ai->game;
    Player *player = ai->player;

    // If we take more than one card, then we must meld the deepest card taken.
    Cards mustMeld = game->turn.taken.size > 1 ? Pile_peek(&game->turn.taken) : 0;

    // Generate a list of possible melds
    MeldList_generate(&ai->meldList, player->hand, &game->meld, mustMeld);

    // Try all of the possible melds, and find the best discard for each.
    for (int i = 0; i < ai->meldList.size; ++i) {
        Player_meld(player, &ai->meldList.melds[i]);
        AI_findBestDiscard(ai, bestTurn);
        Player_undoMeld(player, &ai->meldList.melds[i]);
    }
}

void AI_findBestDiscard(AI *ai, Turn *bestTurn) {
    Game *game = ai->game;
    Player *player = ai->player;

    // SPECIAL RULE:  If you took 1 card and didn't meld, then you must discard a different card.
    // This helps prevent infinite loops, where each player keeps taking and discarding the same
    // card over and over.
    Cards illegalDiscard = 0;
    if (Meld_cards(&game->turn.meld) == 0 && game->turn.taken.size == 1) {
        illegalDiscard = game->turn.taken.allCards;
    }
    
    // Consider all legal discards.
    for (Cards c = Cards_first(player->hand); c != 0; c = Cards_next(player->hand, c)) {
        if (c != illegalDiscard) {
            Player_discard(player, c);
            game->turn.eval = AI_evaluateGame(ai);
            Turn_max(bestTurn, &game->turn);
            Player_undoDiscard(player);
        }
    }
}

int AI_evaluateGame(AI *ai) {
    Game *game = ai->game;
    Player *player = ai->player;

    // Credit the player for their current score.
    int baseCentipoints = player->score * 100;

    // If the player's hand is empty, then the game is over.
    // Assume each opponent loses about 700 centipoints per card
    // remaining in their hand.
    if (player->hand == 0) {  
        int penaltyTotal = 0;
        for (int i = 0; i < game->numPlayers; ++i) {
            penaltyTotal += Cards_size(Game_player(game, i)->hand) * 700;
        }
        int penaltyAverage = penaltyTotal / (game->numPlayers - 1);
        return baseCentipoints + penaltyAverage;
    }

    // Count points in the player's hand.  Presumably, these points are
    // helpful early in the game (because we can probably score them), but
    // hurtful later in the game (because we may be stuck with them).
    int handCentipoints = 100 * Cards_points(player->hand);

    // In mode 1, the evaluation is just the base score plus 1 centipoint
    // per point in the player's hand.  So keeping higher cards in your
    // hand is slightly better.  This may be wrong late in the game.
    if (ai->mode == 1) {
        return baseCentipoints + 0.01 * handCentipoints;
    }

    // In mode 2, we also consider the playability of the player's hand.
    if (ai->mode == 2) {
        Cards drawable = Player_couldDraw(player);
        int handPlayability = AI_evaluateHandPlayability(player->hand, &game->meld, drawable);
        return baseCentipoints + handPlayability * 0.5 + 0.01 * handCentipoints;
    }

    // In any other mode, just return the base score.
    return baseCentipoints;
}

int AI_evaluateHandPlayability(Cards hand, Meld *meld, Cards drawable) {
    // Evaluate the quality of a hand given the table meld and
    // cards that could possibly be drawn.  In a good hand, there
    // are many possible draws that enable many playable cards.
    // So, for each drawable card, we'll estimate the number of
    // centipoints (cpts) that can be played from the hand into the meld.
    // Then we'll average over all playable cards.

    int totalCentipoints = 0;
    for (Cards c = Cards_first(drawable); c != 0; c = Cards_next(drawable, c)) {
        // Simulate adding this card to the hand
        Cards simulatedHand = hand | c;
        Cards playable = AI_playableCards(simulatedHand, meld);
        int centipoints = Cards_points(playable) * 100;  // convert to cpts
        totalCentipoints += centipoints;
    }

    // Return the average centipoints per drawable cards
    int numDrawable = Cards_size(drawable);
    int averageCentipoints = numDrawable > 0 ? totalCentipoints / numDrawable : 0;

    return averageCentipoints;
}

Cards AI_playableCards(Cards hand, Meld *meld) {
    // Find all cards in the hand that can be played in the meld.
    // These may not be simultaneously playable.
    Cards lowHand = Cards_addLowAces(hand);
    Cards runCenters = hand & (lowHand << 1) & (hand >> 1);
    Cards setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    Cards runExtensions = ((meld->runs << 1) | (meld->runs >> 1)) & lowHand;
    Cards extendedMeldsRuns = meld->runs | runExtensions;
    runExtensions = ((extendedMeldsRuns << 1) | (extendedMeldsRuns >> 1)) & lowHand;
    Cards setExtensions = ((meld->sets << 16) | (meld->sets >> 16)) & lowHand;
    Cards playable = runCenters | (runCenters << 1) | (runCenters >> 1) | runExtensions |
                     setCenters | (setCenters << 16) | (setCenters >> 16) |
                     (setCenters << 48) | (setCenters >> 48) | setExtensions;
    // Kick out low aces if the corresponding high aces are playable.
    playable &= ~((playable & kHighAces) >> 13);
    assert(Cards_isLegal(playable));
    return playable;
}
