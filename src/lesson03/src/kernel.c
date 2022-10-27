#include "printf.h"
//#include "timer.h"
//#include "irq.h"
#include "utils.h"
#include "mini_uart.h"

void kernel_main(void)
{ 
	int el = get_el();
	
	uart_init();
	uart_send_string("after uart init ,  send string : Hello, world!\r\n");
	init_printf(0, putc);
	
	uart_send_string("in uart send string : Hello, world!\r\n");
	printf("in printf hello world ");
	printf("Exception level: %d \r\n", el);

	//irq_vector_init();
	//uart_send_string("after uart init ,  send string : Hello, world!\r\n");

	//timer_init();
	//enable_interrupt_controller();
	//enable_irq();

	while (1){
		uart_send(uart_recv());
	}	
}
