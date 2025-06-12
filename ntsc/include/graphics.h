#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "sprite.h"

#define CARDS_PER_SUIT 13
#define CARD_LOC_X 0
#define CARD_LOC_Y 0
#define CARD_BACK_X 0
#define CARD_BACK_Y 32
#define CARD_WIDTH 6
#define CARD_HEIGHT 8

#define NUM_LOC_X 32
#define NUM_LOC_Y 32
#define CHAR_LOC_X 6
#define CHAR_LOC_Y 32
#define CHAR_WIDTH 1
#define CHAR_HEIGHT 2

#define NEO_DOT_LOC_X (43)
#define NEO_DOT_LOC_Y (32)
#define NEO_DOT_WIDTH (1)
#define NEO_DOT_HEIGHT (1)

struct SpriteString {
    struct Sprite *arr;
    uint8_t len;
};

void change_card(struct Sprite *sprite, uint8_t card_id);
void flip_card(struct Sprite *sprite);
void init_card(struct Sprite *sprite, uint8_t card_id, uint16_t x, uint16_t y);
void init_char(struct Sprite *sprite, char chr, uint16_t x, uint16_t y);
void change_char(struct Sprite *sprite, char chr);
void init_string(struct SpriteString *fresh, char *str, uint8_t len, uint16_t x, uint16_t y);
void draw_string(struct SpriteString *obj);
void erase_string(struct SpriteString *obj);
void move_string(struct SpriteString *obj, uint16_t x, uint16_t y);
void init_dot(struct Sprite *sprite, uint8_t dot_id, uint16_t x, uint16_t y);
void init_neo_dot(struct Sprite *sprite, uint16_t x, uint16_t y);

void clear_screen();

void string_convert(char *arr, int len, int val);
void string_copy(char *dest, char *src, int len);

#endif