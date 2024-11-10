#include "bme680.h"
#include "twi.h"
#include "uart.h"
#include "timer.h"
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

#define BME680_WAIT_PERIOD_MS  10000  // 10 Sekunden zwischen Messungen

#define BME680_REG_CTRL_GAS1   0x71    // Gas control register
#define BME680_REG_CTRL_MEAS   0x74    // Measurement control register
#define BME680_REG_CONFIG      0x75    // Configuration register

// Verwende die existierende Struktur
static struct bme680_calib calib_data;

// Angepasste Funktionsdeklarationen
static bool twi_read_reg(uint8_t reg, uint8_t* data);
static bool read_calibration_data(void);
struct bme680_calib* bme680_get_calib(void);
static bool twi_write_reg(uint8_t reg, uint8_t value);

// Funktionsprototyp am Anfang der Datei
static bool twi_recover(void);

bool bme680_init(void) {
    uart_send_string("Performing soft reset...\r\n");
    
    // Soft Reset mit Debug
    uart_send_string("Writing reset command...\r\n");
    if (!bme680_write_register(0xE0, 0xB6)) {
        uart_send_string("Failed to write reset command!\r\n");
        return false;
    }
    
    uart_send_string("Waiting after reset...\r\n");
    _delay_ms(100);  // 100ms statt 10ms
    
    uart_send_string("Reading chip ID...\r\n");
    uint8_t chip_id = bme680_read_register(0xD0);
    uart_send_string("Chip ID: 0x");
    uart_send_byte((chip_id >> 4) < 10 ? '0' + (chip_id >> 4) : 'A' + (chip_id >> 4) - 10);
    uart_send_byte((chip_id & 0x0F) < 10 ? '0' + (chip_id & 0x0F) : 'A' + (chip_id & 0x0F) - 10);
    uart_send_string("\r\n");
    
    if (chip_id != 0x61) {
        return false;
    }
    
    _delay_ms(10);
    
    // Längere Wartezeit nach Reset
    _delay_ms(100);
    
    // Gas-Messung komplett deaktivieren
    if (!bme680_write_register(BME680_REG_CTRL_GAS1, 0x00)) {
        return false;
    }
    
    // Nur Temperaturmessung aktivieren
    if (!bme680_write_register(BME680_REG_CTRL_MEAS, 0x20)) { 
        return false;
    }
    
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
    
    // Längere Wartezeit vor Kalibrierung
    _delay_ms(100);
    
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
    static uint32_t last_read = 0;
    uint32_t now = millis();
    
    // 10 Sekunden zwischen Messungen warten
    if (now - last_read < 10000) {
        return 0;  // Zu früh für neue Messung
    }
    last_read = now;
    
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
    uint8_t data = 0;
    uint8_t retries = 3;
    
    while (retries--) {
        if (!twi_start(BME680_ADDR << 1)) {
            twi_recover();
            continue;
        }
        
        if (!twi_write(reg)) {
            twi_stop();
            twi_recover();
            continue;
        }
        
        if (!twi_start((BME680_ADDR << 1) | 1)) {
            twi_stop();
            twi_recover();
            continue;
        }
        
        data = twi_read(false);
        twi_stop();
        twi_recover();
        
        return data;  // Erfolgreicher Read
    }
    
    return 0;  // Alle Versuche fehlgeschlagen
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

static bool twi_write_reg(uint8_t reg, uint8_t value) {
    if (!twi_start(BME680_ADDR << 1)) {
        return false;
    }
    
    if (!twi_write(reg)) {
        twi_stop();
        return false;
    }
    
    if (!twi_write(value)) {
        twi_stop();
        return false;
    }
    
    twi_stop();
    return true;
}

static bool twi_recover(void) {
    // Kurze Pause
    _delay_ms(1);
    
    // Bus-Status prüfen
    if ((TWI0.MSTATUS & TWI_BUSSTATE_gm) == TWI_BUSSTATE_IDLE_gc) {
        return true;  // Bus ist bereits OK
    }
    
    // Stop senden
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;
    _delay_ms(1);
    
    // Bus-Status zurücksetzen
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
    
    return true;
}