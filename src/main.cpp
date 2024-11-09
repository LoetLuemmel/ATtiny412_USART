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
    uart_send_string("\r\nReading 0x");
    uart_send_byte((reg >> 4) < 10 ? '0' + (reg >> 4) : 'A' + (reg >> 4) - 10);
    uart_send_byte((reg & 0x0F) < 10 ? '0' + (reg & 0x0F) : 'A' + (reg & 0x0F) - 10);
    uart_send_string("...");
    
    if (!twi_start(BME680_ADDR << 1)) {
        uart_send_string("S1 fail ");
        return 0;
    }
    uart_send_string("S1 W ");
    
    if (!twi_write(reg)) {
        uart_send_string("W fail ");
        twi_stop();
        return 0;
    }
    
    _delay_us(100);  // Pause nach dem Write
    
    if (!twi_start((BME680_ADDR << 1) | 1)) {
        uart_send_string("S2 fail ");
        twi_stop();
        return 0;
    }
    uart_send_string("S2 ");
    
    uint8_t data = twi_read(false);
    uart_send_string("R ");
    
    twi_stop();
    uart_send_string("P got 0x");
    uart_send_byte((data >> 4) < 10 ? '0' + (data >> 4) : 'A' + (data >> 4) - 10);
    uart_send_byte((data & 0x0F) < 10 ? '0' + (data & 0x0F) : 'A' + (data & 0x0F) - 10);
    uart_send_string("\r\n");
    
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
        while(1);
    }
    twi_stop();
    uart_send_string("BME680 found at address 0x");
    uart_send_byte((BME680_ADDR >> 4) < 10 ? '0' + (BME680_ADDR >> 4) : 'A' + (BME680_ADDR >> 4) - 10);
    uart_send_byte((BME680_ADDR & 0x0F) < 10 ? '0' + (BME680_ADDR & 0x0F) : 'A' + (BME680_ADDR & 0x0F) - 10);
    uart_send_string("\r\n\n");

    uart_send_string("Resetting...\r\n");
    // Soft reset
    write_register(0xE0, 0xB6);
    _delay_ms(2);
    
    uart_send_string("Reading calibration...\r\n");
    struct bme680_calib calib;
    
    // T1 (0xE9/0xE8)
    uint8_t val_e9 = read_register(0xE9);
    uint8_t val_e8 = read_register(0xE8);
    uint16_t T1 = (val_e9 << 8) | val_e8;
    uart_send_string("T1 = ");
    uart_send_byte((T1 >> 12) < 10 ? '0' + (T1 >> 12) : 'A' + (T1 >> 12) - 10);
    uart_send_byte(((T1 >> 8) & 0x0F) < 10 ? '0' + ((T1 >> 8) & 0x0F) : 'A' + ((T1 >> 8) & 0x0F) - 10);
    uart_send_byte(((T1 >> 4) & 0x0F) < 10 ? '0' + ((T1 >> 4) & 0x0F) : 'A' + ((T1 >> 4) & 0x0F) - 10);
    uart_send_byte((T1 & 0x0F) < 10 ? '0' + (T1 & 0x0F) : 'A' + (T1 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // T2 (0x8A/0x8B)
    uint8_t val_8a = read_register(0x8A);
    uint8_t val_8b = read_register(0x8B);
    int16_t T2 = (val_8b << 8) | val_8a;
    uart_send_string("T2 = ");
    if (T2 < 0) {
        uart_send_string("-");
        T2 = -T2;
    }
    uart_send_byte((T2 >> 12) < 10 ? '0' + (T2 >> 12) : 'A' + (T2 >> 12) - 10);
    uart_send_byte(((T2 >> 8) & 0x0F) < 10 ? '0' + ((T2 >> 8) & 0x0F) : 'A' + ((T2 >> 8) & 0x0F) - 10);
    uart_send_byte(((T2 >> 4) & 0x0F) < 10 ? '0' + ((T2 >> 4) & 0x0F) : 'A' + ((T2 >> 4) & 0x0F) - 10);
    uart_send_byte((T2 & 0x0F) < 10 ? '0' + (T2 & 0x0F) : 'A' + (T2 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // T3 (0x8C/0x8D)
    uint8_t val_8c = read_register(0x8C);
    uint8_t val_8d = read_register(0x8D);
    int8_t T3 = val_8d;  // Nur das MSB ist relevant (signed 8-bit)
    
    uart_send_string("T3 = ");
    if (T3 < 0) {
        uart_send_string("-");
        T3 = -T3;
    }
    uart_send_byte((T3 >> 4) < 10 ? '0' + (T3 >> 4) : 'A' + (T3 >> 4) - 10);
    uart_send_byte((T3 & 0x0F) < 10 ? '0' + (T3 & 0x0F) : 'A' + (T3 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P1 (0x8E/0x8F)
    uint8_t val_8e = read_register(0x8E);
    uint8_t val_8f = read_register(0x8F);
    uint16_t P1 = (val_8f << 8) | val_8e;
    
    uart_send_string("P1 = ");
    uart_send_byte((P1 >> 12) < 10 ? '0' + (P1 >> 12) : 'A' + (P1 >> 12) - 10);
    uart_send_byte(((P1 >> 8) & 0x0F) < 10 ? '0' + ((P1 >> 8) & 0x0F) : 'A' + ((P1 >> 8) & 0x0F) - 10);
    uart_send_byte(((P1 >> 4) & 0x0F) < 10 ? '0' + ((P1 >> 4) & 0x0F) : 'A' + ((P1 >> 4) & 0x0F) - 10);
    uart_send_byte((P1 & 0x0F) < 10 ? '0' + (P1 & 0x0F) : 'A' + (P1 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P2 (0x90/0x91)
    uint8_t val_90 = read_register(0x90);
    uint8_t val_91 = read_register(0x91);
    int16_t P2 = (val_91 << 8) | val_90;
    
    uart_send_string("P2 = ");
    if (P2 < 0) {
        uart_send_string("-");
        P2 = -P2;
    }
    uart_send_byte((P2 >> 12) < 10 ? '0' + (P2 >> 12) : 'A' + (P2 >> 12) - 10);
    uart_send_byte(((P2 >> 8) & 0x0F) < 10 ? '0' + ((P2 >> 8) & 0x0F) : 'A' + ((P2 >> 8) & 0x0F) - 10);
    uart_send_byte(((P2 >> 4) & 0x0F) < 10 ? '0' + ((P2 >> 4) & 0x0F) : 'A' + ((P2 >> 4) & 0x0F) - 10);
    uart_send_byte((P2 & 0x0F) < 10 ? '0' + (P2 & 0x0F) : 'A' + (P2 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P3 (0x92/0x93)
    uint8_t val_92 = read_register(0x92);
    uint8_t val_93 = read_register(0x93);
    int16_t P3 = (val_93 << 8) | val_92;
    
    uart_send_string("P3 = ");
    if (P3 < 0) {
        uart_send_string("-");
        P3 = -P3;
    }
    uart_send_byte((P3 >> 12) < 10 ? '0' + (P3 >> 12) : 'A' + (P3 >> 12) - 10);
    uart_send_byte(((P3 >> 8) & 0x0F) < 10 ? '0' + ((P3 >> 8) & 0x0F) : 'A' + ((P3 >> 8) & 0x0F) - 10);
    uart_send_byte(((P3 >> 4) & 0x0F) < 10 ? '0' + ((P3 >> 4) & 0x0F) : 'A' + ((P3 >> 4) & 0x0F) - 10);
    uart_send_byte((P3 & 0x0F) < 10 ? '0' + (P3 & 0x0F) : 'A' + (P3 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P4 (0x94/0x95)
    uint8_t val_94 = read_register(0x94);
    uint8_t val_95 = read_register(0x95);
    int16_t P4 = (val_95 << 8) | val_94;
    
    uart_send_string("P4 = ");
    if (P4 < 0) {
        uart_send_string("-");
        P4 = -P4;
    }
    uart_send_byte((P4 >> 12) < 10 ? '0' + (P4 >> 12) : 'A' + (P4 >> 12) - 10);
    uart_send_byte(((P4 >> 8) & 0x0F) < 10 ? '0' + ((P4 >> 8) & 0x0F) : 'A' + ((P4 >> 8) & 0x0F) - 10);
    uart_send_byte(((P4 >> 4) & 0x0F) < 10 ? '0' + ((P4 >> 4) & 0x0F) : 'A' + ((P4 >> 4) & 0x0F) - 10);
    uart_send_byte((P4 & 0x0F) < 10 ? '0' + (P4 & 0x0F) : 'A' + (P4 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P5 (0x96/0x97)
    uint8_t val_96 = read_register(0x96);
    uint8_t val_97 = read_register(0x97);
    int16_t P5 = (val_97 << 8) | val_96;
    
    uart_send_string("P5 = ");
    if (P5 < 0) {
        uart_send_string("-");
        P5 = -P5;
    }
    uart_send_byte((P5 >> 12) < 10 ? '0' + (P5 >> 12) : 'A' + (P5 >> 12) - 10);
    uart_send_byte(((P5 >> 8) & 0x0F) < 10 ? '0' + ((P5 >> 8) & 0x0F) : 'A' + ((P5 >> 8) & 0x0F) - 10);
    uart_send_byte(((P5 >> 4) & 0x0F) < 10 ? '0' + ((P5 >> 4) & 0x0F) : 'A' + ((P5 >> 4) & 0x0F) - 10);
    uart_send_byte((P5 & 0x0F) < 10 ? '0' + (P5 & 0x0F) : 'A' + (P5 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P6 (0x98/0x99) - single byte, signed
    int8_t P6 = (int8_t)read_register(0x99);
    
    uart_send_string("P6 = ");
    if (P6 < 0) {
        uart_send_string("-");
        P6 = -P6;
    }
    uart_send_byte((P6 >> 4) < 10 ? '0' + (P6 >> 4) : 'A' + (P6 >> 4) - 10);
    uart_send_byte((P6 & 0x0F) < 10 ? '0' + (P6 & 0x0F) : 'A' + (P6 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P7 (0x9A) - single byte, signed
    int8_t P7 = (int8_t)read_register(0x9A);
    
    uart_send_string("P7 = ");
    if (P7 < 0) {
        uart_send_string("-");
        P7 = -P7;
    }
    uart_send_byte((P7 >> 4) < 10 ? '0' + (P7 >> 4) : 'A' + (P7 >> 4) - 10);
    uart_send_byte((P7 & 0x0F) < 10 ? '0' + (P7 & 0x0F) : 'A' + (P7 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P8 (0x9C/0x9B) - Beachte: Reihenfolge vertauscht!
    uint8_t val_9b = read_register(0x9B);
    uint8_t val_9c = read_register(0x9C);
    int16_t P8 = (val_9c << 8) | val_9b;
    
    uart_send_string("P8 = ");
    if (P8 < 0) {
        uart_send_string("-");
        P8 = -P8;
    }
    uart_send_byte((P8 >> 12) < 10 ? '0' + (P8 >> 12) : 'A' + (P8 >> 12) - 10);
    uart_send_byte(((P8 >> 8) & 0x0F) < 10 ? '0' + ((P8 >> 8) & 0x0F) : 'A' + ((P8 >> 8) & 0x0F) - 10);
    uart_send_byte(((P8 >> 4) & 0x0F) < 10 ? '0' + ((P8 >> 4) & 0x0F) : 'A' + ((P8 >> 4) & 0x0F) - 10);
    uart_send_byte((P8 & 0x0F) < 10 ? '0' + (P8 & 0x0F) : 'A' + (P8 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P9 (0x9E/0x9D) - Beachte: Reihenfolge vertauscht!
    uint8_t val_9d = read_register(0x9D);
    uint8_t val_9e = read_register(0x9E);
    int16_t P9 = (val_9e << 8) | val_9d;
    
    uart_send_string("P9 = ");
    if (P9 < 0) {
        uart_send_string("-");
        P9 = -P9;
    }
    uart_send_byte((P9 >> 12) < 10 ? '0' + (P9 >> 12) : 'A' + (P9 >> 12) - 10);
    uart_send_byte(((P9 >> 8) & 0x0F) < 10 ? '0' + ((P9 >> 8) & 0x0F) : 'A' + ((P9 >> 8) & 0x0F) - 10);
    uart_send_byte(((P9 >> 4) & 0x0F) < 10 ? '0' + ((P9 >> 4) & 0x0F) : 'A' + ((P9 >> 4) & 0x0F) - 10);
    uart_send_byte((P9 & 0x0F) < 10 ? '0' + (P9 & 0x0F) : 'A' + (P9 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P10 (0xA0/0x9F) - Beachte: Reihenfolge vertauscht!
    uint8_t val_9f = read_register(0x9F);
    uint8_t val_a0 = read_register(0xA0);
    int8_t P10 = (int8_t)val_a0;  // Nur MSB ist relevant
    
    uart_send_string("P10 = ");
    if (P10 < 0) {
        uart_send_string("-");
        P10 = -P10;
    }
    uart_send_byte((P10 >> 4) < 10 ? '0' + (P10 >> 4) : 'A' + (P10 >> 4) - 10);
    uart_send_byte((P10 & 0x0F) < 10 ? '0' + (P10 & 0x0F) : 'A' + (P10 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // H1/H2 (0xE1/0xE2/0xE3)
    uint8_t val_e1 = read_register(0xE1);
    uint8_t val_e2 = read_register(0xE2);
    uint8_t val_e3 = read_register(0xE3);
    
    uint16_t H1 = ((val_e3 & 0xF0) << 4) | (val_e2 & 0x0F);
    uint16_t H2 = ((val_e2 & 0xF0) << 4) | (val_e1 & 0x0F);
    
    uart_send_string("H1 = ");
    uart_send_byte((H1 >> 12) < 10 ? '0' + (H1 >> 12) : 'A' + (H1 >> 12) - 10);
    uart_send_byte(((H1 >> 8) & 0x0F) < 10 ? '0' + ((H1 >> 8) & 0x0F) : 'A' + ((H1 >> 8) & 0x0F) - 10);
    uart_send_byte(((H1 >> 4) & 0x0F) < 10 ? '0' + ((H1 >> 4) & 0x0F) : 'A' + ((H1 >> 4) & 0x0F) - 10);
    uart_send_byte((H1 & 0x0F) < 10 ? '0' + (H1 & 0x0F) : 'A' + (H1 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uart_send_string("H2 = ");
    uart_send_byte((H2 >> 12) < 10 ? '0' + (H2 >> 12) : 'A' + (H2 >> 12) - 10);
    uart_send_byte(((H2 >> 8) & 0x0F) < 10 ? '0' + ((H2 >> 8) & 0x0F) : 'A' + ((H2 >> 8) & 0x0F) - 10);
    uart_send_byte(((H2 >> 4) & 0x0F) < 10 ? '0' + ((H2 >> 4) & 0x0F) : 'A' + ((H2 >> 4) & 0x0F) - 10);
    uart_send_byte((H2 & 0x0F) < 10 ? '0' + (H2 & 0x0F) : 'A' + (H2 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // H3-H7 (0xE4-0xE8)
    int8_t H3 = (int8_t)read_register(0xE4);
    int8_t H4 = (int8_t)read_register(0xE5);
    int8_t H5 = (int8_t)read_register(0xE6);
    int8_t H6 = (int8_t)read_register(0xE7);
    int8_t H7 = (int8_t)read_register(0xE8);
    
    uart_send_string("H3 = ");
    if (H3 < 0) {
        uart_send_string("-");
        H3 = -H3;
    }
    uart_send_byte((H3 >> 4) < 10 ? '0' + (H3 >> 4) : 'A' + (H3 >> 4) - 10);
    uart_send_byte((H3 & 0x0F) < 10 ? '0' + (H3 & 0x0F) : 'A' + (H3 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uart_send_string("H4 = ");
    if (H4 < 0) {
        uart_send_string("-");
        H4 = -H4;
    }
    uart_send_byte((H4 >> 4) < 10 ? '0' + (H4 >> 4) : 'A' + (H4 >> 4) - 10);
    uart_send_byte((H4 & 0x0F) < 10 ? '0' + (H4 & 0x0F) : 'A' + (H4 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uart_send_string("H5 = ");
    if (H5 < 0) {
        uart_send_string("-");
        H5 = -H5;
    }
    uart_send_byte((H5 >> 4) < 10 ? '0' + (H5 >> 4) : 'A' + (H5 >> 4) - 10);
    uart_send_byte((H5 & 0x0F) < 10 ? '0' + (H5 & 0x0F) : 'A' + (H5 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uart_send_string("H6 = ");
    if (H6 < 0) {
        uart_send_string("-");
        H6 = -H6;
    }
    uart_send_byte((H6 >> 4) < 10 ? '0' + (H6 >> 4) : 'A' + (H6 >> 4) - 10);
    uart_send_byte((H6 & 0x0F) < 10 ? '0' + (H6 & 0x0F) : 'A' + (H6 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uart_send_string("H7 = ");
    if (H7 < 0) {
        uart_send_string("-");
        H7 = -H7;
    }
    uart_send_byte((H7 >> 4) < 10 ? '0' + (H7 >> 4) : 'A' + (H7 >> 4) - 10);
    uart_send_byte((H7 & 0x0F) < 10 ? '0' + (H7 & 0x0F) : 'A' + (H7 & 0x0F) - 10);
    uart_send_string("\r\n");
}