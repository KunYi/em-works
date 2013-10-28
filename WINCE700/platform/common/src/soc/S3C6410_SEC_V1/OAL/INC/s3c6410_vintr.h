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

#ifndef __S3C6410_VINTR_H
#define __S3C6410_VINTR_H

#include <oal_intr.h>

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define: IRQ_XXX
//
//  Virtual Interrupt Sources Numbers
//  Flatten Physical IRQ Numbers
//  
//  This IRQ will be tied up with SysIntr statically or dynamically.
//  Drivers and Application have to use this Virtual IRQ Numbers
//

#define IRQ_EINT0           0    // 0
#define IRQ_EINT1           1    // 0
#define IRQ_EINT2           2    // 0
#define IRQ_EINT3           3    // 0
#define IRQ_EINT4           4    // 1
#define IRQ_EINT5           5    // 1
#define IRQ_EINT6           6    // 1
#define IRQ_EINT7           7    // 1
#define IRQ_EINT8           8    // 1
#define IRQ_EINT9           9    // 1

#define IRQ_EINT10          10    // 1
#define IRQ_EINT11          11    // 1
#define IRQ_RTC_TIC         12    // 2
#define IRQ_CAMIF_C         13    // 3
#define IRQ_CAMIF_P         14    // 4

#define IRQ_I2C1            15    // 5
#define IRQ_I2S_V40         16   // 6
#define IRQ_SSS             17   // 7
#define IRQ_3D              18   // 8

#define IRQ_POST            19    // 9

#define IRQ_ROTATOR         20    // 10
#define IRQ_2D              21    // 11
#define IRQ_TVENC           22    // 12
#define IRQ_TVSCALER        23    // 13
#define IRQ_BATF            24    // 14
#define IRQ_JPEG            25    // 15
#define IRQ_MFC             26    // 16
#define IRQ_SDMA0           27    // 17
#define IRQ_SDMA1           28    // 18
#define IRQ_ARM_DMAERR      29    // 19

#define IRQ_ARM_DMA         30    // 20
#define IRQ_ARM_DMAS        31    // 21
#define IRQ_KEYPAD          32    // 22
#define IRQ_TIMER0          33    // 23
#define IRQ_TIMER1          34    // 24
#define IRQ_TIMER2          35    // 25
#define IRQ_WDT             36    // 26
#define IRQ_TIMER3          37    // 27
#define IRQ_TIMER4          38    // 28
#define IRQ_LCD0_FIFO       39    // 29

#define IRQ_LCD1_FRAME      40    // 30
#define IRQ_LCD2_SYSIF      41    // 31
#define IRQ_EINT12          42    // 32
#define IRQ_EINT13          43    // 32
#define IRQ_EINT14          44    // 32
#define IRQ_EINT15          45    // 32
#define IRQ_EINT16          46    // 32
#define IRQ_EINT17          47    // 32
#define IRQ_EINT18          48    // 32
#define IRQ_EINT19          49    // 32

#define IRQ_EINT20          50    // 33
#define IRQ_EINT21          51    // 33
#define IRQ_EINT22          52    // 33
#define IRQ_EINT23          53    // 33
#define IRQ_EINT24          54    // 33
#define IRQ_EINT25          55    // 33
#define IRQ_EINT26          56    // 33
#define IRQ_EINT27          57    // 33
#define IRQ_PCM0            58    // 34
#define IRQ_PCM1            59    // 35

#define IRQ_AC97            60    // 36
#define IRQ_UART0           61    // 37
#define IRQ_UART1           62    // 38
#define IRQ_UART2           63    // 39
#define IRQ_UART3           64    // 40
#define IRQ_DMA0_CH0        65    // 41
#define IRQ_DMA0_CH1        66    // 41
#define IRQ_DMA0_CH2        67    // 41
#define IRQ_DMA0_CH3        68    // 41
#define IRQ_DMA0_CH4        69    // 41

#define IRQ_DMA0_CH5        70    // 41
#define IRQ_DMA0_CH6        71    // 41
#define IRQ_DMA0_CH7        72    // 41
#define IRQ_DMA1_CH0        73    // 42
#define IRQ_DMA1_CH1        74    // 42
#define IRQ_DMA1_CH2        75    // 42
#define IRQ_DMA1_CH3        76    // 42
#define IRQ_DMA1_CH4        77    // 42
#define IRQ_DMA1_CH5        78    // 42
#define IRQ_DMA1_CH6        79    // 42

#define IRQ_DMA1_CH7        80    // 42
#define IRQ_ONENAND0        81    // 43
#define IRQ_ONENAND1        82    // 44
#define IRQ_NFC             83    // 45
#define IRQ_CFC             84    // 46
#define IRQ_UHOST           85    // 47
#define IRQ_SPI0            86    // 48
#define IRQ_SPI1            87    // 49
#define IRQ_I2C             88    // 50
#define IRQ_HSITX           89    // 51

#define IRQ_HSIRX           90    // 52
#define IRQ_RESERVED        91    // 53
#define IRQ_MSM             92    // 54
#define IRQ_HOSTIF          93    // 55
#define IRQ_HSMMC0          94    // 56
#define IRQ_HSMMC1          95    // 57
#define IRQ_OTG             96    // 58
#define IRQ_IRDA            97    // 59
#define IRQ_RTC_ALARM       98    // 60
#define IRQ_SEC             99    // 61

#define IRQ_PENDN           100    // 62
#define IRQ_ADC             101    // 63

#define IRQ_MAX_S3C6410     102

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//
//  Define:  IRQ_MAXIMUM
//
//  This value define maximum number of IRQs. Even if there isn't any
//  limitation for this number in OAL library, Windows CE resource manager
//  support only 64 IRQs currently.
//
#undef OAL_INTR_IRQ_MAXIMUM
#define OAL_INTR_IRQ_MAXIMUM        IRQ_MAX_S3C6410

#if __cplusplus
}
#endif

#endif

