/*
    Minimal USART test for attiny816
 */ 
 
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "twi.h"
 
#ifndef F_CPU
#warning  "You haven't defined F_CPU. I'm using F_CPU = 3333333 "
#define F_CPU 3333333
#endif
 

 
// Set up USART0 in asynchronous mode.
// Default pin locations (i.e. not remaped)
// RX = PB3
// TX = PB2
void usart_init(void)
{
    PORTA.OUTSET = PIN6_bm;
    PORTA.DIRSET = PIN6_bm;
    USART0.BAUD = UROUND(64UL*F_CPU/16/BAUD_RATE);
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}
 
uint8_t usart_ischar(void)
{
    return (USART0.STATUS & USART_RXCIF_bm) != 0;
}
 
char usart_getchar(void)
{
    while(!usart_ischar())
        ;
    return USART0_RXDATAL;
}
 
void usart_putchar(char ch)
{
    while((USART0.STATUS & USART_DREIF_bm) == 0)
        ;
    USART0.TXDATAL = ch;
}

void uart_print(const char* str) {
    while (*str) {
        usart_putchar(*str++);
    }
}
 
void setup() {
    uart_init();
    twi_init();
}
 
void loop() {
    // uart_echo() wird nicht mehr benötigt
    // Hier nur deine normale Loop-Funktionalität
}
 
int main(void)
{
    // UART initialisieren
    uart_init();
    
    while (1) {
        // Einzelnes 'A' senden
        USART0.TXDATAL = 'A';
        
        // Warten bis Zeichen gesendet
        while (!(USART0.STATUS & USART_DREIF_bm));
        
        _delay_ms(1000);
    }
    
    return 0;
}