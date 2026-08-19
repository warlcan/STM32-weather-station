#ifndef BMP280_H
#define BMP280_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32l0xx_ll_i2c.h"
#include "i2c_utils.h"

typedef struct {
    float pressure;
    float temperature;
} BMP280_Data_t;

bool bmp280_get_data(BMP280_Data_t *out_data);
bool bmp280_init(void);

#endif