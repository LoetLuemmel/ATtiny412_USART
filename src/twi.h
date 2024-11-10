#ifndef TWI_H
#define TWI_H

#include <stdint.h>

// I2C-Adresse des BME680 (0x76 oder 0x77)
#define BME680_ADDR 0x76

// Funktionsdeklarationen
void twi_init(void);
bool twi_start(uint8_t addr);
bool twi_write(uint8_t data);
uint8_t twi_read(bool ack);
void twi_stop(void);

#endif 