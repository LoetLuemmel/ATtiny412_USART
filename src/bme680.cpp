#include "bme680.h"
#include "twi.h"
#include "uart.h"
#include <util/delay.h>

#define BME680_ADDR 0x76

static struct bme680_calib calib;

bool bme680_init(void) {
    uart_send_string("Performing soft reset...\r\n");
    
    // Soft Reset
    bme680_write_register(0xE0, 0xB6);
    _delay_ms(5);
    
    uart_send_string("Reading chip ID...\r\n");
    uint8_t chip_id = bme680_read_register(0xD0);
    uart_send_string("Chip ID: 0x");
    uart_send_byte((chip_id >> 4) < 10 ? '0' + (chip_id >> 4) : 'A' + (chip_id >> 4) - 10);
    uart_send_byte((chip_id & 0x0F) < 10 ? '0' + (chip_id & 0x0F) : 'A' + (chip_id & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // if (chip_id != 0x61) {  // Auskommentiert - akzeptiere auch 0x60
    //     uart_send_string("Error: Wrong chip ID!\r\n");
    //     return false;
    // }
    
    uart_send_string("Reading calibration data...\r\n");
    
    // Temperatur-Kalibrierung
    uint8_t val_e9 = bme680_read_register(0xE9);
    uint8_t val_e8 = bme680_read_register(0xE8);
    calib.T1 = (val_e9 << 8) | val_e8;
    
    uint8_t val_8a = bme680_read_register(0x8A);
    uint8_t val_8b = bme680_read_register(0x8B);
    calib.T2 = (val_8b << 8) | val_8a;
    
    calib.T3 = (int8_t)bme680_read_register(0x8C);
    
    // Temperatur-Messung konfigurieren
    bme680_write_register(0x74, 0x01);  // osrs_t = 1 (1x oversampling)
    
    uart_send_string("Initialization complete\r\n");
    return true;
}

// Temperatur messen (in °C * 100)
int16_t bme680_read_temperature(void) {
    // Messung starten
    bme680_write_register(0x74, 0x21);  // Forced mode + 1x oversampling
    
    // Warten bis Messung fertig (measuring bit = 0)
    while(bme680_read_register(0x1D) & 0x20) {
        _delay_ms(10);
    }
    
    // ADC-Wert lesen
    uint32_t adc_temp = ((uint32_t)bme680_read_register(0x22) << 12) | 
                       ((uint32_t)bme680_read_register(0x23) << 4) | 
                       (bme680_read_register(0x24) >> 4);
    
    return bme680_calc_temperature(adc_temp, &calib);
}

void bme680_write_register(uint8_t reg, uint8_t value) {
    twi_start(BME680_ADDR << 1);
    twi_write(reg);
    twi_write(value);
    twi_stop();
    twi_init();
}

uint8_t bme680_read_register(uint8_t reg) {
    uint8_t value;
    twi_start(BME680_ADDR << 1);
    twi_write(reg);
    twi_start((BME680_ADDR << 1) | 1);
    value = twi_read(false);
    twi_stop();
    twi_init();
    return value;
}

// Funktion zum Abrufen der Kalibrierungswerte
struct bme680_calib* bme680_get_calib(void) {
    return &calib;
}

int16_t bme680_calc_temperature(uint32_t adc_temp, struct bme680_calib *calib) {
    int32_t var1, var2, t_fine;
    
    // Erste Berechnung
    var1 = ((((adc_temp >> 3) - ((int32_t)calib->T1 << 1))) * ((int32_t)calib->T2)) >> 11;
    
    // Zweite Berechnung
    var2 = (((((adc_temp >> 4) - ((int32_t)calib->T1)) * ((adc_temp >> 4) - ((int32_t)calib->T1))) >> 12) * ((int32_t)calib->T3)) >> 14;
    
    // t_fine berechnen
    t_fine = var1 + var2;
    
    // Temperatur berechnen (in °C * 100)
    return ((t_fine * 5 + 128) >> 8) / 10;
} 