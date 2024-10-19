#include "LED.h"

void LED_Init(void)
{
    RCC->AHB1ENR	|= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER	&=~(3U<<10);  	// Clear mode bits for PA5
    GPIOA->MODER	|= (1U<<10);   	// Set PA5 as output (01)
    GPIOA->OTYPER	&=~(1U<<5);     // Set PA5 as push-pull
    GPIOA->OSPEEDR	|= (3U<<10); 	// High speed for PA5
    GPIOA->PUPDR	&=~(3U<<10);  	// No pull-up/pull-down for PA5
}

void LED_On(void)
{
	GPIOA->ODR |= (1<<5);
}

void LED_Off(void)
{
	GPIOA->ODR &= ~(1<<5);
}

void LED_Toggle(void)
{
    GPIOA->ODR ^= (1U << 5);
}
