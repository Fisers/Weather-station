#ifndef UART_H
#define UART_H

void uart_putchar(char c, FILE *stream);
char uart_getchar(FILE *stream);

void uart_init(void);



#endif