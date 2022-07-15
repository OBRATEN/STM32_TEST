#ifndef W25Q32_MC_H
#define W25Q32_MC_H

#include <stm32f4xx_hal.h>

/* Библиотека работы с SPI чипом FLASH памяти W25Q32
   Автор: Гарагуля Артур, г. Курск, "SPORADIC"
   Целевой микроконтроллер: STM32F411CEU6
 */

/* W25Q32 представляет собой чип памяти с интерфейсом SPI.
   Память в чипе разделена на блоки по 64Кб, каждый из которых
   состоит из секторов по 4Кб, состоящие из страниц по 256 байт.
   Состояние отсутствия данных в ячейке равно 256 (0xFF), а не 
   0 (0x00), как это работает в картах SD/MMC.
 */

#define W25Q32_WRITE_DISABLE  0x04
#define W25Q32_WRITE_ENABLE   0x06
#define W25Q32_CHIP_ERASE     0xC7 //0x60
#define W25Q32_SECTOR_ERASE   0x20
#define W25Q32_BLOCK_ERASE    0xD8
#define W25Q32_FAST_READ      0x0B
#define W25Q32_PAGE_PROGRAMM  0x02
#define W25Q32_GET_JEDEC_ID   0x9F
#define W25Q32_READ_STATUS_1  0x05
#define W25Q32_READ_STATUS_2  0x35
#define W25Q32_READ_STATUS_3  0x15
#define W25Q32_WRITE_STATUS_1 0x01
#define W25Q32_WRITE_STATUS_2 0x31
#define W25Q32_WRITE_STATUS_3 0x11
#define W25Q32_READ_UNIQUE_ID 0x4B

class W25Q32_memory {
public:
  W25Q32_memory(SPI_HandleTypeDef *spi, GPIO_TypeDef *port) : _spi(spi), _gpio(port), _cs(0),
  _id(0), _status(0), _totalBlocks(64), _blockSize(_sectorSize * 16),
  _pageSize(256), _pageCount(_sectorCount * _sectorSize / _pageSize),
  _sectorSize(4096), _sectorCount(_totalBlocks * 16) {}
  void setCS(GPIO_TypeDef *port, uint16_t cs) { _gpio = port, _cs = cs; }
  uint8_t init(uint16_t cs);
  // Не забудьте выставить CS! (setCS(port, pin))
  uint8_t init(void) { return init(_cs); }

  uint8_t writeByte(uint8_t byte, uint32_t addr);
  uint8_t writePage(uint8_t *page_buffer, uint32_t pageAddr, uint32_t offset, uint32_t size);

  uint8_t readByte(uint32_t addr);
  uint8_t readByte(uint8_t *buffer, uint32_t byteAddr);
  uint8_t readBytes(uint8_t *buffer, uint32_t startAddr, uint32_t count);
  uint8_t readPage(uint8_t *buffer, uint32_t pageAddr, uint32_t offset, uint32_t count);

  uint8_t eraseChip(void);
  uint8_t eraseBlock(uint16_t block);
  uint8_t eraseSector(uint16_t sector);

  uint8_t isPageClear(uint32_t pageAddr, uint32_t offset);
  uint8_t isSectorClear(uint32_t sectorAddr, uint32_t offset);
  uint8_t isBlockClear(uint32_t blockAddr, uint32_t offset);

  uint8_t available(void);
  void waitAvailable(void);
  uint32_t readID(void);
private:
  inline void csHigh(void);
  inline void csLow(void);
  void enableWriting(void);
  void disableWriting(void);
  uint8_t transferSPI(uint8_t data);
private:
  SPI_HandleTypeDef *_spi;
  GPIO_TypeDef *_gpio;
  uint16_t _cs;

  uint32_t _id;
  uint8_t _status;
  uint8_t _totalBlocks;
  uint32_t _blockSize;
  uint16_t _pageSize;
  uint32_t _pageCount;
  uint32_t _sectorSize;
  uint16_t _sectorCount;
};

#endif // W25Q32_MC_H