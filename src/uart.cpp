#include "uart.h"
#include <avr/io.h>
#include <util/delay.h>


void uart_init(void) {
    // Clock auf 3.333MHz konfigurieren
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);  
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_OSC20M_gc);
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PDIV_6X_gc | CLKCTRL_PEN_bm);
    
    // Minimale TX Konfiguration
    PORTA.DIRSET = PIN6_bm;    // TX als Ausgang
    PORTA.OUTSET = PIN6_bm;    // TX idle high
    
    // UART Basis-Konfiguration
    USART0.BAUD = UROUND(64UL * 3333333UL / 16 / 9600);
    USART0.CTRLC = USART_CHSIZE_8BIT_gc;   // 8 Datenbits, keine Parität, 1 Stopbit
    USART0.CTRLB = USART_TXEN_bm;          // Nur TX aktivieren
}

