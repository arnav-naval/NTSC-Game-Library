#include "sprite.h"
#include "tilemap.h"
#include "render.h"

static void _sprite_draw(struct Sprite *sprite);
static void _sprite_erase(struct Sprite *sprite);

struct Sprite * sprite_register(struct Sprite *new, uint16_t x, uint16_t y) {
    if (new == NULL) return NULL;
    new->x = x;
    new->y = y;
    new->status = 0;
    return new;
}

void sprite_draw(struct Sprite *sprite) {
    if (!(sprite->status & SPRITE_IS_VISIBLE)) {
        _sprite_draw(sprite);
        sprite->status |= SPRITE_ERASE;
    }
}

static void _sprite_draw(struct Sprite *sprite) {
    draw_arr[draw_count] = sprite;
    draw_count++;
}

void sprite_erase(struct Sprite *sprite) {
    if (sprite->status & SPRITE_IS_VISIBLE) {
        _sprite_erase(sprite);
        sprite->status |= SPRITE_DRAW;
    }
}

static void _sprite_erase(struct Sprite *sprite) {
    sprite->x_alt = sprite->x;
    sprite->y_alt = sprite->y;
    erase_arr[erase_count] = sprite;
    erase_count++;
}

void sprite_remap(struct Sprite *sprite, uint16_t col, uint16_t row) {
    if (tile_update_coord(&(sprite->mapping), col, row)) return;
    if (sprite->status & SPRITE_IS_VISIBLE) {
        _sprite_draw(sprite);
    }
}

void sprite_move(struct Sprite *sprite, uint16_t x, uint16_t y) {
    if (sprite->x == x && sprite->y == y) return;
    if (sprite->status & SPRITE_IS_VISIBLE) {
        _sprite_erase(sprite);
        _sprite_draw(sprite);
        sprite->status |= SPRITE_MOVE;
    }
    sprite->x = x;
    sprite->y = y;
}

void sort_sprites(struct Sprite *arr[], int n, bool alt)
{
    // Insertion sort
    for (int i = 1; i < n; i++) {
        struct Sprite *key = arr[i];
        int j = i - 1;
        
        if (alt) {
            while (j >= 0 && arr[j]->y_alt > key->y_alt) {
                arr[j + 1] = arr[j];
                j = j - 1;
            }
        } else {
            while (j >= 0 && arr[j]->y > key->y) {
                arr[j + 1] = arr[j];
                j = j - 1;
            }
        }
        arr[j + 1] = key;
    }
}

int tile_get_row(uint16_t *loc) {
    if (loc == NULL || (size_t)loc < (size_t)tilemap || (size_t)loc > (size_t)MAX_TILE_LOC) {
        return -1;
    }
    return ((size_t)loc - (size_t)tilemap) / MAP_WIDTH_BYTES;
}

bool tile_invalid_coord(uint16_t x, uint16_t y) {
    return (x >= MAP_WIDTH / TILE_WIDTH || y >= MAP_HEIGHT / TILE_HEIGHT);
}

int tile_update_coord(struct TileEntry *entry, uint16_t col, uint16_t row) {
    if (!tile_invalid_coord(col, row)) {
        entry->col = col;
        entry->row = row;
        return 0;
    } else {
        return 1;
    }
}