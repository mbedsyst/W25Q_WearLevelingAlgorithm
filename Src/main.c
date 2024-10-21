#include <stdint.h>
#include "SWAP_FS.h"

int main()
{
	uint32_t eraseCountArray[128];
	uint8_t blockMapArray[128];

	SFS_ReadFS(eraseCountArray, blockMapArray);

	while(1)
	{

	}
}
