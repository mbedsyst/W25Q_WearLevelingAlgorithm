#include "SWAP_FS.h"

/*
Wear Leveling Algorithm

Read_EraseCount
* Fetch the first 64 blocks erase count array from Security Register 1
* Fetch the next 64 blocks erase count array from Security Register 2

Read_BlockMap
* Fetch 128 element array from Security Register 3

CheckEraseCount
* Return the erase count of that block

CheckLowestCount
* Find the index of the block with the lowest erase count
* Return index of block

WriteData()
* Check the block's erase count
* Find the block with the lowest erase count
* Write to the block with the lowest erase count
* Increment erase count in the array
* Update Block map array
* Update Erase Count array in Memory
* Update Block map array in Memory
*/

static void SFS_ReadEraseCount(uint32_t *eraseCountArr)
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

static void SFS_ReadBlockMap(uint8_t *blockMapArr)
{
	uint8_t tempBuffer[128];
	W25Q_ReadSecurityRegister(3, 0, tempBuffer, 128);
	for (int i = 0; i < 128; i++)
    	{
        	blockMapArr[i] = tempBuffer[i];
    	}
}

static uint32_t SFS_CheckEraseCount(uint32_t *eraseCountArr, uint8_t blockNumber)
{
	return eraseCountArr[blockNumber];
}

static uint8_t SFS_FindLowestEraseCount(uint32_t *eraseCountArr, uint8_t blockNumber)
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

static void SFS_IncrementEraseCount(uint32_t *eraseCountArr, uint8_t blockNumber)
{
	eraseCountArr[blockNumber] += 1;
}

static void SFS_LinkBlockMap(uint8_t *blockMap, uint8_t blockNumber, uint8_t lowestCountBlock)
{
	blockMap[blockNumber] = lowestCountBlock;
}

static void SFS_UpdateEraseCountInMemory(uint8_t blockNumber, uint32_t eraseCount)
{
	uint8_t countArr[4];

	uint8_t regNumber = (blockNumber < 64) ? 1 : 2;
	uint8_t regOffset = (blockNumber % 64) * 4;

	for(int i = 0; i < 4; i++)
	{
		countArr[i] = (uint8_t)(eraseCount >> (8*(3-i)));
	}

	W25Q_WriteSecurityRegister(regNumber, regOffset, countArr, 4);
}

static void SFS_UpdateBlockMapinMemory(uint8_t blockNumber, uint8_t position)
{
	uint8_t	regOffset = (blockNumber % 64) * 2;

	W25Q_WriteSecurityRegister(3, regOffset, &position, 1);
}


static void SFS_PrintHorizontalLine(void)
{
    printf("+");
    for (int i = 0; i < COLUMNS; i++)
    {
        printf("--------+");
    }
    printf("\n");
}


static void SFS_DisplayConsole(uint32_t *eraseCountArr, uint8_t *blockMap)
{
	// Set color to Yellow
    printf("\033[33m");
    printf("Erase Count Array:\n");

    // Print the grid with horizontal and vertical lines
    SFS_PrintHorizontalLine();
    for (int i = 0; i < ROWS; i++)
    {
        printf("|");
        for (int j = 0; j < COLUMNS; j++)
        {
            int index = i * COLUMNS + j;
            printf(" %6lu |", eraseCountArr[index]);
        }
        printf("\n");
        SFS_PrintHorizontalLine();
    }

    // Set color to white
    printf("\033[37m");
    printf("Block Map Array:\n");

    SFS_PrintHorizontalLine();
    for (int i = 0; i < ROWS; i++)
    {
        printf("|");
        for (int j = 0; j < COLUMNS; j++)
        {
            int index = i * COLUMNS + j;
            printf(" %6d |", blockMap[index]);
        }
        printf("\n");
        SFS_PrintHorizontalLine();
    }
    printf("\033[0m");
}

static void SFS_UpdateConsole(uint32_t *eraseCountArr, uint8_t *blockMap)
{
    // Move cursor up enough lines to overwrite both arrays (ROWS * 2 + headers + gridlines)
    for (int i = 0; i < (ROWS + 1) * 2 + 4; i++)
    {
        printf("\033[A");  // Move cursor up one line
    }
    // Re-display the arrays with updated values
    SFS_DisplayConsole(eraseCountArr, blockMap);
}

void SFS_InitFS(void)
{
	static int exec = 0;

	if(!exec)
	{
		uint8_t dummy_byte = 0xFF;
		W25Q_WriteSecurityRegister(1, 0, &dummy_byte, 256);
		W25Q_WriteSecurityRegister(2, 0, &dummy_byte, 256);
		W25Q_WriteSecurityRegister(3, 0, &dummy_byte, 256);
		printf("File-system Initialized for first time\n\r");
		exec = 1;
	}
	else
	{
		printf("File-system already Initialized\n\r");
	}
}

void SFS_ReadFS(uint32_t *eraseCountArr, uint8_t *blockMapArr)
{
	SFS_ReadEraseCount(eraseCountArr);
	SFS_ReadBlockMap(blockMapArr);
}

void SFS_WriteData(uint32_t *eraseCountArr, uint8_t *blockMap, uint8_t blockNumber, uint8_t *data, uint32_t len)
{
	uint32_t currentEraseCount = SFS_CheckEraseCount(eraseCountArr, blockNumber);
	uint8_t lowestCountBlock = SFS_FindLowestEraseCount(eraseCountArr, blockNumber);

	if(eraseCountArr[lowestCountBlock] < currentEraseCount)
	{
		lowestCountBlock = blockNumber;
	}

	uint16_t page = lowestCountBlock * 65536;

	W25Q_WriteData(page, 0, len, data);

	SFS_IncrementEraseCount(eraseCountArr, blockNumber);
	SFS_LinkBlockMap(blockMap, blockNumber, lowestCountBlock);

	SFS_UpdateEraseCountInMemory(blockNumber, eraseCountArr[lowestCountBlock]+1);
	SFS_UpdateBlockMapinMemory(blockNumber, lowestCountBlock);

	SFS_UpdateConsole(eraseCountArr, blockMap);
}
