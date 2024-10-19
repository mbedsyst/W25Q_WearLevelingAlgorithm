#include "SWAP_FS.h"

/*
Wear Leveling Algorithm

Read_EraseCount
* Fetch first 64 block's erase count array from Security Register 1
* Fetch next 64 block's erase count array from Security Register 2

Read_BlockMap
* Fetch 128 element array from Security Register 3

CheckEraseCount
* Return the erase count of that block

CheckLowestCount
* Find index of block with lowest erase count
* Return index of block

WriteData()
* Check block's erase count
* Find block with lowest erase count
* Write to the block with lowest erase count
* Increment erase count in array
* Update Block map array
* Update Erase Count array in Memory
* Update Block map array in Memory*/


void SFS_ReadEraseCount(uint32_t *eraseCountArr)
{
    uint8_t tempBuffer[256];

    W25Q_ReadSecurityRegister(1, 0, tempBuffer, 256);
    for (int i = 0; i < 256; i += 4)
    {
        eraseCountArr[i / 4] = (tempBuffer[i] << 24) |
        					   (tempBuffer[i + 1] << 16) |
							   (tempBuffer[i + 2] << 8) |
							   (tempBuffer[i + 3]);
    }

    W25Q_ReadSecurityRegister(2, 0, tempBuffer, 256);
    for (int i = 0; i < 256; i += 4)
    {
        eraseCountArr[(i / 4) + 64] = (tempBuffer[i] << 24) |
        							  (tempBuffer[i + 1] << 16) |
                                      (tempBuffer[i + 2] << 8) |
									  (tempBuffer[i + 3]);
    }
}

void SFS_ReadBlockMap(uint8_t *blockMapArr)
{
	uint8_t tempBuffer[256];

    W25Q_ReadSecurityRegister(3, 0, tempBuffer, 256);
    for (int i = 0; i < 256; i += 4)
    {
        blockMapArr[(i / 4) + 64] = (tempBuffer[i] << 24) |
        							(tempBuffer[i + 1] << 16) |
                                    (tempBuffer[i + 2] << 8) |
									(tempBuffer[i + 3]);
    }
}

uint32_t SFS_CheckEraseCount(uint32_t *eraseCountArr, uint8_t blockNumber)
{
	return eraseCountArr[blockNumber];
}

uint8_t SFS_FindLowestEraseCount(uint32_t *eraseCountArr, uint8_t blockNumber)
{
	int lowestCount = eraseCountArr[blockNumber];
	int lowestIndex = blockNumber;

	for(int i = 0; i < 128; i++)
	{
		if(eraseCountArr[i] < lowestCount)
		{
			lowestCount = eraseCountArr[i];
			lowestIndex = i;
		}
	}

	return lowestIndex;
}

void SFS_IncrementEraseCount(uint32_t *eraseCountArr, uint8_t blockNumber)
{
	eraseCountArr[blockNumber] += 1;
}

void SFS_LinkBlockMap(uint8_t *blockMap, uint8_t blockNumber, uint8_t lowestCountBlock)
{
	blockMap[blockNumber] = lowestCountBlock;
}

void SFS_UpdateEraseCountInMemory(uint8_t blockNumber, uint32_t eraseCount)
{
	uint8_t regNumber;
	uint8_t regOffset;
	uint8_t countArr[4];
	regNumber = (blockNumber < 64) ? 1 : 2;
	regOffset = (blockNumber % 64) * 4;
	for(i = 0; i < 4; i++)
	{
		countArr[i] = (uint8_t)(eraseCount >> (8*(3-i)));
	}
	W25Q_WriteSecurityRegister(regNumber, regOffset, countArr, 4);
}

void SFS_UpdateBlockMapinMemory(uint8_t blockNumber, uint8_t position)
{
	uint8_t regOffset;
	regOffset = (blockNumber % 64) * 2;
	W25Q_WriteSecurityRegister(3, regOffset, position, 1);
}

void SFS_WriteData(uint32_t *eraseCountArr, uint8_t *blockMap, uint8_t blockNumber, uint8_t *data, uint32_t len)
{
	uint32_t currentEraseCount = SFS_CheckEraseCount(eraseCountArr, blockNumber);
	uint8_t lowestCountBlock = SFS_FindLowestEraseCount(eraseCountArr, blockNumber);

	uint16_t page = blockNumber * 65536;
	uint8_t offset = 0;

	W25Q_WriteData(page, 0, len, data);

	SFS_IncrementEraseCount(eraseCountArr, blockNumber);
	SFS_LinkBlockMap(blockMap, blockNumber, lowestCountBlock);

	SFS_UpdateEraseCountInMemory(blockNumber, eraseCountArr[lowestCountBlock]+1);
	SFS_UpdateBlockMapinMemory(blockNumber, lowestCountBlock);
}
