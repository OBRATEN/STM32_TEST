#include "w25q.h"

uint8_t W25Q_memory::transferSPI(uint8_t data) {
  uint8_t result;
  HAL_SPI_TransmitReceive(_spi, &data, &result, 1, 100);
  return result;
}

inline void W25Q_memory::csHigh(void) {
  HAL_GPIO_WritePin(_gpio, _cs, GPIO_PIN_SET);
}

inline void W25Q_memory::csLow(void) {
  HAL_GPIO_WritePin(_gpio, _cs, GPIO_PIN_RESET);
}

uint32_t W25Q_memory::readID(void) {
  uint32_t t1, t2, t3;
  csLow();
  transferSPI(W25_GET_JEDEC_ID);
  t1 = transferSPI(0xA5);
  t2 = transferSPI(0xA5);
  t3 = transferSPI(0xA5);
  csHigh();
  _realid = (t1 << 16) | (t2 << 8) | t3;
  return _realid;
}

void W25Q_memory::enableWriting(void) {
  csLow();
  transferSPI(W25_WRITE_ENABLE);
  csHigh();
  HAL_Delay(1);
}

void W25Q_memory::disableWriting(void) {
  csLow();
  transferSPI(W25_WRITE_DISABLE);
  csHigh();
  HAL_Delay(1);
}

// 0 - not available, 1 - available
uint8_t W25Q_memory::available(void) {
  uint8_t result;
	csLow();
	transferSPI(W25_READ_STATUS_1);
  _status = transferSPI(0xA5);
  if ((_status & 0x01) != 0x01) result = 1;
  else result = 0;
  csHigh();
  return result;
}

void W25Q_memory::waitAvailable(void) {
  HAL_Delay(1);
  csLow();
  transferSPI(W25_READ_STATUS_1);
  _status = transferSPI(0xA5);
  do {
    _status = transferSPI(0xA5);
    HAL_Delay(1);
  } while ((_status & 0x01) == 0x01);
  csHigh();
}

// 0 - error, 1 - success, 2 - wrong chip id, 3 - maybe unsupported chip?
uint8_t W25Q_memory::init(uint32_t type, uint16_t cs) {
  _typeid = type;
  _cs = cs;
	uint8_t result;
  while(HAL_GetTick() < 100);
  HAL_Delay(1);
  csHigh();
  readID();
	if (_typeid != 0x4016 && _typeid != 0x4017 && _typeid != 0x4018) {
		_eid = W25_WRONG_ID;
		_inited = 0;
		_initcode = W25_ERR_WRONG_TYPEID;
		result = 0;
	} else if (((_realid & 0x0000FFFF) == 0x4016) && (_typeid == 0x4016)) {
		_eid = W25Q32;
		_inited = 1;
		_initcode = 1;
		_totalBlocks = 64;
		result = 1;
	} else if (((_realid & 0x0000FFFF) == 0x4017) && (_typeid == 0x4017)) {
		_eid = W25Q64;
		_inited = 1;
		_initcode = 1;
		_totalBlocks = 128;
		result = 1;
	} else if (((_realid & 0x0000FFFF) == 0x4018) && (_typeid == 0x4018)) {
		_eid = W25Q128;
		_inited = 1;
		_initcode = 1;
		_totalBlocks = 256;
		result = 1;
	} else {
		_eid = W25_WRONG_ID;
		_inited = 0;
		_initcode = W25_ERR_UNSUPPORTED;
		result = 0;
	}
	if (!result) return 0;
	_totalSectors = _totalBlocks * 16;
	_totalPages = (_totalSectors * _sectorSize) / _pageSize;
	_blockSize = _sectorSize * 16;
	return 1; 
}

uint8_t W25Q_memory::eraseSector(uint16_t sector) {
  if (!available()) return 0;
  uint32_t addr = sector * _sectorSize;
  enableWriting();
  csLow();
  transferSPI(W25_SECTOR_ERASE);
  transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
  csHigh();
  waitAvailable();
  HAL_Delay(1);
  return 1;
}

uint8_t W25Q_memory::eraseBlock(uint16_t block) {
  if (!available()) return 0;
  int32_t addr = block * _sectorSize * 16;
  enableWriting();
  csLow();
  transferSPI(W25_BLOCK_ERASE);
  transferSPI((addr & 0xFF0000) >> 16);
  transferSPI((addr & 0xFF00) >> 8);
  transferSPI(addr & 0xFF);
  csHigh();
  waitAvailable();
  HAL_Delay(1);
  return 1;
}

uint8_t W25Q_memory::isPageClear(uint32_t addr, uint32_t offset) {
	HAL_Delay(1);
	uint8_t	buff[256] = {0,};
	uint32_t startAddr = 0;
	uint16_t size = _pageSize - offset;
	startAddr = (offset + addr * _pageSize);
	csLow();
	transferSPI(W25_FAST_READ);
	transferSPI((startAddr & 0xFF0000) >> 16);
  transferSPI((startAddr & 0xFF00) >> 8);
  transferSPI(startAddr & 0xFF);
	transferSPI(0);
	HAL_SPI_Receive(_spi, buff, size, 100);
	csHigh();
	for(uint16_t i = 0; i < size; i++) if (buff[i] != 0xFF) return 0;
	return 1;
}

uint8_t W25Q_memory::isSectorClear(uint32_t addr, uint32_t offset) {
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
		transferSPI(W25_FAST_READ);
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

uint8_t W25Q_memory::writeByte(uint8_t byte, uint32_t addr) {
  if (!available()) return 0;
	HAL_Delay(1);
	enableWriting();
	csLow();
	transferSPI(W25_PAGE_PROGRAMM);
	transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
	transferSPI(byte);
	csHigh();
	waitAvailable();
  return 1;
}

uint8_t W25Q_memory::writePage(uint8_t *buff, uint32_t pageAddr, uint32_t offset, uint32_t size) {
	if (!available()) return 0;
  if (size <= 0 || size > _pageSize || offset >= _pageSize) return 2;
  if (offset + size > _pageSize) return 2;
  pageAddr = (pageAddr * _pageSize) + offset;
	enableWriting();
	csLow();
	transferSPI(W25_PAGE_PROGRAMM);
	transferSPI((pageAddr & 0xFF0000) >> 16);
	transferSPI((pageAddr & 0xFF00) >> 8);
	transferSPI(pageAddr & 0xFF);
	HAL_SPI_Transmit(_spi, buff, size, 100);
	csHigh();
	waitAvailable();
	return 1;
}

uint8_t W25Q_memory::writeSector(uint8_t *buff, uint32_t sectorAddr, uint32_t offset, uint32_t size) {
	if (!available()) return 0;
  if (size <= 0 || size > _sectorSize || offset >= _sectorSize) return 2;
  if (offset + size > _sectorSize) return 2;
	uint32_t startPage = (sectorAddr * _sectorSize / _blockSize) + (offset / _pageSize);
	uint32_t pageOffset = offset % _pageSize;
	do {
		writePage(buff, startPage, pageOffset, size);
		startPage++;
		size -= _pageSize - pageOffset;
		buff += _pageSize - pageOffset;
		pageOffset = 0;
	} while(size > 0);
	return 1;
}

uint8_t W25Q_memory::readByte(uint8_t *buff, uint32_t addr) {
	if (!available()) return 0;
	csLow();
	transferSPI(W25_FAST_READ);
	transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
	transferSPI(0);
	*buff = transferSPI(0xA5);
	csHigh();
  waitAvailable();
  return 1;
}

uint8_t W25Q_memory::readBytes(uint8_t *buff, uint32_t addr, uint32_t len) {
	if (!available()) return 0;
	csLow();
	transferSPI(W25_FAST_READ);
	transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
	transferSPI(0);
	HAL_SPI_Receive(_spi, buff, len, 2000);
	csHigh();
  waitAvailable();
	return 1;
}

uint8_t W25Q_memory::readPage(uint8_t *buff, uint32_t addr, uint32_t offset, uint32_t len) {
	if (!available()) return 0;
  if (offset >= _pageSize || len > _pageSize || len <= 0) return 2;
  if (offset + len > _pageSize) return 2;
	addr = addr * _pageSize + offset;
	csLow();
	transferSPI(W25_FAST_READ);
	transferSPI((addr & 0xFF0000) >> 16);
	transferSPI((addr & 0xFF00) >> 8);
	transferSPI(addr & 0xFF);
	transferSPI(0);
	HAL_SPI_Receive(_spi, buff, len, 100);
	csHigh();
	waitAvailable();
	return 1;
}