#ifndef	_P_GPIO_H
#define	_P_GPIO_H

#include "peripherals/base.h"

#define GPFSEL1         (PBASE+0x00200004)
#define GPSET0          (PBASE+0x0020001C)
#define GPCLR0          (PBASE+0x00200028)
#define GPPUD           (PBASE+0x00200094)
#define GPPUDCLK0       (PBASE+0x00200098)

// Add for UART0 porting
#define GPFSEL0         (PBASE+0x00200000)

#define GPFSEL2         (PBASE+0x00200008)
#define GPFSEL3         (PBASE+0x0020000C)
#define GPFSEL4         (PBASE+0x00200010)
#define GPFSEL5         (PBASE+0x00200014)

#define GPSET1          (PBASE+0x00200020)
#define GPLEV0          (PBASE+0x00200034)
#define GPLEV1          (PBASE+0x00200038)
#define GPEDS0          (PBASE+0x00200040)
#define GPEDS1          (PBASE+0x00200044)
#define GPHEN0          (PBASE+0x00200064)
#define GPHEN1          (PBASE+0x00200068)

#define GPPUDCLK1       (PBASE+0x0020009C)

/* PL011 UART0 registers */
#define UART0_DR        (PBASE+0x00201000)
#define UART0_FR        (PBASE+0x00201018)
#define UART0_IBRD      (PBASE+0x00201024)
#define UART0_FBRD      (PBASE+0x00201028)
#define UART0_LCRH      (PBASE+0x0020102C)
#define UART0_CR        (PBASE+0x00201030)
#define UART0_IMSC      (PBASE+0x00201038)
#define UART0_ICR       (PBASE+0x00201044)



/*
in https://github.com/bztsrc/raspi3-tutorial
03_uart1  uart.c
*/
#if 0 // for reference
/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(PBASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(PBASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(PBASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(PBASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(PBASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(PBASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(PBASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(PBASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(PBASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(PBASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(PBASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(PBASE+0x00215068))
#endif
#endif  /*_P_GPIO_H */
