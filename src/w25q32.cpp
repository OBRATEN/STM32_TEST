#include "w25q32.h"

uint8_t W25Q32_memory::transferSPI(uint8_t data) {
  uint8_t result;
  HAL_SPI_TransmitReceive(_spi, &data, &result, 1, 100);
  return result;
}

inline void W25Q32_memory::csHigh(void) {
  HAL_GPIO_WritePin(_gpio, _cs, GPIO_PIN_SET);
}

inline void W25Q32_memory::csLow(void) {
  HAL_GPIO_WritePin(_gpio, _cs, GPIO_PIN_RESET);
}

uint32_t W25Q32_memory::readID(void) {
  uint32_t t1, t2, t3;
  csLow();
  transferSPI(W25Q32_GET_JEDEC_ID);
  t1 = transferSPI(0xA5);
  t2 = transferSPI(0xA5);
  t3 = transferSPI(0xA5);
  csHigh();
  _id = (t1 << 16) | (t2 << 8) | t3;
  return _id;
}

void W25Q32_memory::enableWriting(void) {
  csLow();
  transferSPI(W25Q32_WRITE_ENABLE);
  csHigh();
  HAL_Delay(1);
}

void W25Q32_memory::disableWriting(void) {
  csLow();
  transferSPI(W25Q32_WRITE_DISABLE);
  csHigh();
  HAL_Delay(1);
}

// 0 - not available, 1 - available
uint8_t W25Q32_memory::available(void) {
  uint8_t result;
	csLow();
	transferSPI(W25Q32_READ_STATUS_1);
  _status = transferSPI(0xA5);
  if ((_status & 0x01) != 0x01) result = 1;
  else result = 0;
  csHigh();
  return result;
}

void W25Q32_memory::waitAvailable(void) {
  HAL_Delay(1);
  csLow();
  transferSPI(W25Q32_READ_STATUS_1);
  _status = transferSPI(0xA5);
  do {
    _status = transferSPI(0xA5);
    HAL_Delay(1);
  } while ((_status & 0x01) == 0x01);
  csHigh();
}

// 0 - error, 1 - success, 2 - maybe another chip?
uint8_t W25Q32_memory::init(uint16_t cs) {
  _cs = cs;
  while(HAL_GetTick() < 100);
  HAL_Delay(1);
  csHigh();
  readID();
  if ((_id & 0x0000FFFF) != 0x4016) return 2;
  return 1;
}

uint8_t W25Q32_memory::eraseSector(uint16_t sector) {
  if (!available()) return 0;
  uint32_t addr = sector * _sectorSize;
  enableWriting();
  csLow();
  transferSPI(W25Q32_SECTOR_ERASE);
  transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
  csHigh();
  waitAvailable();
  HAL_Delay(1);
  return 1;
}

uint8_t W25Q32_memory::eraseBlock(uint16_t block) {
  if (!available()) return 0;
  int32_t addr = block * _sectorSize * 16;
  enableWriting();
  csLow();
  transferSPI(W25Q32_BLOCK_ERASE);
  transferSPI((addr & 0xFF0000) >> 16);
  transferSPI((addr & 0xFF00) >> 8);
  transferSPI(addr & 0xFF);
  csHigh();
  waitAvailable();
  HAL_Delay(1);
  return 1;
}

uint8_t W25Q32_memory::isPageClear(uint32_t addr, uint32_t offset) {
	HAL_Delay(1);
	uint8_t	buff[256] = {0,};
	uint32_t startAddr = 0;
	uint16_t size = _pageSize - offset;
	startAddr = (offset + addr * _pageSize);
	csLow();
	transferSPI(W25Q32_FAST_READ);
	transferSPI((startAddr & 0xFF0000) >> 16);
  transferSPI((startAddr & 0xFF00) >> 8);
  transferSPI(startAddr & 0xFF);
	transferSPI(0);
	HAL_SPI_Receive(_spi, buff, size, 100);
	csHigh();
	for(uint16_t i = 0; i < size; i++) if (buff[i] != 0xFF) return 0;
	return 1;
}

uint8_t W25Q32_memory::isSectorClear(uint32_t addr, uint32_t offset) {
	HAL_Delay(1);
	uint8_t	buff[256] = {0,};
	uint32_t startAddr = 0;
	uint16_t buffSize = 256;
	uint16_t size = _sectorSize - offset;
	startAddr = (offset + addr * _sectorSize);
	uint16_t t0 = size / 256;
	uint16_t t1 = size % 256;
	uint16_t ct = 0;
	if (size <= 256) ct = 1;
	else if (t1 == 0) ct = t0;
	else ct = t0 + 1;
	for(uint16_t i = 0; i < ct; i++) {
		csLow();
		transferSPI(W25Q32_FAST_READ);
		transferSPI((startAddr & 0xFF0000) >> 16);
    transferSPI((startAddr & 0xFF00) >> 8);
    transferSPI(startAddr & 0xFF);
		transferSPI(0);
		if (size < 256) buffSize = size;
		HAL_SPI_Receive(_spi, buff, buffSize, 100);
		csHigh();
		for(uint16_t i = 0; i < buffSize; i++) if (buff[i] != 0xFF) return 0;
		size -= 256;
		startAddr = startAddr + 256;
	} return 1;
}

uint8_t W25Q32_memory::writeByte(uint8_t byte, uint32_t addr) {
  if (!available()) return 0;
	HAL_Delay(1);
	enableWriting();
	csLow();
	transferSPI(W25Q32_PAGE_PROGRAMM);
	transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
	transferSPI(byte);
	csHigh();
	waitAvailable();
  return 1;
}

uint8_t W25Q32_memory::writePage(uint8_t *buff, uint32_t pageAddr, uint32_t offset, uint32_t size) {
	if (!available()) return 0;
  if (size <= 0 || size > 256 || offset >= 255) return 2;
  if (offset + size > _pageSize) return 2;
  pageAddr = (pageAddr * _pageSize) + offset;
	enableWriting();
	csLow();
	transferSPI(W25Q32_PAGE_PROGRAMM);
	transferSPI((pageAddr & 0xFF0000) >> 16);
	transferSPI((pageAddr & 0xFF00) >> 8);
	transferSPI(pageAddr & 0xFF);
	HAL_SPI_Transmit(_spi, buff, size, 100);
	csHigh();
	waitAvailable();
	HAL_Delay(1);
	return 1;
}

uint8_t W25Q32_memory::readByte(uint8_t *buff, uint32_t addr) {
	if (!available()) return 0;
	csLow();
	transferSPI(W25Q32_FAST_READ);
	transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
	transferSPI(0);
	*buff = transferSPI(0xA5);
	csHigh();
  waitAvailable();
  return 1;
}

uint8_t W25Q32_memory::readBytes(uint8_t *buff, uint32_t addr, uint32_t len) {
	if (!available()) return 0;
	csLow();
	transferSPI(W25Q32_FAST_READ);
	transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
	transferSPI(0);
	HAL_SPI_Receive(_spi, buff, len, 2000);
	csHigh();
  waitAvailable();
	return 1;
}

uint8_t W25Q32_memory::readPage(uint8_t *buff, uint32_t addr, uint32_t offset, uint32_t len) {
	if (!available()) return 0;
  if (offset >= _pageSize || len > _pageSize || len <= 0) return 2;
  if (offset + len > _pageSize) return 2;
	addr = addr * _pageSize + offset;
	csLow();
	transferSPI(W25Q32_FAST_READ);
	transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
	transferSPI(0);
	HAL_SPI_Receive(_spi, buff, len, 100);
	csHigh();
	waitAvailable();
	return 1;
}