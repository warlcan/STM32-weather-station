#ifndef I2C_UTILS_H
#define I2C_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "stm32l0xx_ll_i2c.h" 
#include "bsp.h"

bool I2C_TransmitData(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t *cmd_data, uint8_t cmd_size);
bool I2C_ReceiveData(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t *data_buffer, uint8_t data_size);
bool I2C_ReceiveRegsData(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_buffer, uint8_t data_size);

#endif