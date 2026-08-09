#ifndef __AHT20_H
#define __AHT20_H

#include <stdbool.h>
#include "stm32l0xx_ll_i2c.h" 
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_utils.h"

#define AHT20_I2C_ADDRESS 0x38 << 1

#define AHT20_CMD_MEASURE        0xAC
#define AHT20_CMD_MEASURE_PARAM1 0x33
#define AHT20_CMD_MEASURE_PARAM2 0x00

#define AHT20_MEASURE_DELAY_MS 90
#define AHT20_STATUS_BUSY_BIT  0x80

typedef struct {
    float humidity;
    float temperature;
} AHT20_Data_t;

bool aht20_get_data(AHT20_Data_t *out_data);

#endif