#include <stdlib.h>
#include <stdio.h>

#include "rumbot.h"
#include "game.h"
#include "ai.h"
#include "log.h"

int DEB = 2;  // Debug level (0=none, 1=some, 2=more, 3=lots)

void playGame(AI *ais[], Game *game, FILE *log_file) {
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        AI_joinGame(ais[i], game, Game_player(game, i));
    }

    if (DEB >= 1) {
        printf("\n=== NEW GAME ===\n");
    }

    if (log_file) {
        Log_writeGame(log_file, game);
    }

    while (!game->isOver) {
        if (DEB >= 1) {
            printf("\n=== Player %d ===\n", game->currentPlayer->id);
            Game_print(game);
        }

        AI *ai = ais[game->currentPlayerId];
        Turn *turn = AI_go(ai);

        if (DEB >= 1) {
            printf("AI: ");
            Turn_print(turn);
        }

        Player_play(game->currentPlayer, turn);
        if (log_file) {
            Log_writeTurn(log_file, turn);
        }

        if (DEB >= 2) {
            printf("  ");
            Player_print(game->currentPlayer);
        }

        Game_nextTurn(game);

        if (DEB >= 1) {
            printf("...\n");
        }
    }

    if (DEB >= 1) {
        printf("\n=== Game Over ===\n");
        Game_print(game);
    }

    // Add scores to AI total scores
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        ais[i]->totalScore += ais[i]->player->score;
    }
}

void playAllOrders(AI *ai, Game *game) {
    Game copy;
    AI *shuffled_ais[NUM_PLAYERS];

    Game_copy(game, &copy);
    shuffled_ais[0] = ai;
    shuffled_ais[1] = ai + 1;
    shuffled_ais[2] = ai + 2;
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai;
    shuffled_ais[1] = ai + 2;
    shuffled_ais[2] = ai + 1;
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 1;
    shuffled_ais[1] = ai;
    shuffled_ais[2] = ai + 2;
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 1;
    shuffled_ais[1] = ai + 2;
    shuffled_ais[2] = ai;
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 2;
    shuffled_ais[1] = ai;
    shuffled_ais[2] = ai + 1;
    playGame(shuffled_ais, &copy, NULL);

    Game_copy(game, &copy);
    shuffled_ais[0] = ai + 2;
    shuffled_ais[1] = ai + 1;
    shuffled_ais[2] = ai;
    playGame(shuffled_ais, &copy, NULL);
}

void readGameFromLog() {
    Game game;
    Game_init(&game);

    FILE *log_file = NULL;
    log_file = fopen("logs/game.log", "r");
    if (!log_file) {
        fprintf(stderr, "Error: could not open log file for reading\n");
        exit(1);
    }

    Log_readGame(log_file, &game);
    printf("Done reading game from log file. Current game state:\n");
    Game_print(&game);
    printf("exiting...\n");
    fclose(log_file);
    exit(0);
}  

int main(int argc, char *argv[]) {
    FILE *log_file = NULL;
    if (argc > 1) {
        log_file = fopen(argv[1], "w");
        if (!log_file) {
            fprintf(stderr, "Error: could not open log file %s for writing\n", argv[1]);
            return 1;
        }
    }

    AI ais[NUM_PLAYERS];
    AI *ai_ptrs[NUM_PLAYERS];

    AI_init(&ais[0], 2);
    AI_init(&ais[1], 1);
    AI_init(&ais[2], 1);
    
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        ai_ptrs[i] = &ais[i];
    }

    const int numGames = 1;

    /*
    
-> Player 0 (  0 pts)  2C 6D QD 6S 8S TS KH
   Player 1 (  0 pts)  TC 5D 9D KD 2H 5H TH
   Player 2 (  0 pts)  9C QC TD 7S AS 4H JH
Draw: 3C AC 4D QH 2S 8D 4C 5S 9S 3H JD JS 5C 9H 2D 6C QS 8C 7D AD 4S AH KC 3S JC 3D 8H 7H 7C KS
Discard: 6H

     Take 1:
  Meld :
   Discard KH:  Games: 100  Scores:   3945   3740   3825
   Discard TS:  Games: 100  Scores:   3245   3750   3955
   Discard 8S:  Games: 100  Scores:   4250   3775   4065
   Discard 6S:  Games: 100  Scores:   3955   4135   3840
   Discard QD:  Games: 100  Scores:   3840   4170   3620
   Discard 6D:  Games: 100  Scores:   3800   3755   4155
   Discard 2C:  Games: 100  Scores:   3835   3660   3330
  Meld {6D 6S 6H} :
   Discard KH:  Games: 100  Scores:   3970   3815   3835
   Discard TS:  Games: 100  Scores:   3340   3990   4115
   Discard 8S:  Games: 100  Scores:   3570   3645   3810
   Discard QD:  Games: 100  Scores:   4010   4210   3500
   Discard 2C:  Games: 100  Scores:   3700   3465   3120
    

 Take 1:
  Meld :
   Discard KH:  Games: 100  Scores:   3475   3830   4115
   Discard TS:  Games: 100  Scores:   3690   3850   4360
   Discard 8S:  Games: 100  Scores:   3400   3515   4040
   Discard 6S:  Games: 100  Scores:   3830   2980   3825
   Discard QD:  Games: 100  Scores:   3905   3645   3680
   Discard 6D:  Games: 100  Scores:   3705   3220   3775
   Discard 2C:  Games: 100  Scores:   3565   3695   3570
  Meld {6D 6S 6H} :
   Discard KH:  Games: 100  Scores:   3530   3570   3940
   Discard TS:  Games: 100  Scores:   3380   3750   4360
   Discard 8S:  Games: 100  Scores:   3610   2900   3990
   Discard QD:  Games: 100  Scores:   3915   3525   3505
   Discard 2C:  Games: 100  Scores:   3510   3590   3345


Take 1:
  Meld :
   Discard KH:  Games: 100  Scores:   4315   3995   4075
   Discard TS:  Games: 100  Scores:   3515   3960   4235
   Discard 8S:  Games: 100  Scores:   3710   4035   4335
   Discard 6S:  Games: 100  Scores:   4185   3830   4135
   Discard QD:  Games: 100  Scores:   4325   4395   3850
   Discard 6D:  Games: 100  Scores:   4195   4195   4095
   Discard 2C:  Games: 100  Scores:   4350   3890   3970
  Meld {6D 6S 6H} :
   Discard KH:  Games: 100  Scores:   4050   3870   3975
   Discard TS:  Games: 100  Scores:   3780   4300   4770
   Discard 8S:  Games: 100  Scores:   3470   4260   4550
   Discard QD:  Games: 100  Scores:   4035   4300   3705
   Discard 2C:  Games: 100  Scores:   3910   4285   4270


 Take 1:
  Meld :
   Discard KH:  Games: 1000  Scores:  38145  37750  37640
   Discard TS:  Games: 1000  Scores:  36150  41120  41255
   Discard 8S:  Games: 1000  Scores:  37325  40330  38555
   Discard 6S:  Games: 1000  Scores:  37070  37430  37415
   Discard QD:  Games: 1000  Scores:  36925  39025  34790
   Discard 6D:  Games: 1000  Scores:  36585  37330  38220
   Discard 2C:  Games: 1000  Scores:  35670  38005  36685
  Meld {6D 6S 6H} :
   Discard KH:  Games: 1000  Scores:  37915  38230  36560
   Discard TS:  Games: 1000  Scores:  35450  40800  40890
   Discard 8S:  Games: 1000  Scores:  37120  40155  38765
   Discard QD:  Games: 1000  Scores:  37000  38290  35805
   Discard 2C:  Games: 1000  Scores:  34890  39785  37965

Take 1:
  Meld :
   Discard KH:  Games: 100  Scores:   3120 (  -692)   3880 (   448)   3745 (   245)
   Discard TS:  Games: 100  Scores:   3605 (  -942)   4295 (    93)   4800 (   850)
   Discard 8S:  Games: 100  Scores:   4145 (   600)   3740 (    -7)   3350 (  -592)
   Discard 6S:  Games: 100  Scores:   4755 (   740)   4090 (  -257)   3940 (  -482)
   Discard QD:  Games: 100  Scores:   3865 (   345)   3710 (   113)   3330 (  -457)
   Discard 6D:  Games: 100  Scores:   4215 (   -52)   4215 (   -52)   4320 (   105)
   Discard 2C:  Games: 100  Scores:   4155 (   400)   3770 (  -177)   3740 (  -222)
  Meld {6D 6S 6H} :
   Discard KH:  Games: 100  Scores:   3065 (  -527)   3875 (   688)   3310 (  -160)
   Discard TS:  Games: 100  Scores:   3885 (  -560)   4305 (    70)   4585 (   490)
   Discard 8S:  Games: 100  Scores:   4040 (   293)   3985 (   210)   3510 (  -502)
   Discard QD:  Games: 100  Scores:   3290 (  -455)   4000 (   610)   3490 (  -155)
   Discard 2C:  Games: 100  Scores:   4245 (   610)   3370 (  -702)   3900 (    93)

   Best option is to discard the 6S.  That seems nuts!

    Take 1:
  Meld :
   Discard KH:  Games: 100  Scores:   3720 (  -352)   4505 (   825)   3640 (  -472)
   Discard TS:  Games: 100  Scores:   3810 (  -670)   4700 (   665)   4260 (     5)
   Discard 8S:  Games: 100  Scores:   3865 (   -30)   4280 (   593)   3510 (  -562)
   Discard 6S:  Games: 100  Scores:   4105 (   450)   3965 (   240)   3345 (  -690)
   Discard QD:  Games: 100  Scores:   3645 (  -477)   3860 (  -155)   4385 (   633)
   Discard 6D:  Games: 100  Scores:   3870 (   170)   4115 (   538)   3285 (  -707)
   Discard 2C:  Games: 100  Scores:   4045 (   310)   3720 (  -177)   3750 (  -132)
  Meld {6D 6S 6H} :
   Discard KH:  Games: 100  Scores:   3265 (  -870)   4410 (   848)   3860 (    23)
   Discard TS:  Games: 100  Scores:   3505 (  -927)   4475 (   528)   4390 (   400)
   Discard 8S:  Games: 100  Scores:   3850 (   183)   4070 (   513)   3265 (  -695)
   Discard QD:  Games: 100  Scores:   3765 (  -737)   4380 (   185)   4625 (   553)
   Discard 2C:  Games: 100  Scores:   3400 (  -490)   4165 (   658)   3615 (  -167)
 
 
    Take 1:
  Meld :
   Discard KH:  Games: 1000  Scores:  37605 (  1753)  35815 (  -932)  35890 (  -820)
   Discard TS:  Games: 1000  Scores:  37380 ( -2577)  40175 (  1615)  39740 (   963)
   Discard 8S:  Games: 1000  Scores:  38055 ( -2122)  40685 (  1823)  39670 (   300)
   Discard 6S:  Games: 1000  Scores:  40475 (  2478)  38445 (  -567)  37550 ( -1910)
   Discard QD:  Games: 1000  Scores:  37620 (  -455)  38785 (  1293)  37365 (  -837)
   Discard 6D:  Games: 1000  Scores:  40375 (  2815)  37325 ( -1760)  37795 ( -1055)
   Discard 2C:  Games: 1000  Scores:  36170 ( -1497)  38210 (  1563)  37125 (   -65)
  Meld {6D 6S 6H} :
   Discard KH:  Games: 1000  Scores:  37280 (  -420)  37580 (    30)  37820 (   390)
   Discard TS:  Games: 1000  Scores:  37220 ( -3175)  40730 (  2090)  40060 (  1085)
   Discard 8S:  Games: 1000  Scores:  37810 ( -1497)  39435 (   940)  39180 (   558)
   Discard QD:  Games: 1000  Scores:  37585 (    93)  38475 (  1428)  36510 ( -1520)
   Discard 2C:  Games: 1000  Scores:  35555 ( -1742)  36570 (  -220)  38025 (  1963)
 
   */

    Game game;
    for (int i = 0; i < numGames; ++i) {
        Game_init(&game);
        Game_shuffle(&game);
        Pile_print(&game.drawPile);
        Pile_fromString(&game.drawPile, "3C AC 4D QH 2S 8D 4C 5S 9S 3H JD JS 5C 9H 2D 6C QS 8C 7D AD 4S AH KC 3S JC 3D 8H 7H 7C KS 6H 7S 9C JH AS TD QC 4H KD 5D TC 5H 2H TH 9D 2C 6S 6D TS QD 8S KH");
        Game_deal(&game);
        playGame(ai_ptrs, &game, log_file);
    }

    // Print final AI scores
    printf("\n=== FINAL SCORES AFTER %d GAMES ===\n", numGames);
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        printf("AI %d (mode %d): %d points\n", i, ais[i].mode, ais[i].totalScore);
    }

    if (log_file) {
        fclose(log_file);
    }

    return 0;
}