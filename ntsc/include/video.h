#ifndef VIDEO_H
#define VIDEO_H

#include "common.h"

//#define SCANLINE_COUNT 32//262
#define SCANLINE_COUNT 262

#define ROW_WIDTH 640
#define ROW_MAX (ROW_WIDTH / 16)
#define END_BLANK 2
#define ROW_MAX_RAW (ROW_MAX + END_BLANK)
#define ROW_MAX_RAW (ROW_MAX + END_BLANK)
//#define VERT_RES_RAW 10//240
#define HORIZONTAL_RES (ROW_WIDTH / 4)
#define VERT_RES_RAW 240
#define VERT_RES (VERT_RES_RAW / 2)
#define VERT_START 1
#define VERT_LINES (VERT_RES + VERT_START)

#define HORIZONTAL_TICKS 1824   // MUST be a multiple of 8, otherwise timing breaks
#define HORIZONTAL_ARR (HORIZONTAL_TICKS - 1)
#define ACTIVE_REP ((VERT_LINES - 1) & TIM_RCR_REP)
#define EQUALIZING_TICKS (HORIZONTAL_TICKS / 2)
#define EQUALIZING_CYCLES 6
#define EQUALIZING_ARR   (EQUALIZING_TICKS - 1)
#define EQUALIZING_CCR   67
#define EQUALIZING_REP ((EQUALIZING_CYCLES - 1) & TIM_RCR_REP)
#define VSYNC_CCR 778
//#define VSYNC_CCR 600
#define HSYNC_CCR 134

#define PRE_ACTIVE_CYCLES (SCANLINE_COUNT - VERT_RES_RAW - VERT_START  - (3 * EQUALIZING_CYCLES / 2))
#define PRE_ACTIVE_REP_OFFSET (0)//(-1) // adjust offset to mitigate CRT flickering
#define PRE_ACTIVE_REP ((PRE_ACTIVE_CYCLES + PRE_ACTIVE_REP_OFFSET - 1) & TIM_RCR_REP)

#define BURST_SIZE 2

enum sync_state {
    SYNC_EQUALIZE_0,
    SYNC_EQUALIZE_1,
    SYNC_VERTICAL,
    SYNC_PRE_ACTIVE,
    SYNC_ACTIVE
};

//extern const uint16_t color_burst[BURST_SIZE];
extern uint16_t vbuff [VERT_LINES][ROW_MAX_RAW];
extern volatile int active_line_vol;
extern volatile bool render_lock;

void enable_video();
void init_vbuff();
void wait_frames(uint32_t frames);

#endif