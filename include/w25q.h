#ifndef W25Q_MC_H
#define W25Q_MC_H

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

#define W25_WRITE_DISABLE  0x04
#define W25_WRITE_ENABLE   0x06
#define W25_CHIP_ERASE     0xC7 //0x60
#define W25_SECTOR_ERASE   0x20
#define W25_BLOCK_ERASE    0xD8
#define W25_FAST_READ      0x0B
#define W25_PAGE_PROGRAMM  0x02
#define W25_GET_JEDEC_ID   0x9F
#define W25_READ_STATUS_1  0x05
#define W25_READ_STATUS_2  0x35
#define W25_READ_STATUS_3  0x15
#define W25_WRITE_STATUS_1 0x01
#define W25_WRITE_STATUS_2 0x31
#define W25_WRITE_STATUS_3 0x11
#define W25_READ_UNIQUE_ID 0x4B

#define W25_TYPE_W25Q32  0x4016
#define W25_TYPE_W25Q64  0x4017
#define W25_TYPE_W25Q128 0x4018

#define W25_ERR_WRONG_TYPEID -1
#define W25_ERR_UNSUPPORTED  -2

typedef enum {
  W25Q32 = 1,
  W25Q64,
  W25Q128,
  W25_WRONG_ID,
} W25_ID_e;

class W25Q_memory {
public:
  W25Q_memory(SPI_HandleTypeDef *spi, GPIO_TypeDef *port) : _spi(spi), _gpio(port), _cs(0) {}
  void setCS(GPIO_TypeDef *port, uint16_t cs) { _gpio = port, _cs = cs; }
  uint8_t init(uint32_t type, uint16_t cs);
  // Не забудьте выставить CS! (setCS(port, pin))
  uint8_t init(uint32_t type) { return init(type, _cs); }
  uint8_t available(void);
  void waitAvailable(void);
  uint32_t readID(void);

  int8_t getInitCode(void) { return _initcode; }
  int8_t isInited(void) { return _inited; }
  uint32_t getRealId(void) { return _realid; }
  uint32_t getTypeId(void) { return _typeid; }
  uint8_t getStatus(void) { return _status; }
public:
  uint8_t writeByte(uint8_t byte, uint32_t addr);
  uint8_t writePage(uint8_t *buffer, uint32_t pageAddr, uint32_t offset, uint32_t size);
  uint8_t writeSector(uint8_t *buffer, uint32_t sectorAddr, uint32_t offset, uint32_t size);

  uint8_t readByte(uint8_t *buffer, uint32_t byteAddr);
  uint8_t readBytes(uint8_t *buffer, uint32_t startAddr, uint32_t count);
  uint8_t readPage(uint8_t *buffer, uint32_t pageAddr, uint32_t offset, uint32_t count);

  //uint8_t eraseChip(void);
  uint8_t eraseBlock(uint16_t block);
  uint8_t eraseSector(uint16_t sector);

  uint8_t isPageClear(uint32_t pageAddr, uint32_t offset);
  uint8_t isSectorClear(uint32_t sectorAddr, uint32_t offset);
  uint8_t isBlockClear(uint32_t blockAddr, uint32_t offset);
private:
  inline void csHigh(void);
  inline void csLow(void);
  void enableWriting(void);
  void disableWriting(void);
  uint8_t transferSPI(uint8_t data);
private:
  SPI_HandleTypeDef *_spi;
  GPIO_TypeDef *_gpio;
private:
  uint16_t _cs;
  uint32_t _typeid {0x4016};
  uint32_t _realid {0};
private:
  W25_ID_e _eid;
  uint8_t _status;
  int8_t _inited = 0;
  int8_t _initcode = 0;
private:
  uint32_t _totalBlocks {0};
  uint32_t _totalSectors {0};
  uint32_t _totalPages {0};

  uint16_t _pageSize = 256;
  uint32_t _sectorSize = 0x1000;
  uint32_t _blockSize {0};

};

#endif // W25Q_MC_H