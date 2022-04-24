/*----------------------------------------------------------------------------
  Funktionen f�r Schrittmotor
 *----------------------------------------------------------------------------*/

#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_utils.h>
#include <stm32f4xx_ll_cortex.h>
#include "stepper.h"
#include "USART2.h"

int stepdir = 0;  						// Richtung des Schrittmotors
int akt_pos=0;								// aktuelle Position des Schrittmotors
int ziel_pos=0;								// Zielposition des Schrittmotors
int stepmode=STEPMODE_NONE;		// Modus des Schrittmotors
int ticks=0;									// Z�hler, der bei jedem Systick-Aufruf erh�ht wird

void InitSysTick (int freq)
// initialisiert SysTick f�r freq Hz und startet ihn
{
	LL_InitTick(16000000,freq);							// SysTick initialisieren und starten auf 16MHz/40000 => 400 Hz
	LL_SYSTICK_EnableIT();									// Interrupt aktivieren
}


void InitStepper(void)
// initialisiert Schrittmotorausg�nge GPIOB0...3 und Referenzeingang GPIOB4
{
	LL_GPIO_InitTypeDef GPIO_InitStructure;

	// GPIOB0...3 initialisieren
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);			// GPIOB Takt einschalten
	GPIO_InitStructure.Pin=LL_GPIO_PIN_0 | LL_GPIO_PIN_1;
	GPIO_InitStructure.Pin|=LL_GPIO_PIN_2 | LL_GPIO_PIN_3;
	GPIO_InitStructure.Mode=LL_GPIO_MODE_OUTPUT;			
	GPIO_InitStructure.Speed=LL_GPIO_SPEED_FREQ_LOW;				
	GPIO_InitStructure.OutputType=LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStructure.Pull=LL_GPIO_PULL_NO;
	// GPIO_InitStructure.Alternate=LL_GPIO_AF_7;								
	LL_GPIO_Init(GPIOB,&GPIO_InitStructure);

	InitSysTick(400);												// Schrittfrequenz in Hz setzen
}

void Step_Out(int step_pattern)
// gibt das Schrittmotormuster step_pattern an Motor aus
{	
	// Bit 0 setzen/r�cksetzen
	if (step_pattern & 0x01)
		LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_0);
	else
		LL_GPIO_ResetOutputPin(GPIOB,LL_GPIO_PIN_0);

	// Bit 1 setzen/r�cksetzen
	if (step_pattern & 0x02)
		LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_1);
	else
		LL_GPIO_ResetOutputPin(GPIOB,LL_GPIO_PIN_1);
	
	// Bit 2 setzen/r�cksetzen
	if (step_pattern & 0x04)
		LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_2);
	else
		LL_GPIO_ResetOutputPin(GPIOB,LL_GPIO_PIN_2);
	
	// Bit 3 setzen/r�cksetzen
	if (step_pattern & 0x08)
		LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_3);
	else
		LL_GPIO_ResetOutputPin(GPIOB,LL_GPIO_PIN_3);
}

/*----------------------------------------------------------------------------
  Interrupt-Handler SysTick
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void)
{
	static int steps[8] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09};		// Abfolge Schrittmotormuster
	static int stepnr = 0;									// aktuelles Schrittmuster

	ticks++;
	
	switch (stepmode)						// abh�ngig von Schrittmotormodus
	{
		case STEPMODE_POS:				// Modus Positionierung
			if (ziel_pos > akt_pos)	 stepdir = 1;
			if (ziel_pos < akt_pos)  stepdir = -1;
			if (ziel_pos == akt_pos) stepdir = 0;
			break;			
	  case STEPMODE_FWD:				// Modus Vorw�rtsdrehen
		  stepdir = 1;
		  break;
		case STEPMODE_BWD:				// Modus R�ckw�rtsdrehen
		  stepdir = -1;
			break;
		case STEPMODE_REF:						// Referenzfahrt
			stepdir = -1;								// vorw�rts
			if (!(GPIOB->IDR & 0x10)) {	// bis Referenz erreicht
				stepdir=0;								// dann stop und nullen der Position
				akt_pos=0;
				stepmode = STEPMODE_NONE;
			}
			break;
		case STEPMODE_NONE:				// keine Bewegung
		  stepdir = 0;
			break;
  }
	
	stepnr = stepnr + stepdir;		// n�chstes Schrittmuster
	akt_pos = akt_pos + stepdir; 	// aktuelle Position erh�hen
	if (stepnr >7) stepnr = 0;		// Schrittmuster in g�ltigen Bereich bringen
	if (stepnr < 0) stepnr = 7;
	
	// Schrittausgabe
	Step_Out(steps[stepnr]);
}
