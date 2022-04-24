/*----------------------------------------------------------------------------
  Funktionen für Temperatursensor
	MCP 9808
 *----------------------------------------------------------------------------*/

#include "I2C.h"

#define TEMP_ADR    0x18  // Slave Adresse Temperaturfühler
#define TEMP_RES    0x08  // Resolution Register
#define TEMP_TR     0x05  // Temperature Register

int read_Temp()
// liest Temperatur aus und gibt sie in 1/16 Grad zurück
{
	unsigned char i2c_buf[10] ={0,0,0,0,0,0};					// Buffer für I2C-Kommunikation
	int temp_value;

	i2c_buf[0] = TEMP_TR;               							// Register-Pointer => Temp-Register 
	I2C_write( i2c_buf, 1, TEMP_ADR );  							// Temperatur Register adressieren

	I2C_read( i2c_buf, 2, TEMP_ADR );   							// Temperatur einlesen
	temp_value = (i2c_buf[0]*256.0 + i2c_buf[1]);
	temp_value &= 0x1FFF;															// Statusbits entfernen
	if ((temp_value & 0x1000) == 0) 									// Test auf SignBit
	  temp_value = temp_value <<4;
  else
		temp_value = 256*256 - (temp_value <<4);
	return temp_value;
}

void set_TempRes(int resolution)
// Auflösung des Temperatursensors setzen (Modi 0...3 erlaubt)
{ 
  unsigned char i2c_buf[10];
	
	if (resolution<0 || resolution>3)									// Abbruch bei falschem Modus
		return;

	i2c_buf[0] = TEMP_RES;                  					// Register-Pointer => Resolution 
	i2c_buf[1] = resolution;
	I2C_write(i2c_buf, 2, TEMP_ADR);	  							// Temperatur Register adressieren
}
