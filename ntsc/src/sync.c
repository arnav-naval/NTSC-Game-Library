#include "flex.h"
#include "video.h"
#include "leaderboard.h"

uint32_t joystickX; //hold joystick position on X axis
uint32_t joystickY; //hold joystick position on Y axis
volatile int joystick_analog_x = 0;
volatile int joystick_analog_y = 0;
volatile enum Direction joystick_state;
volatile bool button_state;
int happy_sound_trigger;
int sad_sound_trigger;

static void setup_tim3(void);
static void controls_logic(void);
static void eeprom_logic(void);
static void sound_logic(void);
static void evaluate_direction(int x, int y);
static int absolute(int x);

void enable_button(){
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~GPIO_MODER_MODER0_Msk; //set PB0 as input (for select button)
}

uint32_t volume = 2048;
#define N 1000
#define RATE 15700
short int wavetable[N];
int step0 = 0;
int offset0 = 0;

float happy_sound_freq[10] = {493.88, 329.63, 415.30, 493.88, 659.26, 440, 493.88, 659.26, 830.61, 0};
int happy_sound_beats[10] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
float sad_sound_freq[6] = {261.63, 220, 196, 174.61, 155.56, 0};
int sad_sound_beats[6] = {1000, 2000, 3000, 4000, 5000, 6000};

static void setup_tim3(void);

void enable_flex(void) {
    init_wavetable();
    enable_button();
    setup_dac();
    setup_adc();
    setup_tim3();
}

void init_wavetable(void) {
    for(int i=0; i < N; i++)
        wavetable[i] = 32767 * sin(2 * 3.1415 * i / N);
}

void set_freq(int chan, float f) {
    if (chan == 0) {
        if (f == 0.0) {
            step0 = 0;
            offset0 = 0;
        } else
            step0 = (f * N / RATE) * (1<<16);
    }
}

void setup_dac(void) {
    //enable the clock to GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= (0x3 << (4 * 2)); //PA4 is DAC_OUT1
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR |= 0x11;
    DAC->CR |= DAC_CR_EN1;
}

void setup_adc(void) {
    //setup ADC1
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~111100;
    GPIOA->MODER |= 111100; //PA1 and PA2
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->CR2 |= RCC_CR2_HSI14ON;
    while(!(RCC->CR2 & RCC_CR2_HSI14RDY));
    ADC1->CR |= ADC_CR_ADEN;
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
    ADC1->CHSELR = ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL2;
    while(!(ADC1->ISR & ADC_ISR_ADRDY));    
}

static void setup_tim3(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    TIM3->PSC = 0;
    TIM3->ARR = HORIZONTAL_ARR;
    TIM3->RCR = (SCANLINE_COUNT - 1) & 0xFF;

    // Tim1 sync setup
    TIM3->SMCR &= ~(TIM_SMCR_TS | TIM_SMCR_SMS);
    TIM3->SMCR |= TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1;

    TIM3->DIER |= TIM_DIER_CC1IE;
    TIM3->CCR1 = HORIZONTAL_TICKS / 2;
    TIM3->CCER |= TIM_CCER_CC1E;
    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_SetPriority(TIM3_IRQn, 2);

    #ifdef PWM_CHECK
    TIM3->CCMR1 |= TIM_CCMR1_OC1M;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~GPIO_MODER_MODER6;
    GPIOA->MODER |= GPIO_MODER_MODER6_1;
    GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL6;
    GPIOA->AFR[0] |= 0x1 << GPIO_AFRL_AFSEL6_Pos;
    #endif

    // Will be enabled by TIM1 UEV
}

int h_beat = 0;
int s_beat = 0;
void TIM3_IRQHandler(void) {
    TIM3->SR = ~TIM_SR_CC1IF;
    // Goofy ah code snippet
    sound_logic();
    controls_logic();
    eeprom_logic();
}

static void sound_logic(void) {
    offset0 += step0;
    if(offset0 >= (N << 16)) {
        offset0 -= (N << 16);
    }
    int samp = wavetable[offset0 >> 16];
    samp = (samp * volume) >> 17;
    samp += 2048;

    if(h_beat >= 9999){
        h_beat = 0;
        happy_sound_trigger = 0;
    }

    if(s_beat >= 5555){
        s_beat = 0;
        sad_sound_trigger = 0;
    }

    if((h_beat <= happy_sound_beats[h_beat / 1000]) && (happy_sound_trigger == 1)){
        set_freq(0, happy_sound_freq[h_beat / 1000]);
        h_beat++;
    }
    
    if((s_beat <= sad_sound_beats[s_beat / 1000]) && (sad_sound_trigger == 1)){
        set_freq(0, sad_sound_freq[s_beat / 1000]);
        s_beat++;
    }
    DAC->DHR12R1 = samp;
}

static void eeprom_logic(void) {
    switch (eeprom_status) {
    case EEPROM_READ:
        _read_leaderboard(board_ptr);
        eeprom_status = EEPROM_IDLE;
        break;
    case EEPROM_WRITE:
        _write_leaderboard(board_ptr);
        eeprom_status = EEPROM_IDLE;
        break;
    case EEPROM_IDLE:
        break;
    }
}

static void controls_logic(void) {
    ADC1->CR |= ADC_CR_ADSTART;
    while(!(ADC1->ISR & ADC_ISR_EOC));
    joystickX = ADC1->DR;
    while(!(ADC1->ISR & ADC_ISR_EOC));
    joystickY = ADC1->DR;
    joystick_analog_x = ADC_CENTER - (int)joystickX;
    joystick_analog_y = ADC_CENTER - (int)joystickY; // invert val
    evaluate_direction(joystick_analog_x, joystick_analog_y);
    button_state = GPIOB->IDR & GPIO_IDR_0;
}

static int absolute(int x) {
    if (x < 0) return -x;
    else return x;
}

static void evaluate_direction(int x, int y) {
    int abs_x = absolute(x);
    int abs_y = absolute(y);
    if (abs_x < DEADZONE_RADIUS && abs_y < DEADZONE_RADIUS) {
        joystick_state = JOYSTICK_IDLE;
    }
    else if (abs_x > abs_y * 2) {
        if (x > DEADZONE_RADIUS) {
            joystick_state = JOYSTICK_RIGHT;
        }
        else if (x < -DEADZONE_RADIUS) {
            joystick_state = JOYSTICK_LEFT;
        }
    } else if (abs_y > abs_x * 2) {
        if (y > DEADZONE_RADIUS) {
            joystick_state = JOYSTICK_UP;
        }
        else if (y < -DEADZONE_RADIUS) {
            joystick_state= JOYSTICK_DOWN;
        }
    }
}
