/*----------------------------------------------------------------------------
  Funktionen für Initialisierung des LC-Displays
 *----------------------------------------------------------------------------*/
#include <stm32f4xx.h>
#include "I2C.h"

#define LCD_ADR    0x27//   

#define RS 0x01					// Bit für Daten (gesetzt für Daten, gelöcht für Kommando)
#define RW 0x02					// Read/Write (gesetzt für Read)
#define E 0x04					// Enabe (fallende Flanke übernimmt Daten/Kommandos)
#define BL 0x08					// Hintergrundbeleuchtung an/aus

void lcd_waitms(int ms)
{
	volatile int i;
	int j;
	j= 1800*ms;
  for (i =1; i<j; i++)
  { i = i;}	
}

void lcd_cmd (char cmd)
// sendet Kommando an Display
{
  unsigned char data_u, data_l;
	unsigned char data_t[4];
	
	data_u = (cmd&0xf0);			// obere 4 Bit des Kommandos
	data_l = ((cmd<<4)&0xf0);	// untere 4 Bit des Kommandos
	
	// Kommando senden
	data_t[0] = data_u|BL|E;  //en=1, rs=0
	data_t[1] = data_u|BL;  	//en=0, rs=0
	data_t[2] = data_l|BL|E;  //en=1, rs=0
	data_t[3] = data_l|BL;  	//en=0, rs=0
	I2C_write(data_t,4,LCD_ADR);
}

void lcd_init (void)
// initialisiert Display
{
	// 4 bit Initialisierung
	lcd_waitms(50);  					// wait for >40ms
	lcd_cmd (0x30);
	lcd_waitms(5);  					// wait for >4.1ms
	lcd_cmd (0x30);
	lcd_waitms(1);  					// wait for >100us
	lcd_cmd (0x30);
	lcd_waitms(10);
	lcd_cmd (0x20);  					// 4bit mode
	lcd_waitms(10);

  // Display Initialisierung
	lcd_cmd (0x28); 					// Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x7 characters)
	lcd_waitms(1);
	lcd_cmd (0x08); 					//Display on/off control --> D=0,C=0, B=0  ---> display off
	lcd_waitms(1);
	lcd_cmd (0x01);  					// clear display
	lcd_waitms(2);

	lcd_cmd (0x06); 					//Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	lcd_waitms(1);
	lcd_cmd (0x0C); 					//Display on/off control --> D = 1, C and B = 0. (Diplay on, Cursor off, no blink)
	lcd_waitms(50);  					// wait for >40ms
}
