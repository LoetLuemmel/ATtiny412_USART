#ifndef TWI_H
#define TWI_H

#include <avr/io.h>

// TWI Konstanten
#define TWI_STOP_gc 0x03
#define TWI_ACK_gc  0x02
#define TWI_NACK_gc 0x03

// TWI Funktionen
void twi_init(void);
bool twi_start(uint8_t addr);
void twi_stop(void);
bool twi_write(uint8_t data);
uint8_t twi_read(bool ack);

#endif 