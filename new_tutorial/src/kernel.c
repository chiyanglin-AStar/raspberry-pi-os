#include "mini_uart.h"
#include "hw.h"
void kernel_main(void)
{
	uart_init();
	uart_send_string("Hello, world!\r\n");
	mbox_fun();
	while (1) {
		uart_send(uart_recv());
	}
}

void mbox_fun(void){
	// get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message

    mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_send_string("My serial number is: ");
        uart_send(mbox[6]);
        uart_send(mbox[5]);
        uart_send_string("\n");
    } else {
        uart_send_string("Unable to query serial!\n");
    }
}