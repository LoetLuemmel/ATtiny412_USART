#include "bme680.h"
#include "twi.h"
#include "uart.h"

#define BME680_ADDR 0x76

static struct bme680_calib calib;  // Globale Variable für Kalibrierungswerte

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

bool bme680_init(void) {
    uart_send_string("Reading calibration...\r\n");
    
    // Temperatur-Kalibrierung
    uint8_t val_e9 = bme680_read_register(0xE9);
    uint8_t val_e8 = bme680_read_register(0xE8);
    calib.T1 = (val_e9 << 8) | val_e8;
    
    uart_send_string("T1 = ");
    uart_send_byte((calib.T1 >> 12) < 10 ? '0' + (calib.T1 >> 12) : 'A' + (calib.T1 >> 12) - 10);
    uart_send_byte(((calib.T1 >> 8) & 0x0F) < 10 ? '0' + ((calib.T1 >> 8) & 0x0F) : 'A' + ((calib.T1 >> 8) & 0x0F) - 10);
    uart_send_byte(((calib.T1 >> 4) & 0x0F) < 10 ? '0' + ((calib.T1 >> 4) & 0x0F) : 'A' + ((calib.T1 >> 4) & 0x0F) - 10);
    uart_send_byte((calib.T1 & 0x0F) < 10 ? '0' + (calib.T1 & 0x0F) : 'A' + (calib.T1 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uint8_t val_8a = bme680_read_register(0x8A);
    uint8_t val_8b = bme680_read_register(0x8B);
    calib.T2 = (val_8b << 8) | val_8a;
    
    uart_send_string("T2 = ");
    if (calib.T2 < 0) {
        uart_send_string("-");
        calib.T2 = -calib.T2;
    }
    uart_send_byte((calib.T2 >> 12) < 10 ? '0' + (calib.T2 >> 12) : 'A' + (calib.T2 >> 12) - 10);
    uart_send_byte(((calib.T2 >> 8) & 0x0F) < 10 ? '0' + ((calib.T2 >> 8) & 0x0F) : 'A' + ((calib.T2 >> 8) & 0x0F) - 10);
    uart_send_byte(((calib.T2 >> 4) & 0x0F) < 10 ? '0' + ((calib.T2 >> 4) & 0x0F) : 'A' + ((calib.T2 >> 4) & 0x0F) - 10);
    uart_send_byte((calib.T2 & 0x0F) < 10 ? '0' + (calib.T2 & 0x0F) : 'A' + (calib.T2 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    return true;
}

// Funktion zum Abrufen der Kalibrierungswerte
struct bme680_calib* bme680_get_calib(void) {
    return &calib;
} 