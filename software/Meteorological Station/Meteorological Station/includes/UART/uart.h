#ifndef UART_H
#define UART_H

int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);

void uart_init(void);



#endif