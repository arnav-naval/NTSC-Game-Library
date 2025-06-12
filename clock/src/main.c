/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An, Niraj Menon
  * @date    Jan 15, 2024
  * @brief   ECE 362 Lab 2 Student template
  ******************************************************************************
*/


/**
******************************************************************************/

// Fill out your username, otherwise your completion code will have the 
// wrong username!
const char* username = "cbehrend";

/******************************************************************************
*/ 

#include "stm32f0xx.h"
#include <stdint.h>

void initc();
void initb();
void togglexn(GPIO_TypeDef *port, int n);
void init_exti();
void set_col(int col);
void SysTick_Handler();
void init_systick();
void adjust_priorities();
//static void _adjust_priorities(int pos, uint8_t priority);

extern void autotest();
extern void internal_clock();
extern void nano_wait(int);
extern void StartHSE(void);

/**
* Description: This function handles RCC interrupt request
* and switch the system clock to HSE.
*/
void RCC_CRS_IRQHandler(void)
{
    /* (1) Check the flag HSE ready */
    /* (2) Clear the flag HSE ready */
    /* (3) Switch the system clock to HSE */
    while ((RCC->CIR & RCC_CIR_HSERDYF) == 0); /* (1) */
    RCC->CIR |= RCC_CIR_HSERDYC; /* (2) */
    RCC->CFGR = ((RCC->CFGR & (~RCC_CFGR_SW)) | RCC_CFGR_SW_0); /* (3) */
}

int main(void) {
    //internal_clock();
    StartHSE();
    RCC->CFGR |= RCC_CFGR_MCOSEL_HSE;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODER8_1;
    // Uncomment when most things are working
    //autotest();
    
    initc();
    init_systick();

    // Slowly blinking
    for(;;) {
        togglexn(GPIOC, 9);
        nano_wait(500000000);
    }
}

/**
 * @brief Init GPIO port C
 *        PC0-PC3 as input pins with the pull down resistor enabled
 *        PC4-PC9 as output pins
 * 
 */
void initc() {
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
  GPIOC->MODER &= ~(0xFFFFF);
  GPIOC->MODER |= 0x555 << 8;
  GPIOC->PUPDR &= ~0xFF;
  GPIOC->PUPDR |= 0xAA;
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

/**
 * @brief Enable the SysTick interrupt to occur every 1/16 seconds.
 * 
 */
void init_systick() {
  SysTick->CTRL &= ~0b111;
  SysTick->LOAD = 0x5B8D7;
  SysTick->VAL = 0;
  SysTick->CTRL |= 0b011;
}

volatile int current_col = 1;

/**
 * @brief The ISR for the SysTick interrupt.
 * 
 */
void SysTick_Handler() {
  if (current_col >= 4)
    current_col = 1;
  else
    current_col++;
  set_col(current_col);
}

/**
 * @brief For the keypad pins, 
 *        Set the specified column level to logic "high".
 *        Set the other three three columns to logic "low".
 * 
 * @param col 
 */
void set_col(int col) {
  GPIOC->ODR &= ~(0xF << 4);
  GPIOC->ODR |= (1 << (8 - col));
}