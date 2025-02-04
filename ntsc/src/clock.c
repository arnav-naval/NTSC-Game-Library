#include "clock.h"

void internal_clock()
{
    /* Disable HSE to allow use of the GPIOs */
    RCC->CR &= ~RCC_CR_HSEON;
    /* Enable Prefetch Buffer and set Flash Latency */
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
    /* HCLK = SYSCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;
    /* PCLK = HCLK */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;
    /* PLL configuration = (HSI/2) * 12 = ~48 MHz */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMUL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_DIV2 | RCC_CFGR_PLLXTPRE_HSE_PREDIV_DIV1 | RCC_CFGR_PLLMUL12);
    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    /* Wait till PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0);
    /* Select PLL as system clock source */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;
    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL);
}

/**
* Description: This function enables the interrupt on HSE ready,
* and start the HSE as external clock.
*/
void StartHSE(void)
{
    //FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
    NVIC_EnableIRQ(RCC_CRS_IRQn);
    NVIC_SetPriority(RCC_CRS_IRQn,0);
    RCC->CIR |= RCC_CIR_HSERDYIE;
    RCC->CR |= RCC_CR_CSSON | RCC_CR_HSEON;
}

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
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_HSE);
    // Disable prescaling for AHB and APB
    RCC->CFGR &= ~(RCC_CFGR_PPRE | RCC_CFGR_HPRE);

    // Decrease SYSCLK
    #ifdef CHOKE_SYSCLK
    RCC->CFGR |= RCC_CFGR_HPRE;
    #endif
}
