#include "graphics.h"

void change_card(struct Sprite *sprite, uint8_t card_id) {
    sprite_remap(sprite, CARD_WIDTH * (card_id % CARDS_PER_SUIT) + CARD_LOC_X, CARD_HEIGHT * (card_id / CARDS_PER_SUIT) + CARD_LOC_Y);
}

void flip_card(struct Sprite *sprite) {
    sprite_remap(sprite, CARD_BACK_X, CARD_BACK_Y);
}

void init_card(struct Sprite *sprite, uint8_t card_id, uint16_t x, uint16_t y) {
    sprite->mapping.height = CARD_HEIGHT;
    sprite->mapping.width = CARD_WIDTH;
    sprite_register(sprite, x, y);
    change_card(sprite, card_id);
}

void init_dot(struct Sprite *sprite, uint8_t dot_id, uint16_t x, uint16_t y){
    sprite->mapping.height = 1;
    sprite->mapping.width = 1;
    sprite_register(sprite, x, y);
    change_card(sprite, dot_id);
}

void init_neo_dot(struct Sprite *sprite, uint16_t x, uint16_t y) {
    sprite->mapping.height = NEO_DOT_HEIGHT;
    sprite->mapping.width = NEO_DOT_WIDTH;
    sprite_register(sprite, x, y);
    sprite_remap(sprite, NEO_DOT_LOC_X, NEO_DOT_LOC_Y);
}

void init_string(struct SpriteString *fresh, char *str, uint8_t len, uint16_t x, uint16_t y) {
    for (int i = 0; i < fresh->len && i < len; i++) {
        init_char(&(fresh->arr)[i], str[i], x + 5 * i * CHAR_WIDTH, y);
    }
}

void update_string(struct SpriteString *fresh, char *str, uint8_t len) {
    for (int i = 0; i < fresh->len || i < len; i++) {
        change_char(&(fresh->arr)[i], str[i]);
    }
}

void draw_string(struct SpriteString *obj) {
    for (int i = 0; i < obj->len; i++) {
        sprite_draw(&(obj->arr)[i]);
    }
}

void erase_string(struct SpriteString *obj) {
    for (int i = 0; i < obj->len; i++) {
        sprite_erase(&(obj->arr)[i]);
    }
}

void move_string(struct SpriteString *obj, uint16_t x, uint16_t y) {
    int delta_x, delta_y;
    delta_x = x - obj->arr->x;
    delta_y = y - obj->arr->y;
    for (int i = 0; i < obj->len; i++) {
        sprite_move(&(obj->arr)[i], obj->arr[i].x + delta_x, obj->arr[i].y + delta_y);
    }
}

void change_char(struct Sprite *sprite, char chr) {
    if (chr >= '0' && chr <= '9') {
        sprite_remap(sprite, CHAR_WIDTH * (chr - '0') + NUM_LOC_X, NUM_LOC_Y);
    } else if (chr >= 'A' && chr <= 'Z') {
        sprite_remap(sprite, CHAR_WIDTH * (chr - 'A') + CHAR_LOC_X, CHAR_LOC_Y);
    } else if (chr == ' ') {
        sprite_remap(sprite, CHAR_LOC_X, CHAR_LOC_Y + CHAR_HEIGHT);
    }
}

void init_char(struct Sprite *sprite, char chr, uint16_t x, uint16_t y) {
    sprite->mapping.height = CHAR_HEIGHT;
    sprite->mapping.width = CHAR_WIDTH;
    sprite_register(sprite, x, y);
    change_char(sprite, chr);
}

void string_convert(char *arr, int len, int val) {
    for (int i = len - 1; i >= 0; i--) {
        arr[i] = (val % 10) + '0';
        val /= 10;
    }
}

void string_copy(char *dest, char *src, int len) {
    for (int i = 0; i < len; i++) {
        dest[i] = src[i];
    }
}

void clear_screen(void) {
    struct Sprite screen;
    screen.mapping.width = ROW_MAX;
    screen.mapping.height = VERT_RES / 4;
    sprite_register(&screen, 0, 0);
    screen.status |= SPRITE_IS_VISIBLE;
    sprite_erase(&screen);
    render_pass();
}