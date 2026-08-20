#include "bmp280.h"

#define BMP280_I2C I2C1
#define BMP280_I2C_ADDRESS (0x77 << 1)

extern void LowPower_Delay(uint32_t Delay);

typedef struct {
    uint16_t dig_T1; int16_t  dig_T2; int16_t  dig_T3;
    
    uint16_t dig_P1; int16_t  dig_P2; int16_t  dig_P3;
    int16_t  dig_P4; int16_t  dig_P5; int16_t  dig_P6;
    int16_t  dig_P7; int16_t  dig_P8; int16_t  dig_P9;
} BMP280_calibrate_bytes_t;
static BMP280_calibrate_bytes_t cb;
static bool BMP280_is_init = false;

// Temperature compensation from BMP280 datasheet
static int32_t t_fine;
int32_t bmp280_compensate_T_int32(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T>>3) - ((int32_t)cb.dig_T1<<1))) * ((int32_t)cb.dig_T2)) >> 11;
    var2 = (((((adc_T>>4) - ((int32_t)cb.dig_T1)) * ((adc_T>>4) - ((int32_t)cb.dig_T1))) >> 12) *
    ((int32_t)cb.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}
// Pressure compensation from BMP280 datasheet
uint32_t bmp280_compensate_P_int64(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)cb.dig_P6;
    var2 = var2 + ((var1*(int64_t)cb.dig_P5)<<17);
    var2 = var2 + (((int64_t)cb.dig_P4)<<35);
    var1 = ((var1 * var1 * (int64_t)cb.dig_P3)>>8) + ((var1 * (int64_t)cb.dig_P2)<<12);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)cb.dig_P1)>>33;
    if (var1 == 0) { return 0; }
    p = 1048576-adc_P;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((int64_t)cb.dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((int64_t)cb.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)cb.dig_P7)<<4);
    return (uint32_t)p;
}

bool bmp280_init(void) {
    uint8_t calib[24]; //Receive compensation coefficients
    if (!i2c_receive_reg_data(BMP280_I2C, BMP280_I2C_ADDRESS, 0x88, calib, sizeof(calib))) return false;

    cb.dig_T1 = (uint16_t)(calib[0]  | (calib[1] << 8));
    cb.dig_T2 = (int16_t) (calib[2]  | (calib[3] << 8));
    cb.dig_T3 = (int16_t) (calib[4]  | (calib[5] << 8));
    cb.dig_P1 = (uint16_t)(calib[6]  | (calib[7] << 8));
    cb.dig_P2 = (int16_t) (calib[8]  | (calib[9] << 8));
    cb.dig_P3 = (int16_t) (calib[10] | (calib[11] << 8));
    cb.dig_P4 = (int16_t) (calib[12] | (calib[13] << 8));
    cb.dig_P5 = (int16_t) (calib[14] | (calib[15] << 8));
    cb.dig_P6 = (int16_t) (calib[16] | (calib[17] << 8));
    cb.dig_P7 = (int16_t) (calib[18] | (calib[19] << 8));
    cb.dig_P8 = (int16_t) (calib[20] | (calib[21] << 8));
    cb.dig_P9 = (int16_t) (calib[22] | (calib[23] << 8));

    BMP280_is_init = true;
    return true;
}

bool bmp280_get_data(BMP280_Data_t *out_data) {
    if (!BMP280_is_init) return false; //foolproofing

    //Transmit configuration
    uint8_t bmp280_config_data[2] = {0xF4, 0x4D}; // 0x010_011_01
    if (!i2c_transmit_data(BMP280_I2C, BMP280_I2C_ADDRESS, bmp280_config_data, sizeof(bmp280_config_data))){ return false; }
    
    LowPower_Delay(20);

    //Receive measurement data
    uint8_t measure_buffer[6];
    if (!i2c_receive_reg_data(BMP280_I2C, BMP280_I2C_ADDRESS, 0xF7, measure_buffer, sizeof(measure_buffer))){ return false; }

    //Parsing
    int32_t adc_P = (int32_t)((((uint32_t)(measure_buffer[0])) << 12) | 
                              (((uint32_t)(measure_buffer[1])) << 4)  | 
                              (((uint32_t)(measure_buffer[2])) >> 4));

    int32_t adc_T = (int32_t)((((uint32_t)(measure_buffer[3])) << 12) | 
                              (((uint32_t)(measure_buffer[4])) << 4)  | 
                              (((uint32_t)(measure_buffer[5])) >> 4));
    
    //Compensation
    int32_t raw_temp = bmp280_compensate_T_int32(adc_T);
    uint32_t raw_press = bmp280_compensate_P_int64(adc_P);

    out_data->temperature = (float)raw_temp / 100.0f;
    out_data->pressure = (float)(raw_press >> 8) / 100.0f; //Q24.8 to hPa

    return true;
}