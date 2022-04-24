/*----------------------------------------------------------------------------
	MCT-Praktikumsversuch V1.3 Nucleo
	mit Lauflicht/Warteschleife in C
	
	06/2020
 *----------------------------------------------------------------------------*/

#include "ownIO.h"

											// Ein-/Ausgabewerte
		
		
	void SysTick_Handler(void)
{			
	static int output = 0x1;
	static int input;	
	input = GPIOC->IDR;
			input &= 0x7; 												// Die ersten 3 Taster, 2 fuer Geschw. 1 fuer Richtung 
			SysTick->STK_LOAD = ((input & 0x3)+1)*187500;
	
			int richtung = input &0x4;
			if (richtung)													//Bestimme die Richtung
			{
							output = output << 1;					//Verschiebe das Bit in die Richtung
							if (output > 0x80)	
				{
					output = 0x01;									// Bit out of Scope
				}
			}
			else
			{
						output = output >>1;
						if (output < 0x01)
				{
							output = 0x80;											// Falls Bits out of Scope sind
				}
			}	
			GPIOB->ODR = output;	
			


}



void Init_Systick()
{
	SysTick->STK_LOAD = 150000;
	SysTick->STK_CTRL = 0x3;
}


/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void)
{

	// GPIO initialisieren
  RCC->AHB1ENR |= 0x06;											// Takt für GPIO B und C einschalten
	GPIOB->MODER = 0x5555;										// PB0...7 als Ausgang
	GPIOC->MODER = 0x0000;										// PC0...7 als Floating Input
	GPIOB->ODR = 0x1;								// Start mit LED 0
	Init_Systick();
	}		
