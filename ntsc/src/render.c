#include "render.h"

static struct ListNode * init_nodes(struct ListNode *base, int count);
static void render_draw(uint16_t row);
static bool _render_draw(struct Sprite *sprite, uint16_t row);
static void render_erase(uint16_t row);
static bool _render_erase(struct Sprite *sprite, uint16_t row);
static void queue_sprites(int row);
static void _queue_add(struct Sprite *sprite, struct ListNode **head);
static void clear_queues(void);
static void _clear_queues(struct ListNode **head);

uint16_t current_row;
struct ListNode open_nodes[NODE_COUNT];
struct ListNode *open_head;
struct ListNode *draw_head;
struct ListNode *erase_head;
struct Sprite *erase_arr[MAX_SPRITES];
int erase_count = 0;
static int erase_idx = 0;
struct Sprite *draw_arr[MAX_SPRITES];
int draw_count = 0;
int draw_idx = 0;



volatile int toy;
volatile bool write_vram = false;

void setup_render(void) {
    //setup_tim3();
    open_head = init_nodes(open_nodes, NODE_COUNT);
}

void render_pass(void) {
    //TIM3->SR = (int16_t)(~TIM_SR_CC1IF);
    if (draw_count == 0 && erase_count == 0) return;
    current_row = VERT_START;
    draw_idx = 0;
    erase_idx = 0;
    sort_sprites(draw_arr, draw_count, false);
    sort_sprites(erase_arr, erase_count, true);
    queue_sprites(VERT_START);
    while(active_line_vol < VERT_START);
    while (current_row != 0) {
        while (current_row >= active_line_vol || render_lock == false);
        if (current_row != VERT_LINES - 1) {
            render_erase(current_row);
            render_draw(current_row);
            queue_sprites(current_row + 1);
        } else {
            clear_queues();
        }
        current_row = (current_row + 1) % VERT_LINES;
    }
    draw_count = 0;
    erase_count = 0;
    write_vram = false;
    return;
}

static void render_erase(uint16_t row) {
    struct ListNode *node, *prev_node;
    prev_node = NULL;
    node = erase_head;
    while (node != NULL) {
        struct Sprite *sprite = node->sprite;

        if (_render_erase(sprite, row)) {
            if (node != erase_head) {
                prev_node->next = node->next;
                free_node(node);
                node = prev_node->next;
            }
            else {
                erase_head = node->next;
                free_node(node);
                node = erase_head;
            }
            if (sprite->status & SPRITE_MOVE) {
                sprite->status &= ~(SPRITE_MOVE);
            } else {
                sprite->status &= ~(SPRITE_ERASE | SPRITE_IS_VISIBLE);
            }
        } else {
            prev_node = node;
            node = node->next;
        }
    }
}

static bool _render_erase(struct Sprite *sprite, uint16_t row) {
    uint16_t tile_row_offset, vram_base_col, offset;

    tile_row_offset = row - sprite->y_alt;

    vram_base_col = sprite->x_alt / 4;//TILE_PIXEL_MULT;
    if (vram_base_col >= ROW_MAX) goto end;
    offset = sprite->x_alt % 4;

    if (!offset) {
        for (int i = 0; i < sprite->mapping.width; i++) {
            vbuff[row][vram_base_col + i] = BACKGROUND_COLOR;
        }
    } else {
        uint16_t temp, mask;
        mask = 0xFFFF << (16 - 4 * offset);
        temp = vbuff[row][vram_base_col] & mask;
        temp |= BACKGROUND_COLOR >> (4 * offset);
        vbuff[row][vram_base_col] = temp;

        for (uint16_t i = 1; i < sprite->mapping.width; i++) {
            if (vram_base_col + i >= ROW_MAX) goto end;
            vbuff[row][vram_base_col + i] = BACKGROUND_COLOR;
        }
        temp = BACKGROUND_COLOR & mask;
        temp |= vbuff[row][vram_base_col + sprite->mapping.width] & ~mask;
        vbuff[row][vram_base_col + sprite->mapping.width] = temp;
    }

end:
    if (4 * sprite->mapping.height - 1 <= tile_row_offset) {
        return true;
    } else {
        return false;
    }
}

static void render_draw(uint16_t row) {
    struct ListNode *node, *prev_node;
    prev_node = NULL;
    node = draw_head;
    while (node != NULL) {
        struct Sprite *sprite = node->sprite;

        if (_render_draw(sprite, row)) {
            if (node != draw_head) {
                prev_node->next = node->next;
                free_node(node);
                node = prev_node->next;
            }
            else {
                draw_head = node->next;
                free_node(node);
                node = draw_head;
            }
            sprite->status &= ~SPRITE_DRAW;
            sprite->status |= SPRITE_IS_VISIBLE;
        } else {
            prev_node = node;
            node = node->next;
        }
    }
}

static bool _render_draw(struct Sprite *sprite, uint16_t row) {
    uint16_t tile_row_offset, vram_base_col, offset;

    tile_row_offset = row - sprite->y;

    vram_base_col = sprite->x / 4;//TILE_PIXEL_MULT;
    offset = sprite->x % 4;

    if (!offset) {
        for (int i = 0; i < sprite->mapping.width; i++) {
            vbuff[row][vram_base_col + i] = GET_TILE_DATA(sprite->mapping.col + i, sprite->mapping.row, tile_row_offset);
        }
    } else {
        uint16_t temp, mask, data;
        mask = 0xFFFF << (16 - 4 * offset);
        temp = vbuff[row][vram_base_col] & mask;

        for (int i = 0; i < sprite->mapping.width; i++) {
            data = GET_TILE_DATA(sprite->mapping.col + i, sprite->mapping.row, tile_row_offset);
            temp |= data >> (4 * offset);
            vbuff[row][vram_base_col + i] = temp;
            temp = data << (16 - 4 * offset);
        }
        temp |= vbuff[row][vram_base_col + sprite->mapping.width] & ~mask;
        vbuff[row][vram_base_col + sprite->mapping.width] = temp;
    }

    if (4 * sprite->mapping.height - 1 <= tile_row_offset) {
        return true;
    } else {
        return false;
    }
}

static void queue_sprites(int row) {
    while ((draw_idx < draw_count) && (draw_arr[draw_idx]->y <= row)) {
        _queue_add(draw_arr[draw_idx], &draw_head);
        draw_idx++;
    }
    while ((erase_idx < erase_count) && (erase_arr[erase_idx]->y_alt <= row)) {
        _queue_add(erase_arr[erase_idx], &erase_head);
        erase_idx++;
    }
}

static void _queue_add(struct Sprite *sprite, struct ListNode **head) {
    struct ListNode *node = req_node();
    node->sprite = sprite;
    node->next = *head;
    *head = node;
}

static void clear_queues(void) {
    _clear_queues(&draw_head);
    _clear_queues(&erase_head);
}

static void _clear_queues(struct ListNode **head) {
    struct ListNode *node, *next_node;
    node = *head;
    while (node != NULL) {
        next_node = node->next;
        free_node(node);
        node = next_node;
    }
    *head = NULL;
}

static struct ListNode * init_nodes(struct ListNode *base, int count) {
    for (int i = 0; i < count - 1; i++) {
        base[i].sprite = NULL;
        base[i].next = &base[i + 1];
    }
    base[count - 1].sprite = NULL;
    base[count - 1].next = NULL;
    return base;
}

struct ListNode * req_node(void) {
    if (open_head == NULL) return NULL;
    struct ListNode *old_head = open_head;
    open_head = open_head->next;
    return old_head;
}

void free_node(struct ListNode *node) {
    node->sprite = NULL;
    node->next = open_head;
    open_head = node;
}
