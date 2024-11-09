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
    
    uart_send_string("T1 = 0x");
    uart_send_byte((calib.T1 >> 12) < 10 ? '0' + (calib.T1 >> 12) : 'A' + (calib.T1 >> 12) - 10);
    uart_send_byte(((calib.T1 >> 8) & 0x0F) < 10 ? '0' + ((calib.T1 >> 8) & 0x0F) : 'A' + ((calib.T1 >> 8) & 0x0F) - 10);
    uart_send_byte(((calib.T1 >> 4) & 0x0F) < 10 ? '0' + ((calib.T1 >> 4) & 0x0F) : 'A' + ((calib.T1 >> 4) & 0x0F) - 10);
    uart_send_byte((calib.T1 & 0x0F) < 10 ? '0' + (calib.T1 & 0x0F) : 'A' + (calib.T1 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uint8_t val_8a = bme680_read_register(0x8A);
    uint8_t val_8b = bme680_read_register(0x8B);
    calib.T2 = (val_8b << 8) | val_8a;
    
    uart_send_string("T2 = 0x");
    uart_send_byte((calib.T2 >> 12) < 10 ? '0' + (calib.T2 >> 12) : 'A' + (calib.T2 >> 12) - 10);
    uart_send_byte(((calib.T2 >> 8) & 0x0F) < 10 ? '0' + ((calib.T2 >> 8) & 0x0F) : 'A' + ((calib.T2 >> 8) & 0x0F) - 10);
    uart_send_byte(((calib.T2 >> 4) & 0x0F) < 10 ? '0' + ((calib.T2 >> 4) & 0x0F) : 'A' + ((calib.T2 >> 4) & 0x0F) - 10);
    uart_send_byte((calib.T2 & 0x0F) < 10 ? '0' + (calib.T2 & 0x0F) : 'A' + (calib.T2 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    calib.T3 = (int8_t)bme680_read_register(0x8C);
    
    uart_send_string("T3 = 0x");
    uart_send_byte((calib.T3 >> 4) < 10 ? '0' + (calib.T3 >> 4) : 'A' + (calib.T3 >> 4) - 10);
    uart_send_byte((calib.T3 & 0x0F) < 10 ? '0' + (calib.T3 & 0x0F) : 'A' + (calib.T3 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // Temperatur-Messung konfigurieren
    bme680_write_register(0x74, 0x01);  // osrs_t = 1 (1x oversampling)
    
    return true;
}

// Temperatur messen (in °C * 100)
int16_t bme680_read_temperature(void) {
    // Messung starten
    bme680_write_register(0x74, 0x21);  // Forced mode + 1x oversampling
    
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
    int64_t var1, var2, var3;
    
    uart_send_string("ADC = ");
    uart_send_byte((adc_temp >> 28) < 10 ? '0' + (adc_temp >> 28) : 'A' + (adc_temp >> 28) - 10);
    uart_send_byte(((adc_temp >> 24) & 0x0F) < 10 ? '0' + ((adc_temp >> 24) & 0x0F) : 'A' + ((adc_temp >> 24) & 0x0F) - 10);
    uart_send_byte(((adc_temp >> 20) & 0x0F) < 10 ? '0' + ((adc_temp >> 20) & 0x0F) : 'A' + ((adc_temp >> 20) & 0x0F) - 10);
    uart_send_byte(((adc_temp >> 16) & 0x0F) < 10 ? '0' + ((adc_temp >> 16) & 0x0F) : 'A' + ((adc_temp >> 16) & 0x0F) - 10);
    uart_send_byte(((adc_temp >> 12) & 0x0F) < 10 ? '0' + ((adc_temp >> 12) & 0x0F) : 'A' + ((adc_temp >> 12) & 0x0F) - 10);
    uart_send_byte(((adc_temp >> 8) & 0x0F) < 10 ? '0' + ((adc_temp >> 8) & 0x0F) : 'A' + ((adc_temp >> 8) & 0x0F) - 10);
    uart_send_string("\r\n");
    
    var1 = ((int32_t)adc_temp >> 3) - ((int32_t)calib->T1 << 1);
    uart_send_string("var1 = ");
    uart_send_byte((var1 >> 28) < 10 ? '0' + (var1 >> 28) : 'A' + (var1 >> 28) - 10);
    uart_send_byte(((var1 >> 24) & 0x0F) < 10 ? '0' + ((var1 >> 24) & 0x0F) : 'A' + ((var1 >> 24) & 0x0F) - 10);
    uart_send_byte(((var1 >> 20) & 0x0F) < 10 ? '0' + ((var1 >> 20) & 0x0F) : 'A' + ((var1 >> 20) & 0x0F) - 10);
    uart_send_byte(((var1 >> 16) & 0x0F) < 10 ? '0' + ((var1 >> 16) & 0x0F) : 'A' + ((var1 >> 16) & 0x0F) - 10);
    uart_send_string("\r\n");
    
    var2 = (var1 * (int32_t)calib->T2) >> 11;
    uart_send_string("var2 = ");
    uart_send_byte((var2 >> 28) < 10 ? '0' + (var2 >> 28) : 'A' + (var2 >> 28) - 10);
    uart_send_byte(((var2 >> 24) & 0x0F) < 10 ? '0' + ((var2 >> 24) & 0x0F) : 'A' + ((var2 >> 24) & 0x0F) - 10);
    uart_send_byte(((var2 >> 20) & 0x0F) < 10 ? '0' + ((var2 >> 20) & 0x0F) : 'A' + ((var2 >> 20) & 0x0F) - 10);
    uart_send_byte(((var2 >> 16) & 0x0F) < 10 ? '0' + ((var2 >> 16) & 0x0F) : 'A' + ((var2 >> 16) & 0x0F) - 10);
    uart_send_string("\r\n");
    
    var3 = ((var1 >> 1) * (var1 >> 1)) >> 12;
    var3 = ((var3) * ((int32_t)calib->T3 << 4)) >> 14;
    
    return (var2 + var3) / 100;
} 