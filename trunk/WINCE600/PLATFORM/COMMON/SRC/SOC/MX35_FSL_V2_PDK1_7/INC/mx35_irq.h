//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx35_irq.h
//
//  This file defines names for IRQ. These names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
//  This header contains IRQ definitions for the MX35 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX35_IRQ_H
#define __MX35_IRQ_H

//------------------------------------------------------------------------------
// DEFINITIONS
//------------------------------------------------------------------------------
// Support of multiple IRQs per one SYSINTR mapping
#define IRQ_PER_SYSINTR         4

// The maximum IRQ number supported by mapping, which should 
// accommodate both SoC and board level IRQs.
#define IRQ_MAXIMUM             256


//------------------------------------------------------------------------------
// MX35-SPECIFIC IRQ DEFINITIONS
//------------------------------------------------------------------------------
#define IRQ_RESERVED0           0
#define IRQ_RESERVED1           1
#define IRQ_OWIRE               2
#define IRQ_I2C3                3
#define IRQ_I2C2                4
#define IRQ_RESERVED5           5
#define IRQ_RTIC                6
#define IRQ_ESDHC1              7
#define IRQ_ESDHC2              8
#define IRQ_ESDHC3              9
#define IRQ_I2C1                10
#define IRQ_SSI1                11
#define IRQ_SSI2                12
#define IRQ_CSPI2               13
#define IRQ_CSPI1               14
#define IRQ_ATA                 15
#define IRQ_GPU2D               16
#define IRQ_ASRC                17
#define IRQ_UART3               18
#define IRQ_IIM                 19
#define IRQ_RESERVED20          20
#define IRQ_RESERVED21          21
#define IRQ_RNGC                22
#define IRQ_EVTMON              23
#define IRQ_KPP                 24
#define IRQ_RTC                 25
#define IRQ_PWM                 26
#define IRQ_EPIT2               27
#define IRQ_EPIT1               28
#define IRQ_GPT                 29
#define IRQ_PWR_FAIL            30
#define IRQ_CCM                 31
#define IRQ_UART2               32
#define IRQ_NANDFC              33
#define IRQ_SDMA                34
#define IRQ_USB_HOST            35
#define IRQ_RESERVED36          36
#define IRQ_USB_OTG             37
#define IRQ_RESERVED38          38
#define IRQ_MSHC                39
#define IRQ_ESAI                40
#define IRQ_IPU_ERROR           41
#define IRQ_IPU_GENERAL         42
#define IRQ_CAN1                43
#define IRQ_CAN2                44
#define IRQ_UART1               45
#define IRQ_MLB                 46
#define IRQ_SPDIF               47
#define IRQ_ECT                 48
#define IRQ_SCC_SCM             49
#define IRQ_SCC_SMN             50
#define IRQ_GPIO2               51
#define IRQ_GPIO1               52
#define IRQ_RESERVED53          53
#define IRQ_RESERVED54          54
#define IRQ_WDOG                55
#define IRQ_GPIO3               56
#define IRQ_FEC                 57
#define IRQ_EXT_INT5            58
#define IRQ_EXT_INT4            59
#define IRQ_EXT_INT3            60
#define IRQ_EXT_INT2            61
#define IRQ_EXT_INT1            62
#define IRQ_EXT_INT0            63


//------------------------------------------------------------------------------
// SECONDARY IRQ DEFINITIONS
//------------------------------------------------------------------------------
#define IRQ_GPIO1_PIN0          (64)
#define IRQ_GPIO1_PIN1          (IRQ_GPIO1_PIN0 + 1)
#define IRQ_GPIO1_PIN2          (IRQ_GPIO1_PIN0 + 2)
#define IRQ_GPIO1_PIN3          (IRQ_GPIO1_PIN0 + 3)
#define IRQ_GPIO1_PIN4          (IRQ_GPIO1_PIN0 + 4)
#define IRQ_GPIO1_PIN5          (IRQ_GPIO1_PIN0 + 5)
#define IRQ_GPIO1_PIN6          (IRQ_GPIO1_PIN0 + 6)
#define IRQ_GPIO1_PIN7          (IRQ_GPIO1_PIN0 + 7)
#define IRQ_GPIO1_PIN8          (IRQ_GPIO1_PIN0 + 8)
#define IRQ_GPIO1_PIN9          (IRQ_GPIO1_PIN0 + 9)
#define IRQ_GPIO1_PIN10         (IRQ_GPIO1_PIN0 + 10)
#define IRQ_GPIO1_PIN11         (IRQ_GPIO1_PIN0 + 11)
#define IRQ_GPIO1_PIN12         (IRQ_GPIO1_PIN0 + 12)
#define IRQ_GPIO1_PIN13         (IRQ_GPIO1_PIN0 + 13)
#define IRQ_GPIO1_PIN14         (IRQ_GPIO1_PIN0 + 14)
#define IRQ_GPIO1_PIN15         (IRQ_GPIO1_PIN0 + 15)
#define IRQ_GPIO1_PIN16         (IRQ_GPIO1_PIN0 + 16)
#define IRQ_GPIO1_PIN17         (IRQ_GPIO1_PIN0 + 17)
#define IRQ_GPIO1_PIN18         (IRQ_GPIO1_PIN0 + 18)
#define IRQ_GPIO1_PIN19         (IRQ_GPIO1_PIN0 + 19)
#define IRQ_GPIO1_PIN20         (IRQ_GPIO1_PIN0 + 20)
#define IRQ_GPIO1_PIN21         (IRQ_GPIO1_PIN0 + 21)
#define IRQ_GPIO1_PIN22         (IRQ_GPIO1_PIN0 + 22)
#define IRQ_GPIO1_PIN23         (IRQ_GPIO1_PIN0 + 23)
#define IRQ_GPIO1_PIN24         (IRQ_GPIO1_PIN0 + 24)
#define IRQ_GPIO1_PIN25         (IRQ_GPIO1_PIN0 + 25)
#define IRQ_GPIO1_PIN26         (IRQ_GPIO1_PIN0 + 26)
#define IRQ_GPIO1_PIN27         (IRQ_GPIO1_PIN0 + 27)
#define IRQ_GPIO1_PIN28         (IRQ_GPIO1_PIN0 + 28)
#define IRQ_GPIO1_PIN29         (IRQ_GPIO1_PIN0 + 29)
#define IRQ_GPIO1_PIN30         (IRQ_GPIO1_PIN0 + 30)
#define IRQ_GPIO1_PIN31         (IRQ_GPIO1_PIN0 + 31)

#define IRQ_GPIO2_PIN0          (IRQ_GPIO1_PIN31 + 1)
#define IRQ_GPIO2_PIN1          (IRQ_GPIO2_PIN0 + 1)
#define IRQ_GPIO2_PIN2          (IRQ_GPIO2_PIN0 + 2)
#define IRQ_GPIO2_PIN3          (IRQ_GPIO2_PIN0 + 3)
#define IRQ_GPIO2_PIN4          (IRQ_GPIO2_PIN0 + 4)
#define IRQ_GPIO2_PIN5          (IRQ_GPIO2_PIN0 + 5)
#define IRQ_GPIO2_PIN6          (IRQ_GPIO2_PIN0 + 6)
#define IRQ_GPIO2_PIN7          (IRQ_GPIO2_PIN0 + 7)
#define IRQ_GPIO2_PIN8          (IRQ_GPIO2_PIN0 + 8)
#define IRQ_GPIO2_PIN9          (IRQ_GPIO2_PIN0 + 9)
#define IRQ_GPIO2_PIN10         (IRQ_GPIO2_PIN0 + 10)
#define IRQ_GPIO2_PIN11         (IRQ_GPIO2_PIN0 + 11)
#define IRQ_GPIO2_PIN12         (IRQ_GPIO2_PIN0 + 12)
#define IRQ_GPIO2_PIN13         (IRQ_GPIO2_PIN0 + 13)
#define IRQ_GPIO2_PIN14         (IRQ_GPIO2_PIN0 + 14)
#define IRQ_GPIO2_PIN15         (IRQ_GPIO2_PIN0 + 15)
#define IRQ_GPIO2_PIN16         (IRQ_GPIO2_PIN0 + 16)
#define IRQ_GPIO2_PIN17         (IRQ_GPIO2_PIN0 + 17)
#define IRQ_GPIO2_PIN18         (IRQ_GPIO2_PIN0 + 18)
#define IRQ_GPIO2_PIN19         (IRQ_GPIO2_PIN0 + 19)
#define IRQ_GPIO2_PIN20         (IRQ_GPIO2_PIN0 + 20)
#define IRQ_GPIO2_PIN21         (IRQ_GPIO2_PIN0 + 21)
#define IRQ_GPIO2_PIN22         (IRQ_GPIO2_PIN0 + 22)
#define IRQ_GPIO2_PIN23         (IRQ_GPIO2_PIN0 + 23)
#define IRQ_GPIO2_PIN24         (IRQ_GPIO2_PIN0 + 24)
#define IRQ_GPIO2_PIN25         (IRQ_GPIO2_PIN0 + 25)
#define IRQ_GPIO2_PIN26         (IRQ_GPIO2_PIN0 + 26)
#define IRQ_GPIO2_PIN27         (IRQ_GPIO2_PIN0 + 27)
#define IRQ_GPIO2_PIN28         (IRQ_GPIO2_PIN0 + 28)
#define IRQ_GPIO2_PIN29         (IRQ_GPIO2_PIN0 + 29)
#define IRQ_GPIO2_PIN30         (IRQ_GPIO2_PIN0 + 30)
#define IRQ_GPIO2_PIN31         (IRQ_GPIO2_PIN0 + 31)

#define IRQ_GPIO3_PIN0          (IRQ_GPIO2_PIN31 + 1)
#define IRQ_GPIO3_PIN1          (IRQ_GPIO3_PIN0 + 1)
#define IRQ_GPIO3_PIN2          (IRQ_GPIO3_PIN0 + 2)
#define IRQ_GPIO3_PIN3          (IRQ_GPIO3_PIN0 + 3)
#define IRQ_GPIO3_PIN4          (IRQ_GPIO3_PIN0 + 4)
#define IRQ_GPIO3_PIN5          (IRQ_GPIO3_PIN0 + 5)
#define IRQ_GPIO3_PIN6          (IRQ_GPIO3_PIN0 + 6)
#define IRQ_GPIO3_PIN7          (IRQ_GPIO3_PIN0 + 7)
#define IRQ_GPIO3_PIN8          (IRQ_GPIO3_PIN0 + 8)
#define IRQ_GPIO3_PIN9          (IRQ_GPIO3_PIN0 + 9)
#define IRQ_GPIO3_PIN10         (IRQ_GPIO3_PIN0 + 10)
#define IRQ_GPIO3_PIN11         (IRQ_GPIO3_PIN0 + 11)
#define IRQ_GPIO3_PIN12         (IRQ_GPIO3_PIN0 + 12)
#define IRQ_GPIO3_PIN13         (IRQ_GPIO3_PIN0 + 13)
#define IRQ_GPIO3_PIN14         (IRQ_GPIO3_PIN0 + 14)
#define IRQ_GPIO3_PIN15         (IRQ_GPIO3_PIN0 + 15)
#define IRQ_GPIO3_PIN16         (IRQ_GPIO3_PIN0 + 16)
#define IRQ_GPIO3_PIN17         (IRQ_GPIO3_PIN0 + 17)
#define IRQ_GPIO3_PIN18         (IRQ_GPIO3_PIN0 + 18)
#define IRQ_GPIO3_PIN19         (IRQ_GPIO3_PIN0 + 19)
#define IRQ_GPIO3_PIN20         (IRQ_GPIO3_PIN0 + 20)
#define IRQ_GPIO3_PIN21         (IRQ_GPIO3_PIN0 + 21)
#define IRQ_GPIO3_PIN22         (IRQ_GPIO3_PIN0 + 22)
#define IRQ_GPIO3_PIN23         (IRQ_GPIO3_PIN0 + 23)
#define IRQ_GPIO3_PIN24         (IRQ_GPIO3_PIN0 + 24)
#define IRQ_GPIO3_PIN25         (IRQ_GPIO3_PIN0 + 25)
#define IRQ_GPIO3_PIN26         (IRQ_GPIO3_PIN0 + 26)
#define IRQ_GPIO3_PIN27         (IRQ_GPIO3_PIN0 + 27)
#define IRQ_GPIO3_PIN28         (IRQ_GPIO3_PIN0 + 28)
#define IRQ_GPIO3_PIN29         (IRQ_GPIO3_PIN0 + 29)
#define IRQ_GPIO3_PIN30         (IRQ_GPIO3_PIN0 + 30)
#define IRQ_GPIO3_PIN31         (IRQ_GPIO3_PIN0 + 31)

#define IRQ_SDMA_CH0            (IRQ_GPIO3_PIN31 + 1)
#define IRQ_SDMA_CH1            (IRQ_SDMA_CH0 + 1)
#define IRQ_SDMA_CH2            (IRQ_SDMA_CH0 + 2)
#define IRQ_SDMA_CH3            (IRQ_SDMA_CH0 + 3)
#define IRQ_SDMA_CH4            (IRQ_SDMA_CH0 + 4)
#define IRQ_SDMA_CH5            (IRQ_SDMA_CH0 + 5)
#define IRQ_SDMA_CH6            (IRQ_SDMA_CH0 + 6)
#define IRQ_SDMA_CH7            (IRQ_SDMA_CH0 + 7)
#define IRQ_SDMA_CH8            (IRQ_SDMA_CH0 + 8)
#define IRQ_SDMA_CH9            (IRQ_SDMA_CH0 + 9)
#define IRQ_SDMA_CH10           (IRQ_SDMA_CH0 + 10)
#define IRQ_SDMA_CH11           (IRQ_SDMA_CH0 + 11)
#define IRQ_SDMA_CH12           (IRQ_SDMA_CH0 + 12)
#define IRQ_SDMA_CH13           (IRQ_SDMA_CH0 + 13)
#define IRQ_SDMA_CH14           (IRQ_SDMA_CH0 + 14)
#define IRQ_SDMA_CH15           (IRQ_SDMA_CH0 + 15)
#define IRQ_SDMA_CH16           (IRQ_SDMA_CH0 + 16)
#define IRQ_SDMA_CH17           (IRQ_SDMA_CH0 + 17)
#define IRQ_SDMA_CH18           (IRQ_SDMA_CH0 + 18)
#define IRQ_SDMA_CH19           (IRQ_SDMA_CH0 + 19)
#define IRQ_SDMA_CH20           (IRQ_SDMA_CH0 + 20)
#define IRQ_SDMA_CH21           (IRQ_SDMA_CH0 + 21)
#define IRQ_SDMA_CH22           (IRQ_SDMA_CH0 + 22)
#define IRQ_SDMA_CH23           (IRQ_SDMA_CH0 + 23)
#define IRQ_SDMA_CH24           (IRQ_SDMA_CH0 + 24)
#define IRQ_SDMA_CH25           (IRQ_SDMA_CH0 + 25)
#define IRQ_SDMA_CH26           (IRQ_SDMA_CH0 + 26)
#define IRQ_SDMA_CH27           (IRQ_SDMA_CH0 + 27)
#define IRQ_SDMA_CH28           (IRQ_SDMA_CH0 + 28)
#define IRQ_SDMA_CH29           (IRQ_SDMA_CH0 + 29)
#define IRQ_SDMA_CH30           (IRQ_SDMA_CH0 + 30)
#define IRQ_SDMA_CH31           (IRQ_SDMA_CH0 + 31)

// The total IRQ number in SoC level
#define IRQ_SOC_NUMBER          (IRQ_SDMA_CH31 + 1)


//------------------------------------------------------------------------------

#endif // __MX35_IRQ_H
