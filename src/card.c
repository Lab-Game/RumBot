#include "card.h"

const Card kIllegalCard = 0xff;

const char *kCardValues = "a23456789TJQKA";
const char *kCardSuits = "CDSH";

static const char *kCardName[64] = {
    "aC", "2C", "3C", "4C", "5C", "6C", "7C", "8C", "9C", "TC", "JC", "QC", "KC", "AC", "14?", "15?",
    "aD", "2D", "3D", "4D", "5D", "6D", "7D", "8D", "9D", "TD", "JD", "QD", "KD", "AD", "30?", "31?",
    "aS", "2S", "3S", "4S", "5S", "6S", "7S", "8S", "9S", "TS", "JS", "QS", "KS", "AS", "62?", "63?",
    "aH", "2H", "3H", "4H", "5H", "6H", "7H", "8H", "9H", "TH", "JH", "QH", "KH", "AH", "46?", "47?",
};

// Escape the two-letter code to add color.
static const char *kColorCardName[64] = {
    "\x1B[34maC\x1B[0m", "\x1B[34m2C\x1B[0m", "\x1B[34m3C\x1B[0m", "\x1B[34m4C\x1B[0m", "\x1B[34m5C\x1B[0m", "\x1B[34m6C\x1B[0m", "\x1B[34m7C\x1B[0m", "\x1B[34m8C\x1B[0m", "\x1B[34m9C\x1B[0m", "\x1B[34mTC\x1B[0m", "\x1B[34mJC\x1B[0m", "\x1B[34mQC\x1B[0m", "\x1B[34mKC\x1B[0m", "\x1B[34mAC\x1B[0m", "14?", "15?",
    "\x1B[31maD\x1B[0m", "\x1B[31m2D\x1B[0m", "\x1B[31m3D\x1B[0m", "\x1B[31m4D\x1B[0m", "\x1B[31m5D\x1B[0m", "\x1B[31m6D\x1B[0m", "\x1B[31m7D\x1B[0m", "\x1B[31m8D\x1B[0m", "\x1B[31m9D\x1B[0m", "\x1B[31mTD\x1B[0m", "\x1B[31mJD\x1B[0m", "\x1B[31mQD\x1B[0m", "\x1B[31mKD\x1B[0m", "\x1B[31mAD\x1B[0m", "30?", "31?",
    "\x1B[34maS\x1B[0m", "\x1B[34m2S\x1B[0m", "\x1B[34m3S\x1B[0m", "\x1B[34m4S\x1B[0m", "\x1B[34m5S\x1B[0m", "\x1B[34m6S\x1B[0m", "\x1B[34m7S\x1B[0m", "\x1B[34m8S\x1B[0m", "\x1B[34m9S\x1B[0m", "\x1B[34mTS\x1B[0m", "\x1B[34mJS\x1B[0m", "\x1B[34mQS\x1B[0m", "\x1B[34mKS\x1B[0m", "\x1B[34mAS\x1B[0m", "62?", "63?",
    "\x1B[31maH\x1B[0m", "\x1B[31m2H\x1B[0m", "\x1B[31m3H\x1B[0m", "\x1B[31m4H\x1B[0m", "\x1B[31m5H\x1B[0m", "\x1B[31m6H\x1B[0m", "\x1B[31m7H\x1B[0m", "\x1B[31m8H\x1B[0m", "\x1B[31m9H\x1B[0m", "\x1B[31mTH\x1B[0m", "\x1B[31mJH\x1B[0m", "\x1B[31mQH\x1B[0m", "\x1B[31mKH\x1B[0m", "\x1B[31mAH\x1B[0m", "46?", "47?",
};

const char *Card_name(Card i) {
    assert(Card_isLegal(i));
    return kCardName[i];
}

const char *Card_colorName(Card i) {
    assert(Card_isLegal(i));
    assert((i >> 4) <= 3);    // suit 0-3
    return kColorCardName[i];
}
