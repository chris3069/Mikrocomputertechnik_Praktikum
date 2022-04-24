/* ------------------------------------------------------------------------------------------------
	I2C Schnittstelle
	Haunstetter (02.11.2015)
	I2C 1 (STM32vlDiscovery): SCL = PB6, SDA = PB7
	ohne Interrupt, 100kHz Takt
    Datei: i2c.c
   ------------------------------------------------------------------------------------------------
*/
#include <stm32f4xx.h>
#include "I2C.h"

#define SYSCLK   16000
//Enter I2C clock in kHz: 10Khz -> 10
#define I2C_SCLK 400
//Enter max Trise in ns for chosen bit rate (100 kBit/s = 1000 ns, 400 kbit/s = 300 ns
//1000ns -> 1000
#define I2C_TRISE 320

/* ====================================================================
   I2C Config Routine (Standard Mode, 100kHz)
   ==================================================================== */
	 
//	 void I2C_config(
//	unsigned char address              // eigene 7-Bit Adresse
//)
//{
//	
//	I2C1->CR1 |= I2C_CR1_SWRST;         // alles auf Anfangszustand setzen
//	I2C1->CR1 &= ~I2C_CR1_SWRST;
//    
//	// Standards: Hardware angehalten; F_S und DUTY stehen für 100kHz Mode; 7-Bit Adressformat
//	I2C1->CR2 |= (SYSCLK/ 1000) & 0x3f; // Bustaktfrequenz in MHz (SCL 400kHz braucht freq%10==0)
//	
//	I2C1->CCR |= (SYSCLK / (2*I2C_SCLK)) & 0xfff; // Bustakt Frequenz Kennung (MHz), SM Mode, Teiler für SCL = 100kHz
//	I2C1->TRISE &= ~0x3f;
//	I2C1->TRISE |= ((SYSCLK * I2C_TRISE)/ 1000000 +1) & 0x3f; //Anzahl der SYSCLK Takte für Anstiegszeit I2C_TRISE, + 1
//	
//	I2C1->OAR1 |= (address & 0x7f) << 1; // eigene 7-Bit Adresse programmieren

//	I2C1->CR1 |= I2C_CR1_PE;            // Hardware freigeben
//}


void I2C_config(	unsigned char address)           // eigene 7-Bit Adresse

{
	unsigned int freq = (unsigned short) (SystemCoreClock/1000000); // Bustakt Frequenz Kennung (MHz)
	I2C1->CR1 |= I2C_CR1_SWRST;         // alles auf Anfangszustand setzen
	I2C1->CR1 &= ~I2C_CR1_SWRST;
    
    // Standards: Hardware angehalten; F_S und DUTY stehen für 100kHz Mode; 7-Bit Adressformat
	I2C1->CR2 |= freq & 0x3f;           // Bustaktfrequenz in MHz (SCL 400kHz braucht freq%10==0)
	
	I2C1->CCR |= (5*freq) & 0xfff;      // Teiler für SCL = 100kHz
	I2C1->TRISE &= ~0x3f;               // SCL Anstiegszeit 1µs
	I2C1->TRISE |= (freq+1) & 0x3f;
	
	I2C1->OAR1 |= (address & 0x7f) << 1; // eigene 7-Bit Adresse programmieren

	I2C1->CR1 |= I2C_CR1_PE;            // Hardware freigeben
}



/* ====================================================================
   I2C Init Routine
   ==================================================================== */
void I2C_init(
	unsigned char address)              // eigene 7-Bit Adresse

{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;		// GPIOA Takt einschalten

  GPIOB->MODER &= 0xFFF0FFFF;							// GPIOB8/9 Mode-Bits löschen
	GPIOB->MODER |= 0x000A0000;							// GPIOB8/9 AF
	GPIOB->OTYPER |= 0x300;									// GPIOA8/9 Open Drain
	GPIOB->AFR[1] &= 0xFFFFFF00;						// GPIOB8/9 AF auf I2C schalten (s. Tab 11 Datenblatt) AF4 bei Pin 8 und 9 Pin 8 und 9 = erste Elemente in [1]
	GPIOB->AFR[1] |= 0x00000044;
	
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // I2C1 Hardware Takt einschalten
	I2C_config( address );       // Hardware konfigurieren
}



/* ====================================================================
   I2C Write n Byte From Buffer Routine, Master Mode
   ==================================================================== */
void I2C_write(
    const unsigned char * buf           // Puffer der zu sendenden Byte
    ,unsigned int n                     // Anzahl zu sendender Byte
    ,unsigned char dest                 // Zieladresse
)
{
	if (n == 0)                         // n = 0 -> ignorieren
			return;

	while (I2C1->SR2 & I2C_SR2_BUSY)		// warten, bis Bus frei
		;
	
	I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_START; // Startbedingung setzen und Acknowledge zulassen
	while (!(I2C1->SR1 & I2C_SR1_SB));  // warten auf Startbedingung OK (I2C_SR1_TIMEOUT)

	// Adresse senden, mit Schreibanforderung (LSB = 0)
	I2C1->DR = (uint8_t) (dest << 1);   // Zieladresse für Senden
	while (!(I2C1->SR1 & I2C_SR1_ADDR)); // warten auf Acknowledge OK (I2C_SR1_TIMEOUT)
	while (!(I2C1->SR2 & I2C_SR2_TRA)); // warten auf Transmit Mode (I2C_SR1_TIMEOUT)

	// Datenbytes senden, ACK vom Slave notwendig
	for (; n != 0; n--)
	{
			while (!(I2C1->SR1 & I2C_SR1_TXE)); // warten auf freien Transmitter (I2C_SR1_TIMEOUT)
			I2C1->DR = *buf++;              // Byte senden
	}

	while (!(I2C1->SR1 & I2C_SR1_BTF)); // warten auf freien Bus (I2C_SR1_TIMEOUT)
	I2C1->CR1 |= I2C_CR1_STOP;          // wenn fertig: Stopbedingung setzen
}

/*
void I2C_write(const uint8_t * outBuf, unsigned int numSend, uint8_t destAddr) {

	if ((numSend == 0)) // numSend = 0 -> ignorieren
			return;

	while (I2C1->SR2 & I2C_SR2_BUSY);		// warten, bis Bus frei

	I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_START; // Startbedingung setzen und Acknowledge zulassen
	while (!(I2C1->SR1 & I2C_SR1_SB));  // warten auf Startbedingung OK (I2C_SR1_TIMEOUT)

	// Adresse senden, mit Schreibanforderung (LSB = 0)
	I2C1->DR = (uint8_t) (destAddr << 1);   // Zieladresse für Senden
	while (!(I2C1->SR1 & I2C_SR1_ADDR)); // warten auf Acknowledge OK (I2C_SR1_TIMEOUT)
	while (!(I2C1->SR2 & I2C_SR2_TRA)); // warten auf Transmit Mode (I2C_SR1_TIMEOUT)

	// Datenbytes senden, ACK vom Slave notwendig
	while (numSend > 0)
	{
		while (!(I2C1->SR1 & I2C_SR1_TXE)); // warten auf freien Transmitter (I2C_SR1_TIMEOUT)
		I2C1->DR = *outBuf++;              // Byte senden
		numSend--;
	}

	while (!(I2C1->SR1 & I2C_SR1_BTF)); // warten auf freien Bus (I2C_SR1_TIMEOUT)
	
	I2C1->CR1 |= I2C_CR1_STOP;          // wenn fertig: Stopbedingung setzen
}
/*

/* ====================================================================
   I2C Read Byte Routine, Master Mode
   ==================================================================== */
void I2C_read(
	unsigned char * buf                 // Puffer für eingelesene Byte
	,unsigned int n                     // Anzahl zu lesender Byte
	,unsigned char source               // Adresse der Datenquelle 
)
{
	volatile unsigned int dmy;
	
	if (n == 0)                         // n = 0 -> ignorieren
			return;

	while (I2C1->SR2 & I2C_SR2_BUSY)		// warten, bis Bus frei
		;
	
	I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_START; // Startbedingung setzen und Acknowledge zulassen
	while (!(I2C1->SR1 & I2C_SR1_SB));  // warten auf Startbedingung OK (I2C_SR1_TIMEOUT)

// Adresse senden, mit Leseanforderung (LSB = 1)
	I2C1->DR = (uint8_t)(source << 1 | 1);  // Zieladresse für Lesen
	while (!(I2C1->SR1 & I2C_SR1_ADDR));    // warten auf Acknowledge OK (I2C_SR1_TIMEOUT)
 

	if (n == 1)
	{
			// Einzelbyte lesen (ACK abschalten, bevor Empfangsbyte einläuft)
			DI
			I2C1->CR1 &= ~I2C_CR1_ACK;      // Acknowledge für letztes Byte abschalten
			dmy = I2C1->SR2;                // Adressbestätigung löschen
			I2C1->CR1 |= I2C_CR1_STOP;      // Stopbedingung einstellen
			EI
			while (I2C1->SR2 & I2C_SR2_TRA); // warten auf Receive Mode (I2C_SR1_TIMEOUT)
	}
	else if (n == 2)
	{
			// Doppelbyte lesen (ACK für 2. Byte abschalten: DR = Byte0, Schiebereg. = Byte1)
			DI
			I2C1->CR1 |= I2C_CR1_POS;       // verschobenes Acknowledge zulassen
			dmy = I2C1->SR2;                // Adressbestätigung löschen
			I2C1->CR1 &= ~I2C_CR1_ACK;      // Acknowledge für letztes Byte abschalten
			while (!(I2C1->SR1 & I2C_SR1_BTF)); // warten auf Daten im Schiebereg. (I2C_SR1_TIMEOUT)
			I2C1->CR1 |= I2C_CR1_STOP;      // Stopbedingung einstellen
			EI
			while (!(I2C1->SR1 & I2C_SR1_RXNE)); // warten auf Daten im Receiver (I2C_SR1_TIMEOUT)
			*buf++ = I2C1->DR;           // erstes Byte abholen
	}
	else
	{
			// Multibyte lesen
			while (I2C1->SR2 & I2C_SR2_TRA); // warten auf Receive Mode (I2C_SR1_TIMEOUT)

			// n-1 Datenbytes lesen, ACK nach jedem Byte
			while (n > 1)                   // nur (n-1) Byte in der Schleife
			{
					while (!(I2C1->SR1 & I2C_SR1_RXNE)); // warten auf Daten im Receiver (I2C_SR1_TIMEOUT)
					*buf++ = I2C1->DR;       // Byte abholen
					n--;
			}
			// letztes Datenbyte ohne ACK (Interrupts bei der Konfiguration verhindern)
			DI
			I2C1->CR1 &= ~I2C_CR1_ACK;      // Acknowledge für letztes Byte abschalten
			I2C1->CR1 |= I2C_CR1_STOP;      // Stopbedingung einstellen
			EI
	}

	while (!(I2C1->SR1 & I2C_SR1_RXNE)); // warten auf Daten im Receiver (I2C_SR1_TIMEOUT)
	*buf++ = I2C1->DR;                  // letztes Byte abholen
}
