#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32l0xx_ll_i2c.h" 

#define I2C_TIMEOUT_MS 10

bool i2c_transmit_data(I2C_TypeDef *I2Cx, uint32_t slave_addr, uint8_t *cmd_data, uint8_t cmd_size);
bool i2c_receive_data(I2C_TypeDef *I2Cx,uint32_t slave_addr, uint8_t *data_buffer, uint8_t data_size);

#endif