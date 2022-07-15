#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx_hal.h"

#define MYLED_GPIO_PORT GPIOC
#define MYLED_PIN       GPIO_PIN_13

#define SPI1_GPIO_PORT GPIOA
#define SPI1_SCK_PIN   GPIO_PIN_5
#define SPI1_MISO_PIN  GPIO_PIN_6
#define SPI1_MOSI_PIN  GPIO_PIN_7
#define NRF_SS_PIN     GPIO_PIN_3
#define NRF_CE_PIN     GPIO_PIN_4

#define SPI2_GPIO_PORT   GPIOB
#define SPI2_SCK_PIN     GPIO_PIN_13
#define SPI2_MISO_PIN    GPIO_PIN_14
#define SPI2_MOSI_PIN    GPIO_PIN_15
#define W25Q32_SS_PIN    GPIO_PIN_0
#define W25Q32_GPIO_PORT GPIOB

#endif // MAIN_H