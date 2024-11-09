#include "twi.h"

void twi_init(void) {
    // Set SCL and SDA pins as outputs
    PORTA.DIRSET = PIN6_bm | PIN7_bm;
    
    // Enable pull-ups
    PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL |= PORT_PULLUPEN_bm;
    
    // Set up TWI (100kHz at 3.3MHz CPU)
    TWI0.MBAUD = 15;
    TWI0.MCTRLA = TWI_ENABLE_bm;
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
}

bool twi_start(uint8_t addr) {
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    TWI0.MADDR = addr;
    while (!(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm)));
    return !(TWI0.MSTATUS & TWI_RXACK_bm);
}

void twi_stop(void) {
    TWI0.MCTRLB = TWI_STOP_gc;
}

bool twi_write(uint8_t data) {
    TWI0.MDATA = data;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    return !(TWI0.MSTATUS & TWI_RXACK_bm);
}

uint8_t twi_read(bool ack) {
    while (!(TWI0.MSTATUS & TWI_RIF_bm));
    uint8_t data = TWI0.MDATA;
    TWI0.MCTRLB = ack ? TWI_ACK_gc : TWI_NACK_gc;
    return data;
} 