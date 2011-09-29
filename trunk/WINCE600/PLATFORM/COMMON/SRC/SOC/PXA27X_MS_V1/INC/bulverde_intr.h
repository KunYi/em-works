//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  Header:  bulverde_intr.h
//
//  Defines the interrupt controller register layout and associated interrupt
//  sources and bit masks.
//
#ifndef __BULVERDE_INTR_H
#define __BULVERDE_INTR_H

#if __cplusplus
extern "C" {
#endif

//-----------------------------------------------------
//
//  Define: Maximum number of IRQs and number of IRQs per SYSINTR
//
#define PXA27X_IRQ_MAXIMUM         256
#define PXA27X_IRQ_PER_SYSINTR     4

//-----------------------------------------------------
//
//  Define: IRQ_XXX
//
//  Interrupt sources numbers
//

#define IRQ_CAMQCKCAP  33
#define IRQ_WTM        32
#define IRQ_RTCALARM   31
#define IRQ_RTC_TIC    30
#define IRQ_OSMR3      29
#define IRQ_OSMR2      28
#define IRQ_OSMR1      27
#define IRQ_OSMR0      26
#define IRQ_DMAC       25
#define IRQ_SSP        24
#define IRQ_MMC        23
#define IRQ_FFUART     22
#define IRQ_BTUART     21
#define IRQ_STUART     20
#define IRQ_ICP        19
#define IRQ_I2C        18
#define IRQ_LCD        17
#define IRQ_SSP2       16
#define IRQ_USIM       15
#define IRQ_AC97       14
#define IRQ_I2S        13
#define IRQ_PMU        12
#define IRQ_USBFN      11
#define IRQ_GPIOXX_2   10
#define IRQ_GPIO1      9
#define IRQ_GPIO0      8
#define IRQ_OSMRXX_4   7
#define IRQ_PWRI2C     6
#define IRQ_MEMSTICK   5
#define IRQ_KEYPAD     4
#define IRQ_USBOHCI    3
#define IRQ_USBNONOHCI 2
#define IRQ_BASEBAND   1
#define IRQ_SSP3       0

#define IRQ_BULVERDE_MAX    63      //icmr (32) + icmr2 (32) 

//-----------------------------------------------------
//
//  Define: IRQ_GPIOXX_2_GPIO2 to IRQ_GPIOXX_2_GPIO120
//
//  GPIO interrupts on GPIO pins 120:2 that raise IRQ_GPIOXX_2
//  

#define IRQ_GPIOXX_2_GPIO2 100
#define IRQ_GPIOXX_2_GPIO3 (IRQ_GPIOXX_2_GPIO2 + 1)
#define IRQ_GPIOXX_2_GPIO4 (IRQ_GPIOXX_2_GPIO2 + 2)
#define IRQ_GPIOXX_2_GPIO5 (IRQ_GPIOXX_2_GPIO2 + 3)
#define IRQ_GPIOXX_2_GPIO6 (IRQ_GPIOXX_2_GPIO2 + 4)
#define IRQ_GPIOXX_2_GPIO7 (IRQ_GPIOXX_2_GPIO2 + 5)
#define IRQ_GPIOXX_2_GPIO8 (IRQ_GPIOXX_2_GPIO2 + 6)
#define IRQ_GPIOXX_2_GPIO9 (IRQ_GPIOXX_2_GPIO2 + 7)
#define IRQ_GPIOXX_2_GPIO10 (IRQ_GPIOXX_2_GPIO2 + 8)
#define IRQ_GPIOXX_2_GPIO11 (IRQ_GPIOXX_2_GPIO2 + 9)
#define IRQ_GPIOXX_2_GPIO12 (IRQ_GPIOXX_2_GPIO2 + 10)
#define IRQ_GPIOXX_2_GPIO13 (IRQ_GPIOXX_2_GPIO2 + 11)
#define IRQ_GPIOXX_2_GPIO14 (IRQ_GPIOXX_2_GPIO2 + 12)
#define IRQ_GPIOXX_2_GPIO15 (IRQ_GPIOXX_2_GPIO2 + 13)
#define IRQ_GPIOXX_2_GPIO16 (IRQ_GPIOXX_2_GPIO2 + 14)
#define IRQ_GPIOXX_2_GPIO17 (IRQ_GPIOXX_2_GPIO2 + 15)
#define IRQ_GPIOXX_2_GPIO18 (IRQ_GPIOXX_2_GPIO2 + 16)
#define IRQ_GPIOXX_2_GPIO19 (IRQ_GPIOXX_2_GPIO2 + 17)
#define IRQ_GPIOXX_2_GPIO20 (IRQ_GPIOXX_2_GPIO2 + 18)
#define IRQ_GPIOXX_2_GPIO21 (IRQ_GPIOXX_2_GPIO2 + 19)
#define IRQ_GPIOXX_2_GPIO22 (IRQ_GPIOXX_2_GPIO2 + 20)
#define IRQ_GPIOXX_2_GPIO23 (IRQ_GPIOXX_2_GPIO2 + 21)
#define IRQ_GPIOXX_2_GPIO24 (IRQ_GPIOXX_2_GPIO2 + 22)
#define IRQ_GPIOXX_2_GPIO25 (IRQ_GPIOXX_2_GPIO2 + 23)
#define IRQ_GPIOXX_2_GPIO26 (IRQ_GPIOXX_2_GPIO2 + 24)
#define IRQ_GPIOXX_2_GPIO27 (IRQ_GPIOXX_2_GPIO2 + 25)
#define IRQ_GPIOXX_2_GPIO28 (IRQ_GPIOXX_2_GPIO2 + 26)
#define IRQ_GPIOXX_2_GPIO29 (IRQ_GPIOXX_2_GPIO2 + 27)
#define IRQ_GPIOXX_2_GPIO30 (IRQ_GPIOXX_2_GPIO2 + 28)
#define IRQ_GPIOXX_2_GPIO31 (IRQ_GPIOXX_2_GPIO2 + 29)
#define IRQ_GPIOXX_2_GPIO32 (IRQ_GPIOXX_2_GPIO2 + 30)
#define IRQ_GPIOXX_2_GPIO33 (IRQ_GPIOXX_2_GPIO2 + 31)
#define IRQ_GPIOXX_2_GPIO34 (IRQ_GPIOXX_2_GPIO2 + 32)
#define IRQ_GPIOXX_2_GPIO35 (IRQ_GPIOXX_2_GPIO2 + 33)
#define IRQ_GPIOXX_2_GPIO36 (IRQ_GPIOXX_2_GPIO2 + 34)
#define IRQ_GPIOXX_2_GPIO37 (IRQ_GPIOXX_2_GPIO2 + 35)
#define IRQ_GPIOXX_2_GPIO38 (IRQ_GPIOXX_2_GPIO2 + 36)
#define IRQ_GPIOXX_2_GPIO39 (IRQ_GPIOXX_2_GPIO2 + 37)
#define IRQ_GPIOXX_2_GPIO40 (IRQ_GPIOXX_2_GPIO2 + 38)
#define IRQ_GPIOXX_2_GPIO41 (IRQ_GPIOXX_2_GPIO2 + 39)
#define IRQ_GPIOXX_2_GPIO42 (IRQ_GPIOXX_2_GPIO2 + 40)
#define IRQ_GPIOXX_2_GPIO43 (IRQ_GPIOXX_2_GPIO2 + 41)
#define IRQ_GPIOXX_2_GPIO44 (IRQ_GPIOXX_2_GPIO2 + 42)
#define IRQ_GPIOXX_2_GPIO45 (IRQ_GPIOXX_2_GPIO2 + 43)
#define IRQ_GPIOXX_2_GPIO46 (IRQ_GPIOXX_2_GPIO2 + 44)
#define IRQ_GPIOXX_2_GPIO47 (IRQ_GPIOXX_2_GPIO2 + 45)
#define IRQ_GPIOXX_2_GPIO48 (IRQ_GPIOXX_2_GPIO2 + 46)
#define IRQ_GPIOXX_2_GPIO49 (IRQ_GPIOXX_2_GPIO2 + 47)
#define IRQ_GPIOXX_2_GPIO50 (IRQ_GPIOXX_2_GPIO2 + 48)
#define IRQ_GPIOXX_2_GPIO51 (IRQ_GPIOXX_2_GPIO2 + 49)
#define IRQ_GPIOXX_2_GPIO52 (IRQ_GPIOXX_2_GPIO2 + 50)
#define IRQ_GPIOXX_2_GPIO53 (IRQ_GPIOXX_2_GPIO2 + 51)
#define IRQ_GPIOXX_2_GPIO54 (IRQ_GPIOXX_2_GPIO2 + 52)
#define IRQ_GPIOXX_2_GPIO55 (IRQ_GPIOXX_2_GPIO2 + 53)
#define IRQ_GPIOXX_2_GPIO56 (IRQ_GPIOXX_2_GPIO2 + 54)
#define IRQ_GPIOXX_2_GPIO57 (IRQ_GPIOXX_2_GPIO2 + 55)
#define IRQ_GPIOXX_2_GPIO58 (IRQ_GPIOXX_2_GPIO2 + 56)
#define IRQ_GPIOXX_2_GPIO59 (IRQ_GPIOXX_2_GPIO2 + 57)
#define IRQ_GPIOXX_2_GPIO60 (IRQ_GPIOXX_2_GPIO2 + 58)
#define IRQ_GPIOXX_2_GPIO61 (IRQ_GPIOXX_2_GPIO2 + 59)
#define IRQ_GPIOXX_2_GPIO62 (IRQ_GPIOXX_2_GPIO2 + 60)
#define IRQ_GPIOXX_2_GPIO63 (IRQ_GPIOXX_2_GPIO2 + 61)
#define IRQ_GPIOXX_2_GPIO64 (IRQ_GPIOXX_2_GPIO2 + 62)
#define IRQ_GPIOXX_2_GPIO65 (IRQ_GPIOXX_2_GPIO2 + 63)
#define IRQ_GPIOXX_2_GPIO66 (IRQ_GPIOXX_2_GPIO2 + 64)
#define IRQ_GPIOXX_2_GPIO67 (IRQ_GPIOXX_2_GPIO2 + 65)
#define IRQ_GPIOXX_2_GPIO68 (IRQ_GPIOXX_2_GPIO2 + 66)
#define IRQ_GPIOXX_2_GPIO69 (IRQ_GPIOXX_2_GPIO2 + 67)
#define IRQ_GPIOXX_2_GPIO70 (IRQ_GPIOXX_2_GPIO2 + 68)
#define IRQ_GPIOXX_2_GPIO71 (IRQ_GPIOXX_2_GPIO2 + 69)
#define IRQ_GPIOXX_2_GPIO72 (IRQ_GPIOXX_2_GPIO2 + 70)
#define IRQ_GPIOXX_2_GPIO73 (IRQ_GPIOXX_2_GPIO2 + 71)
#define IRQ_GPIOXX_2_GPIO74 (IRQ_GPIOXX_2_GPIO2 + 72)
#define IRQ_GPIOXX_2_GPIO75 (IRQ_GPIOXX_2_GPIO2 + 73)
#define IRQ_GPIOXX_2_GPIO76 (IRQ_GPIOXX_2_GPIO2 + 74)
#define IRQ_GPIOXX_2_GPIO77 (IRQ_GPIOXX_2_GPIO2 + 75)
#define IRQ_GPIOXX_2_GPIO78 (IRQ_GPIOXX_2_GPIO2 + 76)
#define IRQ_GPIOXX_2_GPIO79 (IRQ_GPIOXX_2_GPIO2 + 77)
#define IRQ_GPIOXX_2_GPIO80 (IRQ_GPIOXX_2_GPIO2 + 78)
#define IRQ_GPIOXX_2_GPIO81 (IRQ_GPIOXX_2_GPIO2 + 79)
#define IRQ_GPIOXX_2_GPIO82 (IRQ_GPIOXX_2_GPIO2 + 80)
#define IRQ_GPIOXX_2_GPIO83 (IRQ_GPIOXX_2_GPIO2 + 81)
#define IRQ_GPIOXX_2_GPIO84 (IRQ_GPIOXX_2_GPIO2 + 82)
#define IRQ_GPIOXX_2_GPIO85 (IRQ_GPIOXX_2_GPIO2 + 83)
#define IRQ_GPIOXX_2_GPIO86 (IRQ_GPIOXX_2_GPIO2 + 84)
#define IRQ_GPIOXX_2_GPIO87 (IRQ_GPIOXX_2_GPIO2 + 85)
#define IRQ_GPIOXX_2_GPIO88 (IRQ_GPIOXX_2_GPIO2 + 86)
#define IRQ_GPIOXX_2_GPIO89 (IRQ_GPIOXX_2_GPIO2 + 87)
#define IRQ_GPIOXX_2_GPIO90 (IRQ_GPIOXX_2_GPIO2 + 88)
#define IRQ_GPIOXX_2_GPIO91 (IRQ_GPIOXX_2_GPIO2 + 89)
#define IRQ_GPIOXX_2_GPIO92 (IRQ_GPIOXX_2_GPIO2 + 90)
#define IRQ_GPIOXX_2_GPIO93 (IRQ_GPIOXX_2_GPIO2 + 91)
#define IRQ_GPIOXX_2_GPIO94 (IRQ_GPIOXX_2_GPIO2 + 92)
#define IRQ_GPIOXX_2_GPIO95 (IRQ_GPIOXX_2_GPIO2 + 93)
#define IRQ_GPIOXX_2_GPIO96 (IRQ_GPIOXX_2_GPIO2 + 94)
#define IRQ_GPIOXX_2_GPIO97 (IRQ_GPIOXX_2_GPIO2 + 95)
#define IRQ_GPIOXX_2_GPIO98 (IRQ_GPIOXX_2_GPIO2 + 96)
#define IRQ_GPIOXX_2_GPIO99 (IRQ_GPIOXX_2_GPIO2 + 97)
#define IRQ_GPIOXX_2_GPIO100 (IRQ_GPIOXX_2_GPIO2 + 98)
#define IRQ_GPIOXX_2_GPIO101 (IRQ_GPIOXX_2_GPIO2 + 99)
#define IRQ_GPIOXX_2_GPIO102 (IRQ_GPIOXX_2_GPIO2 + 100)
#define IRQ_GPIOXX_2_GPIO103 (IRQ_GPIOXX_2_GPIO2 + 101)
#define IRQ_GPIOXX_2_GPIO104 (IRQ_GPIOXX_2_GPIO2 + 102)
#define IRQ_GPIOXX_2_GPIO105 (IRQ_GPIOXX_2_GPIO2 + 103)
#define IRQ_GPIOXX_2_GPIO106 (IRQ_GPIOXX_2_GPIO2 + 104)
#define IRQ_GPIOXX_2_GPIO107 (IRQ_GPIOXX_2_GPIO2 + 105)
#define IRQ_GPIOXX_2_GPIO108 (IRQ_GPIOXX_2_GPIO2 + 106)
#define IRQ_GPIOXX_2_GPIO109 (IRQ_GPIOXX_2_GPIO2 + 107)
#define IRQ_GPIOXX_2_GPIO110 (IRQ_GPIOXX_2_GPIO2 + 108)
#define IRQ_GPIOXX_2_GPIO111 (IRQ_GPIOXX_2_GPIO2 + 109)
#define IRQ_GPIOXX_2_GPIO112 (IRQ_GPIOXX_2_GPIO2 + 110)
#define IRQ_GPIOXX_2_GPIO113 (IRQ_GPIOXX_2_GPIO2 + 111)
#define IRQ_GPIOXX_2_GPIO114 (IRQ_GPIOXX_2_GPIO2 + 112)
#define IRQ_GPIOXX_2_GPIO115 (IRQ_GPIOXX_2_GPIO2 + 113)
#define IRQ_GPIOXX_2_GPIO116 (IRQ_GPIOXX_2_GPIO2 + 114)
#define IRQ_GPIOXX_2_GPIO117 (IRQ_GPIOXX_2_GPIO2 + 115)
#define IRQ_GPIOXX_2_GPIO118 (IRQ_GPIOXX_2_GPIO2 + 116)
#define IRQ_GPIOXX_2_GPIO119 (IRQ_GPIOXX_2_GPIO2 + 117)
#define IRQ_GPIOXX_2_GPIO120 (IRQ_GPIOXX_2_GPIO2 + 118)

#define IRQ_GPIOXX_2_GPIOMIN   IRQ_GPIOXX_2_GPIO2
#define IRQ_GPIOXX_2_GPIOMAX   IRQ_GPIOXX_2_GPIO120


//------------------------------------------------------------------------------
//
//  Type: BULVERDE_INTR_REG    
//
//  Interrupt control registers.
//

#include <xllp_intc.h>

typedef XLLP_INTC_T  BULVERDE_INTR_REG;
typedef XLLP_INTC_T *PBULVERDE_INTR_REG;

#if __cplusplus
}
#endif

#endif 
