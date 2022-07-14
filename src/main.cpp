#include "main.h"
#include "nrf24l01.h"

static SPI_HandleTypeDef hspi1 = { .Instance = SPI1 };

struct status_interface {
    uint8_t SPI;
    uint8_t NRF;
} status_interfaceStruct;

void PIN_Init(void);
void SPI_Init(void);

int main(void) {
    HAL_Init();
    PIN_Init();
    SPI_Init();
    HAL_GPIO_WritePin(MYLED_GPIO_PORT, MYLED_PIN, (status_interfaceStruct.SPI) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_Delay(500);
    nRF_radio nrf(&hspi1, SPI_GPIO_PORT);
    nrf.setPins(NRF_SS_PIN, NRF_CE_PIN);
    if (nrf.init(0x5C, 0x7878787878) != 1) status_interfaceStruct.NRF = 0;
    else status_interfaceStruct.NRF = 1;
    HAL_GPIO_WritePin(MYLED_GPIO_PORT, MYLED_PIN, (status_interfaceStruct.NRF) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_Delay(500);
    while (1) {
        nrf.send(&status_interfaceStruct);
        HAL_Delay(100);
    }
    return 0;
}

void SPI_Init(void) {
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_LSB;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) status_interfaceStruct.SPI = 0;
    else status_interfaceStruct.SPI = 1;
}

void PIN_Init(void) {
    GPIO_InitTypeDef gpio;
    gpio.Pin = SPI_SCK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(SPI_GPIO_PORT, &gpio);
    gpio.Pin = NRF_SS_PIN | NRF_CE_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(SPI_GPIO_PORT, &gpio);
    __SPI1_CLK_ENABLE();
    gpio.Pin = MYLED_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(MYLED_GPIO_PORT, &gpio);
    __GPIOC_CLK_ENABLE();
}

void SysTick_Handler(void) {
    HAL_IncTick();
}