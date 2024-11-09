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
    
    if (bme680_init()) {
        uart_send_string("BME680 initialized successfully\r\n\n");
    }
    
    while(1) {
        int16_t temp = bme680_read_temperature();
        
        uart_send_string("Temperature: ");
        
        // Vorkomma
        uart_send_byte('0' + (temp/1000) % 10);
        uart_send_byte('0' + (temp/100) % 10);
        uart_send_byte('.');
        // Nachkomma
        uart_send_byte('0' + (temp/10) % 10);
        uart_send_byte('0' + temp % 10);
        uart_send_string(" C\r\n");
        
        _delay_ms(10000);  // 10 Sekunden warten
    }
    
    return 0;
}