#include "bme680.h"
#include "twi.h"
#include "uart.h"
#include <util/delay.h>

#define BME680_ADDR 0x76

static struct bme680_calib calib;

bool bme680_init(void) {
    // Soft Reset
    bme680_write_register(0xE0, 0xB6);
    _delay_ms(5);
    
    // Temperatur-Kalibrierung
    uint8_t val_e9 = bme680_read_register(0xE9);
    uint8_t val_e8 = bme680_read_register(0xE8);
    calib.T1 = (val_e9 << 8) | val_e8;
    
    uint8_t val_8a = bme680_read_register(0x8A);
    uint8_t val_8b = bme680_read_register(0x8B);
    calib.T2 = (val_8b << 8) | val_8a;
    
    calib.T3 = (int8_t)bme680_read_register(0x8C);
    
    // ... weitere Kalibrierungswerte ...
    
    // Temperatur-Messung konfigurieren
    bme680_write_register(0x74, 0x01);  // osrs_t = 1 (1x oversampling)
    
    return true;
}

// Temperatur messen (in °C * 100)
int16_t bme680_read_temperature(void) {
    // Messung starten
    bme680_write_register(0x74, bme680_read_register(0x74) | 0x20);  // Forced mode
    
    // Warten bis Messung fertig
    while(bme680_read_register(0x1D) & 0x20) {
        _delay_ms(1);
    }
    
    // ADC-Wert lesen
    uint32_t adc_temp = bme680_read_register(0x22) << 12;
    adc_temp |= bme680_read_register(0x23) << 4;
    adc_temp |= bme680_read_register(0x24) >> 4;
    
    // Temperatur berechnen
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