/*----------------------------------------------------------------------------
	MCT-Praktikumsversuch V4.3 Nucleo
	mit Tmeperatursensor und LED-Display
 *----------------------------------------------------------------------------*/

#include <stm32f4xx.h>
#include <stdio.h>
#include "USART2.h"
#include "DAC.h"
#include "TIM3_PWM.h"
#include "stepper.h"
#include "I2C.h"
#include "Temperatur.h"
#include "LCDInit.h"
#include "LCD.h"
#include "ESP_USART3.h"

void Help_Display(void);
extern int ticks;

//Sinustabelle
int sinus[32] = { 2048, 2419, 2775, 3104, 3392, 3628, 3803, 3911, 3948,
									3911, 3803, 3628, 3392, 3104, 2775, 2419, 2048, 1677,
									1321, 992, 704, 468, 293, 185, 148, 185, 293, 468,
									704, 992, 1321, 1677};

int amp =9;						// Amplitude Sinus
int freq =300;				// Frequenz Sinus 300 = Ton a

int tempon=0;					// Modus Temperaturmessung

void ExecuteCmd (void)
// Ausführung des eingegebenen Befehls
{
	int pos, wert;			// Position und Wert in der Sinustabelle
	int puls;						// Position Servo
	int r,g,b;					// Werte für RGB-LED
	char led;						// Wert für einzelne RGB-LED			
	int temp;						// Temperatur in 1/256 Grad
	char out_str[15]; 	// String für Ausgabe

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
		// Servoposition anfahren
		case 's':
			sscanf(inputBuffer,"s%d",&puls);
			TIM3_servo(puls);
			break;
		// RGB-LED setzen
		case 'r': 
			sscanf(inputBuffer,"r%dg%db%d",&r,&g,&b);
			TIM3_RGB(r,g,b);
			break;
		// einzelne Farbe der RGB-LED setzen
		case 'l':
			sscanf(inputBuffer,"l%c%d",&led,&puls);
			switch (led) {
				case 'r': TIM3_RGB(puls,-1,-1);	break;
				case 'g': TIM3_RGB(-1,puls,-1);	break;
				case 'b': TIM3_RGB(-1,-1,puls); break;
			}
			break;
		// Schrittmotor vorwärts
		case '+':
			stepmode = STEPMODE_FWD;
			break;			
		// Schrittmotor rückwärts
		case '-':
			stepmode = STEPMODE_BWD;
			break;
		// Schrittmotorposition anfahren
		case 'm':
			if (inputBuffer[1]=='r')				// Referenzfahrt
				stepmode = STEPMODE_REF;
			else {
				sscanf(inputBuffer,"m%d", &ziel_pos);
				stepmode = STEPMODE_POS;			// Position anfahren
			}
			break;

		//Temperatur
		case 't':	
			switch(inputBuffer[1]) {
				// auslesen
				case 'r':
					temp = read_Temp();
					sprintf(out_str,"Temperatur: %.3lf C\n\r",(double)temp/256.0);
					WriteString(out_str);
					break;
				// Auflösung setzen
				case 's':
					set_TempRes( (int)(inputBuffer[2]-'0'));
					break;
			 // Temperaturanzeige an/aus
				case 'a':
					 if (inputBuffer[2] == '0') {
							tempon=0;
							stepmode = STEPMODE_NONE;
					 } else if (inputBuffer[2] == '1') {
							tempon=1;
						 stepmode = STEPMODE_POS;
					 }
					break;
			}
			break;

		//Display
		case 'd':					
			// LCD-Kommando senden
			if (inputBuffer[1] == 'c')
			{ 
				sscanf(inputBuffer,"dc%x",&wert);
				lcd_send_cmd((char)wert);
			}
			 
			// LCD-Daten senden
			if (inputBuffer[1] == 'd')
			{ 
				 sscanf(inputBuffer,"dd%x", &wert);
				 lcd_send_data((char)wert);
			}
			break;										

		// Anzeige Kommandos
		case 'h':
			Help_Display();
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
	WriteString("\ttr\t\t\tTemperatur messen\n\r");
	WriteString("\tta[0|1]\t\t\tTemperaturanzeige aus/an\n\r");
	WriteString("\tts[0/1/2/3]\t\tAufloesung Temperatur einstellen\n\r");
	WriteString("\tdc[cmd]\tLCD Kommando\n\r");
	WriteString("\tdd[data]\tLCD Char anzeigen\n\r\n\r");
}

void delay (int del) 
// einige µs warten	
{
  int  d1;
   for (d1 = 0; d1< del; d1++);
}

void Temp_Display()
// Temperatur auslesen und anzeigen, Schrittmotor und Servo positionieren
{
	int Temp;
	int puls;	
	char out_str[20];
  Temp = (double)read_Temp() / 2.56;	// Umrechnung in 1/100 Grad

	sprintf(out_str,"T = %.3f ",(double)Temp/100.0);
	lcd_print_x_y(0,0,out_str);
	lcd_send_data(0x00);								// Gradzeichen
	lcd_send_data('C');
	ziel_pos = (Temp-2000)/5;						// Schrittmotor fahren
	puls = (Temp-2000)/10;							// Servomotor fahren
	TIM3_servo(puls);
}	

void GradZeichen()
// definiert Gradzeichen als Zeichen 0 für LCD
{
	lcd_send_cmd(0x40);	
	lcd_send_data(0x1C);	//xxx..
	lcd_send_data(0x14);	//x.x..
	lcd_send_data(0x1C);	//xxx..
	lcd_send_data(0x00);	//.....
	lcd_send_data(0x00);	//.....
	lcd_send_data(0x00);	//.....
	lcd_send_data(0x00);	//.....
	lcd_send_data(0x00);	//.....
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
	InitStepper();
	I2C_init(8);
	lcd_init();
	TIM3_RGB(0, 0, 0);
  WriteString("\n\r\n\rV5.3 Nucleo\n\r");	
	lcd_send_string("V4.3");
	GradZeichen();
	Program_to_ESP(33);
	
	while (1) 
	{
		if ((ticks%400==0) && tempon)			// alle 200 Ticks (bei 400 Hz SysTick = 0,5s)
			Temp_Display();

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
