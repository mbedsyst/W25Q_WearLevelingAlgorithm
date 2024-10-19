#include "W25Qxx.h"

static void W25Q_WriteEnable(void)
{
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(ENABLE_WRITE);
	SPI2_DeselectSlave();
	delay_ms(10);
}

static void W25Q_WriteDisable(void)
{
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(DISABLE_WRITE);
	SPI2_DeselectSlave();
	delay_ms(10);
}

static void W25Q_Reset(void)
{
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(ENABLE_RESET);
	SPI2_TransmitReceiveByte(EXECUTE_RESET);
	SPI2_DeselectSlave();
	delay_ms(100);
}

void W25Q_Init(void)
{
	SPI2_Init();
	W25Q_Reset();
}

void W25Q_PowerDown(void)
{
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(POWER_DOWN);
	SPI2_DeselectSlave();
}

void W25Q_PowerUp(void)
{
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(POWER_UP);
	SPI2_DeselectSlave();
}

uint32_t W25Q_ReadID(void)
{
    	uint8_t dummyByte = 0xFF;
    	uint8_t id[3];
    	SPI2_SelectSlave();
    	SPI2_TransmitReceiveByte(READ_ID);
    	SPI2_TransmitReceive_MultiByte(&dummyByte, id, 3);
    	SPI2_DeselectSlave();
    	return ((id[0] << 16) | (id[1] << 8) | (id[2]));
}

uint32_t W25Q_ReadUID(void)
{
	uint8_t dummyByte = 0xFF;
	uint8_t id[4];
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(READ_UID);
	SPI2_TransmitReceive_MultiByte(&dummyByte, id, 4);
	SPI2_DeselectSlave();
	return ((id[0] << 24) | (id[1] << 16) | (id[2] << 8) | (id[3]));
}

void W25Q_ReadData(uint32_t startPage, uint8_t offset, uint8_t *buffer, uint16_t length)
{
	uint32_t memAddress = (startPage * 256) + offset;

	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(NORMAL_READ);
	SPI2_TransmitReceiveByte((memAddress >> 16) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress >> 8) & 0xFF);
	SPI2_TransmitReceiveByte(memAddress & 0xFF);
	for (uint16_t i = 0; i < length; i++)
	{
		// Send dummy byte and receive data
		buffer[i] = SPI2_TransmitReceiveByte(0xFF);
	}
	SPI2_DeselectSlave();
}

void W25Q_FastReadData(uint32_t startPage, uint8_t offset, uint8_t *buffer, uint16_t length)
{
	uint32_t memAddress = (startPage * 256) + offset;

	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(FAST_READ);
	SPI2_TransmitReceiveByte((memAddress >> 16) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress >> 8) & 0xFF);
	SPI2_TransmitReceiveByte(memAddress & 0xFF);
	SPI2_TransmitReceiveByte(0x00);
	for (uint16_t i = 0; i < length; i++)
	{
		// Send dummy byte and receive data
		buffer[i] = SPI2_TransmitReceiveByte(0xFF);
	}
	SPI2_DeselectSlave();
}

static void W25Q_WritePage(uint32_t startPage, uint16_t offset, uint32_t size, uint8_t *data)
{
	uint32_t memAddress = (startPage * 256) + offset;
	W25Q_WriteEnable();
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(PAGE_WRITE);
	SPI2_TransmitReceiveByte((memAddress >> 16) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress >> 8) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress) & 0xFF);
	SPI2_TransmitReceive_MultiByte(data, NULL, size);
	SPI2_DeselectSlave();
	W25Q_WriteDisable();
	delay_ms(5);
}

void W25Q_WriteData(uint32_t startPage, uint16_t offset, uint32_t size, uint8_t *data)
{
    uint32_t bytesToWrite;
    uint32_t remainingBytes = size;
    uint32_t currentPage = startPage;
    uint16_t currentOffset = offset;
    uint8_t *currentData = data;

    // While there's data left to write
    while (remainingBytes > 0)
    {
        // Calculate how many bytes can be written to the current page
        bytesToWrite = 256 - currentOffset;
        if (remainingBytes < bytesToWrite)
        {
            bytesToWrite = remainingBytes;
        }

        // Call W25Q_WritePage() function
        W25Q_WritePage(currentPage, currentOffset, bytesToWrite, currentData);

        // Update the remaining data, current data pointer, and Page Number
        remainingBytes -= bytesToWrite;
        currentData += bytesToWrite;
        currentPage++;
        currentOffset = 0;
    }
}

void W25Q_EraseSector(uint8_t blockNumber, uint8_t sectorNumber)
{
	uint32_t memAddress = (blockNumber * 65536) + (sector * 4096);
	W25Q_WriteEnable();
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(ERASE_SECTOR);
	SPI2_TransmitReceiveByte((memAddress >> 16) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress >> 8) & 0xFF);
	SPI2_TransmitReceiveByte(memAddress & 0xFF);
	SPI2_DeselectSlave();
	delay_ms(500);
}

void W25Q_Erase32kBlock(uint8_t blockNumber, uint8_t half)
{
	uint32_t memAddress = (blockNumber * 65536) + (half * 32768);
	W25Q_WriteEnable();
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(ERASE_32KBLOCK);
	SPI2_TransmitReceiveByte((memAddress >> 16) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress >> 8) & 0xFF);
	SPI2_TransmitReceiveByte(memAddress & 0xFF);
	SPI2_DeselectSlave();
	delay_ms(1700);
}

void W25Q_Erase64kBlock(uint8_t blockNumber)
{
	uint32_t memAddress = (blockNumber * 65536);
	W25Q_WriteEnable();
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(ERASE_64KBLOCK);
	SPI2_TransmitReceiveByte((memAddress >> 16) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress >> 8) & 0xFF);
	SPI2_TransmitReceiveByte(memAddress & 0xFF);
	SPI2_DeselectSlave();
	delay_ms(2100);
}

void W25Q_EraseChip(void)
{
	W25Q_WriteEnable();
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(ERASE_CHIP);
	SPI2_DeselectSlave();
	delay_ms(100100);
}

uint8_t W25Q_ReadStatusRegister1(void)
{
	uint8_t statusReg;
	W25Q_WriteEnable();
	SPI2_SelectSlave();
	statusReg = SPI2_TransmitReceiveByte(READ_STATUS_R1);
	SPI2_DeselectSlave();
	return statusReg;
}

uint8_t W25Q_ReadStatusRegister2(void)
{
	uint8_t statusReg;
	W25Q_WriteEnable();
	SPI2_SelectSlave();
	statusReg = SPI2_TransmitReceiveByte(READ_STATUS_R2);
	SPI2_DeselectSlave();
	return statusReg;
}

void W25Q_WriteStatusRegister(uint8_t statusReg1, uint8_t statusReg2)
{
	W25Q_WriteEnable();
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(WRITE_STATUS_REG);
	SPI2_TransmitReceiveByte(statusReg1);
	SPI2_TransmitReceiveByte(statusReg2);
	SPI2_DeselectSlave();
}
void W25Q_WriteSecurityRegister(uint8_t reg, uint8_t offset, uint8_t *data, uint8_t len)
{
	uint32_t memAddress;

	switch(reg)
	{
		case '1':	memAddress = SECURITY_REG_1; break;
		case '2':	memAddress = SECURITY_REG_2; break;
		case '3':	memAddress = SECURITY_REG_3; break;
		default : 	break;
	}

	memAddress = memAddress + offset;

	W25Q_WriteEnable();
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(WRITE_SECURITY_REG);
	SPI2_TransmitReceiveByte((memAddress >> 16) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress >> 8) & 0xFF);
	SPI2_TransmitReceiveByte(memAddress & 0xFF);
	for (uint8_t i = 0; i < len; i++)
	{
		// Send data and discard dummy data
		SPI2_TransmitReceiveByte(data[i]);
	}
	SPI2_DeselectSlave();
}

void W25Q_ReadSecurityRegister(uint8_t reg, uint8_t offset, uint8_t *data, uint8_t len)
{
	uint32_t memAddress;

	switch(reg)
	{
		case '1':	memAddress = SECURITY_REG_1; break;
		case '2':	memAddress = SECURITY_REG_2; break;
		case '3':	memAddress = SECURITY_REG_3; break;
		default : 	break;
	}

	memAddress = memAddress + offset;

	W25Q_WriteEnable();
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(READ_SECURITY_REG);
	SPI2_TransmitReceiveByte((memAddress >> 16) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress >> 8) & 0xFF);
	SPI2_TransmitReceiveByte(memAddress & 0xFF);
	for (uint8_t i = 0; i < len; i++)
	{
		// Send dummy byte and receive data
		data[i] = SPI2_TransmitReceiveByte(0xFF);
	}
	SPI2_DeselectSlave();
}

void W25Q_EraseSecurityRegister(uint8_t reg)
{
	uint32_t memAddress;

	switch(reg)
	{
		case '1':	memAddress = SECURITY_REG_1; break;
		case '2':	memAddress = SECURITY_REG_2; break;
		case '3':	memAddress = SECURITY_REG_3; break;
		default : 	break;
	}

	W25Q_WriteEnable();
	SPI2_SelectSlave();
	SPI2_TransmitReceiveByte(ERASE_SECURITY_REG);
	SPI2_TransmitReceiveByte((memAddress >> 16) & 0xFF);
	SPI2_TransmitReceiveByte((memAddress >> 8) & 0xFF);
	SPI2_TransmitReceiveByte(memAddress & 0xFF);
	SPI2_DeselectSlave();
}
