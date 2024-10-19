#include "UART.h"

#define UART_BAUDRATE	115200
#define SYS_FREQ		16000000
#define APB1_CLK		SYS_FREQ

void UART2_Write(int ch);

static uint16_t Compute_UART_Baud(uint32_t periph_clk, uint32_t baudrate)
{
	return ((periph_clk + (baudrate/2U))/baudrate);
}

static void UART2_SetBaudRate(uint32_t periph_clk, uint32_t baudrate)
{
	USART2->BRR = Compute_UART_Baud(periph_clk,baudrate);
}

void UART2_Init(void)
{
	/*Enable clock access to GPIOA*/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	/*Enable clock access to UART2*/
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	/*Set PA2 mode to alternate function mode */
	GPIOA->MODER &=~(1U<<4);
	GPIOA->MODER |=(1U<<5);
	/*Set PA2 alternate function type to UART_TX(AF07)*/
	GPIOA->AFR[0] |=(0x7<<8);
	/*Configure Baud Rate*/
	UART2_SetBaudRate(APB1_CLK,UART_BAUDRATE);
	/*Configure the Transfer directions*/
	USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE);
	/*Enable UART module*/
	USART2->CR1 |= USART_CR1_UE;
}

void UART2_TxChar(char ch)
{
	/*Wait for Transmit Data Register to be empty*/
	while(!(USART2->SR & USART_SR_TXE));
	/*Write to the Transmit Data Register*/
	USART2->DR = (ch &0xFF);
}

void UART2_TxString(char *str)
{
	while(*str)
	{
		UART2_TxChar(*str++);
	}
}

uint8_t UART2_RxChar(void)
{
	while(!(USART2->SR & (1<<5)));
	return USART2->DR;
}

