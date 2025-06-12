#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "eeprom.h"

#define LEADERBOARD_LOC 0x0
#define SCORE_DIGITS_BASE10 (6)
#define LETTER_BITS 5
#define NAME_LENGTH 3
#define LETTER_MASK 0x1F
#define POINTS_MASK 0x1FFFF

struct Score {
    char name[NAME_LENGTH];
    uint32_t points;
};

struct Leaderboard {
    struct Score entry[MAX_SCORES];
};


void read_leaderboard(struct Leaderboard *board);
void write_leaderboard(struct Leaderboard *board);
void _read_leaderboard(struct Leaderboard *board);
void _write_leaderboard(struct Leaderboard *board);
void sort_leaderboard(struct Leaderboard *board);
bool high_score_check(struct Leaderboard *board, uint32_t points);
void insert_known_highscore(struct Leaderboard *board, struct Score *score);

#endif