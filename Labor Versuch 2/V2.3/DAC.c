/*----------------------------------------------------------------------------
  Funktionen für Digital-Analog-Konverter
 *----------------------------------------------------------------------------*/

#include <stm32f4xx.h>

void InitDAC(void)
// Initialisierung des DAC-Konverters
{
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;				// Peripherietakt für DAC einschalten
	DAC->CR |= DAC_CR_EN1;										// DAC enable
	GPIOA->MODER  &= 0xFFFFF0FF;							// GPIOA4 auf alternate function schalten
	GPIOA->MODER  |= 0x00000300;							
	GPIOA->AFR[0] &= 0xFFF0FFFF;						// GPIOA2/3 AF auf USART schalten (s. Tab 11 Datenblatt)
	GPIOA->AFR[0] |= 0x00040000;
}

void InitTimer2(void)
{
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;		// Takt für TIM6 einschalten
	TIM2->PSC= 1125;		// Timer-Takt = 4kHz, angenommen der Takt 45Mhz
	TIM2->ARR= 281;				// Frequenz ist 1kHz = 2812.5
	TIM2->DIER=TIM_DIER_UIE;	// Interrupt aktiviert
	TIM2->CR1=TIM_CR1_CEN;		// Timer einschalten

//	NVIC->ISER[1] |= 0x36;	//NVIC fuer TIM2
//	NVIC->ISER[1] = TIM2_IRQn;
	NVIC_EnableIRQ(TIM2_IRQn);
}

void WriteDAC(int analog)
// Ausgabe des Analogwertes
{
	DAC->DHR12R1=analog;											// DAC-Wert setzen	
}

