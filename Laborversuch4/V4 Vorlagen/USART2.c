/*----------------------------------------------------------------------------
  Funktionen für USART
	
	Nutzung der USART2-Schnittstelle des Nucleo 
	
	05/2020
 *----------------------------------------------------------------------------*/

#include <stm32f4xx.h>									// Definitionen

char inputBuffer[64];											// Eingabepuffer für Befehle
int cmdflag=0;														// Flag für anliegendes Kommando

void InitUSART2(void)
// Initialisierung der USART2-Schnittstelle
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;		// GPIOA Takt einschalten
  GPIOA->MODER &= 0xFFFFFF0F;							// GPIOA2/3 auf alternate function schalten
	GPIOA->MODER |= 0x000000A0;							
	GPIOA->AFR[0] &= 0xFFFF00FF;						// GPIOA2/3 AF auf USART schalten (s. Tab 11 Datenblatt)
	GPIOA->AFR[0] |= 0x00007700;
	GPIOA->OTYPER &= 0xFFFB;								// GPIOA2 Push-Pull
	
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;		// Peripherietakt für USART2 einschalten
	USART2->BRR = SystemCoreClock/9600;			// auf 9600 baud setzen
	USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;
																					// USART enable,  Receiver Interrupt enable, Transmitter enable, Receiver enable
	NVIC->ISER[1] = 0x40;										// NVIC für USART2 aktivieren

}

void WriteChar(char c)
// schreibt Zeichen c an USART2
{
	int status;
	
	// warten, bis Transmit-Buffer leer
	do 
	{	
		status = USART2->SR & USART_SR_TXE;
	} while (status == 0);
	
	USART2->DR = c;												// Zeichen schreiben
}

void WriteString(char *str)
// schreibt String str an USART2
{
	while (*str != 0)
	{
		WriteChar(*(str++));
	}
}

/*----------------------------------------------------------------------------
  Interrupt-Handler USART2
 *----------------------------------------------------------------------------*/
void USART2_IRQHandler (void)
{

	char inputChar;							// Eingelesenes Zeichen
	static int bufferPos = 0;		// Pufferposition
	
	// Empfangenes Zeichen speichern
	inputChar = USART2->DR;
	WriteChar(inputChar);				// Echo zurückschicken

	if (inputChar == 0x7F) {			// Backspace
		bufferPos--;
		if (bufferPos<0)
			bufferPos=0;
	} else
		inputBuffer[bufferPos++] = inputChar;	
	
	// Prüfen ob Punkt oder Entertaste eingelesen wurde
	if ((inputChar == '\r')||(inputChar == '.')) 
	{		// falls ja, ist die Zeileneingabe beendet
		inputBuffer[bufferPos] = 0; // String abschließen
		bufferPos = 0;							// Einlesen beginnt von vorne
    cmdflag = 1;							  // Kommandoausführung im Hauptprogramm
		WriteString("\n\r");				// neue Zeile
	}

}
