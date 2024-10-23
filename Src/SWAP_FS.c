#include "SWAP_FS.h"

/**
 * @brief Read Erase Count array from Security Register to retrieve
 * 		  the number of times each block in the storage device has
 * 		  been erased and stores the values in the provided array.
 * @param	eraseCountArr	Pointer to 32-bit integer array to store
 * 							the Erase Count values
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

/**
 * @brief Read Block Map array from Security Register to retrieve
 * 		  the Logical-to-Physical mapping of the memory blocks
 * @param blockMapArr	Pointer to 8-bit integer array to store
 * 						the Block Map values
 */
static void SFS_ReadBlockMap(uint8_t *blockMapArr)
{
	uint8_t tempBuffer[128];
	W25Q_ReadSecurityRegister(3, 0, tempBuffer, 128);
	for (int i = 0; i < 128; i++)
	{
		blockMapArr[i] = tempBuffer[i];
	}
}

/**
 * @brief Returns the Erase Count of the particular block
 * @param 	eraseCountArr 	Pointer to 32-bit Erase Count Array
 * @param 	blockNumber 	Memory Block Number
 * @return 	Erase count of the particular Block
 */
static uint32_t SFS_CheckEraseCount(uint32_t *eraseCountArr, uint8_t blockNumber)
{
	return eraseCountArr[blockNumber];
}

/**
 * @brief Find block with lowest erase count
 * @param	eraseCountArr 	Pointer to 32-bit Erase Count Array
 * @param	blockNumber 	Memory Block Number
 * @return 	Index of block with Lowest Erase Count
 */
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

/**
 * @brief Increment Erase Count in the working copy array
 * @param	eraseCountArr 	Pointer to 32-bit Erase Count Array
 * @param	blockNumber 	Memory Block Number
 */
static void SFS_IncrementEraseCount(uint32_t *eraseCountArr, uint8_t blockNumber)
{
	eraseCountArr[blockNumber] += 1;
}

/**
 * @brief Remap Physical block number to Logical Block Number
 * 		  in the working copy array
 * @param 	eraseCountArr 		Pointer to 32-bit Erase Count Array
 * @param 	blockNumber 		Memory Block Number
 * @param	lowestCountBlock	Block Number with lowest erase count
 */
static void SFS_LinkBlockMap(uint8_t *blockMap, uint8_t blockNumber, uint8_t lowestCountBlock)
{
	blockMap[blockNumber] = lowestCountBlock;
}

/**
 * @brief 	Updates Erase Count in Flash Memories Security Register
 * @param	blockNumber		Memory Block Number
 * @param	eraseCount		Erase Count of the block
 */
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

/**
 * @brief	Updates Block Map in Flash Memories Security Register
 * @param	blockNumber		Logical Memory Block Number
 * @param	position		Physical Memory Block Number

 */
static void SFS_UpdateBlockMapinMemory(uint8_t blockNumber, uint8_t position)
{
	uint8_t	regOffset = (blockNumber % 64) * 2;

	W25Q_WriteSecurityRegister(3, regOffset, &position, 1);
}

/**
 * @brief Print horizontal line to separate values in console
 */
static void SFS_PrintHorizontalLine(void)
{
    printf("+");
    for (int i = 0; i < COLUMNS; i++)
    {
        printf("--------+");
    }
    printf("\n");
}

/**
 * @brief 	Prints Erase Count and Block Map in Console in grid
 * @param	eraseCountArr	Pointer to Erase Count array
 * @param 	blockMap 		Pointer to Block Map array
 */
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

/**
 * @brief	Moves up cursor to Top of screen and Updates values
 * @param	eraseCountArr	Pointer to Erase Count array
 * @param 	blockMap 		Pointer to Block Map array
 */
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

/**
 * @brief 	Initialize the file system by erasing the Erase Count array
 * 			Block Map array in Security Register
 */
void SFS_InitFS(void)
{
	static int exec = 0;
	if(!exec)
	{
		W25Q_EraseSecurityRegister(1);
		W25Q_EraseSecurityRegister(2);
		W25Q_EraseSecurityRegister(3);
		printf("File-system Initialized for first time\n\r");
		exec = 1;
	}
	else
	{
		printf("File-system already Initialized\n\r");
	}
}

/**
 * @brief 	Reads the Erase Count and Block Map array from
 * 			Security Registers and creates a working copy
 * 			of Meta-data in the memory
 * @param	eraseCountArr	Pointer to Erase Count array
 * @param 	blockMap 		Pointer to Block Map array
 */
void SFS_ReadFS(uint32_t *eraseCountArr, uint8_t *blockMapArr)
{
	SFS_ReadEraseCount(eraseCountArr);
	SFS_ReadBlockMap(blockMapArr);
	SFS_UpdateConsole(eraseCountArr, blockMapArr);
}

/**
 * @brief 	Write application data to Flash Memory
 * @param 	eraseCountArr	Pointer to Erase Count Array
 * @param	blockMap		Pointer to Block Map Array
 * @param	blockNumber		Logical Memory block Number
 * @param	data			Pointer to application data
 * @param	len				Length of application data to be written
 */
void SFS_WriteData(uint32_t *eraseCountArr, uint8_t *blockMap, uint8_t blockNumber, uint8_t *data, uint32_t len)
{
	uint32_t currentEraseCount = SFS_CheckEraseCount(eraseCountArr, blockNumber);
	uint8_t lowestCountBlock = SFS_FindLowestEraseCount(eraseCountArr, blockNumber);

	if(eraseCountArr[lowestCountBlock] < currentEraseCount)
	{
		lowestCountBlock = blockNumber;
	}

	uint32_t page = lowestCountBlock * 65536;

	W25Q_WriteData(page, 0, len, data);

	SFS_IncrementEraseCount(eraseCountArr, blockNumber);
	SFS_LinkBlockMap(blockMap, blockNumber, lowestCountBlock);

	SFS_UpdateEraseCountInMemory(blockNumber, eraseCountArr[lowestCountBlock]+1);
	SFS_UpdateBlockMapinMemory(blockNumber, lowestCountBlock);

	SFS_UpdateConsole(eraseCountArr, blockMap);
}
