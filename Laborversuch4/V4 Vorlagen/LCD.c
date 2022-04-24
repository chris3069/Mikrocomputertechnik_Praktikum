/*----------------------------------------------------------------------------
  Funktionen für LC-Display
 *----------------------------------------------------------------------------*/
#include "I2C.h"

#define LCD_ADR	0x27		// I2C-Adresse des LCDisplays

#define RS 0x01					// Bit für Daten (gesetzt für Daten, gelöcht für Kommando)
#define RW 0x02					// Read/Write (gesetzt für Read)
#define E 0x04					// Enabe (fallende Flanke übernimmt Daten/Kommandos)
#define BL 0x08					// Hintergrundbeleuchtung an/aus

void lcd_send_cmd (char cmd) 
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

void lcd_send_data (char data) 
// sendet Daten an Display
{
	char data_u, data_l;
	unsigned char data_t[4];

	data_u = (data&0xf0);			// obere 4 Bit der Data
	data_l = ((data<<4)&0xf0);	// untere 4 Bit der Data
	
	data_t[0] = data_u|BL|E|RS;  //en=1, rs=1
	data_t[1] = data_u|BL|RS;  	//en=0, rs=1
	data_t[2] = data_l|BL|E|RS;  //en=1, rs=1
	data_t[3] = data_l|BL|RS;  	//en=0, rs=1
	I2C_write(data_t,4,LCD_ADR);

}

void lcd_send_string (char *str)
// String an Display ausgeben
{
	while (*str) lcd_send_data (*str++);
}

void lcd_print_x_y(int line , int  row, char *str )
// String an bestimmter Position des Displays ausgeben
{
	unsigned char  line_x_y ;
 if (line == 0 )
 {
   line_x_y = 0x80 + row ;
   lcd_send_cmd(line_x_y);
 }
 else if (line == 1 )
	{
   line_x_y = 0x80 |( 0x40 + row) ;
   lcd_send_cmd(line_x_y);
  } 
	while (*str) lcd_send_data (*str++);
}
/*

void lcd_def_char(char * chardata, uint8_t number) {
    uint8_t i, line, col;

    lcd_getlc(&line, &col);     // get actual cursor position
    number &= 0x7;              // limit char number to 0-7
    lcd_command(LCD_DEF_CHAR + (number<<3));    // set CGRAM address

    for (i=0; i<8; i++) {
        lcd_putchar(pgm_read_byte(chardata++));
    }

    lcd_gotolc(line, col);        // restore cursor position
}
*/