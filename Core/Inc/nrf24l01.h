#ifndef NRF24L01_H
#define NRF24L01_H

#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "bsp.h"
#include "stm32l0xx_ll_spi.h"
#include "stm32l0xx_ll_gpio.h"

typedef struct __attribute__((packed)) {
    float temperature;
    float humidity;
    float pressure;
} NRF24_Data_t;

void NRF24_Init(void);
bool NRF24_TransmitData(NRF24_Data_t *nrf24_data);

#endif