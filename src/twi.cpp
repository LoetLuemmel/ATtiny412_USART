#include "twi.h"
#include "uart.h"
#include <util/delay.h>

void twi_init(void) {
    PORTMUX.CTRLB |= 1 << 2;              // TWI0 alternative position bit
    PORTA.DIRCLR = PIN1_bm | PIN2_bm;     // Release TWI pins
    
    TWI0.MBAUD = 15;                      // 100kHz bei 4MHz CPU
    TWI0.MCTRLA = TWI_ENABLE_bm;          // TWI Enable
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;  // Bus in IDLE Zustand
}

bool twi_start(uint8_t addr) {
    // Reset bus state
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    _delay_us(10);  // Kurze Pause
    
    uart_send_string("(status=0x");
    uart_send_byte((TWI0.MSTATUS >> 4) < 10 ? '0' + (TWI0.MSTATUS >> 4) : 'A' + (TWI0.MSTATUS >> 4) - 10);
    uart_send_byte((TWI0.MSTATUS & 0x0F) < 10 ? '0' + (TWI0.MSTATUS & 0x0F) : 'A' + (TWI0.MSTATUS & 0x0F) - 10);
    uart_send_string(") ");

    // Clear all flags
    TWI0.MSTATUS |= TWI_WIF_bm | TWI_RIF_bm;
    
    TWI0.MADDR = addr;
    
    uint16_t timeout = 1000;
    while (!(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))) {
        timeout--;
        if (timeout == 0) {
            uart_send_string("timeout ");
            return false;
        }
    }
    
    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        uart_send_string("nack ");
        return false;
    }
    
    return true;
}

void twi_stop(void) {
    uart_send_string("(stop:");
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;
    _delay_us(10);
    
    // TWI neu initialisieren
    TWI0.MCTRLA = 0;  // TWI ausschalten
    _delay_us(10);
    
    // Pins als Eingänge für Open-Drain
    PORTA.DIRCLR = PIN1_bm | PIN2_bm;     // Release TWI pins
    PORTA.OUTCLR = PIN1_bm | PIN2_bm;     // Ausgangsregister auf 0
    
    // TWI neu konfigurieren
    PORTMUX.CTRLB |= 1 << 2;              // TWI0 alternative position bit
    TWI0.MBAUD = 15;                      // 100kHz bei 4MHz CPU
    TWI0.MCTRLA = TWI_ENABLE_bm;          // TWI Enable
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;  // Bus in IDLE Zustand
    
    uart_send_string("reinit) ");
}

bool twi_write(uint8_t data) {
    TWI0.MDATA = data;
    while (!(TWI0.MSTATUS & TWI_WIF_bm));
    return !(TWI0.MSTATUS & TWI_RXACK_bm);
}

uint8_t twi_read(bool ack) {
    uart_send_string("(wait_read:");
    
    uint16_t timeout = 1000;
    while (!(TWI0.MSTATUS & TWI_RIF_bm)) {
        timeout--;
        if (timeout == 0) {
            uart_send_string("timeout) ");
            return 0;
        }
    }
    
    uart_send_string("ok) ");
    uint8_t data = TWI0.MDATA;
    
    uart_send_string("(cmd:");
    TWI0.MCTRLB = ack ? TWI_MCMD_RECVTRANS_gc : TWI_MCMD_NOACT_gc;
    uart_send_string("ok) ");
    
    return data;
} 