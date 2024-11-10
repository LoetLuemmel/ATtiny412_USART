#include <avr/io.h>
#include "twi.h"
#include "uart.h"

void twi_init(void) {
    uart_send_string("Initializing TWI...\r\n");
    
    PORTA.DIRSET = PIN2_bm | PIN3_bm;
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
    
    TWI0.MBAUD = 12;
    TWI0.MCTRLA = TWI_ENABLE_bm;
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    
    uart_send_string("TWI initialized\r\n");
}

bool twi_start(uint8_t addr) {
    TWI0.MADDR = addr;
    
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    
    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        uart_send_string("TWI: No ACK received!\r\n");
        return false;
    }
    return true;
}

bool twi_write(uint8_t data) {
    TWI0.MDATA = data;
    
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    
    bool success = !(TWI0.MSTATUS & TWI_RXACK_bm);
    if (!success) {
        uart_send_string("TWI: Write failed!\r\n");
    }
    return success;
}

uint8_t twi_read(bool ack) {
    while (!(TWI0.MSTATUS & TWI_RIF_bm));
    
    uint8_t data = TWI0.MDATA;
    
    TWI0.MCTRLB = ack ? TWI_MCMD_RECVTRANS_gc : TWI_MCMD_STOP_gc;
    
    return data;
}

void twi_stop(void) {
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;
    
    while (TWI0.MSTATUS & TWI_BUSSTATE_BUSY_gc);
} 