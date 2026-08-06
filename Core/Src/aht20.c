#include "aht20.h"

static bool wait_flag(I2C_TypeDef *I2Cx, uint32_t (*flag_func)(const I2C_TypeDef*))  
{
    volatile uint32_t timeout = AHT20_TIMEOUT_COUNT;
    while (!flag_func(I2Cx)) 
    {
        if (--timeout == 0) return false;
    }
    return true;
}


bool aht20_get_measurements(AHT20_Data_t *out_data){
    LL_I2C_HandleTransfer(I2C1, AHT20_I2C_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, AHT20_WRITE_PACKET_SIZE, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
    if (!wait_flag(I2C1, LL_I2C_IsActiveFlag_TXIS)) return false;
    LL_I2C_TransmitData8(I2C1, AHT20_CMD_MEASURE);
    if (!wait_flag(I2C1, LL_I2C_IsActiveFlag_TXIS)) return false;
    LL_I2C_TransmitData8(I2C1, AHT20_CMD_MEASURE_PARAM1);
    if (!wait_flag(I2C1, LL_I2C_IsActiveFlag_TXIS)) return false;
    LL_I2C_TransmitData8(I2C1, AHT20_CMD_MEASURE_PARAM2);
    if (!wait_flag(I2C1, LL_I2C_IsActiveFlag_STOP)) return false;
    LL_I2C_ClearFlag_STOP(I2C1);

    LL_mDelay(80);

    uint8_t data_buffer[AHT20_PACKET_SIZE];
    LL_I2C_HandleTransfer(I2C1, AHT20_I2C_ADDRESS, LL_I2C_ADDRSLAVE_7BIT, AHT20_READ_PACKET_SIZE, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

    for (uint8_t i = 0; i < AHT20_PACKET_SIZE; i++) {
        if (!wait_flag(I2C1, LL_I2C_IsActiveFlag_RXNE)) return false;
        data_buffer[i] = LL_I2C_ReceiveData8(I2C1);
    }

    if (!wait_flag(I2C1, LL_I2C_IsActiveFlag_STOP)) return false;
    LL_I2C_ClearFlag_STOP(I2C1);

    if ((data_buffer[0] & AHT20_STATUS_BUSY_BIT) != 0) return false;

    uint32_t humidity = (((uint32_t)data_buffer[1]) << 12) | (((uint32_t)data_buffer[2]) << 4) | ((data_buffer[3] & 0xF0) >> 4);
    uint32_t temperature = (((uint32_t)(data_buffer[3] & 0x0F)) << 16) | (((uint32_t)data_buffer[4]) << 8) | data_buffer[5];

    out_data->humidity = ((float)humidity * 100.0f) / 1048576.0f;
    out_data->temperature = (((float)temperature * 200.0f) / 1048576.0f) - 50.0f;

    return true;
}