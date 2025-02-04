/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An, Niraj Menon
  * @date    Jan 15, 2024
  * @brief   ECE 362 Lab 2 Student template
  ******************************************************************************
*/

#include "video.h"

static void setup_tim1();
static void setup_sync();
static void init_spi1();
static void spi1_setup_dma(void);

static enum sync_state sync_nxt;
static bool repeat;
static bool burst;

uint16_t vbuff [VERT_LINES][ROW_MAX_RAW];
#ifdef ENABLE_COLOR
// 5 cycles fixes everything DO NOT TOUCH AT ALL COSTS
static const uint16_t color_burst[BURST_SIZE] = {0x3333, 0x3000};
#endif
int active_line;
volatile int active_line_vol = 0;
volatile bool render_lock = false;
static volatile bool count_frames = false;
static volatile int delay_counter;

void enable_video(void) {
    setup_sync();
    init_spi1();
}

static void setup_sync(void) {
    setup_tim1();
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~GPIO_MODER_MODER8;
    GPIOA->MODER |= GPIO_MODER_MODER8_1;
    GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL8;
    GPIOA->AFR[1] |= 0x2 << GPIO_AFRH_AFSEL8_Pos;
}

void wait_frames(uint32_t frames) {
    delay_counter = 0;
    count_frames = true;
    while (delay_counter < frames);
    count_frames = false;
}

static void setup_tim1(void) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    sync_nxt = SYNC_VERTICAL;

    TIM1->PSC = 0;
    TIM1->CCR1 = EQUALIZING_CCR;
    TIM1->ARR = EQUALIZING_ARR;
    TIM1->RCR &= ~(TIM_RCR_REP);
    TIM1->RCR |= EQUALIZING_REP;

    // Enable PWM mode 2, preload for OC1
    TIM1->CCMR1 |= TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE | TIM_CCMR2_OC3PE;
    // Preloading for ARR
    TIM1->CR1 |= TIM_CR1_ARPE;
    // Enable update interrupt
    TIM1->DIER |= TIM_DIER_UIE;

    TIM1->CCER |= TIM_CCER_CC1E;
    TIM1->BDTR |= TIM_BDTR_MOE;

    TIM1->CCR2 = 302;
    TIM1->CCMR1 &= ~(TIM_CCMR1_OC2M);
    TIM1->DIER &= ~(TIM_DIER_CC2IE);

    TIM1->CCR3 = 64 + 7;
    TIM1->CCMR2 &= ~(TIM_CCMR2_OC3M);
    TIM1->DIER &= ~(TIM_DIER_CC3IE);


    // Use UEV as trigger sync
    TIM1->CR2 &= ~(TIM_CR2_MMS);
    TIM1->CR2 |= TIM_CR2_MMS_1;


    NVIC_EnableIRQ(TIM1_CC_IRQn);
    NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
    NVIC_SetPriority(TIM1_CC_IRQn, 0);
    NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0);
    TIM1->CR1 |= TIM_CR1_CEN;
}

void init_vbuff(void) {
    for (int x = 0; x < ROW_MAX_RAW; x++) {
        vbuff[0][x] = 0;
    }
    for (int y = VERT_START; y < VERT_LINES; y++) {
        for (int x = ROW_MAX; x < ROW_MAX_RAW; x++) {
            vbuff[y][x] = 0;
        }
    }
}

static void init_spi1(void) {
    // SDI/MOSI (PA7)
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(GPIO_MODER_MODER7);
    GPIOA->MODER |= GPIO_MODER_MODER7_1;
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL7);

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 &= ~(SPI_CR1_SPE);
    SPI1->CR1 &= ~(SPI_CR1_BR);

    SPI1->CR1 |= SPI_CR1_MSTR;
    // 16-bit data length
    SPI1->CR2 |= SPI_CR2_DS | SPI_CR2_SSOE;
    // NSSP IS THE ROOT OF ALL EVIL DO NOT ENABLE AT ALL COSTS
    SPI1->CR2 &= ~(SPI_CR2_NSSP);

    spi1_setup_dma();
}

static void spi1_setup_dma(void) {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel3->CPAR = (uint32_t)(&(SPI1->DR));
    DMA1_Channel3->CMAR = (uint32_t)(vbuff[0]);
    DMA1_Channel3->CNDTR = ROW_MAX_RAW;
    DMA1_Channel3->CCR &= ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE | DMA_CCR_CIRC);
    DMA1_Channel3->CCR |= DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0;
    DMA1_Channel3->CCR |= DMA_CCR_PL;
    DMA1->CSELR &= ~(DMA_CSELR_C3S);
    DMA1->CSELR |= DMA1_CSELR_CH3_SPI1_TX;
    SPI1->CR2 |= SPI_CR2_TXDMAEN;
}


// IRQ Section

void TIM1_BRK_UP_TRG_COM_IRQHandler() {
    TIM1->SR = (int16_t)(~TIM_SR_UIF);
    switch (sync_nxt) {
    case SYNC_EQUALIZE_0:
        #ifdef ENABLE_COLOR
        TIM1->DIER |= TIM_DIER_CC2IE | TIM_DIER_CC3IE;
        #else
        TIM1->DIER |= TIM_DIER_CC2IE;
        #endif
        render_lock = true;
        TIM1->CCR1 = EQUALIZING_CCR;
        TIM1->ARR = EQUALIZING_ARR;
        TIM1->RCR &= ~(TIM_RCR_REP);
        TIM1->RCR |= EQUALIZING_REP;
        sync_nxt = SYNC_VERTICAL;
        break;
    case SYNC_VERTICAL:
        render_lock = false;
        TIM1->DIER &= ~(TIM_DIER_CC2IE | TIM_DIER_CC3IE);
        TIM1->SR = ~(TIM_SR_CC2IF | TIM_SR_CC3IF);
        TIM1->CCR1 = VSYNC_CCR;
        sync_nxt = SYNC_EQUALIZE_1;
        if (count_frames) delay_counter++;
        break;
    case SYNC_EQUALIZE_1:
        TIM1->CCR1 = EQUALIZING_CCR;
        active_line = 0;
        burst = true;
        repeat = true;
        sync_nxt = SYNC_PRE_ACTIVE;
        break;
    case SYNC_PRE_ACTIVE:
        TIM1->CCR1 = HSYNC_CCR;
        TIM1->ARR = HORIZONTAL_ARR;
        TIM1->RCR &= ~(TIM_RCR_REP);
        TIM1->RCR |= PRE_ACTIVE_REP;
        sync_nxt = SYNC_ACTIVE;
        break;
    case SYNC_ACTIVE:
        TIM1->RCR &= ~(TIM_RCR_REP);
        TIM1->RCR |= (VERT_RES_RAW) & 0xFF; // Glitch in the system don't worry about it
        sync_nxt = SYNC_EQUALIZE_0;
        break;
    }
}

void TIM1_CC_IRQHandler(void) {
    #ifdef ENABLE_COLOR
    if (burst) {
        TIM1->SR = ~TIM_SR_CC3IF;
        burst = false;
        if(active_line) {
            DMA1_Channel3->CCR &= ~(DMA_CCR_EN);
            DMA1_Channel3->CMAR = (uint32_t)(color_burst);
            DMA1_Channel3->CNDTR = BURST_SIZE;
            DMA1_Channel3->CCR |= DMA_CCR_EN;
        }
        SPI1->CR1 &= ~SPI_CR1_SPE;
        SPI1->CR1 |= SPI_CR1_SPE;
        return;
    }
    #endif

    // Maintain consistency for each line,
    // no diverging until after SPE enable
    TIM1->SR = ~TIM_SR_CC2IF;
    #ifdef ENABLE_COLOR
    burst = true;
    #endif

    DMA1_Channel3->CCR &= ~(DMA_CCR_EN);
    DMA1_Channel3->CMAR = (uint32_t)(vbuff[active_line]);
    DMA1_Channel3->CNDTR = ROW_MAX_RAW;
    DMA1_Channel3->CCR |= DMA_CCR_EN;
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 |= SPI_CR1_SPE;
    if (repeat) {
        repeat = false;
    } else {
        active_line++;
        active_line_vol = active_line;
        repeat = true;
    }
}
