#ifndef SWAP_FS_H_
#define SWAP_FS_H_

#include <stdio.h>
#include "W25Qxx.h"

void SFS_Read_EraseCount(uint32_t *eraseCountArr);
void SFS_Read_BlockMap(uint8_t *blockMapArr);
void SFS_WriteData(uint32_t *eraseCountArr, uint8_t *blockMap, uint8_t blockNumber, uint8_t *data, uint32_t len);

#endif
