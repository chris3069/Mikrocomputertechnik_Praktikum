/*----------------------------------------------------------------------------
  Funktionen für USART
	
	Nutzung der USART2-Schnittstelle des Nucleo 
	
	05/2020
 *----------------------------------------------------------------------------*/

#include <stm32f4xx.h>									// Definitionen
#include <string.h>

char inputBuffer[64];											// Eingabepuffer für Befehle
int cmdflag=0;														// Flag für anliegendes Kommando

void initUSART2(void)
// Initialisierung der USART2-Schnittstelle
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;		// GPIOA Takt einschalten
  GPIOA->MODER &= 0xFFFFFF0F;							// GPIOA2/3 auf alternate function schalten
	GPIOA->MODER |= 0x000000A0;							
	GPIOA->AFR[0] &= 0xFFFF00FF;						// GPIOA2/3 AF auf USART schalten (s. Tab 11 Datenblatt)
	GPIOA->AFR[0] |= 0x00007700;
	GPIOA->OTYPER &= 0xFFFB;								// GPIOA2 Push-Pull
	
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;		// Peripherietakt für USART2 einschalten
																					// Clock von APB1 = 45Mhz, siehe Handbook Seite 16
	USART2->BRR = 0x0683;
	USART2->CR1 = 0x0202C;									// Enable USART, Transmit, Recieve

	NVIC->ISER[1] = 0x40;										// NVIC für USART2 aktivieren
	//NVIC->ISER[1] = USART2_IRQn;
	//NVIC->ISR[1] = TIM2_IRQn;
	
	
}

void Init_Systick()
{
	SysTick->LOAD = 27; // Frequenz = 8900 Hz
	SysTick->CTRL |= 0x7;
}











void writeChar(char c)
// schreibt Zeichen c an USART2
{
	//	while(1)
	{
		if(USART2->SR & 0x080)			// Transmit data register not empty
		{
			USART2->DR = c;
			while(!((USART2->SR) & 0x040));		//Transmission Complete
			return;
		}
		//	USART2->SR |= 0x80;
	}
}

void writeString(const char *str)
// schreibt String str an USART2
{

	unsigned int slen = strlen(str);

for (int i = 0; i <= slen; ++i)

	{
		writeChar(str[i]);
		
	}
}

char readChar(void)
// warte auf Zeichen und gibt dieses zurück
{
	char c = NULL;			// eingelesenes Zeichen

	//while(1)
	{
		if(USART2->SR & 0x20)		//Read data Register not Empty
		{
			c = USART2->DR;					//  
			
		}
		return c;
	}
}



