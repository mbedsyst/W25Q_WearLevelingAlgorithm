#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include "stm32f4xx.h"

void UART2_Init(void);
void UART2_TxChar(char ch);
void UART2_TxString(char *str);
uint8_t UART2_RxChar(void);

#endif

