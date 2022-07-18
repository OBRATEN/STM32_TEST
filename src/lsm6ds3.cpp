#include "lsm6ds3.h"

uint8_t LSM6DS3_gax::setPins(uint16_t cs, uint16_t irq) {
  if ((cs > 15) || (cs < 0) || (irq > 15) || (irq < 0)) return LSM6DS3_ERR;
  _cs = cs;
  _irq = irq;
  return LSM6DS3_OK;
}

inline void LSM6DS3_gax::csHigh(void) {
  HAL_GPIO_WritePin(_gpio, _cs, GPIO_PIN_SET);
}

inline void LSM6DS3_gax::csLow(void) {
  HAL_GPIO_WritePin(_gpio, _cs, GPIO_PIN_RESET);
}

void LSM6DS3_gax::writeReg(uint8_t addr, uint8_t data) {
  csLow();
  HAL_SPI_Transmit(_spi, &addr, 1, 100);
  HAL_SPI_Transmit(_spi, &data, 1, 100);
  csHigh();
}

uint8_t LSM6DS3_gax::readReg(uint8_t addr) {
  uint8_t spiAddr = 0x80 | addr;
  uint8_t result;
  csLow();
  HAL_SPI_TransmitReceive(_spi, &spiAddr, &result, 1, 100);
  csHigh();
  return result;
}

void LSM6DS3_gax::readRegs(uint8_t addr, uint8_t *buff, uint8_t count) {
  uint8_t spiAddr = 0x80 | addr;
  csLow();
  for (count; count > 0; count--) {
    spiAddr = 0x80 | addr;
    HAL_SPI_TransmitReceive(_spi, &spiAddr, buff, 1, 100);
    addr++;
    buff++;
  } csHigh();
}

uint8_t LSM6DS3_gax::enable(void) {
  csHigh();
  if (readReg(LSM6DS3_WHO_AM_I_REG) != 0x69) goto fail;
  writeReg(LSM6DS3_CTRL2_G, 0x4C);
  writeReg(LSM6DS3_CTRL1_XL, 0x4A);
  writeReg(LSM6DS3_CTRL7_G, 0x00);
  writeReg(LSM6DS3_CTRL8_XL, 0x09);
  _inited = 1;
  _initcode = LSM6DS3_OK;
  return LSM6DS3_OK;
fail:
  _inited = -1;
  _initcode = LSM6DS3_ERR_WAI_REG;
  return LSM6DS3_ERR;
}

void LSM6DS3_gax::readXYZ(float& x, float& y, float& z) {
  int16_t data[3];
  data[0] = (readReg(LSM6DS3_OUTX_H_XL & 0xff) << 8) | readReg(LSM6DS3_OUTX_L_XL);
  data[1] = (readReg(LSM6DS3_OUTY_H_XL & 0xff) << 8) | readReg(LSM6DS3_OUTY_L_XL);
  data[2] = (readReg(LSM6DS3_OUTZ_H_XL & 0xff) << 8) | readReg(LSM6DS3_OUTZ_L_XL);
  x = data[0] * 4.0 / 32768.0;
  y = data[1] * 4.0 / 32768.0;
  z = data[2] * 4.0 / 32768.0;
}

uint8_t LSM6DS3_gax::availableXYZ(void) {
  if (readReg(LSM6DS3_STATUS_REG) & 0x01) return LSM6DS3_OK;
  return LSM6DS3_ERR;
}

void LSM6DS3_gax::readGyro(float& x, float& y, float& z) {
  int16_t data[3];
  data[0] = (readReg(LSM6DS3_OUTX_H_G & 0xff) << 8) | readReg(LSM6DS3_OUTX_L_G);
  data[1] = (readReg(LSM6DS3_OUTY_H_G & 0xff) << 8) | readReg(LSM6DS3_OUTY_L_G);
  data[2] = (readReg(LSM6DS3_OUTZ_H_G & 0xff) << 8) | readReg(LSM6DS3_OUTZ_L_G);
  x = data[0] * 2000.0 / 32768.0;
  y = data[1] * 2000.0 / 32768.0;
  z = data[2] * 2000.0 / 32768.0;
}


uint8_t LSM6DS3_gax::availableGyro(void) {
  if (readReg(LSM6DS3_STATUS_REG) & 0x02) return LSM6DS3_OK;
  return LSM6DS3_ERR;
}