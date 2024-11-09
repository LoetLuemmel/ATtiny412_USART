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
    
    uart_send_string("Reading calibration data...\r\n");
    
    // Temperatur-Kalibrierung
    uint8_t val_e9 = bme680_read_register(0xE9);
    uint8_t val_e8 = bme680_read_register(0xE8);
    calib.T1 = (val_e9 << 8) | val_e8;
    
    uart_send_string("Raw T1: E9=0x");
    uart_send_byte((val_e9 >> 4) < 10 ? '0' + (val_e9 >> 4) : 'A' + (val_e9 >> 4) - 10);
    uart_send_byte((val_e9 & 0x0F) < 10 ? '0' + (val_e9 & 0x0F) : 'A' + (val_e9 & 0x0F) - 10);
    uart_send_string(" E8=0x");
    uart_send_byte((val_e8 >> 4) < 10 ? '0' + (val_e8 >> 4) : 'A' + (val_e8 >> 4) - 10);
    uart_send_byte((val_e8 & 0x0F) < 10 ? '0' + (val_e8 & 0x0F) : 'A' + (val_e8 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uint8_t val_8a = bme680_read_register(0x8A);
    uint8_t val_8b = bme680_read_register(0x8B);
    calib.T2 = (val_8b << 8) | val_8a;
    
    uart_send_string("Raw T2: 8B=0x");
    uart_send_byte((val_8b >> 4) < 10 ? '0' + (val_8b >> 4) : 'A' + (val_8b >> 4) - 10);
    uart_send_byte((val_8b & 0x0F) < 10 ? '0' + (val_8b & 0x0F) : 'A' + (val_8b & 0x0F) - 10);
    uart_send_string(" 8A=0x");
    uart_send_byte((val_8a >> 4) < 10 ? '0' + (val_8a >> 4) : 'A' + (val_8a >> 4) - 10);
    uart_send_byte((val_8a & 0x0F) < 10 ? '0' + (val_8a & 0x0F) : 'A' + (val_8a & 0x0F) - 10);
    uart_send_string("\r\n");
    
    calib.T3 = (int8_t)bme680_read_register(0x8C);
    
    uart_send_string("Raw T3: 8C=0x");
    uart_send_byte((calib.T3 >> 4) < 10 ? '0' + (calib.T3 >> 4) : 'A' + (calib.T3 >> 4) - 10);
    uart_send_byte((calib.T3 & 0x0F) < 10 ? '0' + (calib.T3 & 0x0F) : 'A' + (calib.T3 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    return true;
}

// Temperatur messen (in °C * 100)
int16_t bme680_read_temperature(void) {
    // Messung starten
    bme680_write_register(0x74, 0x01);  // osrs_t = 1 (1x oversampling)
    
    uart_send_string("Waiting for measurement...\r\n");
    
    // Warten bis Messung fertig (measuring bit = 0)
    uint8_t status;
    do {
        status = bme680_read_register(0x1D);
        uart_send_string("Status = 0x");
        uart_send_byte((status >> 4) < 10 ? '0' + (status >> 4) : 'A' + (status >> 4) - 10);
        uart_send_byte((status & 0x0F) < 10 ? '0' + (status & 0x0F) : 'A' + (status & 0x0F) - 10);
        uart_send_string("\r\n");
        _delay_ms(10);
    } while (status & 0x20);
    
    uart_send_string("Reading temperature...\r\n");
    
    // ADC-Wert lesen
    uint8_t temp_msb = bme680_read_register(0x22);
    uint8_t temp_lsb = bme680_read_register(0x23);
    uint8_t temp_xlsb = bme680_read_register(0x24);
    
    uart_send_string("MSB = 0x");
    uart_send_byte((temp_msb >> 4) < 10 ? '0' + (temp_msb >> 4) : 'A' + (temp_msb >> 4) - 10);
    uart_send_byte((temp_msb & 0x0F) < 10 ? '0' + (temp_msb & 0x0F) : 'A' + (temp_msb & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uart_send_string("LSB = 0x");
    uart_send_byte((temp_lsb >> 4) < 10 ? '0' + (temp_lsb >> 4) : 'A' + (temp_lsb >> 4) - 10);
    uart_send_byte((temp_lsb & 0x0F) < 10 ? '0' + (temp_lsb & 0x0F) : 'A' + (temp_lsb & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uart_send_string("XLSB = 0x");
    uart_send_byte((temp_xlsb >> 4) < 10 ? '0' + (temp_xlsb >> 4) : 'A' + (temp_xlsb >> 4) - 10);
    uart_send_byte((temp_xlsb & 0x0F) < 10 ? '0' + (temp_xlsb & 0x0F) : 'A' + (temp_xlsb & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uint32_t adc_temp = ((uint32_t)temp_msb << 12) | ((uint32_t)temp_lsb << 4) | (temp_xlsb >> 4);
    
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
    
    uart_send_string("ADC = ");
    uart_send_byte((adc_temp >> 20) < 10 ? '0' + (adc_temp >> 20) : 'A' + (adc_temp >> 20) - 10);
    uart_send_byte(((adc_temp >> 16) & 0x0F) < 10 ? '0' + ((adc_temp >> 16) & 0x0F) : 'A' + ((adc_temp >> 16) & 0x0F) - 10);
    uart_send_byte(((adc_temp >> 12) & 0x0F) < 10 ? '0' + ((adc_temp >> 12) & 0x0F) : 'A' + ((adc_temp >> 12) & 0x0F) - 10);
    uart_send_byte(((adc_temp >> 8) & 0x0F) < 10 ? '0' + ((adc_temp >> 8) & 0x0F) : 'A' + ((adc_temp >> 8) & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // Erste Berechnung
    var1 = ((((adc_temp >> 3) - ((int32_t)calib->T1 << 1))) * ((int32_t)calib->T2)) >> 11;
    
    // Zweite Berechnung
    var2 = (((((adc_temp >> 4) - ((int32_t)calib->T1)) * ((adc_temp >> 4) - ((int32_t)calib->T1))) >> 12) * ((int32_t)calib->T3)) >> 14;
    
    // t_fine berechnen
    t_fine = var1 + var2;
    
    // Temperatur berechnen (in °C * 100)
    return ((t_fine * 5 + 128) >> 8) / 10;
} 