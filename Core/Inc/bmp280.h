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

bool BMP280_GetData(BMP280_Data_t *out_data);
bool BMP280_GetCoef(void);

#endif