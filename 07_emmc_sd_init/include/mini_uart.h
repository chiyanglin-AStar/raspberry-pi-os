#ifndef	_MINI_UART_H
#define	_MINI_UART_H

void uart_init ( void );
char uart_recv ( void );
void uart_send ( char c );
void uart_hex(unsigned int d);
void uart_send_string(char* str);
void uart_dump(void *ptr);
#endif  /*_MINI_UART_H */
