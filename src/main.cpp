#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"

struct bme680_calib {
    uint16_t par_t1;
    int16_t par_t2;
    int8_t par_t3;
};

int32_t t_fine;

int16_t calc_temp(uint32_t temp_raw, struct bme680_calib *calib) {
    int64_t var1, var2, var3;
    var1 = ((int64_t)temp_raw >> 3) - ((int64_t)calib->par_t1 << 1);
    var2 = (var1 * (int64_t)calib->par_t2) >> 11;
    var3 = ((var1 >> 1) * (var1 >> 1)) >> 12;
    var3 = ((var3) * ((int64_t)calib->par_t3 << 4)) >> 14;
    t_fine = (int32_t)(var2 + var3);
    return (int16_t)((t_fine * 5 + 128) >> 8);
}

void twi_init(void) {
    TWI0.MCTRLA = 0;                     // Disable TWI
    _delay_us(10);
    
    PORTA.DIRCLR = PIN1_bm | PIN2_bm;    // Release TWI pins
    PORTA.PIN1CTRL = 0;                  // No pullup for SCL
    PORTA.PIN2CTRL = 0;                  // No pullup for SDA
    
    TWI0.MBAUD = 5;                      // 100kHz @ 3.33MHz
    TWI0.MCTRLA = TWI_ENABLE_bm;         // Enable TWI
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc; // Force IDLE state
    
    _delay_us(10);                       // Wait a bit
}

uint8_t read_register(uint8_t reg) {
    // Write register address
    TWI0.MADDR = 0x76 << 1;              // Write address
    while (!(TWI0.MSTATUS & TWI_WIF_bm)) { ; }
    
    TWI0.MDATA = reg;                    // Register to read
    while (!(TWI0.MSTATUS & TWI_WIF_bm)) { ; }
    
    // Read register value
    TWI0.MADDR = (0x76 << 1) | 1;        // Read address
    while (!(TWI0.MSTATUS & TWI_RIF_bm)) { ; }
    
    uint8_t data = TWI0.MDATA;           // Read data
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;      // Send STOP
    
    // Re-init TWI
    twi_init();
    
    return data;
}

void write_register(uint8_t reg, uint8_t value) {
    // Write register address
    TWI0.MADDR = 0x76 << 1;              // Write address
    while (!(TWI0.MSTATUS & TWI_WIF_bm)) { ; }
    
    TWI0.MDATA = reg;                    // Register to write
    while (!(TWI0.MSTATUS & TWI_WIF_bm)) { ; }
    
    TWI0.MDATA = value;                  // Write data
    while (!(TWI0.MSTATUS & TWI_WIF_bm)) { ; }
    
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;      // Send STOP
    
    // Re-init TWI
    twi_init();
}

int main(void) {
    uart_init();
    twi_init();
    uart_send_string("BME\r\n");
    
    write_register(0xE0, 0xB6);  // Reset
    _delay_ms(2);
    
    struct bme680_calib calib;
    calib.par_t1 = read_register(0xE9) << 8 | read_register(0xE8);
    calib.par_t2 = (int16_t)(read_register(0x8A) << 8 | read_register(0x89));
    calib.par_t3 = (int8_t)read_register(0x8C);
    
    write_register(0x74, 0x24);  // Temp x1, Sleep mode
    _delay_ms(50);
    
    while(1) {
        write_register(0x74, 0x25);  // Start measurement
        
        uint8_t status;
        do {
            status = read_register(0x1D);
            _delay_ms(10);
        } while (status & 0x20);
        
        uint32_t temp_raw = (read_register(0x22) << 12) | 
                           (read_register(0x23) << 4) | 
                           (read_register(0x24) >> 4);
        
        int16_t temp = calc_temp(temp_raw, &calib);
        
        uart_send_string("T:");
        if(temp < 0) {
            uart_send_string("-");
            temp = -temp;
        }
        uart_send_byte('0' + (temp/1000) % 10);
        uart_send_byte('0' + (temp/100) % 10);
        uart_send_byte('.');
        uart_send_byte('0' + (temp/10) % 10);
        uart_send_string("\r\n");
        
        _delay_ms(10000);
    }
}