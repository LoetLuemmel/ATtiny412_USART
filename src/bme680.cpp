#include "bme680.h"
#include "twi.h"
#include "uart.h"
#include <util/delay.h>

#define BME680_ADDR 0x76

// BME680 Register Map
#define BME680_REG_T1_LSB     0x8A
#define BME680_REG_T1_MSB     0x8B
#define BME680_REG_T2_LSB     0x8C
#define BME680_REG_T2_MSB     0x8D
#define BME680_REG_T3         0x8E

#define BME680_REG_TEMP_MSB   0x22
#define BME680_REG_TEMP_LSB   0x23
#define BME680_REG_TEMP_XLSB  0x24

// Verwende die existierende Struktur
static struct bme680_calib calib_data;

// Angepasste Funktionsdeklarationen
static bool twi_read_reg(uint8_t reg, uint8_t* data);
static bool read_calibration_data(void);
struct bme680_calib* bme680_get_calib(void);

bool bme680_init(void) {
    uart_send_string("Performing soft reset...\r\n");
    
    // Soft Reset mit Debug
    uart_send_string("Writing reset command...\r\n");
    if (!bme680_write_register(0xE0, 0xB6)) {
        uart_send_string("Failed to write reset command!\r\n");
        return false;
    }
    
    uart_send_string("Waiting after reset...\r\n");
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
    
    if (!read_calibration_data()) {
        uart_send_string("Failed to read calibration data!\r\n");
        return false;
    }
    
    // Temperatur-Messung konfigurieren
    bme680_write_register(0x74, 0x01);  // osrs_t = 1 (1x oversampling)
    
    uart_send_string("Initialization complete\r\n");
    return true;
}

static bool twi_read_reg(uint8_t reg, uint8_t* data) {
    if (!twi_start(BME680_ADDR << 1)) {
        return false;
    }
    
    if (!twi_write(reg)) {
        twi_stop();
        return false;
    }
    
    if (!twi_start((BME680_ADDR << 1) | 1)) {
        twi_stop();
        return false;
    }
    
    *data = twi_read(false);
    twi_stop();
    
    return true;
}

static bool read_calibration_data(void) {
    uint8_t data[2];
    
    // Read T1 (LSB + MSB)
    if (!twi_read_reg(BME680_REG_T1_LSB, &data[0]) ||
        !twi_read_reg(BME680_REG_T1_MSB, &data[1])) {
        return false;
    }
    calib_data.T1 = (uint16_t)(data[1] << 8) | data[0];
    
    // Read T2 (LSB + MSB)
    if (!twi_read_reg(BME680_REG_T2_LSB, &data[0]) ||
        !twi_read_reg(BME680_REG_T2_MSB, &data[1])) {
        return false;
    }
    calib_data.T2 = (int16_t)(data[1] << 8) | data[0];
    
    // Read T3 (single byte)
    uint8_t t3;
    if (!twi_read_reg(BME680_REG_T3, &t3)) {
        return false;
    }
    calib_data.T3 = (int8_t)t3;
    
    return true;
}

int16_t bme680_read_temperature(void) {
    uint8_t data[3];
    
    // Read all 3 temperature registers in sequence
    if (!twi_read_reg(BME680_REG_TEMP_MSB, &data[0]) ||
        !twi_read_reg(BME680_REG_TEMP_LSB, &data[1]) ||
        !twi_read_reg(BME680_REG_TEMP_XLSB, &data[2])) {
        return 0;
    }
    
    // Combine the bytes into a 20-bit value
    uint32_t adc_temp = ((uint32_t)data[0] << 12) | 
                       ((uint32_t)data[1] << 4) | 
                       (data[2] >> 4);
    
    return bme680_calc_temperature(adc_temp, &calib_data);
}

bool bme680_write_register(uint8_t reg, uint8_t value) {
    uart_send_string("TWI: Writing to register 0x");
    uart_send_byte((reg >> 4) < 10 ? '0' + (reg >> 4) : 'A' + (reg >> 4) - 10);
    uart_send_byte((reg & 0x0F) < 10 ? '0' + (reg & 0x0F) : 'A' + (reg & 0x0F) - 10);
    uart_send_string("\r\n");
    
    if (!twi_start(BME680_ADDR << 1)) {
        uart_send_string("TWI: Start failed\r\n");
        return false;
    }
    
    if (!twi_write(reg)) {
        uart_send_string("TWI: Register write failed\r\n");
        twi_stop();
        return false;
    }
    
    if (!twi_write(value)) {
        uart_send_string("TWI: Value write failed\r\n");
        twi_stop();
        return false;
    }
    
    twi_stop();
    return true;
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
    return &calib_data;
}

int16_t bme680_calc_temperature(uint32_t adc_temp, struct bme680_calib* calib) {
    int32_t var1, var2;
    
    var1 = ((((adc_temp >> 3) - ((int32_t)calib->T1 << 1))) * 
            ((int32_t)calib->T2)) >> 11;
    
    var2 = (((((adc_temp >> 4) - ((int32_t)calib->T1)) * 
             ((adc_temp >> 4) - ((int32_t)calib->T1))) >> 12) * 
             ((int32_t)calib->T3)) >> 14;
    
    return (var1 + var2) / 100;
}