#ifndef FLEX_H
#define FLEX_H

#include "common.h"

enum Direction {
    JOYSTICK_IDLE,
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT
};

#define ADC_MAX (0xFFF) // 12 bit resolution
#define ADC_CENTER (ADC_MAX / 2)
#define JOYSTICK_MAX (ADC_MAX - ADC_CENTER) // 12 bit resolution
#define JOYSTICK_MIN (-ADC_CENTER) // 12 bit resolution
#define DEADZONE_RADIUS (JOYSTICK_MAX / 7)

extern volatile int joystick_analog_x;
extern volatile int joystick_analog_y;
extern volatile enum Direction joystick_state;
extern volatile bool button_state;
extern int total_chips;

void enable_flex(void);

#endif