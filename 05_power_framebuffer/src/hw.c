#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "hw.h"
#include "homer.h"

/* for frame buffer */
unsigned int width, height, pitch, isrgb;   /* dimensions and channel order */
unsigned char *lfb;                         /* raw frame buffer address */



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


/* frame buffer function */

/**
 * Set screen resolution to 1024x768
 */
void lfb_init()
{
    mbox[0] = 35*4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003;  //set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024;         //FrameBufferInfo.width
    mbox[6] = 768;          //FrameBufferInfo.height

    mbox[7] = 0x48004;  //set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024;        //FrameBufferInfo.virtual_width
    mbox[11] = 768;         //FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; //set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;           //FrameBufferInfo.x_offset
    mbox[16] = 0;           //FrameBufferInfo.y.offset

    mbox[17] = 0x48005; //set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;          //FrameBufferInfo.depth

    mbox[21] = 0x48006; //set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1;           //RGB, not BGR preferably

    mbox[25] = 0x40001; //get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;        //FrameBufferInfo.pointer
    mbox[29] = 0;           //FrameBufferInfo.size

    mbox[30] = 0x40008; //get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;           //FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    //this might not return exactly what we asked for, could be
    //the closest supported resolution instead
    if(mbox_call(MBOX_CH_PROP) && mbox[20]==32 && mbox[28]!=0) {
        mbox[28]&=0x3FFFFFFF;   //convert GPU address to ARM address
        width=mbox[5];          //get actual physical width
        height=mbox[6];         //get actual physical height
        pitch=mbox[33];         //get number of bytes per line
        isrgb=mbox[24];         //get the actual channel order
        lfb=(void*)((unsigned long)mbox[28]);
    } else {
        uart_send_string("Unable to set screen resolution to 1024x768x32\n");
    }
}

/**
 * Show a picture
 */
void lfb_showpicture()
{
    int x,y;
    unsigned char *ptr=lfb;
    char *data=homer_data, pixel[4];

    ptr += (height-homer_height)/2*pitch + (width-homer_width)*2;
    for(y=0;y<homer_height;y++) {
        for(x=0;x<homer_width;x++) {
            HEADER_PIXEL(data, pixel);
            // the image is in RGB. So if we have an RGB framebuffer, we can copy the pixels
            // directly, but for BGR we must swap R (pixel[0]) and B (pixel[2]) channels.
            *((unsigned int*)ptr)=isrgb ? *((unsigned int *)&pixel) : (unsigned int)(pixel[0]<<16 | pixel[1]<<8 | pixel[2]);
            ptr+=4;
        }
        ptr+=pitch-homer_width*4;
    }
}