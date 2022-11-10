//  mbox part definition

/* mbox mmio defintion  */
#define VIDEOCORE_MBOX  (PBASE+0x0000B880)
#define MBOX_READ       (VIDEOCORE_MBOX+0x0)
#define MBOX_POLL       (VIDEOCORE_MBOX+0x10)
#define MBOX_SENDER     (VIDEOCORE_MBOX+0x14)
#define MBOX_STATUS     (VIDEOCORE_MBOX+0x18)
#define MBOX_CONFIG     (VIDEOCORE_MBOX+0x1C)
#define MBOX_WRITE      (VIDEOCORE_MBOX+0x20)
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000
//  mbox mmio ------------------------------------------

/* random number generator mmio definition */
#define RNG_CTRL        (PBASE+0x00104000)
#define RNG_STATUS      (PBASE+0x00104004)
#define RNG_DATA        (PBASE+0x00104008)
#define RNG_INT_MASK    (PBASE+0x00104010)
// random number generator mmio -------------------------

/* Timer mmio definition */
#define SYSTMR_LO        (PBASE+0x00003004)
#define SYSTMR_HI        (PBASE+0x00003008)
// Timer mmio -------------------------------------------

//  mbox operation defintion
#define MBOX_REQUEST    0

/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* tags */
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_LAST           0
//  mbox openration ---------------------------------------

/*  mbox data struction and function declare */
/* a properly aligned buffer */
extern volatile unsigned int mbox[36];

int mbox_call(unsigned char ch);
//  mbox data struction and function -----------------------

/*  random function declare  */
void rand_init();
unsigned int rand(unsigned int min, unsigned int max);
//  random -------------------------------------------------

/*  Timer and Delay function declare  */ 
void wait_cycles(unsigned int n);
void wait_msec(unsigned int n);
unsigned long get_system_timer();
void wait_msec_st(unsigned int n);
//  Timer and Delay ---------------------------------------- 