#include <stdio.h>

#include "ai.h"
#include "game.h"
#include "playgen.h"

static int AI_Evaluate(Game *game) {
    // Take the score of the current player minus the score of the average rival.
    Player *player = Game_currentPlayer(game);
    int playerScore = player->score;
    int averageRivalScore = 0;
    for (int i = 0; i < game->numPlayers; ++i) {
        if (i != player->id) {
            averageRivalScore += game->players[i].score;
        }
    }
    averageRivalScore /= (game->numPlayers - 1);

    // The evaluation is the player's score minus the average rival score.
    return player->turn.eval = playerScore - averageRivalScore;
}

static void AI_take(Game *game, Turn *bestTakeTurn) {
    Player *player = Game_currentPlayer(game);
    Turn_init(&player->turn);
    Pile *discardPile = &game->discardPile;

    // Try taking each number of cards in the discard pile, up to all of them.
    while (Pile_size(discardPile) > 0) {
        Player_take(player);
    }
    Player_undoTakes(player);
}

static void AI_findBestTakeTurn(Game *game, Turn *bestTakeTurn) {
    Player *player = Game_currentPlayer(game);
    Turn_init(&player->turn);

    Pile *discardPile = &game->discardPile;

    // Try taking each number of cards in the discard pile, up to all of them.
    while (Pile_size(discardPile) > 0) {
        Cards c = Player_take(player);
        PlayGen_generate(player->hand, &game->table);

    }
    Player_undoTakes(player);
    printf("Returned cards to discard pile\n");
}

void AI_go(Game *game) {
    // We need to find the best turn for the current player and
    // update the game state accordingly.  These are two main
    // cases:  taking from the discard pile and drawing from the draw pile.

    // Find the best turn where 1 or more cards are taken from the discard pile.
    Turn bestTakeTurn;
    Turn_init(&bestTakeTurn);
    AI_findBestTakeTurn(game, &bestTakeTurn);

    // TODO:
    // Find all cards that could be in the draw pile.
    // Call AI_findBestDrawTurn for each possible drawn card.

    // Is the average draw turn better than the best take turn?
    // If so, then we'll draw; otherwise, we'll take.
    double averageDrawEval = 0.0;

    // If we go with a take turn, then just apply it to the game
    // in a strightforward way.
    // If we got with a draw turn, then do the draw and finish
    // according to the specified best draw turn.
}