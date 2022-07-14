#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx_hal.h"

#define MYLED_GPIO_PORT GPIOC
#define MYLED_PIN       GPIO_PIN_13

#define SPI_GPIO_PORT GPIOA
#define SPI_SCK_PIN   GPIO_PIN_5
#define SPI_MISO_PIN  GPIO_PIN_6
#define SPI_MOSI_PIN  GPIO_PIN_7
#define NRF_SS_PIN    GPIO_PIN_4
#define NRF_CE_PIN    GPIO_PIN_3

#endif // MAIN_H