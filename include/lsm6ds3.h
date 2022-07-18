#ifndef LSM6DS3_H
#define LSM6DS3_H

#include <stm32f4xx_hal.h>

#define LSM6DS3_OK  1
#define LSM6DS3_ERR 0

#define LSM6DS3_ERR_WAI_REG -1

#define LSM6DS3_WHO_AM_I_REG 0X0F
#define LSM6DS3_CTRL1_XL     0X10
#define LSM6DS3_CTRL2_G      0X11

#define LSM6DS3_STATUS_REG 0X1E

#define LSM6DS3_CTRL6_C  0X15
#define LSM6DS3_CTRL7_G  0X16
#define LSM6DS3_CTRL8_XL 0X17

#define LSM6DS3_OUTX_L_G 0X22
#define LSM6DS3_OUTX_H_G 0X23
#define LSM6DS3_OUTY_L_G 0X24
#define LSM6DS3_OUTY_H_G 0X25
#define LSM6DS3_OUTZ_L_G 0X26
#define LSM6DS3_OUTZ_H_G 0X27

#define LSM6DS3_OUTX_L_XL 0X28
#define LSM6DS3_OUTX_H_XL 0X29
#define LSM6DS3_OUTY_L_XL 0X2A
#define LSM6DS3_OUTY_H_XL 0X2B
#define LSM6DS3_OUTZ_L_XL 0X2C
#define LSM6DS3_OUTZ_H_XL 0X2D

class LSM6DS3_gax {
public:
  LSM6DS3_gax(SPI_HandleTypeDef *spi, GPIO_TypeDef *gpio) { _spi = spi; _gpio = gpio; }
  ~LSM6DS3_gax(void) {}
  uint8_t setPins(uint16_t cs, uint16_t irq);
  uint8_t setPins(uint16_t cs) { return setPins(cs, 0); }
  uint8_t init(void);

  int8_t isInited(void) { return _inited; }
  int8_t getInitCode(void) { return _initcode; }
public:
  void readXYZ(float &x, float &y, float &z);
  uint8_t availableXYZ(void);
  void readGyro(float &x, float &y, float &z);
  uint8_t availableGyro(void);
private:
  inline void csHigh(void);
  inline void csLow(void);

  uint8_t readReg(uint8_t addr);
  void readRegs(uint8_t addr, uint8_t *buff, uint8_t count);
  void writeReg(uint8_t addr, uint8_t data);
private:
  SPI_HandleTypeDef *_spi;
  GPIO_TypeDef *_gpio;
  uint16_t _cs;
  uint16_t _irq;
private:
  int8_t _inited;
  int8_t _initcode;
};

#endif // LSM6DS3_H