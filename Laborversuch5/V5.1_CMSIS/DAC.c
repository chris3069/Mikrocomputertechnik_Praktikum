/*----------------------------------------------------------------------------
  Funktionen für Digital-Analog-Konverter miz LL-Library
	
	06/2020
 *----------------------------------------------------------------------------*/

#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_dac.h>

void InitDAC(void)
// Initialisierung des DAC-Konverters
{
	LL_DAC_InitTypeDef DAC_InitStructure;
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC1);			// DAC Takt einschalten

	LL_DAC_StructInit(&DAC_InitStructure);
	LL_DAC_Init(DAC,LL_DAC_CHANNEL_1,&DAC_InitStructure);		// DAC initialisieren
	LL_DAC_Enable(DAC,LL_DAC_CHANNEL_1);										// DAC enable
}

void WriteDAC(int analog)
// Ausgabe des Analogwertes
{
		LL_DAC_ConvertData12RightAligned(DAC,LL_DAC_CHANNEL_1,analog);	 				// DAC-Wert setzen
}

