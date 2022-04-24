#include <stdint.h>

#ifndef I2C_NACKPosition_Current
#define I2C_NACKPosition_Current ((uint16_t)0xF7FF)
#endif

#ifndef I2C_NACKPosition_Next
#define I2C_NACKPosition_Next ((uint16_t)0x0800)
#endif

#define EI {unsigned int r0 = 0; __asm { msr primask,r0}} // enable configurable interrupts
#define DI {unsigned int r0 = 1; __asm { msr primask,r0}} // disable configurable interrupts

#define I2Cx I2C1					// Auswahl des I2C-Bus

void I2C_init(uint8_t OwnAddress);
void I2C_write(const uint8_t* buf, uint32_t nbyte , uint8_t SlaveAddress);
void I2C_read(uint8_t *buf, uint32_t nbyte , uint8_t SlaveAddress);
//void I2C_NACKPositionConfig(LL_I2C_TypeDef* I2Cx, uint16_t I2C_NACKPosition);
