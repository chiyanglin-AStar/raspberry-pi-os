#include "printf.h"
#include "timer.h"
#include "irq.h"
#include "mini_uart.h"


void kernel_main(void)
{
	uart_init();
	uart_send_string("after uart init ,  send string : Hello, world!\r\n");
	init_printf(0, putc);
	printf("after printf , hello world ");
	irq_vector_init();
	//uart_send_string("after uart init ,  send string : Hello, world!\r\n");

	timer_init();
	enable_interrupt_controller();
	enable_irq();

	while (1){
		uart_send(uart_recv());
	}	
}
