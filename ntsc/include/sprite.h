#ifndef SPRITE_H
#define SPRITE_H

#include "common.h"
#include "video.h"
#include "tilemap.h"


#define MAX_TILE_LOC (&(tilemap[MAP_HEIGHT - 1][MAP_WIDTH - 1]))
#define GET_TILE_DATA(x, y, vert_off) (tilemap[(y) * TILE_HEIGHT + (vert_off)][(x) * TILE_WIDTH])

struct TileEntry {
    uint16_t col;
    uint16_t row;
    uint16_t width;
    uint16_t height;
};

int tile_get_row(uint16_t *loc);
bool tile_invalid_coord(uint16_t x, uint16_t y);
int tile_update_coord(struct TileEntry *entry, uint16_t col, uint16_t row);

#define TILE_PIXEL_MULT (sizeof(uint16_t) * 8 / PIXEL_WIDTH_BITS)
#define TILE_WIDTH 1
#define TILE_HEIGHT 4



#define MAX_SPRITES 400

#define SPRITE_IS_VISIBLE_Pos 0
#define SPRITE_IS_VISIBLE (0x1 << SPRITE_IS_VISIBLE_Pos)
#define SPRITE_DRAW_Pos 1
#define SPRITE_DRAW (0x1 << SPRITE_DRAW_Pos)
#define SPRITE_ERASE_Pos 2
#define SPRITE_ERASE (0x1 << SPRITE_ERASE_Pos)
#define SPRITE_MOVE_Pos 3
#define SPRITE_MOVE (0x1 << SPRITE_MOVE_Pos)
//#define SPRITE_REMAP_Pos 4
//#define SPRITE_REMAP (0x1 << SPRITE_REMAP_Pos)
#define SPRITE_OPERATIONS (SPRITE_DRAW | SPRITE_ERASE | SPRITE_MOVE)// | SPRITE_REMAP)

#define RENDER_LOCK while(render_lock)


struct Sprite {
    struct TileEntry mapping;
    uint16_t x;
    uint16_t y;
    uint16_t x_alt;
    uint16_t y_alt;
    uint8_t status;
};

struct Sprite * sprite_register(struct Sprite *new, uint16_t x, uint16_t y);
void sprite_move(struct Sprite *sprite, uint16_t x, uint16_t y);
int get_sprite_row(struct Sprite *sprite);
void sort_sprites(struct Sprite *arr[], int n, bool alt);

void sprite_draw(struct Sprite *sprite);
void sprite_remap(struct Sprite *sprite, uint16_t col, uint16_t row);
void sprite_erase(struct Sprite *sprite);
void create_string(struct Sprite sprite[], char *str, uint8_t len);

//void sprite_move()
#endif