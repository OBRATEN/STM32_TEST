#include "main.h"
#include "nrf24l01.h"
#include "lsm6ds3.h"
#include "w25q.h"

static SPI_HandleTypeDef hspi1 = { .Instance = SPI1 };
static SPI_HandleTypeDef hspi2 = { .Instance = SPI2 };

static nRF_radio nrf(&hspi1, SPI1_GPIO_PORT);
static LSM6DS3_gax gax(&hspi1, SPI1_GPIO_PORT);
static W25Q_memory memarr(&hspi2, SPI2_GPIO_PORT);

typedef struct statusStruct {
  uint8_t spi_1;
  uint8_t spi_2;
  uint8_t w25q32;
  uint8_t nrf;
  uint8_t isBlockClear;
  uint8_t lsm6ds3;
} status_typedef;

typedef struct gaxStruct {
  float aX;
  float aY;
  float aZ;
  float gX;
  float gY;
  float gZ;
} gaxStruct_typedef;

static status_typedef status_struct;
static gaxStruct_typedef gax_data;
static uint32_t pageAddr = 0;

void PIN_Init(void);
void SPI_Init(void);
void PER_Init(void);

int main(void) {
  HAL_Init();
  PIN_Init();
  SPI_Init();
  PER_Init();
  memarr.eraseBlock(0);
  if (memarr.isPageClear(0, 0)) status_struct.isBlockClear = 1;
  else status_struct.isBlockClear = 0;
  for(;;) {
    if (gax.availableXYZ()) gax.readXYZ(gax_data.aX, gax_data.aY, gax_data.aZ);
    if (gax.availableGyro()) gax.readGyro(gax_data.gX, gax_data.gY, gax_data.gZ);
    uint8_t *buff = reinterpret_cast<uint8_t*>(&gax_data);
    nrf.send(&status_struct);
    memarr.writePage(buff, pageAddr++, 0, sizeof(status_struct));
    HAL_GPIO_TogglePin(MYLED_GPIO_PORT, MYLED_PIN);
    HAL_Delay(100);
  } return 0;
}

void PER_Init(void) {
    nrf.setPins(NRF_SS_PIN, NRF_CE_PIN);
    gax.setPins(LSM6DS3_SS_PIN);
    if (nrf.init(0x5C, 0x7878787878) == 1) status_struct.nrf = 1;
    else status_struct.nrf = 0;
    memarr.setCS(W25Q32_GPIO_PORT, W25Q32_SS_PIN);
    if (memarr.init(W25_TYPE_W25Q32) == 1) status_struct.w25q32 = 1;
    else status_struct.w25q32 = 0;
    if (gax.init()) status_struct.lsm6ds3 = 1;
    else status_struct.lsm6ds3 = 0;
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
    if (HAL_SPI_Init(&hspi1) != HAL_OK) status_struct.spi_1 = 0;
    else status_struct.spi_1 = 1;
    __SPI1_CLK_ENABLE();
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_LSB;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    if (HAL_SPI_Init(&hspi2) != HAL_OK) status_struct.spi_2 = 0;
    else status_struct.spi_2 = 1;
    __SPI2_CLK_ENABLE();
}

void PIN_Init(void) {
    GPIO_InitTypeDef gpio;
    gpio.Pin = SPI1_SCK_PIN | SPI1_MISO_PIN | SPI1_MOSI_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Alternate = GPIO_AF5_SPI1;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(SPI1_GPIO_PORT, &gpio);
    gpio.Pin = NRF_SS_PIN | NRF_CE_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(SPI1_GPIO_PORT, &gpio);
    __GPIOA_CLK_ENABLE();
    gpio.Pin = MYLED_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(MYLED_GPIO_PORT, &gpio);
    __GPIOC_CLK_ENABLE();
    gpio.Pin = W25Q32_SS_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(W25Q32_GPIO_PORT, &gpio);
    __GPIOB_CLK_ENABLE();
    gpio.Pin = LSM6DS3_SS_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(LSM6DS3_GPIO_PORT, &gpio);
    __GPIOB_CLK_ENABLE();
}

void SysTick_Handler(void) {
    HAL_IncTick();
}