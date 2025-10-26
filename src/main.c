#include <stdio.h>

#include "cards.h"
#include "pile.h"
#include "table.h"
#include "game.h"
#include "turn.h"

// While searching for the best turn, we need to consider:
// - The best turn that involves initially taking cards from the discard pile.
//   In this case, there is no uncertainty about the point value of the turn.
// - We also need to consider drawing a card from the draw point.  In this
//   case, many cards could come up.  We need to evaluate the best subsequent
//   actions in each case, and then average the results.
// So where do we store all this data?
//
// I think I should score the "scratch" turn in the Player structure, because
// that will be filled out by calling Player functions.
// But how about the "best" turns, including the best for a "take" turn
// and the best possible response to each possible drawn card?
//

// This evaluates the state of the game from the perspective of the current
// player, assuming that the current player has just completed their turn.
// The goal is to estimate the current player's score minus the average
// score of rivals at the end of the game.
int Eval(Game *game) {
    Player *player = Game_currentPlayer(game);

    // First compute a "basic evaluation" based on points already scored.
    int playerScore = player->score;
    int averageRivalScore = 0;
    for (int i = 0; i < game->numPlayers; ++i) {
        if (i != player->id) {
            averageRivalScore += game->players[i].score;
        }
    }
    averageRivalScore /= (game->numPlayers - 1);
    int basicEval = playerScore - averageRivalScore;

    
    int pointsInHand = Cards_points(player->hand);
    if (pointsInHand == 0) {
        // Player has gone out.  Count points in opponents' hands against them.
        int pointsFromRivals = 0;
        for (int p = 0; p < game->numPlayers; ++p) {
            if (p != player->id) {
                pointsFromRivals += 7 * Cards_size(game->players[p].hand);
            }
        }
        return basicEval + pointsFromRivals;
    }

    // Player has not gone out.  Points in hand may be positive or negative,
    // depending on the stage of the game.  In particular, if 
}

void Search_meld(Game *game, Turn *bestTurn) {

}

void Search_take(Game *game, Turn *bestTakeTurn) {
    Player *player = Game_currentPlayer(game);
    Turn_init(&player->turn);
    Pile *discardPile = &game->discardPile;

    // Try taking each number of cards in the discard pile, up to all of them.
    while (Pile_size(discardPile) > 0) {
        Player_take(player);
        Search_meld(game, bestTakeTurn);
    }
    Player_undoTakes(player);
}

int main(void) {
    Game game;
    Game_init(&game);
    Game_print(&game);

    Turn bestTakeTurn;
    Turn_init(&bestTakeTurn);
    Search_take(&game, &bestTakeTurn);
    
    return 0;
}