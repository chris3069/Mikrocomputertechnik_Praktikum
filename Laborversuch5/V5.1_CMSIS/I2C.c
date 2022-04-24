/*----------------------------------------------------------------------------
  Funktionen für I2C mit LL-Library
	
	Nutzung der I2C-Schnittstelle des Nucleo 
	
	06/2020
 *----------------------------------------------------------------------------*/

//#include "i2c.h"
#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_i2c.h>
#include "I2C.h"

void I2C_init(uint8_t OwnAddress)
// initialisiert I2C
{
	int ClockSpeed = 100000;						// Bustakt I2C
	LL_GPIO_InitTypeDef GPIO_InitStructure;
	LL_I2C_InitTypeDef I2C_InitStructure;

	// GPIO PB8/9 initialisieren
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);			// GPIOB Takt einschalten
	GPIO_InitStructure.Pin=LL_GPIO_PIN_8 | LL_GPIO_PIN_9;
	GPIO_InitStructure.Mode=LL_GPIO_MODE_ALTERNATE;							
	GPIO_InitStructure.Speed=LL_GPIO_SPEED_FREQ_MEDIUM;				
	GPIO_InitStructure.OutputType=LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStructure.Pull=LL_GPIO_PULL_NO;
	GPIO_InitStructure.Alternate=LL_GPIO_AF_4;								// Alternate Function I2C
	LL_GPIO_Init(GPIOB,&GPIO_InitStructure);

	// I2C initialisiseren
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);				// I2C Takt einschalten
	LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);				// I2C Reset auslösen
	LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);			// I2C Reset freigeben

	LL_I2C_StructInit(&I2C_InitStructure);
	I2C_InitStructure.ClockSpeed=ClockSpeed;
	I2C_InitStructure.OwnAddress1=OwnAddress<<1;
	I2C_InitStructure.OwnAddrSize=LL_I2C_OWNADDRESS1_7BIT;
	I2C_InitStructure.PeripheralMode=LL_I2C_MODE_I2C;
	I2C_InitStructure.TypeAcknowledge=LL_I2C_ACK;
	LL_I2C_Init(I2Cx, &I2C_InitStructure);
	LL_I2C_Enable(I2Cx);
}

void I2C_write(const uint8_t* buf, uint32_t nbyte , uint8_t SlaveAddress)
{
	if (!nbyte)			// keine Daten zu senden, Ende
		return;
	
	SlaveAddress=SlaveAddress<<1;								// Adresse muss um ein Bit verschoben werden

	while(LL_I2C_IsActiveFlag_BUSY(I2Cx))				// warten, bis Bus frei
		;
	LL_I2C_DisableBitPOS(I2Cx);									// ACK kontrolliert aktuelles Byte 
	LL_I2C_GenerateStartCondition(I2Cx);				// Startbedingung generieren
	while (!LL_I2C_IsActiveFlag_SB(I2Cx))				// wartet auf Startbedingung
		;
	LL_I2C_TransmitData8(I2Cx, SlaveAddress);		// Slave-Adresse senden
	while (!LL_I2C_IsActiveFlag_ADDR(I2Cx))															// wartet auf Antwort Slave
		;
	LL_I2C_ClearFlag_ADDR(I2Cx);
	while (!LL_I2C_IsActiveFlag_TXE(I2Cx))			// wartet bis Sendepuffer leer
		;
	LL_I2C_TransmitData8(I2Cx, *buf++);					// sendet erstes Byte
	
	while (--nbyte) {
		while (!LL_I2C_IsActiveFlag_BTF(I2Cx))		// wartet, bis Byte gesendet
			;
		LL_I2C_TransmitData8(I2Cx, *buf++);				// sendet nächstes Byte
	}
	
	LL_I2C_GenerateStopCondition(I2Cx);					// generiert Stopbedingung
}

void I2C_read(uint8_t *buf, uint32_t nbyte , uint8_t SlaveAddress)
{
	SlaveAddress=SlaveAddress<<1;			// Adresse muss um ein Bit verschoben werden
	
	if (!nbyte)
		return;
	
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx))				// warten, bis Bus frei
		;
	LL_I2C_DisableBitPOS(I2Cx);									// ACK kontrolliert aktuelles Byte 
	LL_I2C_AcknowledgeNextData(I2Cx,LL_I2C_ACK);
	LL_I2C_GenerateStartCondition(I2Cx);				// Startbedingung generieren
	while (!LL_I2C_IsActiveFlag_SB(I2Cx))				// wartet auf Startbedingung
		;
	LL_I2C_TransmitData8(I2Cx, SlaveAddress|1);	// Slave-Adresse für lesen senden
	while (!LL_I2C_IsActiveFlag_ADDR(I2Cx))															// wartet auf Antwort Slave
		;

	if (nbyte==1){															// wenn nur 1 Byte zu senden
		LL_I2C_AcknowledgeNextData(I2Cx,LL_I2C_NACK);
		LL_I2C_ClearFlag_ADDR(I2Cx);
		LL_I2C_GenerateStopCondition(I2Cx);				// generiert Stopbedingung
	} else if (nbyte==2) {											// wenn 2 Byte zu senden
		LL_I2C_AcknowledgeNextData(I2Cx,LL_I2C_NACK);
		LL_I2C_EnableBitPOS(I2Cx);								// ACK kontrolliert nächstes Byte 
		LL_I2C_ClearFlag_ADDR(I2Cx);
	} else {																		// wenn mehr als 2 Byte zu senden
		LL_I2C_AcknowledgeNextData(I2Cx,LL_I2C_ACK);
		LL_I2C_ClearFlag_ADDR(I2Cx);
	}

	while (nbyte>0)	{															// solange noch Bytes zu übertragen sind
		if (nbyte<=3) {
			if (nbyte==1) {
				while (!LL_I2C_IsActiveFlag_RXNE(I2Cx))	// warten, bis Sendepuffer leer
					;
				*buf++ = LL_I2C_ReceiveData8(I2Cx);			// Byte lesen
				nbyte--;
			}
			if (nbyte==2) {
				while (!LL_I2C_IsActiveFlag_BTF(I2Cx))	// warten, bis Byte übertragen
					;
				LL_I2C_GenerateStopCondition(I2Cx);			// generiert Stopbedingung
				*buf++ = LL_I2C_ReceiveData8(I2Cx);			// Byte lesen
				nbyte--;
				*buf++ = LL_I2C_ReceiveData8(I2Cx);			// Byte lesen
				nbyte--;
			} else {
				while (!LL_I2C_IsActiveFlag_BTF(I2Cx))	// warten, bis Byte übertragen
					;
				LL_I2C_AcknowledgeNextData(I2Cx,LL_I2C_NACK);
				*buf++ = LL_I2C_ReceiveData8(I2Cx);			// Byte lesen
				nbyte--;
				while (!LL_I2C_IsActiveFlag_BTF(I2Cx))	// warten, bis Byte übertragen
					;
				LL_I2C_GenerateStopCondition(I2Cx);			// generiert Stopbedingung
				*buf++ = LL_I2C_ReceiveData8(I2Cx);			// Byte lesen
				nbyte--;
				*buf++ = LL_I2C_ReceiveData8(I2Cx);			// Byte lesen
				nbyte--;
			}
		} else {
			while (!LL_I2C_IsActiveFlag_RXNE(I2Cx))	// warten, bis Sendepuffer leer
				;
			*buf++ = LL_I2C_ReceiveData8(I2Cx);			// Byte lesen
			nbyte--;
			if (LL_I2C_IsActiveFlag_BTF(I2Cx)) {
				*buf++ = LL_I2C_ReceiveData8(I2Cx);			// Byte lesen
				nbyte--;
			}
		}
	}

	
	
//	// Wait for idle I2C interface
//	while(LL_I2C_IsActiveFlag_BUSY(I2Cx))
//		;
//	// Enable Acknowledgment , clear POS flag
//	LL_I2C_AcknowledgeNextData(I2C1,LL_I2C_ACK);
//	LL_I2C_DisableBitPOS(I2C1);
//	// Intiate Start Sequence (wait for EV5)
//	LL_I2C_GenerateStartCondition(I2Cx);
//	while(!LL_I2C_IsActiveFlag_MSL(I2Cx))
//		;
//	// Send Address (EV6)
//	LL_I2C_TransmitData8(I2C1, SlaveAddress);
//	while (!LL_I2C_IsActiveFlag_ADDR	(I2C1))
//		;
//	if (nbyte == 1) {
//		// Clear Ack bit
//		LL_I2C_AcknowledgeNextData(I2C1,LL_I2C_ACK);
//		// EV6_1 -- must be atomic -- Clear ADDR, generate STOP
//		DI
//		(void) I2Cx->SR2;
//		LL_I2C_GenerateStopCondition(I2Cx);
//		EI
//		// Receive data EV7
//		while(!LL_I2C_IsActiveFlag_RXNE(I2C1))
//			;
//		*buf++ = LL_I2C_ReceiveData8(I2Cx);
//	}
//	else if (nbyte == 2) {
//		// Set POS flag
//		LL_I2C_EnableBitPOS(I2C1);
//		// EV6_1 -- must be atomic and in this order
//		DI
//		LL_I2C_ClearFlag_ADDR(I2C1); // Clear ADDR flag
//		LL_I2C_AcknowledgeNextData(I2C1,LL_I2C_ACK); // Clear Ack bit
//		EI
//		// EV7_3 -- Wait for BTF, program stop, read data twice
//		while(!LL_I2C_IsActiveFlag_BTF(I2C1))
//			;
//		DI
//		LL_I2C_GenerateStopCondition(I2Cx);
//		*buf++ = LL_I2C_ReceiveData8(I2Cx);
//		EI
//		*buf++ = LL_I2C_ReceiveData8(I2Cx);
//	}
//	else {
//		LL_I2C_ClearFlag_ADDR(I2C1); // Clear ADDR flag
//		while (nbyte -- != 3)
//		{
//			// EV7 -- cannot guarantee 1 transfer completion time,
//			// wait for BTF instead of RXNE
//			while(!LL_I2C_IsActiveFlag_BTF(I2C1))
//				;
//			*buf++ = LL_I2C_ReceiveData8(I2Cx);
//		}
//		while(!LL_I2C_IsActiveFlag_BTF(I2C1))
//			;
//		// EV7_2 -- Figure 1 has an error , doesn't read N-2 !
//		LL_I2C_AcknowledgeNextData(I2C1,LL_I2C_ACK); // Clear Ack bit
//		DI
//		*buf++ = LL_I2C_ReceiveData8(I2Cx);
//		LL_I2C_GenerateStopCondition(I2Cx);
//		EI
//		*buf++ = LL_I2C_ReceiveData8(I2Cx);
//		// wait for byte N
////		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
//		while(!LL_I2C_IsActiveFlag_BTF(I2C1))
//			;
//		*buf++ = LL_I2C_ReceiveData8(I2Cx);
//		nbyte = 0;
//	}
//	// Wait for stop
//	while(LL_I2C_IsActiveFlag_STOP(I2C1))
//		;
}
