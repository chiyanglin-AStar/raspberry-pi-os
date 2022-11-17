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

//  Power mmio definition -------------------------------
#define PM_RSTC         (PBASE+0x0010001c)
#define PM_RSTS         (PBASE+0x00100020)
#define PM_WDOG         (PBASE+0x00100024)
// Power mmio --------------------------------------------


// SD and emmc mmio
#define EMMC_ARG2           (PBASE+0x00300000)
#define EMMC_BLKSIZECNT     (PBASE+0x00300004)
#define EMMC_ARG1           (PBASE+0x00300008)
#define EMMC_CMDTM          (PBASE+0x0030000C)
#define EMMC_RESP0          (PBASE+0x00300010)
#define EMMC_RESP1          (PBASE+0x00300014)
#define EMMC_RESP2          (PBASE+0x00300018)
#define EMMC_RESP3          (PBASE+0x0030001C)
#define EMMC_DATA           (PBASE+0x00300020)
#define EMMC_STATUS         (PBASE+0x00300024)
#define EMMC_CONTROL0       (PBASE+0x00300028)
#define EMMC_CONTROL1       (PBASE+0x0030002C)
#define EMMC_INTERRUPT      (PBASE+0x00300030)
#define EMMC_INT_MASK       (PBASE+0x00300034)
#define EMMC_INT_EN         (PBASE+0x00300038)
#define EMMC_CONTROL2       (PBASE+0x0030003C)
#define EMMC_SLOTISR_VER    (PBASE+0x003000FC)
// SD and emmc mmio ---------------------------------------------------------


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
#define MBOX_TAG_SETPOWER       0x28001
#define MBOX_TAG_SETCLKRATE     0x38002
#define MBOX_TAG_LAST           0

//  mbox openration ---------------------------------------

/* Power Mask definition */
#define PM_WDOG_MAGIC   0x5a000000
#define PM_RSTC_FULLRST 0x00000020
// power --------------------------------------------------

/* SD and emmc control */
// command flags
#define CMD_NEED_APP        0x80000000
#define CMD_RSPNS_48        0x00020000
#define CMD_ERRORS_MASK     0xfff9c004
#define CMD_RCA_MASK        0xffff0000

// COMMANDs
#define CMD_GO_IDLE         0x00000000
#define CMD_ALL_SEND_CID    0x02010000
#define CMD_SEND_REL_ADDR   0x03020000
#define CMD_CARD_SELECT     0x07030000
#define CMD_SEND_IF_COND    0x08020000
#define CMD_STOP_TRANS      0x0C030000
#define CMD_READ_SINGLE     0x11220010
#define CMD_READ_MULTI      0x12220032
#define CMD_SET_BLOCKCNT    0x17020000
#define CMD_APP_CMD         0x37000000
#define CMD_SET_BUS_WIDTH   (0x06020000|CMD_NEED_APP)
#define CMD_SEND_OP_COND    (0x29020000|CMD_NEED_APP)
#define CMD_SEND_SCR        (0x33220010|CMD_NEED_APP)

// STATUS register settings
#define SR_READ_AVAILABLE   0x00000800
#define SR_DAT_INHIBIT      0x00000002
#define SR_CMD_INHIBIT      0x00000001
#define SR_APP_CMD          0x00000020

// INTERRUPT register settings
#define INT_DATA_TIMEOUT    0x00100000
#define INT_CMD_TIMEOUT     0x00010000
#define INT_READ_RDY        0x00000020
#define INT_CMD_DONE        0x00000001

#define INT_ERROR_MASK      0x017E8000

// CONTROL register settings
#define C0_SPI_MODE_EN      0x00100000
#define C0_HCTL_HS_EN       0x00000004
#define C0_HCTL_DWITDH      0x00000002

#define C1_SRST_DATA        0x04000000
#define C1_SRST_CMD         0x02000000
#define C1_SRST_HC          0x01000000
#define C1_TOUNIT_DIS       0x000f0000
#define C1_TOUNIT_MAX       0x000e0000
#define C1_CLK_GENSEL       0x00000020
#define C1_CLK_EN           0x00000004
#define C1_CLK_STABLE       0x00000002
#define C1_CLK_INTLEN       0x00000001

// SLOTISR_VER values
#define HOST_SPEC_NUM       0x00ff0000
#define HOST_SPEC_NUM_SHIFT 16
#define HOST_SPEC_V3        2
#define HOST_SPEC_V2        1
#define HOST_SPEC_V1        0

// SCR flags
#define SCR_SD_BUS_WIDTH_4  0x00000400
#define SCR_SUPP_SET_BLKCNT 0x02000000
// added by my driver
#define SCR_SUPP_CCS        0x00000001

#define ACMD41_VOLTAGE      0x00ff8000
#define ACMD41_CMD_COMPLETE 0x80000000
#define ACMD41_CMD_CCS      0x40000000
#define ACMD41_ARG_HC       0x51ff8000

#define SD_OK                0
#define SD_TIMEOUT          -1
#define SD_ERROR            -2
// SD and emmc --------------------------------------------

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

/*  Power function declare */
void power_off();
void reset();
//  Power --------------------------------------------------

/* frame buffee functions declare  */
void lfb_init();
void lfb_showpicture();
// frame buffer --------------------------------------------

/* SD and emmc control */
int sd_init();
int sd_readblock(unsigned int lba, unsigned char *buffer, unsigned int num);
// SD and emmc ----------------------------------------------
