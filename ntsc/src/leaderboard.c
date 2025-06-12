#include "leaderboard.h"

static void entry_from_leaderboard(struct EepromEntry *entry, struct Leaderboard *board);
static void leaderboard_from_entry(struct Leaderboard *board, struct EepromEntry *entry);
static uint32_t _raw_from_score(struct Score *score);
static void _score_from_raw(struct Score *score, uint32_t raw);

bool high_score_check(struct Leaderboard *board, uint32_t points) {
    sort_leaderboard(board);
    return (points > board->entry[MAX_SCORES - 1].points);
}

void insert_known_highscore(struct Leaderboard *board, struct Score *score) {
    sort_leaderboard(board);
    board->entry[MAX_SCORES - 1] = *score;
    sort_leaderboard(board);
}

void sort_leaderboard(struct Leaderboard *board) {
    struct Score *arr = board->entry;
    // Insertion sort
    for (int i = 1; i < MAX_SCORES; i++) {
        struct Score key = arr[i];
        int j = i - 1;
        
        while (j >= 0 && arr[j].points < key.points) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

static void entry_from_leaderboard(struct EepromEntry *entry, struct Leaderboard *board) {
    for (int i = 0; i < MAX_SCORES; i++) {
        entry->word[i] = _raw_from_score(&board->entry[i]);
    }
}

static void leaderboard_from_entry(struct Leaderboard *board, struct EepromEntry *entry) {
    for (int i = 0; i < MAX_SCORES; i++) {
        _score_from_raw(&board->entry[i], entry->word[i]);
    }
}

static uint32_t _raw_from_score(struct Score *score) {
    uint32_t raw = score->points & POINTS_MASK;
    for (int i = 0; i < NAME_LENGTH; i++) {
        if (score->name[i] < 'A' || score->name[i] > 'Z') return 0;
        raw |= (uint32_t)(score->name[i] - 'A') << (32 - (i + 1) * LETTER_BITS);
    }
    return raw;
}

static void _score_from_raw(struct Score *score, uint32_t raw) {
    score->points = raw & POINTS_MASK;
    raw >>= 32 - NAME_LENGTH * LETTER_BITS;
    for (int i = 0; i < NAME_LENGTH; i++) {
        score->name[NAME_LENGTH - 1 - i] = (raw & LETTER_MASK) + 'A';
        raw >>= LETTER_BITS;
    }
    return;
}

void read_leaderboard(struct Leaderboard *board) {
    board_ptr = board;
    eeprom_status = EEPROM_READ;
    while (eeprom_status != EEPROM_IDLE);
}

void write_leaderboard(struct Leaderboard *board) {
    board_ptr = board;
    eeprom_status = EEPROM_WRITE;
    while (eeprom_status != EEPROM_IDLE);
}

void _read_leaderboard(struct Leaderboard *board) {
    struct EepromEntry entry;
    read_entry(LEADERBOARD_LOC, &entry);
    leaderboard_from_entry(board, &entry);
}

void _write_leaderboard(struct Leaderboard *board) {
    struct EepromEntry entry;
    entry_from_leaderboard(&entry, board);
    write_entry(LEADERBOARD_LOC, &entry);
}