#include "mini_uart.h"
#include "hw.h"
void mbox_fun(void);
void random_demo(void);
void timer_init_delay_demo(void);

void kernel_main(void)
{
    char c;
	uart_init();
	uart_send_string("Hello, world!\r\n");
	mbox_fun();
    random_demo();
    timer_init_delay_demo();
#if 1
    lfb_init();
    // display a pixmap
    lfb_showpicture();
#endif   
	while (1) {
        #if 0
		uart_send(uart_recv());
        #else 
        uart_send_string(" 1 - power off\n 2 - reset\nChoose one: ");
        c=uart_recv();
        uart_send(c);
        uart_send_string("\n\n");
        if(c=='1') power_off();
        if(c=='2') reset();
        #endif 
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
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_send_string("\n");
    } else {
        uart_send_string("Unable to query serial!\n");
    }
}

void random_demo(void){
    rand_init();
    
    uart_send_string("Here goes a random number: ");
    uart_hex(rand(0,4294967295));
    uart_send_string("\n");
}

void timer_init_delay_demo(void){
    uart_send_string("Waiting 1000000 CPU cycles (ARM CPU): ");
    wait_cycles(1000000);
    uart_send_string("OK\n");

    uart_send_string("Waiting 1000000 microsec (ARM CPU): ");
    wait_msec(1000000);
    uart_send_string("OK\n");

    uart_send_string("Waiting 1000000 microsec (BCM System Timer): ");
    if(get_system_timer()==0) {
        uart_send_string("Not available\n");
    } else {
        wait_msec_st(1000000);
        uart_send_string("OK\n");
    }

}
