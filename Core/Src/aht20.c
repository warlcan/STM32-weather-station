#include "aht20.h"

extern volatile uint32_t system_ticks;

#define WAIT_I2C_FLAG(I2Cx, target_func, timeout) do {      \
    uint32_t _start = system_ticks;                         \
    while (!target_func(I2Cx)) {                            \
        if(system_ticks - _start >= timeout) return false;  \
    }                                                       \
} while(0)

bool aht20_get_data(AHT20_Data_t *out_data){
    //Clear flags
    LL_I2C_ClearFlag_NACK(I2C1);
    LL_I2C_ClearFlag_BERR(I2C1);
    //Transmit
    LL_I2C_HandleTransfer(I2C1, AHT20_I2C_ADDRESS, 
                        LL_I2C_ADDRSLAVE_7BIT, 3, 
                        LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
    WAIT_I2C_FLAG(I2C1, LL_I2C_IsActiveFlag_TXIS, 10); // 10 ms
    LL_I2C_TransmitData8(I2C1, AHT20_CMD_MEASURE);
    WAIT_I2C_FLAG(I2C1, LL_I2C_IsActiveFlag_TXIS, 10);
    LL_I2C_TransmitData8(I2C1, AHT20_CMD_MEASURE_PARAM1);
    WAIT_I2C_FLAG(I2C1, LL_I2C_IsActiveFlag_TXIS, 10);
    LL_I2C_TransmitData8(I2C1, AHT20_CMD_MEASURE_PARAM2);
    WAIT_I2C_FLAG(I2C1, LL_I2C_IsActiveFlag_STOP, 10);
    LL_I2C_ClearFlag_STOP(I2C1);
    
    //Error check
    if(LL_I2C_IsActiveFlag_NACK(I2C1) || LL_I2C_IsActiveFlag_BERR(I2C1)) {
        LL_I2C_ClearFlag_NACK(I2C1);
        LL_I2C_ClearFlag_BERR(I2C1);
        return false;
    }

    LL_mDelay(AHT20_MEASURE_DELAY_MS);
    
    //Receive
    uint8_t data_buffer[6];
    LL_I2C_HandleTransfer(I2C1, AHT20_I2C_ADDRESS,
                          LL_I2C_ADDRSLAVE_7BIT, 6, 
                          LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
    for (uint8_t i = 0; i < 6; i++) {
        WAIT_I2C_FLAG(I2C1, LL_I2C_IsActiveFlag_RXNE, 10);
        data_buffer[i] = LL_I2C_ReceiveData8(I2C1);
    }
    WAIT_I2C_FLAG(I2C1, LL_I2C_IsActiveFlag_STOP, 10);
    LL_I2C_ClearFlag_STOP(I2C1);

    //Error check
    if(LL_I2C_IsActiveFlag_NACK(I2C1) || LL_I2C_IsActiveFlag_BERR(I2C1)) {
        LL_I2C_ClearFlag_NACK(I2C1);
        LL_I2C_ClearFlag_BERR(I2C1);
        return false;
    }

    if ((data_buffer[0] & AHT20_STATUS_BUSY_BIT) != 0) return false;

    uint32_t humidity = (((uint32_t)data_buffer[1]) << 12) |
                        (((uint32_t)data_buffer[2]) << 4) |
                        ((data_buffer[3] & 0xF0) >> 4);
    uint32_t temperature = (((uint32_t)(data_buffer[3] & 0x0F)) << 16) |
                           (((uint32_t)data_buffer[4]) << 8) |
                           data_buffer[5];

    out_data->humidity = ((float)humidity * 100.0f) / 1048576.0f;
    out_data->temperature = (((float)temperature * 200.0f) / 1048576.0f) - 50.0f;

    return true;
}