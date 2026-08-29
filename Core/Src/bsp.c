#include "bsp.h"

#define NRF24_SPI                  SPI1

#define NRF24_REG_ADDR_CONFIG      0x00
#define NRF24_REG_ADDR_EN_AA       0x01
#define NRF24_REG_ADDR_SETUP_AW    0x03
#define NRF24_REG_ADDR_SETUP_RETR  0x04
#define NRF24_REG_ADDR_RF_CH       0x05
#define NRF24_REG_ADDR_RF_SETUP    0x06
#define NRF24_REG_ADDR_STATUS      0x07
#define NRF24_REG_ADDR_FIFO_STATUS 0x17

#define NRF24_CMD_TX_WRITE_PAYLOAD 0xA0
#define NRF24_CMD_TX_CLEAR         0xE1

#define NRF24_ON_DELAY_MS          2
#define NRF24_CE_DELAY_US          20

#define I2C_FLAG_BUSY_TIMEOUT      20

extern volatile uint32_t system_ticks;

// === SYSTEM ===

void LowPower_Delay(uint32_t Delay) {
    uint32_t start = system_ticks;
    while ((system_ticks - start) < Delay) {
        __WFI(); 
    }
}

// === SENSORS POWER ===

static inline void sensors_power_on() {
  LL_GPIO_SetPinMode(SENSOR_VDD_GPIO_Port, SENSOR_VDD_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetOutputPin(SENSOR_VDD_GPIO_Port, SENSOR_VDD_Pin);
}
static inline void sensors_power_off() {
  LL_GPIO_SetPinMode(SENSOR_VDD_GPIO_Port, SENSOR_VDD_Pin, LL_GPIO_MODE_ANALOG);
  LL_GPIO_ResetOutputPin(SENSOR_VDD_GPIO_Port, SENSOR_VDD_Pin);
}

// === I2C ===

void i2c_start(void) {
    //Reset triggers
    LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);

    MX_I2C1_Init();
}

void i2c_stop(void) {
    WAIT_FLAG(!LL_I2C_IsActiveFlag_BUSY(I2C1), I2C_FLAG_BUSY_TIMEOUT); 
    
    LL_I2C_Disable(I2C1);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C1);
    
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ANALOG);
}

// === SPI ===

void spi_start(void) {
    MX_SPI1_Init();
    LL_SPI_Enable(NRF24_SPI);  
}

void spi_stop(void){
    LL_SPI_Disable(NRF24_SPI);
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI1);

    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);

    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
    
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_6, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_7, LL_GPIO_PULL_NO);
}

// === PERIPHERAL MODES ===

void periph_mode_active() {
    sensors_power_on();
    LowPower_Delay(50);
    i2c_start();
    spi_start();
}

void periph_mode_sleep(){
    i2c_stop();
    spi_stop();
    sensors_power_off();
}