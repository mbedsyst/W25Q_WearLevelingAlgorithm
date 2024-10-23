#include <stdint.h>
#include "UART.h"
#include "LED.h"
#include "SWAP_FS.h"

int main()
{
	uint32_t eraseCountArray[128];
	uint8_t blockMapArray[128];
	uint8_t data[4096] = "";

	LED_Init();
	UART2_Init();

	SFS_InitFS();
	SFS_ReadFS(eraseCountArray, blockMapArray);

	while(1)
	{
		SFS_WriteData(eraseCountArray, blockMapArray, 5, data, 4096);
	}
}
