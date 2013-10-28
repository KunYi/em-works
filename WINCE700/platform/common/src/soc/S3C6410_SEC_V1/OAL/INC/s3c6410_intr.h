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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Header:  s3c6410_intr.h
//
//  Defines the interrupt controller register layout and associated interrupt
//  sources and bit masks.
//
#ifndef __S3C6410_INTR_H
#define __S3C6410_INTR_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C6410_VIC_REG
//
//  Interrupt control registers. This register bank is located by the constant
//  S3C6410_BASE_REG_XX_VICX in the configuration file s3c6410_base_reg_cfg.h.
//

typedef struct
{
    UINT32 VICIRQSTATUS;        // 0x00
    UINT32 VICFIQSTATUS;        // 0x04
    UINT32 VICRAWINTR;            // 0x08
    UINT32 VICINTSELECT;        // 0x0c

    UINT32 VICINTENABLE;        // 0x10
    UINT32 VICINTENCLEAR;        // 0x14
    UINT32 VICSOFTINT;            // 0x18
    UINT32 VICSOFTINTCLEAR;    // 0x1c

    UINT32 VICPROTECTION;        // 0x20
    UINT32 VICSWPRIORITYMASK;    // 0x24
    UINT32 VICPRIORITYDAISY;    // 0x28
    UINT32 PAD0;                // 0x2c

    UINT32 PAD1[52];                // 0x30~0xff

    UINT32 VICVECTADDR0;        // 0x100
    UINT32 VICVECTADDR1;        // 0x104
    UINT32 VICVECTADDR2;        // 0x108
    UINT32 VICVECTADDR3;        // 0x10c

    UINT32 VICVECTADDR4;        // 0x110
    UINT32 VICVECTADDR5;        // 0x114
    UINT32 VICVECTADDR6;        // 0x118
    UINT32 VICVECTADDR7;        // 0x11c

    UINT32 VICVECTADDR8;        // 0x120
    UINT32 VICVECTADDR9;        // 0x124
    UINT32 VICVECTADDR10;        // 0x128
    UINT32 VICVECTADDR11;        // 0x12c

    UINT32 VICVECTADDR12;        // 0x130
    UINT32 VICVECTADDR13;        // 0x134
    UINT32 VICVECTADDR14;        // 0x138
    UINT32 VICVECTADDR15;        // 0x13c

    UINT32 VICVECTADDR16;        // 0x140
    UINT32 VICVECTADDR17;        // 0x144
    UINT32 VICVECTADDR18;        // 0x148
    UINT32 VICVECTADDR19;        // 0x14c

    UINT32 VICVECTADDR20;        // 0x150
    UINT32 VICVECTADDR21;        // 0x154
    UINT32 VICVECTADDR22;        // 0x158
    UINT32 VICVECTADDR23;        // 0x15c

    UINT32 VICVECTADDR24;        // 0x160
    UINT32 VICVECTADDR25;        // 0x164
    UINT32 VICVECTADDR26;        // 0x168
    UINT32 VICVECTADDR27;        // 0x16c

    UINT32 VICVECTADDR28;        // 0x170
    UINT32 VICVECTADDR29;        // 0x174
    UINT32 VICVECTADDR30;        // 0x178
    UINT32 VICVECTADDR31;        // 0x17c

    UINT32 PAD2[32];                // 0x180~0x1ff

    UINT32 VICVECTPRIORITY0;    // 0x200
    UINT32 VICVECTPRIORITY1;    // 0x204
    UINT32 VICVECTPRIORITY2;    // 0x208
    UINT32 VICVECTPRIORITY3;    // 0x20c

    UINT32 VICVECTPRIORITY4;    // 0x210
    UINT32 VICVECTPRIORITY5;    // 0x214
    UINT32 VICVECTPRIORITY6;    // 0x218
    UINT32 VICVECTPRIORITY7;    // 0x21c

    UINT32 VICVECTPRIORITY8;    // 0x220
    UINT32 VICVECTPRIORITY9;    // 0x224
    UINT32 VICVECTPRIORITY10;    // 0x228
    UINT32 VICVECTPRIORITY11;    // 0x22c

    UINT32 VICVECTPRIORITY12;    // 0x230
    UINT32 VICVECTPRIORITY13;    // 0x234
    UINT32 VICVECTPRIORITY14;    // 0x238
    UINT32 VICVECTPRIORITY15;    // 0x23c

    UINT32 VICVECTPRIORITY16;    // 0x240
    UINT32 VICVECTPRIORITY17;    // 0x244
    UINT32 VICVECTPRIORITY18;    // 0x248
    UINT32 VICVECTPRIORITY19;    // 0x24c

    UINT32 VICVECTPRIORITY20;    // 0x250
    UINT32 VICVECTPRIORITY21;    // 0x254
    UINT32 VICVECTPRIORITY22;    // 0x258
    UINT32 VICVECTPRIORITY23;    // 0x25c

    UINT32 VICVECTPRIORITY24;    // 0x260
    UINT32 VICVECTPRIORITY25;    // 0x264
    UINT32 VICVECTPRIORITY26;    // 0x268
    UINT32 VICVECTPRIORITY27;    // 0x26c

    UINT32 VICVECTPRIORITY28;    // 0x270
    UINT32 VICVECTPRIORITY29;    // 0x274
    UINT32 VICVECTPRIORITY30;    // 0x278
    UINT32 VICVECTPRIORITY31;    // 0x27c

    UINT32 PAD3[800];            // 0x280~0xeff

    UINT32 VICADDRESS;            // 0xf00
    UINT32 PAD4[3];                // 0xf04~0xf0f

    UINT32 PAD5[52];                // 0xf10~0xfdf

    UINT32 VICPERIPHID0;        // 0xfe0
    UINT32 VICPERIPHID1;        // 0xfe4
    UINT32 VICPERIPHID2;        // 0xfe8
    UINT32 VICPERIPHID3;        // 0xfec

    UINT32 VICPCELLID0;            // 0xff0
    UINT32 VICPCELLID1;            // 0xff4
    UINT32 VICPCELLID2;            // 0xff8
    UINT32 VICPCELLID3;            // 0xffc
} S3C6410_VIC_REG, *PS3C6410_VIC_REG;

//------------------------------------------------------------------------------
//
//  Define: PHYIRQ_XXX
//
//  Physical Interrupt Sources Numbers
//  S3C6410 has two VIC controller. This VIC controller has it's own IRQ number
//  to handle interrupt from each interrupt source. But this IRQ has sub interrupt source
//  So we flatten this sub interrupt source into Virtual IRQ concept.
//  In other words, this a PHYIRQ is mapped to some Virtual IRQ.
//
//  Drivers and Application have to use this Virtual IRQ Numbers
//  Virtual IRQ is defined in "s3c6410_vintr.h"

// VIC0
#define PHYIRQ_EINT0            0
#define PHYIRQ_EINT1            1
#define PHYIRQ_RTC_TIC          2
#define PHYIRQ_CAMIF_C          3
#define PHYIRQ_CAMIF_P          4

#define PHYIRQ_I2C1             5
#define PHYIRQ_I2S_V40          6
#define PHYIRQ_SSS              7
#define PHYIRQ_3D               8

#define PHYIRQ_POST             9
#define PHYIRQ_ROTATOR          10
#define PHYIRQ_2D               11
#define PHYIRQ_TVENC            12
#define PHYIRQ_TVSCALER         13
#define PHYIRQ_BATF             14
#define PHYIRQ_JPEG             15
#define PHYIRQ_MFC              16
#define PHYIRQ_SDMA0            17
#define PHYIRQ_SDMA1            18
#define PHYIRQ_ARM_DMAERR       19
#define PHYIRQ_ARM_DMA          20
#define PHYIRQ_ARM_DMAS         21
#define PHYIRQ_KEYPAD           22
#define PHYIRQ_TIMER0           23
#define PHYIRQ_TIMER1           24
#define PHYIRQ_TIMER2           25
#define PHYIRQ_WDT              26
#define PHYIRQ_TIMER3           27
#define PHYIRQ_TIMER4           28
#define PHYIRQ_LCD0_FIFO        29
#define PHYIRQ_LCD1_FRAME       30
#define PHYIRQ_LCD2_SYSIF       31

// VIC1
#define PHYIRQ_EINT2            32
#define PHYIRQ_EINT3            33
#define PHYIRQ_PCM0             34
#define PHYIRQ_PCM1             35
#define PHYIRQ_AC97             36
#define PHYIRQ_UART0            37
#define PHYIRQ_UART1            38
#define PHYIRQ_UART2            39
#define PHYIRQ_UART3            40
#define PHYIRQ_DMA0             41
#define PHYIRQ_DMA1             42
#define PHYIRQ_ONENAND0         43
#define PHYIRQ_ONENAND1         44
#define PHYIRQ_NFC              45
#define PHYIRQ_CFC              46
#define PHYIRQ_UHOST            47
#define PHYIRQ_SPI0             48
#define PHYIRQ_SPI1             49
#define PHYIRQ_I2C              50
#define PHYIRQ_HSITX            51
#define PHYIRQ_HSIRX            52
#define PHYIRQ_RESERVED         53
#define PHYIRQ_MSM              54
#define PHYIRQ_HOSTIF           55
#define PHYIRQ_HSMMC0           56
#define PHYIRQ_HSMMC1           57
#define PHYIRQ_OTG              58
#define PHYIRQ_IRDA             59
#define PHYIRQ_RTC_ALARM        60
#define PHYIRQ_SEC              61
#define PHYIRQ_PENDN            62
#define PHYIRQ_ADC              63

#define PHYIRQ_MAX_S3C6410      64

#define VIC1_BIT_OFFSET         32
//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif    // __S3C6410_INTR_H
