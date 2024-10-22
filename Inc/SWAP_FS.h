#ifndef SWAP_FS_H_
#define SWAP_FS_H_

#include <stdio.h>
#include "W25Qxx.h"

#define TOTAL_BLOCKS 	128
#define ROWS 			16
#define COLUMNS 		8

void SFS_InitFS(void);
void SFS_ReadFS(uint32_t *eraseCountArr, uint8_t *blockMapArr);
void SFS_WriteData(uint32_t *eraseCountArr, uint8_t *blockMap, uint8_t blockNumber, uint8_t *data, uint32_t len);

#endif
