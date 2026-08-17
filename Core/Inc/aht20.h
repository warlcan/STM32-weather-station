#ifndef AHT20_UTILS_H
#define AHT20_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32l0xx_ll_i2c.h"
#include "i2c_utils.h"

#define AHT20_I2C_ADDRESS 0x38 << 1

#define AHT20_CALIBRATE_DELAY_MS 40
#define AHT20_MEASURE_DELAY_MS 90

#define AHT20_STATUS_BUSY_BIT  0x80
#define AHT20_STATUS_CAL_BIT 0x08

typedef struct {
    float humidity;
    float temperature;
} AHT20_Data_t;

bool aht20_get_data(I2C_TypeDef *I2Cx, AHT20_Data_t *out_data);

#endif