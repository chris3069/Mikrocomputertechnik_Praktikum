/*----------------------------------------------------------------------------
  Funktionen für USART mit LL-Library
	
	Nutzung der USART2-Schnittstelle des Nucleo 
	
	06/2020
 *----------------------------------------------------------------------------*/

#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_usart.h>

char inputBuffer[64];											// Eingabepuffer für Befehle
int cmdflag=0;														// Flag für anliegendes Kommando

void InitUSART2(void)
// Initialisierung der USART2-Schnittstelle
{
	LL_GPIO_InitTypeDef GPIO_InitStructure;
	LL_USART_InitTypeDef USART_InitStructure;									//Strukturen anlegen:

	// PA2/3 initialisieren
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);			// GPIOA Takt einschalten
	LL_GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.Pin=LL_GPIO_PIN_2 | LL_GPIO_PIN_3;
	GPIO_InitStructure.Mode=LL_GPIO_MODE_ALTERNATE;							
	GPIO_InitStructure.Speed=LL_GPIO_SPEED_FREQ_LOW;				
	GPIO_InitStructure.Alternate=LL_GPIO_AF_7;								// Alternate Function USART
	LL_GPIO_Init(GPIOA,&GPIO_InitStructure);

	// USART2 initialisieren
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);			// USART2 Takt einschalten
	
	USART_InitStructure.BaudRate = 9600;
	USART_InitStructure.DataWidth = LL_USART_DATAWIDTH_8B;
	USART_InitStructure.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStructure.OverSampling = LL_USART_OVERSAMPLING_16;
	USART_InitStructure.Parity = LL_USART_PARITY_NONE;
	USART_InitStructure.StopBits = LL_USART_STOPBITS_1;
	USART_InitStructure.TransferDirection = LL_USART_DIRECTION_TX_RX;
	
	LL_USART_Init(USART2 , &USART_InitStructure); 			// Init Struct noch leer
	LL_USART_Enable(USART2);
	LL_USART_EnableIT_RXNE(USART2);

	
	
//	LL_USART_SetDataWidth(USART2, LL_USART_DATAWIDTH_8B);
//	LL_USART_SetStopBitsLength(USART2, LL_USART_STOPBITS_1);
//	LL_USART_SetParity(USART2, 	LL_USART_PARITY_NONE);
//	LL_USART_SetTransferDirection(USART2,LL_USART_DIRECTION_TX_RX);
//	//LL_USART_SetBaudRate(USART2, LL_APB1_GRP1_PERIPH_USART2, LL_USART_OVERSAMPLING_16, 9600);
//	LL_USART_SetHWFlowCtrl(USART2, LL_USART_HWCONTROL_NONE);
//	LL_USART_SetOverSampling(USART2, LL_USART_OVERSAMPLING_16);
	
	//int baud = LL_USART_GetBaudRate(USART2, LL_APB1_GRP1_EnableClock
	
	// ...

	NVIC_EnableIRQ(USART2_IRQn);														// NVIC aktivieren
}

void WriteChar(char c)
// schreibt Zeichen c an USART2
{
while(!LL_USART_IsActiveFlag_TXE(USART2));		// USART Transmit Data Register Emtpy Flag set or not

		LL_USART_TransmitData8(USART2, c);
	//wait(1);
	// ...
	//LL_USART_TransmitData(USART2);
	//LL_USART_Tr();
	//while (LL_USART_IsActiveFlag_TC(USART2));
	//LL_USART_ClearFlag_TC(USART2);
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
	inputChar = LL_USART_ReceiveData8(USART2);
	WriteChar(inputChar);				// Echo zurückschicken
	//NVIC_ClearPendingIRQ(USART2_IRQn);
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
