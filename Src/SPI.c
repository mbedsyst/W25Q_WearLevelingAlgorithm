#include "SPI.h"

/* SPI2 Pin Mapping
 *	SPI2_MOSI - PB15
 *	SPI2_MISO - PB14
 *	SPI2_SCK  - PB13
 *	SPI2_NSS  - PB12
 */

void SPI2_Init(void)
{
	// Enable clock for GPIO Port B
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	// Enable clock SPI2 peripheral
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

	// Configure PB13, PB14, PB15 as Alternate Function for SPI2
	GPIOB->MODER &= ~(GPIO_MODER_MODE12 | GPIO_MODER_MODE13 | GPIO_MODER_MODE14 | GPIO_MODER_MODE15);
	GPIOB->MODER |= (GPIO_MODER_MODE12_0 | GPIO_MODER_MODE13_1 | GPIO_MODER_MODE14_1 | GPIO_MODER_MODE15_1);
	GPIOB->AFR[1] |= ((5 << 20) | (5 << 24) | (5 << 28));

	// Pull CS High
	SPI2_DeselectSlave();

	// Configure SPI2 in master mode, baud rate = fPCLK / 16, CPOL = 0, CPHA = 0, 8-bit data format
	SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_1 | SPI_CR1_SSI | SPI_CR1_SSM;
	// Enable SPI2
	SPI2->CR1 |= SPI_CR1_SPE;
}

void SPI2_SelectSlave(void)
{
	// Pull CS low
	GPIOB->BSRR = (1 << 28);
}

void SPI2_DeselectSlave(void)
{
    	// Pull CS high
    	GPIOB->BSRR = (1 << 12);
}

uint8_t SPI2_TransmitReceiveByte(uint8_t data)
{
    	// Wait until TXE (Transmit buffer empty)
    	while(!(SPI2->SR & SPI_SR_TXE));
    	// Send data
    	SPI2->DR = data;
    	// Wait until RXNE (Receive buffer not empty)
    	while(!(SPI2->SR & SPI_SR_RXNE));
    	// Return received data
    	return (uint8_t)(SPI2->DR);
}

void SPI2_TransmitReceive_MultiByte(uint8_t *txData, uint8_t *rxData, uint16_t size)
{
    	uint16_t i = 0;
    	while (i < size)
    	{
        	// Wait until TXE (Transmit buffer empty)
        	while (!(SPI2->SR & SPI_SR_TXE));
        	// Transmit data
        	SPI2->DR = txData[i];
        	// Wait until RXNE (Receive buffer not empty)
       		while (!(SPI2->SR & SPI_SR_RXNE));
        	// Read received data, even if rxData is NULL (to clear RXNE flag)
        	uint8_t receivedByte = (uint8_t)(SPI2->DR);
        	// Store received data only if rxData is not NULL
        	if (rxData != NULL)
        	{
            		rxData[i] = receivedByte;
        	}
        	i++;
    	}
}
