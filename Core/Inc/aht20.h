#ifndef __AHT20_H
#define __AHT20_H

#include <stdbool.h>
#include "stm32l0xx_ll_i2c.h" 
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_utils.h"

#define AHT20_I2C_ADDRESS 0x70
#define AHT20_TIMEOUT_COUNT 100000

#define AHT20_CMD_MEASURE 0xAC
#define AHT20_CMD_MEASURE_PARAM1 0x33
#define AHT20_CMD_MEASURE_PARAM2 0x00

#define AHT20_WRITE_PACKET_SIZE 3
#define AHT20_READ_PACKET_SIZE 6

#define AHT20_MEASURE_DELAY_MS 80
#define AHT20_STATUS_BUSY_BIT 0x80  

#define AHT20_PACKET_SIZE 6
#define AHT20_STATUS_BUSY_BIT 0x80

typedef struct {
    float humidity;
    float temperature;
} AHT20_Data_t;

bool aht20_get_measurements(AHT20_Data_t *out_data);

#endif