#include "printf.h"
#include "utils.h"
#include "mini_uart.h"

void kernel_main(void)
{
	uart_init();
	init_printf(0, putc);
	int el = get_el();
	uart_send_string("in uart send string : Hello, world!\r\n");
	printf("in printf hello world ");
	printf("Exception level: %d \r\n", el);

	while (1) {
		uart_send(uart_recv());
	}
}
