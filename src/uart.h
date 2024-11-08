#ifndef UART_H
#define UART_H

#define F_CPU 3333333UL
#define BAUD_RATE 9600
#define UROUND(x) ((2UL*(x)+1)/2)

void uart_init(void);

#endif 