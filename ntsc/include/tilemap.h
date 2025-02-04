#ifndef TILEMAP_SPRITE_H
#define TILEMAP_SPRITE_H

#include "common.h"

// Sprite dimensions for tilemap
#define MAP_WIDTH_PIXELS 312
#define PIXEL_WIDTH_BITS 4
#define MAP_WIDTH_BYTES (PIXEL_WIDTH_BITS * MAP_WIDTH_PIXELS / 8)
#define MAP_WIDTH (MAP_WIDTH_BYTES / 2)
#define MAP_HEIGHT 280

// Packed 4-bit color indices, four pixels per uint16_t
extern const uint16_t tilemap[MAP_HEIGHT][MAP_WIDTH];

#endif // TILEMAP_SPRITE_H
