#ifndef PLAY_H
#define PLAY_H

#include "cards.h"
#include "game.h"
#include "table.h"

typedef struct PlayStruct {
    Cards runCenters;
    Cards runExtensions;
    Cards setCenters;
    Cards setExtensions;
} Play;

void Play_init(Play *play) {
    play->runCenters = 0;
    play->runExtensions = 0;
    play->setCenters = 0;
    play->setExtensions = 0;
}

void Play_find(Game *game, Play *play) {
    Cards hand = Game_currentPlayer(game)->hand;
    Table *table = &(game->table);

    // Add a low ace for every high ace
    Cards lowHand = Cards_addLowAces(hand);
    play->runCenters = hand & (hand << 1) & (hand >> 1);
    play->setCenters = (hand & ((hand << 16) | (hand >> 48)) & ((hand >> 16) | (hand << 48)));
    play->runExtensions = ((table->runs << 1) | (table->runs >> 1)) & lowHand;
    play->setExtensions = ((table->sets << 16) | (table->sets >> 16)) & lowHand;
}

void Play_exclude(Play *play, Play *rejected) {
    play->runCenters &= ~rejected->runCenters;
    play->runExtensions &= ~rejected->runExtensions;
    play->setCenters &= ~rejected->setCenters;
    play->setExtensions &= ~rejected->setExtensions;
}

bool Play_none(Play *play) {
    return (play->runCenters == 0 && play->runExtensions == 0 &&
            play->setCenters == 0 && play->setExtensions == 0);
}

Cards Play_runCenterToMeld(Cards center) {
    return center | (center << 1) | (center >> 1);
}

Cards Play_setCenterToMeld(Cards center) {
    return center |(center << 16) | (center >> 16) | (center >> 48) | (center << 48);
}

#endif // PLAY_H
