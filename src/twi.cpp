#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "twi.h"

void twi_init(void) {
    // TWI Pins konfigurieren
    PORTA.DIRSET = PIN3_bm;     // SCL Output
    PORTA.DIRCLR = PIN2_bm;     // SDA Input
    PORTA.PIN2CTRL |= PORT_PULLUPEN_bm;  // SDA Pull-up
    PORTA.PIN3CTRL |= PORT_PULLUPEN_bm;  // SCL Pull-up
    
    // TWI Master initialisieren (100kHz)
    TWI0.MBAUD = (uint8_t)((F_CPU / (2 * 100000)) - 5);
    TWI0.MCTRLA = TWI_ENABLE_bm;
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
}

void twi_scan(void) {
    printf("\r\nI2C Scanner\r\n");
    printf("Scanning...\r\n");
    
    uint8_t devices = 0;
    
    for (uint8_t addr = 1; addr < 127; addr++) {
        TWI0.MADDR = (addr << 1) | 0;  // Write-Bit
        
        while (!(TWI0.MSTATUS & (TWI_WIF_bm | TWI_ARBLOST_bm)));
        
        if (!(TWI0.MSTATUS & TWI_RXACK_bm)) {
            printf("Device found at address 0x%02X\r\n", addr);
            devices++;
        }
        
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
    }
    
    printf("\r\nScan complete. Found %d devices.\r\n", devices);
} 