/*----------------------------------------------------------------------------
  Funktionen für Schrittmotor
 *----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stm32f4xx.h>
#include "stepper.h"
#include "USART2.h"

int fahreanposflag = 1;
int fahreanposerreichtflag = 0;
int stepdir = 0;  						// Richtung des Schrittmotors
int akt_pos=0;								// aktuelle Position des Schrittmotors
int ziel_pos=0;								// Zielposition des Schrittmotors
static int index;
				
				
int stepmode=STEPMODE_NONE;		// Modus des Schrittmotors

 int step_pos[][4] = {{1, 1, 0, 0}, {0, 1, 1, 0},{0,0, 1,1}, {1, 0,0,1}};

void InitSysTick (int load)
// initialisiert SysTick mit load und startet ihn
{

	SysTick->LOAD = load; //  10us
	SysTick->CTRL |= 0x7;

}


void InitStepper(void)
// initialisiert Schrittmotorausgänge GPIOB0...3 und Referenzeingang GPIOB4
//Ansteuerung mittels Systick Timers
{

	int PortMode;
	
	// Initialisierung der Ports PA4...7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;   // Enable GPIOB clock
	PortMode=GPIOB->MODER;			// Konfiguration Port A Low holen
	
	PortMode&=0xFFFF0000;			// Konfiguration für PB4...7 behalten
	PortMode|= 0x00005555;			// PB0...3 auf Ausgang
	GPIOB->MODER = PortMode;
	
	PortMode = GPIOB->OSPEEDR; //LOW Speed
	PortMode &= 0xFFFF0000;
	PortMode |= 0x00000000;	
	GPIOB->OSPEEDR = PortMode;
		
	// PortB0-3 = 0, Stromlos, um Motor nicht zu belasten in Aufgabe V3.1 und V3.2
	GPIOB->ODR &= 0xFFFFFFF0; 
	
	
	/*
	// Initialisierung des Timer6
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;		// Takt für TIM6 einschalten
	TIM3->PSC= 2400;		// Timer-Takt = 10kHz, angenommen der Takt ist wieder 24Mhz
	TIM3->ARR= 20;				// Interrupt alle 2 ms = 500 Hz
	TIM3->DIER=TIM_DIER_UIE;	// Interrupt aktiviert
	TIM3->CR1=TIM_CR1_CEN;		// Timer einschalten
	
	NVIC->ISER[1] |= 

	NVIC->ISER[1] |= 0x00400000L;	// NVIC für TIM6 aktivieren
*/

}

void Step_Out(int step_pattern)
// gibt das Schrittmotormuster step_pattern an Motor aus
{	

	GPIOB->ODR &= step_pattern;	// step_pattern auf PB0-3 ausgeben
	GPIOB->ODR &= 0xFFFFFF0F; 		// Motor Stromlos schalten
	
}


void nextstep(int drehrichtung)
{
	//drehrichtung darf nur 1, 0 oder -1 sein

	//const int step_pattern[] = {0xC, 0x6, 0x3, 0x9}; 
	const uint8_t step_pattern[] = {0x3, 0x2, 0x6, 0x4, 0xC, 0x8, 0x9, 0x1}; 
	const uint8_t len = sizeof(step_pattern)/sizeof(step_pattern[0]);
	
	static int index = 0;
	int muster = step_pattern[index];
	uint16_t ausgabe;
			if(drehrichtung == 0)
	{
			GPIOB->ODR &= 0xFFFFFFF0;  //Motor Stromlos schalten
			return;
	}
	
	
	index = index + drehrichtung;
	if(index < 0)
	{
		index = len -1;
	}
	else if(index > len-1)
	{
		index = index % len;
	}
	
		ausgabe = GPIOB->ODR;
	ausgabe &= 0xFFFFFFF0;
	ausgabe |= muster;
	GPIOB->ODR = ausgabe;	
}





/*----------------------------------------------------------------------------
  Interrupt-Handler SysTick
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void)
{
	//Fahre an Position
	if(fahreanposflag == 1 &&	fahreanposerreichtflag == 0)
	{
		
		if (akt_pos == ziel_pos)
		{
			fahreanposflag = 0;
		fahreanposerreichtflag = 1;
		stepdir = 0;
			return;
		}
		
		else //Aktpos != Zielpos
		{
		nextstep(stepdir);
		akt_pos =akt_pos + stepdir; // + oder - abhängig von Drehrichtung
		}
		return;
	}
	//Drehe im Kreis
nextstep(stepdir);
	
		/*
				if(akt_pos != ziel_pos)
			{
				fahreanposflag = 1;
				fahreanposerreichtflag = 0;
				if (abs(ziel_pos - akt_pos) < 2024)
				{
					if(ziel_pos > akt_pos)
					stepdir = 1;
					else
						stepdir = -1;
				}
				else
				{
					if(ziel_pos > akt_pos)
						stepdir = -1;
					else
						stepdir = 1;
				}		
			}
	*/
	
}
