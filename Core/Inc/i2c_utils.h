#ifndef I2C_UTILS_H
#define I2C_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32l0xx_ll_i2c.h" 

bool i2c_transmit_data(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t *cmd_data, uint8_t cmd_size);
bool i2c_receive_data(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t *data_buffer, uint8_t data_size);
bool i2c_receive_reg_data(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_buffer, uint8_t data_size);
 
#endif