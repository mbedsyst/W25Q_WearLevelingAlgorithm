#ifndef W25QXX_H_
#define W25QXX_H_

#include "SPI.h"
#include "SYSTICK.h"

// Flash Memory Parameter macros
#define W25Q_ByteCount		8388608
#define W25Q_PageSize		256
#define W25Q_PageCount		32768
#define W25Q_SectorSize		4096
#define	W25Q_SectorCount	2048
#define W25Q_BlockSize		65536
#define W25Q_BlockCount		128

// Security Register Address macros
#define SECURITY_REG_1		0x001000
#define SECURITY_REG_2		0x002000
#define SECURITY_REG_3		0x003000

// Flash Memory Command macros
#define ENABLE_RESET		0x66
#define EXECUTE_RESET		0x99
#define READ_ID				0x9F
#define READ_UID			0x4B
#define NORMAL_READ			0x03
#define FAST_READ			0x0B
#define ENABLE_WRITE		0x06
#define DISABLE_WRITE		0x04
#define PAGE_WRITE			0x02
#define ERASE_SECTOR		0x20
#define ERASE_32KBLOCK		0x52
#define ERASE_64KBLOCK		0xD8
#define ERASE_CHIP			0x60
#define	POWER_DOWN			0xB9
#define POWER_UP			0xAB
#define READ_STATUS_R1		0x05
#define READ_STATUS_R2		0x35
#define WRITE_STATUS_REG	0x01
#define READ_SECURITY_REG	0x48
#define WRITE_SECURITY_REG	0x42
#define ERASE_SECURITY_REG	0x44

// Control Functions
void W25Q_Init(void);
void W25Q_PowerDown(void);
void W25Q_PowerUp(void);
uint32_t W25Q_ReadID(void);
uint32_t W25Q_ReadUID(void);

// Status Register Functions
uint8_t W25Q_ReadStatusRegister1(void);
uint8_t W25Q_ReadStatusRegister2(void);
void W25Q_WriteStatusRegister(uint8_t statusReg1, uint8_t statusReg2);

// Security Register Functions
void W25Q_WriteSecurityRegister(uint8_t securityReg, uint8_t offset, uint8_t *data, uint8_t len);
void W25Q_ReadSecurityRegister(uint8_t securityReg, uint8_t offset, uint8_t *data, uint8_t len);
void W25Q_EraseSecurityRegister(uint8_t securityReg);

// Read Functions
void W25Q_ReadData(uint32_t startPage, uint8_t offset, uint8_t *buffer, uint16_t length);
void W25Q_FastReadData(uint32_t startPage, uint8_t offset, uint8_t *buffer, uint16_t length);

// Write Functions
void W25Q_WriteData(uint32_t startPage, uint16_t offset, uint32_t size, uint8_t *data);

// Erase Functions
void W25Q_EraseSector(uint8_t blockNumber, uint8_t sectorNumber);
void W25Q_Erase32kBlock(uint8_t blockNumber, uint8_t half);
void W25Q_Erase64kBlock(uint8_t blockNumber);
void W25Q_EraseChip(void);

#endif
