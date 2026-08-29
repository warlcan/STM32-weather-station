#include "nrf24l01.h"

#define NRF24_SPI                  SPI1

#define NRF24_REG_ADDR_CONFIG      0x00
#define NRF24_REG_ADDR_EN_AA       0x01
#define NRF24_REG_ADDR_SETUP_AW    0x03
#define NRF24_REG_ADDR_SETUP_RETR  0x04
#define NRF24_REG_ADDR_RF_CH       0x05
#define NRF24_REG_ADDR_RF_SETUP    0x06
#define NRF24_REG_ADDR_STATUS      0x07

#define NRF24_CMD_TX_WRITE_PAYLOAD 0xA0
#define NRF24_CMD_TX_CLEAR         0xE1

#define NRF24_ON_DELAY_MS          2
#define NRF24_CE_DELAY_US          20

//US delay for 2.1 MHz
#define NRF24_DELAY_US(us) do { \
    volatile uint32_t cycles = ((us) * 7) / 10; \
    if (cycles == 0) cycles = 1; \
    while(cycles--); \
} while(0)

extern void LowPower_Delay(uint32_t Delay);

static uint8_t spi_transmit_byte(uint8_t data) {
    if (LL_SPI_IsActiveFlag_OVR(NRF24_SPI)) {
        LL_SPI_ReceiveData8(NRF24_SPI);
    }
    
    while(!LL_SPI_IsActiveFlag_TXE(NRF24_SPI));
    LL_SPI_TransmitData8(NRF24_SPI, data);
    
    while(!LL_SPI_IsActiveFlag_RXNE(NRF24_SPI));
    return LL_SPI_ReceiveData8(NRF24_SPI);
}

static void nrf24_spi_set_reg(uint8_t reg, uint8_t val) {
    LL_GPIO_ResetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);
    spi_transmit_byte(0x20 | (reg & 0x1F));
    spi_transmit_byte(val);
    LL_GPIO_SetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);
}

static uint8_t nrf24_spi_read_reg(uint8_t reg) {
    uint8_t val;
    LL_GPIO_ResetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);
    spi_transmit_byte(reg & 0x1F);
    val = spi_transmit_byte(0xFF); //0xFF - dummy byte
    LL_GPIO_SetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);
    return val;
}

static void nrf24_spi_transmit_buf(uint8_t reg, uint8_t *buf, uint8_t buf_size) {
    LL_GPIO_ResetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);
    spi_transmit_byte(reg);
    for(uint8_t i = 0; i < buf_size; i++) {
        spi_transmit_byte(buf[i]);
    }
    LL_GPIO_SetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);
}

void nrf24_init(void){
    LL_GPIO_ResetOutputPin(NRF_CE_GPIO_Port, NRF_CE_Pin);
    LL_GPIO_SetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);

    nrf24_spi_set_reg(NRF24_REG_ADDR_EN_AA,      0x00);
    nrf24_spi_set_reg(NRF24_REG_ADDR_SETUP_RETR, 0x00);
    nrf24_spi_set_reg(NRF24_REG_ADDR_SETUP_AW,   0x03);
    nrf24_spi_set_reg(NRF24_REG_ADDR_RF_CH,      100);
    nrf24_spi_set_reg(NRF24_REG_ADDR_RF_SETUP,   0x07);

    uint8_t addr[5] = {0x9c, 0x96, 0xf1, 0x1f, 0x5e};
    nrf24_spi_transmit_buf(0x20 | 0x10, addr, 5);

    nrf24_spi_set_reg(NRF24_REG_ADDR_STATUS, 0x70);
}

bool nrf24_transmit_data(NRF24_Data_t *nrf24_data) {
    uint8_t nrf24_data_size = sizeof(NRF24_Data_t);

    nrf24_spi_set_reg(NRF24_REG_ADDR_CONFIG, 0x0E);
    LowPower_Delay(NRF24_ON_DELAY_MS);

    //Clear fifo buffer
    LL_GPIO_ResetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);
    spi_transmit_byte(NRF24_CMD_TX_CLEAR);
    LL_GPIO_SetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);

    nrf24_spi_set_reg(NRF24_REG_ADDR_STATUS, 0x70);

    nrf24_spi_transmit_buf(NRF24_CMD_TX_WRITE_PAYLOAD, (uint8_t*)nrf24_data, nrf24_data_size);

    LL_GPIO_SetOutputPin(NRF_CE_GPIO_Port, NRF_CE_Pin);
    NRF24_DELAY_US(NRF24_CE_DELAY_US);
    LL_GPIO_ResetOutputPin(NRF_CE_GPIO_Port, NRF_CE_Pin);

    LowPower_Delay(1);

    volatile uint16_t timeout = 60000;
    while(!(nrf24_spi_read_reg(NRF24_REG_ADDR_STATUS)& 0x20)) {
        if(timeout-- == 0) {
            nrf24_spi_set_reg(NRF24_REG_ADDR_STATUS, 0x70);
            return false;
        }
    }

    nrf24_spi_set_reg(NRF24_REG_ADDR_STATUS, 0x20);
    nrf24_spi_set_reg(NRF24_REG_ADDR_CONFIG, 0x00);
    while(LL_SPI_IsActiveFlag_BSY(NRF24_SPI));
    return true;
}