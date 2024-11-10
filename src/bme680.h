#ifndef BME680_H
#define BME680_H

#include <stdint.h>

struct bme680_calib {
    // Temperatur
    uint16_t T1;    // 0x079C (1948)
    int16_t T2;     // 0x6819 (26649)
    int8_t T3;      // 0x10 (16)
    
    // Druck
    uint16_t P1;    // 0x8D70 (36208)
    int16_t P2;     // -0x282C (-10284)
    int8_t P3;      // 0x58 (88)
    int16_t P4;     // 0x2276 (8822)
    int16_t P5;     // -0x87 (-135)
    int16_t P6;     // nicht gelesen
    int8_t P7;      // nicht gelesen
    int16_t P8;     // -0x6E00 (-28160)
    int16_t P9;     // 0x70FB (28923)
    int8_t P10;     // 0x1E (30)
    
    // Feuchtigkeit
    uint16_t H1;    // 0x030A (778)
    uint16_t H2;    // 0x050E (1294)
    int8_t H3;      // 0x00 (0)
    int8_t H4;      // 0x2D (45)
    int8_t H5;      // 0x14 (20)
    int8_t H6;      // 0x78 (120)
    int8_t H7;      // -0x64 (-100)
};

// Funktionsdeklarationen
bool bme680_init(void);
int16_t bme680_calc_temperature(uint32_t adc_temp, struct bme680_calib *calib);
uint8_t bme680_read_register(uint8_t reg);
bool bme680_write_register(uint8_t reg, uint8_t value);
int16_t bme680_read_temperature(void);

#endif