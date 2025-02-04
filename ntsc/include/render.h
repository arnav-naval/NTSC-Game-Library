#ifndef RENDER_H
#define RENDER_H

#include "common.h"
#include "sprite.h"

#define DRAW_TICKS (2 * HORIZONTAL_TICKS)
#define DRAW_ARR (DRAW_TICKS - 1)

#define SCANLINE_ROLLOVER (SCANLINE_COUNT / 2)

#define NODE_COUNT (MAX_SPRITES * 2)

struct ListNode {
    struct Sprite *sprite;
    struct ListNode *next;
};

extern uint16_t current_row;
extern struct ListNode *open_head;
extern volatile bool write_vram;
extern struct Sprite *draw_arr[MAX_SPRITES];
extern int draw_count;
extern struct Sprite *erase_arr[MAX_SPRITES];
extern int erase_count;


void setup_render(void);
void render_pass(void);
struct ListNode * req_node(void);
void free_node(struct ListNode *node);

#endif