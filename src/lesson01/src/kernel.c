#include "mini_uart.h"

void kernel_main(void)
{
	uart_init();
	uart_send_string("Hello, world!\r\n");
	#if 0
	// say hello

	/*
	https://github.com/bztsrc/raspi3-tutorial version
	in main.c
	*/

	uart_puts("Hello World!\n");
	#endif

	while (1) {
		uart_send(uart_recv());
	}
}
