#include "aht20.h"

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

bool transmit_command(I2C_TypeDef *I2Cx, uint8_t *cmd_data) {
    LL_I2C_HandleTransfer(I2Cx, AHT20_I2C_ADDRESS, 
                        LL_I2C_ADDRSLAVE_7BIT, 3, 
                        LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
    for (uint8_t i = 0; i < 3; i++) {
        WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_TXIS, AHT20_I2C_TIMEOUT_MS);
        LL_I2C_TransmitData8(I2Cx, cmd_data[i]);
    }
    WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_STOP, AHT20_I2C_TIMEOUT_MS);
    return true;
}

bool receive_data(I2C_TypeDef *I2Cx, uint8_t *data_buffer) {
    LL_I2C_HandleTransfer(I2Cx, AHT20_I2C_ADDRESS,
                          LL_I2C_ADDRSLAVE_7BIT, 6, 
                          LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
    for (uint8_t i = 0; i < 6; i++) {
        WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_RXNE, AHT20_I2C_TIMEOUT_MS);
        data_buffer[i] = LL_I2C_ReceiveData8(I2Cx);
    }
    WAIT_I2C_FLAG(I2Cx, LL_I2C_IsActiveFlag_STOP, AHT20_I2C_TIMEOUT_MS);
    return true;
}

bool aht20_get_data(AHT20_Data_t *out_data) {
    //Clear flags
    LL_I2C_ClearFlag_NACK(I2C1);
    LL_I2C_ClearFlag_BERR(I2C1);

    //Transmit
    uint8_t measure_cmd_bytes[3] = {0xAC, 0x33, 0x00};
    if(!transmit_command(I2C1, measure_cmd_bytes)) return false;

    LL_mDelay(AHT20_MEASURE_DELAY_MS);
    
    //Receive
    uint8_t receive_data_buffer[6];
    if(!receive_data(I2C1, receive_data_buffer)) return false;

    //Check errors
    if ((receive_data_buffer[0] & AHT20_STATUS_BUSY_BIT) != 0) return false;
    if ((receive_data_buffer[0] & AHT20_STATUS_CAL_BIT)  != 0) {
        uint8_t calibrate_cmd_bytes[] = {0xBE, 0x08, 0x00};
        if(!transmit_command(I2C1, calibrate_cmd_bytes)) return false;
    }

    //Parsing
    uint32_t humidity = (((uint32_t)receive_data_buffer[1]) << 12) |
                        (((uint32_t)receive_data_buffer[2]) << 4) |
                        ((receive_data_buffer[3] & 0xF0) >> 4);
    uint32_t temperature = (((uint32_t)(receive_data_buffer[3] & 0x0F)) << 16) |
                           (((uint32_t)receive_data_buffer[4]) << 8) |
                           receive_data_buffer[5];

    //Calculation
    out_data->humidity = ((float)humidity * 100.0f) / 1048576.0f;
    out_data->temperature = (((float)temperature * 200.0f) / 1048576.0f) - 50.0f;

    return true;
}