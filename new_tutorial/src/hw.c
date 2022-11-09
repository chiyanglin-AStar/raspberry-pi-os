#include "peripherals/gpio.h"
#include "hw.h"

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];

/*
 Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    /* wait until we can write to the mailbox */
    
    // read register value ex : get32(AUX_MU_LSR_REG)&0x20
    // write register value ex : put32(GPPUD,0) 

    do{asm volatile("nop");}while( get32(MBOX_STATUS) & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    put32(MBOX_WRITE,r);
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(get32(MBOX_STATUS) & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == get32(MBOX_READ))
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}
