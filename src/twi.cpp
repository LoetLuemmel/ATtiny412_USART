#include <avr/io.h>
#include <util/delay.h>
#include "twi.h"
#include "uart.h"

#define BME680_ADDR_LOW  0x76  // SDO pin = GND
#define BME680_ADDR_HIGH 0x77  // SDO pin = VDD

void twi_init(void) {
    uart_send_string("Initializing TWI...\r\n");
    
    // TWI0 Pins konfigurieren (PA1=SDA, PA2=SCL)
    PORTA.DIRCLR = PIN1_bm | PIN2_bm;  // Als Eingang konfigurieren
    
    // TWI neu konfigurieren
    TWI0.MBAUD = 16;        // 100kHz bei 3.3MHz CPU
    TWI0.MCTRLA = TWI_ENABLE_bm;
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    
    // Teste beide möglichen Adressen
    if (twi_start(BME680_ADDR_LOW << 1)) {
        uart_send_string("BME680 found at address 0x76\r\n");
        twi_stop();
    } else if (twi_start(BME680_ADDR_HIGH << 1)) {
        uart_send_string("BME680 found at address 0x77\r\n");
        twi_stop();
    } else {
        uart_send_string("No BME680 found!\r\n");
    }
    
    uart_send_string("TWI initialized\r\n");
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
    
    if (TWI0.MSTATUS & TWI_RXACK_bm) {
        uart_send_string("TWI: No ACK received!\r\n");
        return false;
    }
    
    uart_send_string("TWI: Write successful\r\n");
    return true;
}

uint8_t twi_read(bool ack) {
    uart_send_string("TWI: Waiting for data...\r\n");
    
    // Warten auf RIF
    _delay_us(100);  // Kurze Pause vor dem Lesen
    
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
    while (!(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm)) && timeout > 0) {
        _delay_us(1);
        timeout--;
    }
    
    if (timeout == 0 || (TWI0.MSTATUS & TWI_RXACK_bm)) {
        uart_send_string("TWI: Start failed\r\n");
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
        _delay_ms(1);
        return false;
    }
    
    uart_send_string("TWI: Start successful\r\n");
    return true;
}

bool bme680_read_register(uint8_t reg, uint8_t *data) {
    uart_send_string("TWI: Reading register 0x");
    uart_send_byte((reg >> 4) < 10 ? '0' + (reg >> 4) : 'A' + (reg >> 4) - 10);
    uart_send_byte((reg & 0x0F) < 10 ? '0' + (reg & 0x0F) : 'A' + (reg & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // Schreibe Register-Adresse
    if (!twi_start(BME680_ADDR_LOW << 1)) {
        uart_send_string("TWI: Start (write) failed\r\n");
        return false;
    }
    
    if (!twi_write(reg)) {
        uart_send_string("TWI: Register write failed\r\n");
        twi_stop();
        return false;
    }
    
    // Repeated Start für Lesen
    _delay_us(100);  // Kurze Pause vor Repeated Start
    if (!twi_start((BME680_ADDR_LOW << 1) | 1)) {
        uart_send_string("TWI: Start (read) failed\r\n");
        twi_stop();
        return false;
    }
    
    // Lese Daten
    *data = twi_read(false);  // NACK und STOP
    return true;
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