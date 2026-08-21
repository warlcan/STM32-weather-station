#ifndef AHT20_H
#define AHT20_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32l0xx_ll_i2c.h"
#include "i2c_utils.h"

typedef struct {
    float humidity;
    float temperature;
} AHT20_Data_t;

bool aht20_get_data(AHT20_Data_t *out_data);

#endif