#include "draw.h"

void draw_lines(void) {
    for (int y = VERT_START; y < VERT_LINES; y++) {
        for (int x = 0; x < ROW_MAX_RAW - END_BLANK; x++) {
            //vbuff[y][x] = 0x8180;
            switch (x * 16 / (ROW_MAX_RAW - END_BLANK)) {
            //switch ((x + y) % 16) {
            case 0:
                vbuff[y][x] = 0xFFFF;
                break;
            case 1:
                vbuff[y][x] = 0x1111;
                break;
            case 2:
                vbuff[y][x] = 0x2222;
                break;
            case 3:
                vbuff[y][x] = 0x3333;
                break;
            case 4:
                vbuff[y][x] = 0x4444;
                break;
            case 5:
                vbuff[y][x] = 0x5555;
                break;
            case 6:
                vbuff[y][x] = 0x6666;
                break;
            case 7:
                vbuff[y][x] = 0x7777;
                break;
            case 8:
                vbuff[y][x] = 0x8888;
                break;
            case 9:
                vbuff[y][x] = 0x9999;
                break;
            case 10:
                vbuff[y][x] = 0xAAAA;
                break;
            case 11:
                vbuff[y][x] = 0xBBBB;
                break;
            case 12:
                vbuff[y][x] = 0xCCCC;
                break;
            case 13:
                vbuff[y][x] = 0xDDDD;
                break;
            case 14:
                vbuff[y][x] = 0xEEEE;
                break;
            default:
                vbuff[y][x] = 0xFFFF;
            }
            //vbuff[y][x] = 0xFFFF;
        }
    }
}

void draw_square(void) {
    draw_lines();
    for (int y = VERT_START; y < VERT_START + 20; y++) {
        for (int x = 0; x < ROW_MAX_RAW - END_BLANK; x++) {
            vbuff[y][x] = 0;
        }
    }
    for (int y = VERT_START + 20; y < VERT_LINES - 20; y++) {
        for (int x = 0; x < 5; x++) {
            vbuff[y][x] = 0;
        }
        for (int x = ROW_MAX_RAW - END_BLANK - 5; x < ROW_MAX_RAW - END_BLANK; x++) {
            vbuff[y][x] = 0;
        }
    }
    for (int y = VERT_LINES - 20; y < VERT_LINES; y++) {
        for (int x = 0; x < ROW_MAX_RAW - END_BLANK; x++) {
            vbuff[y][x] = 0;
        }
    }
}

void draw_all(uint16_t val) {
    for (int y = VERT_START; y < VERT_LINES; y++) {
        for (int x = 0; x < ROW_MAX_RAW - END_BLANK; x++) {
            vbuff[y][x] = val;
        }
    }
}