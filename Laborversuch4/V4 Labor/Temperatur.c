/*----------------------------------------------------------------------------
  Funktionen für Temperatursensor
 *----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "I2C.h"



#define TEMP_ADR    0x18  // Slave Adresse Temperaturfühler
#define TEMP_RES    0x08  // Resolution Register
#define TEMP_TR     0x05  // Temperature Register


const uint8_t MCP9808 = 0x18;
const uint8_t REG_TEMP = 0x5;	//Temperatur Register
const uint8_t REG_RES = 0x8; 	//Resolution Register
const uint8_t RES_VAL = 0x3;	// Resolution = 0.0625°C
char buf[3];

int read_Temp()
// liest Temperatur aus und gibt sie in 1/16 Grad zurück
{
	int temperature = -100;
//strncpy((char *)buf, (char *)&REG_TEMP, sizeof(REG_TEMP));
	buf[0] = REG_TEMP;
	I2C_write((char *)buf, 1, MCP9808);
	I2C_read((char *)buf, 2, MCP9808);

	uint16_t raw = atoi(buf);
	int UpperByte = buf[0] &0x0F00;
	int LowerByte = buf[1];
	
		uint16_t temp_c;
	uint16_t upper;
	uint16_t lower;
	
					upper = buf[0] &0x0F;
				lower = buf[1] &0xFF;
				temp_c =(int)((upper * 16 + lower /(float)16)*100);
	
	return temp_c;
	
	//int UpperByte = raw & 0x0F00; // Upper Byte
	//int LowerByte = raw & 0x00FF;	// Lower Byte
	if (raw & 0x1000) //Sign Bit -> Temp < 0
	{
	temperature = (256 - (UpperByte * 16 + LowerByte /(float) 16))*100;
		// convert Temp from pos to negative
	}
	else		// Temp is positive
	{
			temperature = (int)((UpperByte *16) + (LowerByte /(double) 16))*100;
	}
	
	return temperature;
}

void set_TempRes(int8_t resolution)
// Auflösung des Temperatursensors setzen (Modi 0...3 erlaubt)
{ 
	strncpy((char *)buf, (char *)&REG_RES, sizeof(REG_RES)); //Buffer beschreiben, mit Adresse von Sensor, Addresse des Registers, Register-Inhalt
//i2c_start(); // send START command
	buf[0] = REG_RES;
	buf[1] = resolution;
I2C_write((char *)buf, 2, MCP9808); //WRITE Command (see Section 4.1.4 “Address Byte”)
}
