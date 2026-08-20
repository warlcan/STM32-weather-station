#include "i2c_utils.h"

extern volatile uint32_t system_ticks;

#define WAIT_I2C_FLAG(I2Cx, target_func, timeout) do {      \
    uint32_t _start = system_ticks;                         \
    while (!target_func(I2Cx)) {                            \
        if(LL_I2C_IsActiveFlag_NACK(I2Cx) ||                \
           LL_I2C_IsActiveFlag_BERR(I2Cx)) {                \
            LL_I2C_ClearFlag_NACK(I2Cx);                    \
            LL_I2C_ClearFlag_BERR(I2Cx);                    \
            return false;                                   \
        }                                                   \
        if(system_ticks - _start >= timeout) return false;  \
    }                                                       \
} while(0)

bool i2c_transmit_data(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t *cmd_data, uint8_t cmd_size) {
    LL_I2C_HandleTransfer(I2Cx, slave_addr, 
                        LL_I2C_ADDRSLAVE_7BIT, cmd_size, 
                        LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
    for (uint8_t i = 0; i < cmd_size; i++) {
        WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_TXIS, I2C_TIMEOUT_MS);
        LL_I2C_TransmitData8(I2Cx, cmd_data[i]);
    }
    WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_STOP, I2C_TIMEOUT_MS);
    return true;
}

bool i2c_receive_data(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t *data_buffer, uint8_t data_size) {
    LL_I2C_HandleTransfer(I2Cx, slave_addr,
                          LL_I2C_ADDRSLAVE_7BIT, data_size, 
                          LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
    for (uint8_t i = 0; i < data_size; i++) {
        WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_RXNE, I2C_TIMEOUT_MS);
        data_buffer[i] = LL_I2C_ReceiveData8(I2Cx);
    }
    WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_STOP, I2C_TIMEOUT_MS);
    return true;
}

bool i2c_receive_reg_data(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t reg_addr, uint8_t *data_buffer, uint8_t data_size) {
    //Transmit register address
    LL_I2C_HandleTransfer(I2Cx, slave_addr, 
                        LL_I2C_ADDRSLAVE_7BIT, 1, 
                        LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);
    WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_TXIS, I2C_TIMEOUT_MS);
    LL_I2C_TransmitData8(I2Cx, reg_addr);
    WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_TC, I2C_TIMEOUT_MS);

    //Receive register data
    LL_I2C_HandleTransfer(I2Cx, slave_addr,
                          LL_I2C_ADDRSLAVE_7BIT, data_size, 
                          LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
    for (uint8_t i = 0; i < data_size; i++) {
        WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_RXNE, I2C_TIMEOUT_MS);
        data_buffer[i] = LL_I2C_ReceiveData8(I2Cx);
    }
    WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_STOP, I2C_TIMEOUT_MS);
    return true;
}