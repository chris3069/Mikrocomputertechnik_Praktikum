/*------- ---------------------------------------------------------------------
	MCT-Praktikumsversuch V2.3 Nucleo
	mit USART2
 *----------------------------------------------------------------------------*/

#include <stm32f4xx.h>
#include <stdio.h>
#include "USART2.h"
#include "DAC.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/

char c[1];
uint8_t executeflag = 0;
uint16_t soundvals[] = {227,   186,   146,   110,    78,    53,    34,    24,    22,    28,    43,    65,    93,   127,   166,   206,   248,   289,   327,   361,   390,   412,   426,   432,   430,   420, 401,   376,   344,   308,   268};
	
		uint16_t frequenzy = 1000;
		uint8_t amplitude = 8;
	
	void interp_input()
	{
		uint16_t tmpfreq = 100;
		sscanf(inputBuffer,"f%d%*c%d",&tmpfreq, &amplitude);
		frequenzy = 60+4.42*tmpfreq;
		static char output[20];
		sprintf(output, "Frequenz: %d \t Amplitude: %d \r\n", frequenzy, amplitude);
		writeString(output);
	}
	
int main (void)
{ 	
	//------- Init ----------
	initUSART2();
	InitDAC();
	Init_Systick();
	//InitTimer2();
	
	const char * str = "Der Apfel feallt nicht weit vom Stamm";
  writeString(str);	
	char debug[30] = "ExecuteFlag = ";
	while (1) 
	{
		sprintf(debug, "ExecuteFlag = %d\r\n",executeflag);
		//writeString(debug);
		
		if(executeflag == 1)
		{
			interp_input();
			executeflag = 0;
		}
	}
}

void USART2_IRQHandler (void)
{

	char inputChar;						// Eingelesenes Zeichen
	static int bufferPos = 0;	// Pufferposition
	
	// Empfangenes Zeichen speichern
	inputChar = readChar();
	writeChar(inputChar);				// Echo zurueckschicken
	inputBuffer[bufferPos++] = inputChar;	

	// Pruefen ob Punkt oder Entertaste eingelesen wurde
	if ((inputChar == '\r')||(inputChar == '.')) 
	{		// falls ja, ist die Zeileneingabe beendet
		inputBuffer[bufferPos] = 0; // String abschliesen
		bufferPos = 0;							// Einlesen beginnt von vorne
    executeflag = 1;							  // Kommandoausfuehrung im Hauptprogramm
		writeString("\n\r");						// neue Zeile
		

	}

return;
}

	void SysTick_Handler(void)
{			
		static int8_t index=0;
	
	++index;
	if (index >=31)
{
	index = 0;
}
		
	WriteDAC(amplitude* soundvals[index]);
	
		uint16_t sysload = 3995-0.445*frequenzy;
		SysTick->LOAD = sysload;

}




void TIM2_IRQHandler(void)
{
	static int8_t index=15, countupflag=0;
	
	WriteDAC(amplitude* soundvals[index]);
	TIM6->ARR= (uint16_t)(0.2812*frequenzy);
	++index;
	if (index >=31)
{
	index = 0;
}
	
}


