#ifndef NRF_RADIO_H
#define NRF_RADIO_H

#include "stm32f4xx_hal.h"

#define NRF_READ_MASK 0x1F
#define NRF_WRITE_MASK 0x20

class nRF_radio {
public:
  nRF_radio(SPI_HandleTypeDef *spi, GPIO_TypeDef *port) : _spi(spi), _port(port), _inited(0), _ss(0), _ce(0) {}
  uint8_t setPins(uint16_t ss, uint16_t ce);
  uint8_t init(uint16_t ss, uint16_t ce, uint8_t channel, uint64_t addr);
  uint8_t init(uint8_t channel, uint64_t addr) { return init(_ss, _ce, channel, addr); }
  void writeReg(uint8_t reg, uint8_t data );
  void writeReg(uint8_t reg, uint8_t* buf, uint8_t len);
  uint8_t readReg(uint8_t reg);
  void flush_tx();
  void set_address(uint64_t c);
  void send(const void* buf);
private:
  inline void ssHigh(void);
  inline void ssLow(void);
  inline void ceHigh(void);
  inline void ceLow(void);
private:
  uint8_t _inited;
  SPI_HandleTypeDef *_spi;
  GPIO_TypeDef *_port;
  uint16_t _ss, _ce;
};

#endif // NRF_RADIO_H