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

void nrf24_init(void);
bool nrf24_transmit_data(NRF24_Data_t *nrf24_data);

#endif