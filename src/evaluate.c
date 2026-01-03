   if (player->hand == 0) {  
        int penaltyTotal = 0;
        for (int i = 0; i < game->numPlayers; ++i) {
            penaltyTotal += Cards_size(Game_player(game, i)->hand) * 700;
        }
        int penaltyAverage = penaltyTotal / (game->numPlayers - 1);
        return eval + penaltyAverage;
    }