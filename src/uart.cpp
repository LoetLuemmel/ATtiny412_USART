#include "uart.h"
#include <util/delay.h>

#define UART_TX_PIN PIN6_bm
#define BIT_TIME_US 103    // Für 9600 baud

void uart_init(void) {
    PORTA.DIRSET = UART_TX_PIN;
    PORTA.OUTSET = UART_TX_PIN;
}

void uart_send_byte(uint8_t byte) {
    PORTA.OUTCLR = UART_TX_PIN;     // Start bit
    _delay_us(BIT_TIME_US);
    
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & 0x01) {
            PORTA.OUTSET = UART_TX_PIN;
        } else {
            PORTA.OUTCLR = UART_TX_PIN;
        }
        byte = byte >> 1;
        _delay_us(BIT_TIME_US);
    }
    
    PORTA.OUTSET = UART_TX_PIN;     // Stop bit
    _delay_us(BIT_TIME_US * 2);
}

void uart_send_string(const char* str) {
    while (*str) {
        uart_send_byte(*str++);
        _delay_ms(2);
    }
}

