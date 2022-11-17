#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "hw.h"
#include "homer.h"

/* for frame buffer */
unsigned int width, height, pitch, isrgb;   /* dimensions and channel order */
unsigned char *lfb;                         /* raw frame buffer address */
/* for sd and emmc */
unsigned long sd_scr[2], sd_ocr, sd_rca, sd_err, sd_hv;


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




/**
 * Wait for data or command ready
 */
int sd_status(unsigned int mask)
{
    int cnt = 500000; while((get32(EMMC_STATUS) & mask) && !(get32(EMMC_INTERRUPT) & INT_ERROR_MASK) && cnt--) wait_msec(1);
    return (cnt <= 0 || (get32(EMMC_INTERRUPT) & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

/**
 * Wait for interrupt
 */
int sd_int(unsigned int mask)
{
    unsigned int r, m=mask | INT_ERROR_MASK;
    int cnt = 1000000; while(!(get32(EMMC_INTERRUPT) & m) && cnt--) wait_msec(1);
    r=get32(EMMC_INTERRUPT);
    if(cnt<=0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT) ) { put32(EMMC_INTERRUPT,r); return SD_TIMEOUT; } else
    if(r & INT_ERROR_MASK) { put32(EMMC_INTERRUPT,r); return SD_ERROR; }
    put32(EMMC_INTERRUPT,mask);
    return 0;
}

/**
 * Send a command
 */
int sd_cmd(unsigned int code, unsigned int arg)
{
    int r=0;
    sd_err=SD_OK;
    if(code&CMD_NEED_APP) {
        r=sd_cmd(CMD_APP_CMD|(sd_rca?CMD_RSPNS_48:0),sd_rca);
        if(sd_rca && !r) { uart_send_string("ERROR: failed to send SD APP command\n"); sd_err=SD_ERROR;return 0;}
        code &= ~CMD_NEED_APP;
    }
    if(sd_status(SR_CMD_INHIBIT)) { uart_send_string("ERROR: EMMC busy\n"); sd_err= SD_TIMEOUT;return 0;}
    uart_send_string("EMMC: Sending command ");uart_hex(code);uart_send_string(" arg ");uart_hex(arg);uart_send_string("\n");
    put32(EMMC_INTERRUPT,get32(EMMC_INTERRUPT)); put32(EMMC_ARG1,arg); put32(EMMC_CMDTM,code);
    if(code==CMD_SEND_OP_COND) wait_msec(1000); else
    if(code==CMD_SEND_IF_COND || code==CMD_APP_CMD) wait_msec(100);
    if((r=sd_int(INT_CMD_DONE))) {uart_send_string("ERROR: failed to send EMMC command\n");sd_err=r;return 0;}
    r=get32(EMMC_RESP0);
    if(code==CMD_GO_IDLE || code==CMD_APP_CMD) return 0; else
    if(code==(CMD_APP_CMD|CMD_RSPNS_48)) return r&SR_APP_CMD; else
    if(code==CMD_SEND_OP_COND) return r; else
    if(code==CMD_SEND_IF_COND) return r==arg? SD_OK : SD_ERROR; else
    if(code==CMD_ALL_SEND_CID) {r|=get32(EMMC_RESP3); r|=get32(EMMC_RESP2); r|=get32(EMMC_RESP1); return r; } else
    if(code==CMD_SEND_REL_ADDR) {
        sd_err=(((r&0x1fff))|((r&0x2000)<<6)|((r&0x4000)<<8)|((r&0x8000)<<8))&CMD_ERRORS_MASK;
        return r&CMD_RCA_MASK;
    }
    return r&CMD_ERRORS_MASK;
    // make gcc happy
    return 0;
}

/**
 * read a block from sd card and return the number of bytes read
 * returns 0 on error.
 */
int sd_readblock(unsigned int lba, unsigned char *buffer, unsigned int num)
{
    int r,c=0,d;
    if(num<1) num=1;
    uart_send_string("sd_readblock lba ");uart_hex(lba);uart_send_string(" num ");uart_hex(num);uart_send_string("\n");
    if(sd_status(SR_DAT_INHIBIT)) {sd_err=SD_TIMEOUT; return 0;}
    unsigned int *buf=(unsigned int *)buffer;
    if(sd_scr[0] & SCR_SUPP_CCS) {
        if(num > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)) {
            sd_cmd(CMD_SET_BLOCKCNT,num);
            if(sd_err) return 0;
        }
        put32(EMMC_BLKSIZECNT, (num << 16) | 512);
        sd_cmd(num == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI,lba);
        if(sd_err) return 0;
    } else {
        put32(EMMC_BLKSIZECNT,(1 << 16) | 512);
    }
    while( c < num ) {
        if(!(sd_scr[0] & SCR_SUPP_CCS)) {
            sd_cmd(CMD_READ_SINGLE,(lba+c)*512);
            if(sd_err) return 0;
        }
        if((r=sd_int(INT_READ_RDY))){uart_send_string("\rERROR: Timeout waiting for ready to read\n");sd_err=r;return 0;}
        for(d=0;d<128;d++) buf[d] = get32(EMMC_DATA);
        c++; buf+=128;
    }
    if( num > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (sd_scr[0] & SCR_SUPP_CCS)) sd_cmd(CMD_STOP_TRANS,0);
    return sd_err!=SD_OK || c!=num? 0 : num*512;
}

/**
 * set SD clock to frequency in Hz
 */
int sd_clk(unsigned int f)
{
    unsigned int d,c=41666666/f,x,s=32,h=0;
    int cnt = 100000;
    while((get32(EMMC_STATUS) & (SR_CMD_INHIBIT|SR_DAT_INHIBIT)) && cnt--) wait_msec(1);
    if(cnt<=0) {
        uart_send_string("ERROR: timeout waiting for inhibit flag\n");
        return SD_ERROR;
    }

    put32(EMMC_CONTROL1,get32(EMMC_CONTROL1) & ~C1_CLK_EN); wait_msec(10);
    x=c-1; if(!x) s=0; else {
        if(!(x & 0xffff0000u)) { x <<= 16; s -= 16; }
        if(!(x & 0xff000000u)) { x <<= 8;  s -= 8; }
        if(!(x & 0xf0000000u)) { x <<= 4;  s -= 4; }
        if(!(x & 0xc0000000u)) { x <<= 2;  s -= 2; }
        if(!(x & 0x80000000u)) { x <<= 1;  s -= 1; }
        if(s>0) s--;
        if(s>7) s=7;
    }
    if(sd_hv>HOST_SPEC_V2) d=c; else d=(1<<s);
    if(d<=2) {d=2;s=0;}
    uart_send_string("sd_clk divisor ");uart_hex(d);uart_send_string(", shift ");uart_hex(s);uart_send_string("\n");
    if(sd_hv>HOST_SPEC_V2) h=(d&0x300)>>2;
    d=(((d&0x0ff)<<8)|h);
    put32(EMMC_CONTROL1,(get32(EMMC_CONTROL1) & 0xffff003f)|d); wait_msec(10);
    put32(EMMC_CONTROL1, get32(EMMC_CONTROL1) | C1_CLK_EN); wait_msec(10);
    cnt=10000; while(!(get32(EMMC_CONTROL1) & C1_CLK_STABLE) && cnt--) wait_msec(10);
    if(cnt<=0) {
        uart_send_string("ERROR: failed to get stable clock\n");
        return SD_ERROR;
    }
    return SD_OK;
}

/**
 * initialize EMMC to read SDHC card
 */
int sd_init()
{
    long r,cnt,ccs=0;
    // GPIO_CD
    r=get32(GPFSEL4); r&=~(7<<(7*3)); put32(GPFSEL4,r);
    put32(GPPUD,2); wait_cycles(150); put32(GPPUDCLK1,(1<<15)); wait_cycles(150); put32(GPPUD,0); put32(GPPUDCLK1,0);
    r=get32(GPHEN1); r|=1<<15; put32(GPHEN1,r);

    // GPIO_CLK, GPIO_CMD
    r=get32(GPFSEL4); r|=(7<<(8*3))|(7<<(9*3)); put32(GPFSEL4,r);
    put32(GPPUD,2); wait_cycles(150); put32(GPPUDCLK1,(1<<16)|(1<<17)); wait_cycles(150); put32(GPPUD,0); put32(GPPUDCLK1,0);

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r=get32(GPFSEL5); r|=(7<<(0*3)) | (7<<(1*3)) | (7<<(2*3)) | (7<<(3*3)); put32(GPFSEL5,r);
    put32(GPPUD,2); wait_cycles(150);
    put32(GPPUDCLK1,(1<<18) | (1<<19) | (1<<20) | (1<<21));
    wait_cycles(150); put32(GPPUD,0); put32(GPPUDCLK1,0);

    sd_hv = (get32(EMMC_SLOTISR_VER) & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;
    uart_send_string("EMMC: GPIO set up\n");
    // Reset the card.
    put32(EMMC_CONTROL0, 0); put32(EMMC_CONTROL1,get32(EMMC_CONTROL1) | C1_SRST_HC);
    cnt=10000; do{wait_msec(10);} while( (get32(EMMC_CONTROL1) & C1_SRST_HC) && cnt-- );
    if(cnt<=0) {
        uart_send_string("ERROR: failed to reset EMMC\n");
        return SD_ERROR;
    }
    uart_send_string("EMMC: reset OK\n");
    put32(EMMC_CONTROL1, get32(EMMC_CONTROL1) | C1_CLK_INTLEN | C1_TOUNIT_MAX);
    wait_msec(10);
    // Set clock to setup frequency.
    if((r=sd_clk(400000))) return r;
    put32(EMMC_INT_EN,0xffffffff);
    put32(EMMC_INT_MASK,0xffffffff);
    sd_scr[0]=sd_scr[1]=sd_rca=sd_err=0;
    sd_cmd(CMD_GO_IDLE,0);
    if(sd_err) return sd_err;

    sd_cmd(CMD_SEND_IF_COND,0x000001AA);
    if(sd_err) return sd_err;
    cnt=6; r=0; while(!(r&ACMD41_CMD_COMPLETE) && cnt--) {
        wait_cycles(400);
        r=sd_cmd(CMD_SEND_OP_COND,ACMD41_ARG_HC);
        uart_send_string("EMMC: CMD_SEND_OP_COND returned ");
        if(r&ACMD41_CMD_COMPLETE)
            uart_send_string("COMPLETE ");
        if(r&ACMD41_VOLTAGE)
            uart_send_string("VOLTAGE ");
        if(r&ACMD41_CMD_CCS)
            uart_send_string("CCS ");
        uart_hex(r>>32);
        uart_hex(r);
        uart_send_string("\n");
        if(sd_err!=SD_TIMEOUT && sd_err!=SD_OK ) {
            uart_send_string("ERROR: EMMC ACMD41 returned error\n");
            return sd_err;
        }
    }
    if(!(r&ACMD41_CMD_COMPLETE) || !cnt ) return SD_TIMEOUT;
    if(!(r&ACMD41_VOLTAGE)) return SD_ERROR;
    if(r&ACMD41_CMD_CCS) ccs=SCR_SUPP_CCS;

    sd_cmd(CMD_ALL_SEND_CID,0);

    sd_rca = sd_cmd(CMD_SEND_REL_ADDR,0);
    uart_send_string("EMMC: CMD_SEND_REL_ADDR returned ");
    uart_hex(sd_rca>>32);
    uart_hex(sd_rca);
    uart_send_string("\n");
    if(sd_err) return sd_err;

    if((r=sd_clk(25000000))) return r;

    sd_cmd(CMD_CARD_SELECT,sd_rca);
    if(sd_err) return sd_err;

    if(sd_status(SR_DAT_INHIBIT)) return SD_TIMEOUT;
    put32(EMMC_BLKSIZECNT, (1<<16) | 8);
    sd_cmd(CMD_SEND_SCR,0);
    if(sd_err) return sd_err;
    if(sd_int(INT_READ_RDY)) return SD_TIMEOUT;

    r=0; cnt=100000; while(r<2 && cnt) {
        if( get32(EMMC_STATUS) & SR_READ_AVAILABLE )
            sd_scr[r++] = get32(EMMC_DATA);
        else
            wait_msec(1);
    }
    if(r!=2) return SD_TIMEOUT;
    if(sd_scr[0] & SCR_SD_BUS_WIDTH_4) {
        sd_cmd(CMD_SET_BUS_WIDTH,sd_rca|2);
        if(sd_err) return sd_err;
        put32(EMMC_CONTROL0,get32(EMMC_CONTROL0) | C0_HCTL_DWITDH);
    }
    // add software flag
    uart_send_string("EMMC: supports ");
    if(sd_scr[0] & SCR_SUPP_SET_BLKCNT)
        uart_send_string("SET_BLKCNT ");
    if(ccs)
        uart_send_string("CCS ");
    uart_send_string("\n");
    sd_scr[0]&=~SCR_SUPP_CCS;
    sd_scr[0]|=ccs;
    return SD_OK;
}