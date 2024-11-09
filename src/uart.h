#pragma once
#include <avr/io.h>

void uart_init(void);
void uart_send_byte(uint8_t byte);
void uart_send_string(const char* str);