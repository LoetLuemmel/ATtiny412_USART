#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "twi.h"

#define BME680_ADDR 0x76

// Struct für Kalibrierungsdaten
struct bme680_calib {
    uint16_t par_t1;
    int16_t par_t2;
    int8_t par_t3;
};

// Globale Variable für t_fine
int32_t t_fine;

// Funktionsdeklarationen
bool write_register(uint8_t reg, uint8_t data);
uint8_t read_register(uint8_t reg);
int16_t calc_temp(uint32_t temp_raw, struct bme680_calib *calib);

// Funktionsimplementierungen
bool write_register(uint8_t reg, uint8_t data) {
    if (!twi_start(BME680_ADDR << 1)) {
        uart_send_string("Error: BME680 not responding!\r\n");
        return false;
    }
    if (!twi_write(reg)) {
        twi_stop();
        return false;
    }
    if (!twi_write(data)) {
        twi_stop();
        return false;
    }
    twi_stop();
    return true;
}

uint8_t read_register(uint8_t reg) {
    if (!twi_start(BME680_ADDR << 1)) {
        uart_send_string("Error: BME680 not responding!\r\n");
        return 0;
    }
    if (!twi_write(reg)) {
        twi_stop();
        return 0;
    }
    if (!twi_start((BME680_ADDR << 1) | 1)) {
        twi_stop();
        return 0;
    }
    uint8_t data = twi_read(false);
    twi_stop();
    return data;
}

int16_t calc_temp(uint32_t temp_raw, struct bme680_calib *calib) {
    int64_t var1, var2, var3;
    var1 = ((int64_t)temp_raw >> 3) - ((int64_t)calib->par_t1 << 1);
    var2 = (var1 * (int64_t)calib->par_t2) >> 11;
    var3 = ((var1 >> 1) * (var1 >> 1)) >> 12;
    var3 = ((var3) * ((int64_t)calib->par_t3 << 4)) >> 14;
    t_fine = (int32_t)(var2 + var3);
    return (int16_t)((t_fine * 5 + 128) >> 8);
}

int main(void) {
    uart_init();
    twi_init();
    
    uart_send_string("\r\nBME680 Test\r\n");
    uart_send_string("-----------\r\n");
    
    // Test if sensor responds
    uart_send_string("Scanning for BME680 at address 0x");
    uart_send_byte((BME680_ADDR >> 4) < 10 ? '0' + (BME680_ADDR >> 4) : 'A' + (BME680_ADDR >> 4) - 10);
    uart_send_byte((BME680_ADDR & 0x0F) < 10 ? '0' + (BME680_ADDR & 0x0F) : 'A' + (BME680_ADDR & 0x0F) - 10);
    uart_send_string("...\r\n");
    
    if (!twi_start(BME680_ADDR << 1)) {
        uart_send_string("Error: No BME680 sensor found!\r\n");
        while(1);  // Stop here
    }
    twi_stop();
    uart_send_string("BME680 found!\r\n\n");
    
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