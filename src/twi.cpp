#include <avr/io.h>
#include <util/delay.h>
#include "twi.h"
#include "uart.h"

void twi_init(void) {
    uart_send_string("Initializing TWI...\r\n");
    
    // TWI0 Pins konfigurieren (PA2=SDA, PA3=SCL)
    PORTA.DIRSET = PIN2_bm | PIN3_bm;
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm;  // Pull-up für SDA
    PORTA.PIN3CTRL = PORT_PULLUPEN_bm;  // Pull-up für SCL
    
    // TWI0 konfigurieren
    TWI0.MBAUD = 12;        // 100kHz bei 3.3MHz
    TWI0.MCTRLA = TWI_ENABLE_bm;
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    
    uart_send_string("TWI initialized\r\n");
}

bool twi_start(uint8_t addr) {
    // Bus zurücksetzen wenn er nicht idle ist
    if ((TWI0.MSTATUS & TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc) {
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
        _delay_ms(1);
        TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    }
    
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
    
    if (timeout == 0 || (TWI0.MSTATUS & TWI_RXACK_bm)) {
        uart_send_string("TWI: Start failed\r\n");
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
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
    uart_send_string("TWI: Waiting for data...\r\n");
    
    // Warten auf Daten mit Timeout
    uint16_t timeout = 1000;
    while (!(TWI0.MSTATUS & TWI_RIF_bm) && timeout > 0) {
        _delay_us(1);
        timeout--;
    }
    
    if (timeout == 0) {
        uart_send_string("TWI: Timeout waiting for data!\r\n");
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
        return 0;
    }
    
    uint8_t data = TWI0.MDATA;
    
    uart_send_string("TWI: Received data 0x");
    uart_send_byte((data >> 4) < 10 ? '0' + (data >> 4) : 'A' + (data >> 4) - 10);
    uart_send_byte((data & 0x0F) < 10 ? '0' + (data & 0x0F) : 'A' + (data & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // ACK/NACK senden
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