#ifndef UART_H
#define UART_H

#include "board.h"

void uart_init(void);
void uart_putc(char c);
void uart_puts(__code const char *str);
void uart_isr() __interrupt(UART_IRQ);

#endif
