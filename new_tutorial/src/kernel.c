#include "printf.h"
#include "utils.h"
#include "timer.h"
#include "irq.h"
#include "mini_uart.h"

void kernel_main(void)
{
	uart_init();
	
	int el = get_el();
	uart_send_string("in uart send string : Hello, world!\r\n");
	
	#if 0
	init_printf(0, putc);
	printf("in printf hello world ");
	printf("Exception level: %d \r\n", el);
	irq_vector_init();
	uart_send_string("after irq vector init : Hello, world!\r\n");
	timer_init();
	uart_send_string("after timer init  : Hello, world!\r\n");
	enable_interrupt_controller();
	enable_irq();
	uart_send_string("enable irg : Hello, world!\r\n");
	printf("in printf irq enable ");
	printf("in printf timer enable");
	#endif 
	while (1) {
		uart_send(uart_recv());
	}
}
