#include "nrf24l01.h"

inline void nRF_radio::ssHigh(void) {
  HAL_GPIO_WritePin(_port, _ss, GPIO_PIN_SET);
}

inline void nRF_radio::ssLow(void) {
  HAL_GPIO_WritePin(_port, _ss, GPIO_PIN_RESET);
}

inline void nRF_radio::ceHigh(void) {
  HAL_GPIO_WritePin(_port, _ce, GPIO_PIN_SET);
}

inline void nRF_radio::ceLow(void) {
  HAL_GPIO_WritePin(_port, _ce, GPIO_PIN_RESET);
}

void nRF_radio::writeReg(uint8_t reg, uint8_t data ) {
  ssHigh();
  HAL_SPI_Transmit(_spi, (uint8_t*)(reg | NRF_WRITE_MASK), 1, 100);
  HAL_SPI_Transmit(_spi, (uint8_t*)(data), 1, 100);
  ssLow();
}

void nRF_radio::writeReg(uint8_t reg, uint8_t* buf, uint8_t len) {
  ssHigh();
  HAL_SPI_Transmit(_spi, (uint8_t*)(reg | NRF_WRITE_MASK), 1, 100);
  HAL_SPI_Transmit(_spi, buf, len, 100);
  ssLow();
}

uint8_t nRF_radio::readReg(uint8_t reg) {
  uint8_t result[1];
  ssHigh();
  HAL_SPI_Transmit(_spi, (uint8_t*)(reg & NRF_READ_MASK), 1, 100);
  HAL_SPI_TransmitReceive(_spi, (uint8_t*)(0xFF), result, 1, 100);
  ssLow();
  return result[0];
}

void nRF_radio::flush_tx() {
  ssHigh();
  HAL_SPI_Transmit(_spi, (uint8_t*)(0xE1), 1, 100);
  ssLow();
}

void nRF_radio::send(const void* buf) {
  ceLow();
  const uint8_t* current = reinterpret_cast<const uint8_t*>(buf);
  flush_tx(); // clear previous transmission
  writeReg(0x07, 0b00110000); // clear previous transmission
  ssHigh();
  HAL_SPI_Transmit(_spi, (uint8_t*)(0xA0), 1, 100);
  for (uint8_t i = 0; i < 32; i++) HAL_SPI_Transmit(_spi, (uint8_t*)(*current++), 1, 100);
  ssLow();
  ceHigh();
}

void nRF_radio::set_address(uint64_t value) {
  writeReg(0x0A, reinterpret_cast<uint8_t*>(&value), 5);
  writeReg(0x10, reinterpret_cast<uint8_t*>(&value), 5);
}


uint8_t nRF_radio::setPins(uint16_t ss, uint16_t ce) {
  if (((ss > 0) && (ss < 13)) && ((ce > 0) && (ce < 13))) {
    _ce = ce;
    _ss = ss;
    return 1;
  } return 0;
}

uint8_t nRF_radio::init(uint16_t ss, uint16_t ce, uint8_t channel, uint64_t addr) {
  if (((ss > 0) && (ss < 13)) && ((ce > 0) && (ce < 13))) {
    _ce = ce;
    _ss = ss;
  } else return 2;
  writeReg(0x00, 0b00001110);
  writeReg(0x01, 0b00111111);
  writeReg(0x02, 0b00000000);
  writeReg(0x03, 0b00000011); // 5 byte adres wight
  writeReg(0x04, 0b11111111); // wait 4000us and repeat 15 times, if data will be not given
  writeReg(0x05, channel);       // frequency set
  writeReg(0x06, 0b00000110); // speed and power rate set
  writeReg(0x07, 0b01110000); // Clear status register
  writeReg(0x11, 0b00010100); // PACKET_SIZE pipe 0
  writeReg(0x12, 0b00010100); // PACKET_SIZE pipe 1
  writeReg(0x13, 0b00010100); // PACKET_SIZE pipe 2
  writeReg(0x14, 0b00010100); // PACKET_SIZE pipe 3
  writeReg(0x15, 0b00010100); // PACKET_SIZE pipe 4
  writeReg(0x16, 0b00010100); // PACKET_SIZE pipe 5
  writeReg(0x17, 0b00000000); // Clear status FIFO register
  writeReg(0x1C, 0b00000000);
  writeReg(0x1D, 0b00000011);
  set_address(addr);
  uint8_t cfg = readReg(0x00);
  return (cfg == 0b00001110) ? 1 : 0;
}