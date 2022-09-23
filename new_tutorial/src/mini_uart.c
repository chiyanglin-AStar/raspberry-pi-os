#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#if 1
void uart_send ( char c )
{
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x20)
			break;
	}
	put32(AUX_MU_IO_REG,c);
}
#else
/*
https://github.com/bztsrc/raspi3-tutorial version
*/
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO=c;
}

void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(*UART0_FR&0x20);
    /* write the character to the buffer */
    *UART0_DR=c;
}

#endif

#if 1
char uart_recv ( void )
{
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x01)
			break;
	}
	return(get32(AUX_MU_IO_REG)&0xFF);
}
#else
/*
https://github.com/bztsrc/raspi3-tutorial version
*/
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carriage return to newline */
    return r=='\r'?'\n':r;
}

char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(*UART0_FR&0x10);
    /* read it and return */
    r=(char)(*UART0_DR);
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

#endif

#if 1
void uart_send_string(char* str)
{
	for (int i = 0; str[i] != '\0'; i ++) {
		uart_send((char)str[i]);
	}
}
#else
/*
https://github.com/bztsrc/raspi3-tutorial version
*/
void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carriage return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}
#endif

#if 1
void uart_init ( void )
{
	unsigned int selector;

  #if 0
	/* initialize UART */
	*UART0_CR = 0;         // turn off UART0

	/* map UART0 to GPIO pins */
	r=*GPFSEL1;
	r&=~((7<<12)|(7<<15)); // gpio14, gpio15
	r|=(4<<12)|(4<<15);    // alt0
	*GPFSEL1 = r;
	*GPPUD = 0;            // enable pins 14 and 15
	r=150; while(r--) { asm volatile("nop"); }
	*GPPUDCLK0 = (1<<14)|(1<<15);
	r=150; while(r--) { asm volatile("nop"); }
	*GPPUDCLK0 = 0;        // flush GPIO setup

	*UART0_ICR = 0x7FF;    // clear interrupts
	*UART0_IBRD = 2;       // 115200 baud
	*UART0_FBRD = 0xB;
	*UART0_LCRH = 0x7<<4;  // 8n1, enable FIFOs
	*UART0_CR = 0x301;     // enable Tx, Rx, UART



	#endif


  #if 0
	//  UART1 map to GPIO part
	selector = get32(GPFSEL1);
	selector &= ~(7<<12);                   // clean gpio14
	selector |= 2<<12;                      // set alt5 for gpio14
	selector &= ~(7<<15);                   // clean gpio15
	selector |= 2<<15;                      // set alt5 for gpio15
	put32(GPFSEL1,selector);
	#else
	/* map UART1 to GPIO pins */
	selector = get32(GPFSEL1);
	selector &= ~((7<<12)|(7<<15)); // gpio14, gpio15
	selector |=(2<<12)|(2<<15);    // alt5
	put32(GPFSEL1,selector);

	#endif
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
#else
/*
https://github.com/bztsrc/raspi3-tutorial version
*/
void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *((volatile unsigned int*)AUX_ENABLES)    |= 1;       // enable UART1, AUX mini uart
    *((volatile unsigned int*)AUX_MU_CNTL_REG)  = 0;
    *((volatile unsigned int*)AUX_MU_LCR_REG)   = 3;       // 8 bits
    *((volatile unsigned int*)AUX_MU_MCR_REG)   = 0;
    *((volatile unsigned int*)AUX_MU_IER_REG)   = 0;
    *((volatile unsigned int*)AUX_MU_IIR_REG)   = 0xc6;    // disable interrupts
    *((volatile unsigned int*)AUX_MU_BAUD_REG)  = 270;    // 115200 baud
    /* map UART1 to GPIO pins */
    r=*((volatile unsigned int*)GPFSEL1);
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *((volatile unsigned int*)GPFSEL1) = r;
    *((volatile unsigned int*)GPPUD) 		   = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *((volatile unsigned int*)GPPUDCLK0) 	   = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *((volatile unsigned int*)GPPUDCLK0)       = 0;        // flush GPIO setup
    *((volatile unsigned int*)AUX_MU_CNTL_REG) = 3;      // enable Tx, Rx
}
#endif
