#include "eeprom.h"

static void enable_ports(void);
static void init_i2c(void);
static void i2c_start(uint32_t targadr, uint8_t size, uint8_t dir);
static void i2c_stop();
static void i2c_waitidle();
static void i2c_clearnack();
static int i2c_checknack();
static int8_t i2c_senddata(uint32_t targadr, uint8_t data[], uint8_t size);
static int i2c_recvdata(uint32_t targadr, void *data, uint8_t size);

volatile enum EepromStatus eeprom_status;
struct Leaderboard *board_ptr;

void setup_eeprom() {
    enable_ports();
    init_i2c();
}

//===========================================================================
// Configure SDA and SCL.
//===========================================================================
static void enable_ports(void) {
    // Enable the clock for GPIOB (I2C1_SCL = PB6, I2C1_SDA = PB7)
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    // Configure PB6 and PB7 for I2C1 alternate function (AF1)
    GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10); // Clear mode bits
    GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1); // Set to alternate function
    // Set alternate function to AF4 (I2C1)
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL9 | GPIO_AFRH_AFSEL10);
    GPIOA->AFR[1] |= (0x4 << GPIO_AFRH_AFSEL9_Pos) | (0x4 << GPIO_AFRH_AFSEL10_Pos);

    // Added cause another student suggested it
    GPIOA->OTYPER |= GPIO_OTYPER_OT_9;
    GPIOA->OTYPER |= GPIO_OTYPER_OT_10;
}

//===========================================================================
// Configure I2C1.
//===========================================================================
static void init_i2c(void) {
    // Enable the peripheral clock for I2C1
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // Ensure I2C is disabled before configuration
    I2C1->CR1 &= ~I2C_CR1_PE;

    // Turn OFF analog noise filter
    // Enable error interrupts
    // Disable clock stretching
    I2C1->CR1 |= I2C_CR1_ANFOFF | I2C_CR1_ERRIE | I2C_CR1_NOSTRETCH;

    // 400 kHz fast mode
    I2C1->TIMINGR = (uint32_t)0x50330309;

    // 100 kHz standard mode (Same results)
    // I2C1->TIMINGR = (uint32_t)0xB0420F13;

    // Set 7-bit addressing mode and enable automatic STOP condition
    I2C1->CR2 &= ~I2C_CR2_ADD10;    // Turning off 10-bit mode for 7-bit addressing mode

    // I2C1->CR2 |= I2C_CR2_AUTOEND;  // Added b/c TA tried
    I2C1->CR2 &= ~I2C_CR2_AUTOEND;  // Turn off AUTOEND as per Niraj's instructions

    // Enable I2C1 peripheral
    I2C1->CR1 |= I2C_CR1_PE;
}

//===========================================================================
// Send a START bit.
//===========================================================================
static void i2c_start(uint32_t targadr, uint8_t size, uint8_t dir) {
    // ChatGPT's debugging tips:
    // Wait until I2C is not busy
    while (I2C1->ISR & I2C_ISR_BUSY);
    // Disable the peripheral before configuration (optional but sometimes necessary)
    // I2C1->CR1 &= ~I2C_CR1_PE;

    // 0. Take current contents of CR2 register.
    uint32_t tmpreg = I2C1->CR2;

    // 1. Clear the following bits in the tmpreg: SADD, NBYTES, RD_WRN, START, STOP
    tmpreg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);

    // 2. Set read/write direction in tmpreg.
    if ((int)dir == 0) { // Write = 0
        tmpreg &= ~I2C_CR2_RD_WRN;
    }
    else { // Read = 1
        tmpreg |= I2C_CR2_RD_WRN;
    }

    // 3. Set the target's address in SADD (shift targadr left by 1 bit) and the data size.
    tmpreg |= ((targadr<<1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);

    // 4. Set the START bit.
    tmpreg |= I2C_CR2_START;

    // 5. Start the conversion by writing the modified value back to the CR2 register.
    I2C1->CR2 = tmpreg;
}

//===========================================================================
// Send a STOP bit.
//===========================================================================
static void i2c_stop(void) {
    // 0. If a STOP bit has already been sent, return from the function.
    // Check the I2C1 ISR register for the corresponding bit.
    if (I2C1->ISR & I2C_ISR_STOPF) {
        return;
    }

    // 1. Set the STOP bit in the CR2 register.
    I2C1->CR2 |= I2C_CR2_STOP;

    // 2. Wait until STOPF flag is reset by checking the same flag in ISR.
    while (!(I2C1->ISR & I2C_ISR_STOPF));

    // 3. Clear the STOPF flag by writing 1 to the corresponding bit in the ICR.
    I2C1->ICR |= I2C_ICR_STOPCF;
}

//===========================================================================
// Wait until the I2C bus is not busy. (One-liner!)
//===========================================================================
static void i2c_waitidle(void) {
    while (I2C1->ISR & I2C_ISR_BUSY);
}

//===========================================================================
// Send each char in data[size] to the I2C bus at targadr.
//===========================================================================
static int8_t i2c_senddata(uint32_t targadr, uint8_t data[], uint8_t size) { // Changed targadr from uint8_t to uint32_t
    // Wait until the I2C is idle.
    i2c_waitidle();

    // Send a START condition to the target address with the write bit set.
    // I2C1->ISR |= I2C_ISR_TXIS;  // Added for testing (removed as per Niraj's instructions)
    i2c_start(targadr, size, 0);  // dir = 0 = write

    for (int i = 0; i <= ((int)size - 1); i++) {
        // Copied from the lab doc vvvvvvvvvv
        int count = 0;
        while ((I2C1->ISR & I2C_ISR_TXIS) == 0) {
            count += 1;
            if (count > 1000000) {
                return -1;
            }
            if (i2c_checknack()) {
                i2c_clearnack();
                i2c_stop();
                return -1;
            }
        }
        // Copied from the lab doc ^^^^^^^^^^
        
        // Mask data[i] with I2C_TXDR_TXDATA so we ensure the data being
        // transmitted is only 8 bits long, and write it to the I2C1 TXDR register.
        I2C1->TXDR = data[i] & I2C_TXDR_TXDATA;
    }

    // Outside the loop, loop until the TC and NACKF flags are not set.
    //Ran through the debugger and it looks like its getting stuck here
    while (!(I2C1->ISR & I2C_ISR_TC) && !(I2C1->ISR & I2C_ISR_NACKF));
    
    // Once the loop quits, if the NACKF flag is still set, return -1 
    // This indicates the target device did not acknowledge the data.
    if (I2C1->ISR & I2C_ISR_NACKF) {
        i2c_stop();
        return -1;
    }

    // Finally, send a STOP bit, and return 0 to indicate success.
    i2c_stop();
    return 0;
}

//===========================================================================
// Receive size chars from the I2C bus at targadr and store in data[size].
//===========================================================================
static int i2c_recvdata(uint32_t targadr, void *data, uint8_t size) { // Changed targadr from uint8_t to uint32_t
    // Wait until the I2C is idle.
    i2c_waitidle();
    // Send a START condition to the target address with the read bit set (1).
    i2c_start(targadr, size, 1);  // dir = 1 = read

    // Start a loop from 0 to size - 1
    for (int i = 0; i < size; i++) {
        // Copied from the lab doc vvvvvvvvvv
        int count = 0;
        while ((I2C1->ISR & I2C_ISR_RXNE) == 0) {
            count += 1;
            if (count > 1000000)
                return -1;
            if (i2c_checknack()) {
                i2c_clearnack();
                i2c_stop();
                return -1;
            }
        }
        // Copied from the lab doc ^^^^^^^^^^
        
        // Receive the data, masking to ensure only 8 bits
        ((uint8_t *)data)[i] = I2C1->RXDR & I2C_RXDR_RXDATA;
    }

    i2c_stop();
    return 0;
}

//===========================================================================
// Clear the NACK bit. (One-liner!)
//===========================================================================
static void i2c_clearnack(void) {
    I2C1->ICR = I2C_ICR_NACKCF; // Writing 1 to this clears the ACKF flag in I2C_ISR
}

//===========================================================================
// Check the NACK bit. (One-liner!)
//===========================================================================
static int i2c_checknack(void) {
    if (I2C1->ISR & I2C_ISR_NACKF) {
        return 1;
    } else {
        return 0;
    }
}

//===========================================================================
// EEPROM functions
// We'll give these so you don't have to figure out how to write to the EEPROM.
// These can differ by device.

void eeprom_write(uint16_t loc, const char* data, uint8_t len) {
    uint8_t bytes[34];
    bytes[0] = loc>>8;
    bytes[1] = loc&0xFF;
    for(int i = 0; i<len; i++){
        bytes[i+2] = data[i];
    }
    i2c_senddata(EEPROM_ADDR, bytes, len+2);
}

void eeprom_read(uint16_t loc, char data[], uint8_t len) {
    // ... your code here
    uint8_t bytes[2];
    bytes[0] = loc>>8;
    bytes[1] = loc&0xFF;
    i2c_senddata(EEPROM_ADDR, bytes, 2);
    i2c_recvdata(EEPROM_ADDR, data, len);
}

void read_entry(uint16_t loc, struct EepromEntry *entry) {
    eeprom_read(loc, (char *)&(entry->word), ENTRY_BYTES);
}

void write_entry(uint16_t loc, struct EepromEntry *entry) {
    eeprom_write(loc, (char *)&(entry->word), ENTRY_BYTES);
}
