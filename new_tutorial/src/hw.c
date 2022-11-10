#include "utils.h"
#include "peripherals/mini_uart.h"
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

/*  Random function declare */
/**
 * Initialize the RNG
 */
void rand_init()
{
    unsigned int r;
    
    r = get32(RNG_STATUS);
    r = 0x40000;
    put32(RNG_STATUS,r);
    // mask interrupt
    r = get32(RNG_INT_MASK);
    r |= 1;
    put32(RNG_INT_MASK,r);
    
    // enable
    r = get32(RNG_CTRL);
    r |= 1;
    put32(RNG_CTRL,r);    
}

/**
 * Return a random number between [min..max]
 */
unsigned int rand(unsigned int min, unsigned int max)
{
    // may need to wait for entropy: bits 24-31 store how many words are
    // available for reading; require at least one
    while(!(get32(RNG_STATUS)>>24)) asm volatile("nop");
    return get32(RNG_DATA)  % (max-min) + min;
}


/**
 * Wait N CPU cycles (ARM CPU only)
 */
void wait_cycles(unsigned int n)
{
    if(n) while(n--) { asm volatile("nop"); }
}


//  Timer and Delay function declare 
/*  Wait N microsec (ARM CPU only)  */
void wait_msec(unsigned int n)
{
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    // calculate expire value for counter
    t+=((f/1000)*n)/1000;
    do{asm volatile ("mrs %0, cntpct_el0" : "=r"(r));}while(r<t);
}

/*  Get System Timer's counter  */
unsigned long get_system_timer()
{
    unsigned int h=-1, l;
    // we must read MMIO area as two separate 32 bit reads
    h=get32(SYSTMR_HI);
    l=get32(SYSTMR_LO);
    // we have to repeat it if high word changed during read
    if(h!=get32(SYSTMR_HI)) {
        h=get32(SYSTMR_HI);
        l=get32(SYSTMR_LO);
    }
    // compose long int value
    return ((unsigned long) h << 32) | l;
}

/* Wait N microsec (with BCM System Timer) */
void wait_msec_st(unsigned int n)
{
    unsigned long t=get_system_timer();
    // we must check if it's non-zero, because qemu does not emulate
    // system timer, and returning constant zero would mean infinite loop
    if(t) while(get_system_timer() < t+n);
}

//  Power functions ---------------------------------------------
/*  Shutdown the board */
void power_off()
{
    unsigned long r;

    // power off devices one by one
    for(r=0;r<16;r++) {
        mbox[0]=8*4;
        mbox[1]=MBOX_REQUEST;
        mbox[2]=MBOX_TAG_SETPOWER; // set power state
        mbox[3]=8;
        mbox[4]=8;
        mbox[5]=(unsigned int)r;   // device id
        mbox[6]=0;                 // bit 0: off, bit 1: no wait
        mbox[7]=MBOX_TAG_LAST;
        mbox_call(MBOX_CH_PROP);
    }

    // power off gpio pins (but not VCC pins)
    put32(GPFSEL0,0);
    put32(GPFSEL1,0);
    put32(GPFSEL2,0);
    put32(GPFSEL3,0);
    put32(GPFSEL4,0);
    put32(GPFSEL5,0);
    put32(GPPUD,0);
        
    wait_cycles(150);
    put32(GPPUDCLK0,0xffffffff);
    put32(GPPUDCLK1,0xffffffff);
    
    wait_cycles(150);
    // flush GPIO setup
    put32(GPPUDCLK0,0);
    put32(GPPUDCLK1,0);
    

    // power off the SoC (GPU + CPU)
    r = get32(PM_RSTS); r &= ~0xfffffaaa;
    r |= 0x555;    // partition 63 used to indicate halt
    put32(PM_RSTS,PM_WDOG_MAGIC | r);
    put32(PM_WDOG,PM_WDOG_MAGIC | 10);
    put32(PM_RSTC,PM_WDOG_MAGIC | PM_RSTC_FULLRST);
}


/*  Reboot  */
void reset()
{
    unsigned int r;
    // trigger a restart by instructing the GPU to boot from partition 0
    r = get32(PM_RSTS); r &= ~0xfffffaaa;
    put32(PM_RSTS,PM_WDOG_MAGIC | r);   // boot from partition 0
    put32(PM_WDOG,PM_WDOG_MAGIC | 10);
    put32(PM_RSTC,PM_WDOG_MAGIC | PM_RSTC_FULLRST);
}
// Power ---------------------------------------------------