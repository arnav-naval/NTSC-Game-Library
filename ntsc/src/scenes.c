#include "scenes.h"
#include "render.h"
#include "graphics.h"
#include "flex.h"
#include "video.h"
#include "leaderboard.h"

#define STRING_CENTER(len) ((HORIZONTAL_RES - 5 * (len)) / 2)
#define ROW_OFFSET 10

#define CONGRATS_STRING ("NEW HIGH SCORE")
#define CONGRATS_LEN (14)
#define CONGRATS_X (STRING_CENTER(CONGRATS_LEN))
#define CONGRATS_Y (10)
#define NEW_SCORE_X (STRING_CENTER(SCORE_DIGITS_BASE10))
#define NEW_SCORE_Y (CONGRATS_Y + ROW_OFFSET + 2)
#define SELECT_MSG_STRING ("JOYSTICK SELECT   BUTTON ENTER")
#define SELECT_MSG_LEN (30)
#define SELECT_MSG_X (STRING_CENTER(SELECT_MSG_LEN))
#define SELECT_MSG_Y (100)
#define NAME_ORIGIN_X (STRING_CENTER(NAME_LENGTH))
#define NAME_Y (50)

#define TITLE_STRING ("HIGH SCORES")
#define TITLE_LEN (11)
#define TITLE_X (STRING_CENTER(TITLE_LEN))
#define TITLE_Y 10
#define SCORE_SPACES 10
#define SCORE_STRING_LEN (NAME_LENGTH + SCORE_SPACES + SCORE_DIGITS_BASE10)
#define SCORE_ORIGIN_X (STRING_CENTER(SCORE_STRING_LEN))
#define SCORE_ORIGIN_Y (TITLE_Y + ROW_OFFSET + 2)
#define MESSAGE_STRING ("PRESS ANY BUTTON TO PLAY")
#define MESSAGE_LEN (24)
#define MESSAGE_X (STRING_CENTER(MESSAGE_LEN))
#define MESSAGE_Y (SCORE_ORIGIN_Y + MAX_SCORES * ROW_OFFSET + 2)

#define YOUR_STRING ("YOUR SCORE")
#define YOUR_LEN (10)
#define YOUR_X (STRING_CENTER(YOUR_LEN))
#define YOUR_Y (10)

static bool score_scene(struct Leaderboard *board);
static bool title_screen(void);
static void name_select_scene(struct Leaderboard *board, uint32_t points);
static void result_scene(uint32_t points);

void scene_manager(void) {
    struct Leaderboard board;
    read_leaderboard(&board);
    sort_leaderboard(&board);
    for (;;) {
        for (;;) {
            if (!score_scene(&board)) break;
            if (!title_screen()) break;
        }
        full_game();
        read_leaderboard(&board);
        sort_leaderboard(&board);
        if (total_chips > 0) {
            result_scene(total_chips);
            if (high_score_check(&board, total_chips)) {
                name_select_scene(&board, total_chips);
            }
        } else if (total_chips == 0) {
            result_scene(total_chips);
        }
    }
}

static void result_scene(uint32_t points) {
    struct Sprite your_char[YOUR_LEN];
    struct SpriteString your = {your_char, YOUR_LEN};
    init_string(&your, YOUR_STRING, YOUR_LEN, YOUR_X, YOUR_Y);
    draw_string(&your);

    char score_string[SCORE_DIGITS_BASE10];
    struct Sprite score_char[SCORE_DIGITS_BASE10];
    struct SpriteString score = {score_char, SCORE_DIGITS_BASE10};
    string_convert(score_string, SCORE_DIGITS_BASE10, points);
    init_string(&score, score_string, SCORE_DIGITS_BASE10, STRING_CENTER(SCORE_DIGITS_BASE10), YOUR_Y + ROW_OFFSET + 2);
    draw_string(&score);

    render_pass();
    wait_frames(3 * 60);
    clear_screen();
}

static void name_select_scene(struct Leaderboard *board, uint32_t points) {
    struct Sprite congrats_char[CONGRATS_LEN];
    struct SpriteString congrats = {congrats_char, CONGRATS_LEN};
    init_string(&congrats, CONGRATS_STRING, CONGRATS_LEN, CONGRATS_X, CONGRATS_Y);
    draw_string(&congrats);

    char temp[SCORE_DIGITS_BASE10];
    struct Sprite score_char[SCORE_DIGITS_BASE10];
    struct SpriteString score = {score_char, SCORE_DIGITS_BASE10};
    string_convert(temp, SCORE_DIGITS_BASE10, points);
    init_string(&score, temp, SCORE_DIGITS_BASE10, NEW_SCORE_X, NEW_SCORE_Y);
    draw_string(&score);

    render_pass();

    //wait_frames(100);

    struct Sprite select_msg_char[SELECT_MSG_LEN];
    struct SpriteString select_msg = {select_msg_char, SELECT_MSG_LEN};
    init_string(&select_msg, SELECT_MSG_STRING, SELECT_MSG_LEN, SELECT_MSG_X, SELECT_MSG_Y);
    draw_string(&select_msg);

    struct Score high_score;
    high_score.points = points;
    struct Sprite name_char[NAME_LENGTH];
    struct SpriteString name = {name_char, NAME_LENGTH};
    for (int i = 0; i < NAME_LENGTH; i++) {
        high_score.name[i] = 'A';
    }
    init_string(&name, high_score.name, NAME_LENGTH, NAME_ORIGIN_X, NAME_Y);
    draw_string(&name);

    struct Sprite dots[2];
    init_neo_dot(&dots[0], NAME_ORIGIN_X, NAME_Y - 5);
    init_neo_dot(&dots[1], NAME_ORIGIN_X, NAME_Y + 8);
    sprite_draw(&dots[0]);
    sprite_draw(&dots[1]);

    render_pass();

    int idx = 0;
    while (!button_state) {
        switch(joystick_state) {
        case JOYSTICK_UP:
            if (high_score.name[idx] > 'A') {
                high_score.name[idx]--;
                change_char(&name_char[idx], high_score.name[idx]);
            }
            break;
        case JOYSTICK_DOWN:
            if (high_score.name[idx] < 'Z') {
                high_score.name[idx]++;
                change_char(&name_char[idx], high_score.name[idx]);
            }
            break;
        case JOYSTICK_LEFT:
            if (idx > 0) {
                idx--;
                sprite_move(&dots[0], NAME_ORIGIN_X + 5 * idx, dots[0].y);
                sprite_move(&dots[1], NAME_ORIGIN_X + 5 * idx, dots[1].y);
            }
            break;
        case JOYSTICK_RIGHT:
            if (idx < NAME_LENGTH - 1) {
                idx++;
                sprite_move(&dots[0], NAME_ORIGIN_X + 5 * idx, dots[0].y);
                sprite_move(&dots[1], NAME_ORIGIN_X + 5 * idx, dots[1].y);
            }
            break;
        default:
            break;
        }
        render_pass();
        wait_frames(10);
    }

    insert_known_highscore(board, &high_score);
    //write_leaderboard(board);

    // hack bc it's 3:52 am before the demo and erase
    //(a very consistent function till now) is drawing past the edges. 
    clear_screen();
}

static bool title_screen(void) {
    struct Sprite title;

    sprite_register(&title, 0, 0);
    sprite_remap(&title, 0, 40);
    title.mapping.width = 40;
    title.mapping.height = 30;
    sprite_draw(&title);

    int count = 0;
    bool timeout = false;
    uint32_t overlaps = 0;
    while (joystick_state == JOYSTICK_IDLE && !button_state) {
        if (count == 0) {
            overlaps++;
            if (overlaps >= 8) {
                timeout = true;
                break;
            }
            render_pass();
        }
        count = (count + 1) % 60;
        wait_frames(1);
    }

    // hack bc it's 3:52 am before the demo and erase
    //(a very consistent function till now) is drawing past the edges. 
    clear_screen();
    return timeout;
}

static bool score_scene(struct Leaderboard *board) {
    struct SpriteString title;
    struct Sprite title_char[TITLE_LEN];
    title.arr = title_char;
    title.len = TITLE_LEN;
    init_string(&title, TITLE_STRING, TITLE_LEN, TITLE_X, TITLE_Y);
    draw_string(&title);

    struct SpriteString scores[MAX_SCORES];
    struct Sprite score_char[MAX_SCORES][SCORE_STRING_LEN];
    for (int i = 0; i < MAX_SCORES; i++) {
        char temp[SCORE_STRING_LEN];
        string_copy(temp, board->entry[i].name, NAME_LENGTH);
        for (int x = 0; x < SCORE_SPACES; x++) temp[NAME_LENGTH + x] = ' ';
        string_convert(&temp[NAME_LENGTH + SCORE_SPACES], SCORE_DIGITS_BASE10, board->entry[i].points);
        scores[i].arr = score_char[i];
        scores[i].len = SCORE_STRING_LEN;
        init_string(&scores[i], temp, SCORE_STRING_LEN, SCORE_ORIGIN_X, SCORE_ORIGIN_Y + i * ROW_OFFSET);
        draw_string(&scores[i]);
    }
    render_pass();

    struct SpriteString message;
    struct Sprite message_char[MESSAGE_LEN];
    message.arr = message_char;
    message.len = MESSAGE_LEN;
    init_string(&message, MESSAGE_STRING, MESSAGE_LEN, MESSAGE_X, MESSAGE_Y);

    wait_frames(60);

    int count = 0;
    bool draw = true;
    bool timeout = false;
    uint32_t overlaps = 0;
    while (joystick_state == JOYSTICK_IDLE && !button_state) {
        if (count == 0) {
            overlaps++;
            if (overlaps >= 8) {
                timeout = true;
                break;
            }
            if (draw) draw_string(&message);
            else erase_string(&message);
            draw = !draw;
            render_pass();
        }
        count = (count + 1) % 60;
        wait_frames(1);
    }

    // hack bc it's 3:52 am before the demo and erase
    //(a very consistent function till now) is drawing past the edges. 
    clear_screen();
    return timeout;
}