#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "twi.h"
#include "bme680.h"

int main(void) {
    uart_init();
    twi_init();
    
    uart_send_string("BME680 Test\r\n");
    uart_send_string("-----------\r\n");
    
    uart_send_string("Scanning for BME680 at address 0x76...\r\n");
    
    twi_start(0x76 << 1);
    if (twi_write(0xD0)) {
        twi_stop();
        twi_init();
        uart_send_string("BME680 found at address 0x76\r\n\n");
    }
    
    uart_send_string("Resetting...\r\n");
    bme680_write_register(0xE0, 0xB6);
    _delay_ms(5);
    
    uart_send_string("Reading calibration...\r\n\n");
    
    // T1 (0xE9/0xE8)
    uint8_t val_e9 = bme680_read_register(0xE9);
    uart_send_string("Reading 0xE9...");
    uart_send_string("P got ");
    uart_send_byte((val_e9 >> 4) < 10 ? '0' + (val_e9 >> 4) : 'A' + (val_e9 >> 4) - 10);
    uart_send_byte((val_e9 & 0x0F) < 10 ? '0' + (val_e9 & 0x0F) : 'A' + (val_e9 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uint8_t val_e8 = bme680_read_register(0xE8);
    uart_send_string("Reading 0xE8...");
    uart_send_string("P got ");
    uart_send_byte((val_e8 >> 4) < 10 ? '0' + (val_e8 >> 4) : 'A' + (val_e8 >> 4) - 10);
    uart_send_byte((val_e8 & 0x0F) < 10 ? '0' + (val_e8 & 0x0F) : 'A' + (val_e8 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    uint16_t T1 = (val_e9 << 8) | val_e8;
    uart_send_string("T1 = ");
    uart_send_byte((T1 >> 12) < 10 ? '0' + (T1 >> 12) : 'A' + (T1 >> 12) - 10);
    uart_send_byte(((T1 >> 8) & 0x0F) < 10 ? '0' + ((T1 >> 8) & 0x0F) : 'A' + ((T1 >> 8) & 0x0F) - 10);
    uart_send_byte(((T1 >> 4) & 0x0F) < 10 ? '0' + ((T1 >> 4) & 0x0F) : 'A' + ((T1 >> 4) & 0x0F) - 10);
    uart_send_byte((T1 & 0x0F) < 10 ? '0' + (T1 & 0x0F) : 'A' + (T1 & 0x0F) - 10);
    uart_send_string("\r\n\n");
    
    // T2 (0x8A/0x8B)
    uint8_t val_8a = bme680_read_register(0x8A);
    uart_send_string("Reading 0x8A...");
    uart_send_string("P got ");
    uart_send_byte((val_8a >> 4) < 10 ? '0' + (val_8a >> 4) : 'A' + (val_8a >> 4) - 10);
    uart_send_byte((val_8a & 0x0F) < 10 ? '0' + (val_8a & 0x0F) : 'A' + (val_8a & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // T3 (0x8C/0x8D)
    uint8_t val_8c = bme680_read_register(0x8C);
    uint8_t val_8d = bme680_read_register(0x8D);
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
    uint8_t val_8e = bme680_read_register(0x8E);
    uint8_t val_8f = bme680_read_register(0x8F);
    uint16_t P1 = (val_8f << 8) | val_8e;
    
    uart_send_string("P1 = ");
    uart_send_byte((P1 >> 12) < 10 ? '0' + (P1 >> 12) : 'A' + (P1 >> 12) - 10);
    uart_send_byte(((P1 >> 8) & 0x0F) < 10 ? '0' + ((P1 >> 8) & 0x0F) : 'A' + ((P1 >> 8) & 0x0F) - 10);
    uart_send_byte(((P1 >> 4) & 0x0F) < 10 ? '0' + ((P1 >> 4) & 0x0F) : 'A' + ((P1 >> 4) & 0x0F) - 10);
    uart_send_byte((P1 & 0x0F) < 10 ? '0' + (P1 & 0x0F) : 'A' + (P1 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P2 (0x90/0x91)
    uint8_t val_90 = bme680_read_register(0x90);
    uint8_t val_91 = bme680_read_register(0x91);
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
    uint8_t val_92 = bme680_read_register(0x92);
    uint8_t val_93 = bme680_read_register(0x93);
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
    uint8_t val_94 = bme680_read_register(0x94);
    uint8_t val_95 = bme680_read_register(0x95);
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
    uint8_t val_96 = bme680_read_register(0x96);
    uint8_t val_97 = bme680_read_register(0x97);
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
    int8_t P6 = (int8_t)bme680_read_register(0x99);
    
    uart_send_string("P6 = ");
    if (P6 < 0) {
        uart_send_string("-");
        P6 = -P6;
    }
    uart_send_byte((P6 >> 4) < 10 ? '0' + (P6 >> 4) : 'A' + (P6 >> 4) - 10);
    uart_send_byte((P6 & 0x0F) < 10 ? '0' + (P6 & 0x0F) : 'A' + (P6 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P7 (0x9A) - single byte, signed
    int8_t P7 = (int8_t)bme680_read_register(0x9A);
    
    uart_send_string("P7 = ");
    if (P7 < 0) {
        uart_send_string("-");
        P7 = -P7;
    }
    uart_send_byte((P7 >> 4) < 10 ? '0' + (P7 >> 4) : 'A' + (P7 >> 4) - 10);
    uart_send_byte((P7 & 0x0F) < 10 ? '0' + (P7 & 0x0F) : 'A' + (P7 & 0x0F) - 10);
    uart_send_string("\r\n");
    
    // P8 (0x9C/0x9B) - Beachte: Reihenfolge vertauscht!
    uint8_t val_9b = bme680_read_register(0x9B);
    uint8_t val_9c = bme680_read_register(0x9C);
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
    uint8_t val_9d = bme680_read_register(0x9D);
    uint8_t val_9e = bme680_read_register(0x9E);
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
    uint8_t val_9f = bme680_read_register(0x9F);
    uint8_t val_a0 = bme680_read_register(0xA0);
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
    uint8_t val_e1 = bme680_read_register(0xE1);
    uint8_t val_e2 = bme680_read_register(0xE2);
    uint8_t val_e3 = bme680_read_register(0xE3);
    
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
    int8_t H3 = (int8_t)bme680_read_register(0xE4);
    int8_t H4 = (int8_t)bme680_read_register(0xE5);
    int8_t H5 = (int8_t)bme680_read_register(0xE6);
    int8_t H6 = (int8_t)bme680_read_register(0xE7);
    int8_t H7 = (int8_t)bme680_read_register(0xE8);
    
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
    
    while(1) {
        _delay_ms(1000);
    }
    
    return 0;
}