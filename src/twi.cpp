#include <avr/io.h>
#include <util/delay.h>
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
    uart_send_string("TWI: Start with addr 0x");
    uart_send_byte((addr >> 4) < 10 ? '0' + (addr >> 4) : 'A' + (addr >> 4) - 10);
    uart_send_byte((addr & 0x0F) < 10 ? '0' + (addr & 0x0F) : 'A' + (addr & 0x0F) - 10);
    uart_send_string("\r\n");
    
    TWI0.MADDR = addr;
    
    // Warten auf RXACK mit Timeout
    uint16_t timeout = 1000;
    while (!(TWI0.MSTATUS & TWI_WIF_bm) && timeout > 0) {
        _delay_us(1);
        timeout--;
    }
    
    if (timeout == 0) {
        uart_send_string("TWI: Timeout waiting for ACK!\r\n");
        return false;
    }
    
    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        uart_send_string("TWI: No ACK received!\r\n");
        return false;
    }
    
    uart_send_string("TWI: Start successful\r\n");
    return true;
}

bool twi_write(uint8_t data) {
    uart_send_string("TWI: Writing data 0x");
    uart_send_byte((data >> 4) < 10 ? '0' + (data >> 4) : 'A' + (data >> 4) - 10);
    uart_send_byte((data & 0x0F) < 10 ? '0' + (data & 0x0F) : 'A' + (data & 0x0F) - 10);
    uart_send_string("\r\n");
    
    TWI0.MDATA = data;
    
    // Warten auf Write Complete mit Timeout
    uint16_t timeout = 1000;
    while (!(TWI0.MSTATUS & TWI_WIF_bm) && timeout > 0) {
        _delay_us(1);
        timeout--;
    }
    
    if (timeout == 0) {
        uart_send_string("TWI: Timeout waiting for write!\r\n");
        return false;
    }
    
    bool success = !(TWI0.MSTATUS & TWI_RXACK_bm);
    if (!success) {
        uart_send_string("TWI: Write failed!\r\n");
    } else {
        uart_send_string("TWI: Write successful\r\n");
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
    uart_send_string("TWI: Sending stop condition\r\n");
    
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;
    
    // Warten bis Bus frei ist mit Timeout
    uint16_t timeout = 1000;
    while ((TWI0.MSTATUS & TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc && timeout > 0) {
        _delay_us(1);
        timeout--;
    }
    
    if (timeout == 0) {
        uart_send_string("TWI: Timeout waiting for bus idle!\r\n");
    } else {
        uart_send_string("TWI: Stop successful\r\n");
    }
} 