/*----------------------------------------------------------------------------
  Funktionen für Digital-Analog-Konverter
 *----------------------------------------------------------------------------*/

#include <stm32f4xx.h>

void InitDAC(void)
// Initialisierung des DAC-Konverters
{
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;				// Peripherietakt für DAC einschalten
	DAC->CR |= DAC_CR_EN1;										// DAC enable
}

void WriteDAC(int analog)
// Ausgabe des Analogwertes
{
	DAC->DHR12R1=analog;											// DAC-Wert setzen	
}

