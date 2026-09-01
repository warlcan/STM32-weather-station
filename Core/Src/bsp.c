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

// === ERROR HANDLERS ===
static volatile uint16_t system_errors = ERR_NO_ERROR;

void BSP_ErrorSet(BSP_ErrMask_t error_mask) {
    system_errors |= error_mask;
}

void BSP_ErrorReset(BSP_ErrMask_t error_mask) {
    system_errors &= ~error_mask;
}

uint16_t BSP_GetErrors(void){
    return system_errors;
}

// === SYSTEM ===

void LowPower_Delay(uint32_t delay) {
    uint32_t start = system_ticks;
    while ((system_ticks - start) < delay) {
        __WFI(); 
    }
}

// === SENSORS POWER ===

static inline void BSP_SensorsPowerOn() {
  LL_GPIO_SetPinMode(SENSOR_VDD_GPIO_Port, SENSOR_VDD_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetOutputPin(SENSOR_VDD_GPIO_Port, SENSOR_VDD_Pin);
}
static inline void BSP_SensorsPowerOff() {
  LL_GPIO_SetPinMode(SENSOR_VDD_GPIO_Port, SENSOR_VDD_Pin, LL_GPIO_MODE_ANALOG);
  LL_GPIO_ResetOutputPin(SENSOR_VDD_GPIO_Port, SENSOR_VDD_Pin);
}

// === I2C ===

static void BSP_I2cStart(void) {
    //Reset i2c
    LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);

    MX_I2C1_Init();
}

static void BSP_I2cStop(void) {    
    LL_I2C_Disable(I2C1);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C1);
    
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ANALOG);
}

// === SPI ===

static void BSP_SpiStart(void) {
    MX_SPI1_Init();
    LL_SPI_Enable(NRF24_SPI);  
}

static void BSP_SpiStop(void){
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

void BSP_PeriphModeActive() {
    BSP_SensorsPowerOn();
    LowPower_Delay(50);
    BSP_I2cStart();
    BSP_SpiStart();
}

void BSP_PeriphModeSleep(){
    BSP_I2cStop();
    BSP_SpiStop();
    BSP_SensorsPowerOff();
}