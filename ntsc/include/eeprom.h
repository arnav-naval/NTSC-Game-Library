#ifndef EEPROM_H
#define EEPROM_H

#include "common.h"

#define EEPROM_ADDR 0x57
#define ENTRY_BYTES 32
#define SCORE_SIZE 4
#define MAX_SCORES (ENTRY_BYTES / SCORE_SIZE)

struct EepromEntry {
    uint32_t word[MAX_SCORES];
};

enum EepromStatus {
    EEPROM_IDLE,
    EEPROM_READ,
    EEPROM_WRITE
};

extern volatile enum EepromStatus eeprom_status;
extern struct Leaderboard *board_ptr;

void setup_eeprom();
void eeprom_write(uint16_t loc, const char* data, uint8_t len);
void eeprom_read(uint16_t loc, char data[], uint8_t len);
void read_entry(uint16_t loc, struct EepromEntry *entry);
void write_entry(uint16_t loc, struct EepromEntry *entry);

#endif