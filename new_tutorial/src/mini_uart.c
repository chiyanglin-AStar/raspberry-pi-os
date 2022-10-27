#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#define UART__1 1

/*
https://github.com/bztsrc/raspi3-tutorial version
*/
#if UART__1 
void uart_send ( char c )
{
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x20)
			break;
	}
	put32(AUX_MU_IO_REG,c);
}

char uart_recv ( void )
{
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x01)
			break;
	}
	return(get32(AUX_MU_IO_REG)&0xFF);
}

void uart_send_string(char* str)
{
	for (int i = 0; str[i] != '\0'; i ++) {
		uart_send((char)str[i]);
	}
}

void uart_init ( void )
{
	unsigned int selector;

	selector = get32(GPFSEL1);
	selector &= ~(7<<12);                   // clean gpio14
	selector |= 2<<12;                      // set alt5 for gpio14
	selector &= ~(7<<15);                   // clean gpio15
	selector |= 2<<15;                      // set alt5 for gpio15
	put32(GPFSEL1,selector);

	put32(GPPUD,0);
	delay(150);
	put32(GPPUDCLK0,(1<<14)|(1<<15));
	delay(150);
	put32(GPPUDCLK0,0);

	put32(AUX_ENABLES,1);                   //Enable mini uart (this also enables access to its registers)
	put32(AUX_MU_CNTL_REG,0);               //Disable auto flow control and disable receiver and transmitter (for now)
	put32(AUX_MU_IER_REG,0);                //Disable receive and transmit interrupts
	put32(AUX_MU_LCR_REG,3);                //Enable 8 bit mode
	put32(AUX_MU_MCR_REG,0);                //Set RTS line to be always high
	put32(AUX_MU_BAUD_REG,270);             //Set baud rate to 115200

	put32(AUX_MU_CNTL_REG,3);               //Finally, enable transmitter and receiver
}


// This function is required by printf function
void putc ( void* p, char c)
{
	uart_send(c);
}
#else
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(*((volatile unsigned int*)UART0_FR) & 0x20);
    /* write the character to the buffer */
    *((volatile unsigned int*)UART0_DR)=c;
}

//char uart_getc() {
char uart_recv(void){
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(*((volatile unsigned int*)UART0_FR) & 0x10);
    /* read it and return */
    r=(char)(*((volatile unsigned int*)UART0_DR));
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

/*
https://github.com/bztsrc/raspi3-tutorial version
*/
void uart_send_string(char* s){
    while(*s) {
        /* convert newline to carriage return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_init ( void )
{
	register unsigned int r;
	/* initialize UART */
	*((volatile unsigned int*)UART0_CR) = 0;         // turn off UART0

	/* map UART0 to GPIO pins */
	r=*((volatile unsigned int*)GPFSEL1);
	r&=~((7<<12)|(7<<15)); // gpio14, gpio15
	r|=(4<<12)|(4<<15);    // alt0
	*((volatile unsigned int*)GPFSEL1) = r;
	*((volatile unsigned int*)GPPUD) = 0;            // enable pins 14 and 15
	r=150; while(r--) { asm volatile("nop"); }
	*((volatile unsigned int*)GPPUDCLK0) = (1<<14)|(1<<15);
	r=150; while(r--) { asm volatile("nop"); }
	*((volatile unsigned int*)GPPUDCLK0) = 0;        // flush GPIO setup

	*((volatile unsigned int*)UART0_ICR)  = 0x7FF;     // clear interrupts
	*((volatile unsigned int*)UART0_IBRD) = 2;         // 115200 baud
	*((volatile unsigned int*)UART0_FBRD) = 0xB;
	*((volatile unsigned int*)UART0_LCRH) = 0x7<<4;    // 8n1, enable FIFOs
	*((volatile unsigned int*)UART0_CR)   = 0x301;     // enable Tx, Rx, UART	
}

// This function is required by printf function
void putc ( void* p, char c)
{
	uart_send(c);
}
#endif 