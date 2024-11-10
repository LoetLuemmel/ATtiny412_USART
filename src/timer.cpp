#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t timer_millis = 0;

// Timer0 Overflow Interrupt
ISR(TCA0_OVF_vect) {
    timer_millis++;
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm; // Clear interrupt flag
}

void timer_init(void) {
    // Configure Timer0 for 1ms interrupts at 3.3MHz
    TCA0.SINGLE.PER = 3300;  // 3300 cycles = 1ms @ 3.3MHz
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;  // Enable overflow interrupt
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
    sei();  // Enable global interrupts
}

uint32_t millis(void) {
    uint32_t m;
    cli();
    m = timer_millis;
    sei();
    return m;
} 