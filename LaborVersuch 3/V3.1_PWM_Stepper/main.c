/*----------------------------------------------------------------------------
	MCT-Praktikumsversuch V4.3 Nucleo
	mit Servomotor, RGB-LED und Schrittmotor
 *----------------------------------------------------------------------------*/

#include <stm32f4xx.h>
#include <stdio.h>
#include "USART2.h"
#include "DAC.h"
#include "TIM3_PWM.h"
#include "Stepper.h"

void Help_Display(void);

//Sinustabelle
int sinus[32] = { 2048, 2419, 2775, 3104, 3392, 3628, 3803, 3911, 3948,
									3911, 3803, 3628, 3392, 3104, 2775, 2419, 2048, 1677,
									1321, 992, 704, 468, 293, 185, 148, 185, 293, 468,
									704, 992, 1321, 1677};

int amp =9;						// Amplitude Sinus
int freq =300;				// Frequenz Sinus 300 = Ton a
									
static int red, green, blue = 50;

void ExecuteCmd (void)
// Ausführung des eingegebenen Befehls
{
	int pos, wert;			// Position und Wert in der Sinustabelle
	int puls;						// Position Servo
	int r,g,b;					// Werte für RGB-LED
	char led;						// Wert für einzelne RGB-LED			

	switch (inputBuffer[0])		// erster Buchstabe des Befehls
	{
		// Sinuswert ändern
		case 'p':
			sscanf(inputBuffer,"p%dw%d",&pos,&wert);
			sinus[pos] = wert;
			break;
		// Frequenz/Amplitude ändern
		case 'f':
			sscanf(inputBuffer,"f%da%d",&freq,&amp);
			break;	
		
			case('+'):
				//TIM2_servochange(1);
			if (stepdir != 1)
			{
				stepdir = stepdir + 1;
			}
			else		
			{
			}
			fahreanposflag = 0;
			fahreanposerreichtflag = 1;

			break;
			
			
			case('-'):
				if (stepdir != -1)
				{
					stepdir = stepdir -1;
				}
				else 
				{
				}
			fahreanposflag = 0;
			fahreanposerreichtflag = 1;
				break;
		

			case('s'):
					//Servo auf Postion fahren
			sscanf(inputBuffer,"s%d",&pos);  // Der BUCHSTABE VOR DEM % ZEICHEN IST DAS EINGLESENE ZEICHEN
			//Zahl von 0 bis 100, pos anfahren
				TIM3_servo(pos);
				break;
		
			case('r'):
				sscanf(inputBuffer,"r%dg%db%d",&red, &green, &blue);	
				TIM3_RGB(red, green, blue);
			break;
			
		// Anzeige Kommandos
		case 'h':
			Help_Display();
			break;	
		
					case('m'):
				//Schrittmotor auf Position fahren, Zahl zwischen 0 und 4096
				//danach step out

			sscanf(&(inputBuffer[1]),"%d",&ziel_pos);
					
				fahreanposflag = 1;
				fahreanposerreichtflag = 0;
			//fahreanpos(ziel_pos);
			if(akt_pos != ziel_pos)
			{
					if(ziel_pos < akt_pos)
					stepdir = -1;
					else
						stepdir = 1;
			}		
			static char output[20];
			sprintf(output, "Zielposition: %d \t Aktuelle Position: %d \r\n", ziel_pos, akt_pos);
			WriteString(output);
			//Drehen stoppen, entweder stepdir = 0, oder Timer ausmachen, erst wieder anmachen, wenn eine stattfindet
			break;
			
			
	}
}

void Help_Display(void)
// Anzeige des Hilfetextes
{
	WriteString("Kommandos:\n\r");
	WriteString("\tp[pos]w[wert]:\t\tSinustabelle: an Stelle pos den Wert wert setzen\n\r");
	WriteString("\tf[freq]a[amp]:\t\tSinustabelle: Frequenz und Amplitude [1-9]  setzen\n\r");
	WriteString("\ts[pos]:\t\t\tServo auf Position pos fahren\n\r");
	WriteString("\tr[r]g[g]b[b]\t\tRGB-LED auf Farbe r/g/b setzen [0...255]\n\r");
	WriteString("\tl[r/g/b][wert]:\t\tBei RGB-LED Farbe r/g/b auf Wert wert setzen\n\r");
	WriteString("\t+:\t\t\tSchrittmotor dreht vorwaerts\n\r");
	WriteString("\t-:\t\t\tSchrittmotor dreht rueckwaerts\n\r");
	WriteString("\tm[pos]\t\t\tSchrittmotor auf Position pos fahren\n\r");
	WriteString("\tmr\t\t\tReferenzfahrt Schrittmotor\n\r");
}

void delay (int del) 
// einige µs warten	
{
  int  d1;
   for (d1 = 0; d1< del; d1++);
}

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void)
{ 	
  int i;											// Zählervariable
	int sin_a;
	
	//------- Init ----------
	InitDAC();
	InitUSART2();
	InitTIM3_PWM();
  WriteString("\n\r\n\rV3.1 Nucleo\n\r");	
	InitStepper();
	InitSysTick(80000); // 5ms 
	
	while (1) 
	{
		// Sinus ausgeben
		for (i=0;i<32;i++)
		{
		  delay(freq);
			sin_a = sinus[i]/(10-amp%10); // sin wird durch (1 bis 9) geteilt
			WriteDAC(sin_a);
			if (cmdflag == 1)			// anliegendes Eingabekommando
			{
				ExecuteCmd();
				cmdflag = 0;
			}
    }
	}
}
