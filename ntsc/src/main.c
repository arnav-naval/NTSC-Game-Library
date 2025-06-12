/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An, Niraj Menon
  * @date    Jan 15, 2024
  * @brief   ECE 362 Lab 2 Student template
  ******************************************************************************
*/

#include "common.h"
#include "clock.h"
#include "video.h"
#include "draw.h"
#include "sprite.h"
#include "tilemap.h"
#include "render.h"
#include "flex.h"
#include "graphics.h"
#include "scenes.h"
#include "eeprom.h"

void initc();
void togglexn(GPIO_TypeDef *port, int n);
void set_col(int col);

uint32_t joystickX; //hold joystick position on X axis
uint32_t joystickY; //hold joystick position on Y axis
int win; // true if win
int lose;  //true if lose
int double_down; //true if user chose to double down
int tie; //true if there is a tie


int main(void) {
    //internal_clock();
    StartHSE();
    RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN;
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM3_STOP;
    DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM1_STOP;

    draw_all(BACKGROUND_COLOR);
    setup_eeprom();
    setup_render();
    enable_flex();
    enable_video();

    scene_manager();

}

/**
 * @brief Init GPIO port C
 *        PC0-PC3 as input pins with the pull down resistor enabled
 *        PC4-PC9 as output pins
 * 
 */
void initc() {
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
  GPIOC->MODER &= ~(GPIO_MODER_MODER9);
  GPIOC->MODER |= GPIO_MODER_MODER9_0;
}

/**
 * @brief Change the ODR value from 0 to 1 or 1 to 0 for a specified 
 *        pin of a port.
 * 
 * @param port : The passed in GPIO Port
 * @param n    : The pin number
 */
void togglexn(GPIO_TypeDef *port, int n) {
  if (1 & (port->ODR >> n))
    port->ODR &= ~(1 << n);
  else
    port->ODR |= (1 << n);
}
