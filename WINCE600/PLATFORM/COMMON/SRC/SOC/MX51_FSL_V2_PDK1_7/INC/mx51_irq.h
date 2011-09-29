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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx51_irq.h
//
//  This file defines names for IRQ. These names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
//------------------------------------------------------------------------------
#ifndef __MX51_IRQ_H
#define __MX51_IRQ_H

//------------------------------------------------------------------------------
// DEFINITIONS
//------------------------------------------------------------------------------
// Support of multiple IRQs per one SYSINTR mapping
#define IRQ_PER_SYSINTR         4

// The maximum IRQ number supported by mapping, which should 
// accommodate both SoC and board level IRQs.
//  128 Primary TZIC interrupts
//  4*32 GPIO secondary interrupts (4 GPIO modules with 32 lines each)
//  32 SDMA channel interrupts
//  64 reserved for board-level
#define IRQ_MAXIMUM             (128+(4*32)+32+64)

//------------------------------------------------------------------------------
// IRQ DEFINITIONS
//------------------------------------------------------------------------------
#define IRQ_RESERVED0           0
#define IRQ_ESDHC1              1
#define IRQ_ESDHC2              2
#define IRQ_ESDHC3              3
#define IRQ_ESDHC4              4
#define IRQ_RESERVED5           5
#define IRQ_SDMA                6
#define IRQ_IOMUX               7
#define IRQ_NFC                 8
#define IRQ_VPU                 9
#define IRQ_IPU_ERROR           10
#define IRQ_IPU_SYNC            11
#define IRQ_GPU                 12
#define IRQ_RESERVED13          13
#define IRQ_USB_HOST1           14
#define IRQ_EMI                 15
#define IRQ_USB_HOST2           16
#define IRQ_USB_HOST3           17
#define IRQ_USB_OTG             18
#define IRQ_SAHARAH_TZ          19
#define IRQ_SAHARAH_NTZ         20
#define IRQ_SCC_HIGH            21
#define IRQ_SCC_TZ              22
#define IRQ_SCC_NTZ             23
#define IRQ_SRTC_NTZ            24
#define IRQ_SRTC_TZ             25
#define IRQ_RTIC                26
#define IRQ_CSU                 27
#define IRQ_SLIMBUS_INT         28
#define IRQ_SSI1                29
#define IRQ_SSI2                30
#define IRQ_UART1               31
#define IRQ_UART2               32
#define IRQ_UART3               33
#define IRQ_RESERVED34          34
#define IRQ_RESERVED35          35
#define IRQ_ECSPI1              36
#define IRQ_ECSPI2              37
#define IRQ_CSPI                38
#define IRQ_GPT                 39
#define IRQ_EPIT1               40
#define IRQ_EPIT2               41
#define IRQ_GPIO1_INT7          42
#define IRQ_GPIO1_INT6          43
#define IRQ_GPIO1_INT5          44
#define IRQ_GPIO1_INT4          45
#define IRQ_GPIO1_INT3          46
#define IRQ_GPIO1_INT2          47
#define IRQ_GPIO1_INT1          48
#define IRQ_GPIO1_INT0          49
#define IRQ_GPIO1_LOWER16       50
#define IRQ_GPIO1_UPPER16       51
#define IRQ_GPIO2_LOWER16       52
#define IRQ_GPIO2_UPPER16       53
#define IRQ_GPIO3_LOWER16       54
#define IRQ_GPIO3_UPPER16       55
#define IRQ_GPIO4_LOWER16       56
#define IRQ_GPIO4_UPPER16       57
#define IRQ_WDOG1               58
#define IRQ_WDOG2               59
#define IRQ_KPP                 60
#define IRQ_PWM1                61
#define IRQ_I2C1                62
#define IRQ_I2C2                63
#define IRQ_HSI2C               64
#define IRQ_RESERVED65          65
#define IRQ_RESERVED66          66
#define IRQ_SIM_IPB             67
#define IRQ_SIM_DATA            68
#define IRQ_IIM                 69
#define IRQ_PATA                70
#define IRQ_CCM1                71
#define IRQ_CCM2                72
#define IRQ_GPC1                73
#define IRQ_GPC2                74
#define IRQ_SRC                 75
#define IRQ_ARM_NEON_MON        76
#define IRQ_ARM_PMU             77
#define IRQ_ARM_CTI             78
#define IRQ_ARM_CTI_TRIG0       79
#define IRQ_ARM_CTI_TRIG1       80
#define IRQ_MIPI_ERROR          81
#define IRQ_MIPI_TIMER          82
#define IRQ_MIPI_FUNC           83
#define IRQ_GPU2D_GENERAL       84
#define IRQ_GPU2D_BUSY          85
#define IRQ_RESERVED86          86
#define IRQ_FEC                 87
#define IRQ_OWIRE               88
#define IRQ_ARM_DEBUG           89
#define IRQ_SJC                 90
#define IRQ_SPDIF               91
#define IRQ_TVE                 92
#define IRQ_FIRI                93
#define IRQ_PWM2                94
#define IRQ_SLIMBUS_EXCEP       95
#define IRQ_SSI3                96
#define IRQ_EMI_BOOT            97
#define IRQ_ARM_CTI_TRIG3       98
#define IRQ_SLIMBUS_RX          99
#define IRQ_VPU_IDLE            100
#define IRQ_NFC_AUTOPROG        101
#define IRQ_GPU_IDLE            102
#define IRQ_RESERVED103         103
#define IRQ_RESERVED104         104
#define IRQ_RESERVED105         105
#define IRQ_RESERVED106         106
#define IRQ_RESERVED107         107
#define IRQ_RESERVED108         108
#define IRQ_RESERVED109         109
#define IRQ_RESERVED110         110
#define IRQ_RESERVED111         111
#define IRQ_RESERVED112         112
#define IRQ_RESERVED113         113
#define IRQ_RESERVED114         114
#define IRQ_RESERVED115         115
#define IRQ_RESERVED116         116
#define IRQ_RESERVED117         117
#define IRQ_RESERVED118         118
#define IRQ_RESERVED119         119
#define IRQ_RESERVED120         120
#define IRQ_RESERVED121         121
#define IRQ_RESERVED122         122
#define IRQ_RESERVED123         123
#define IRQ_RESERVED124         124
#define IRQ_RESERVED125         125
#define IRQ_RESERVED126         126
#define IRQ_RESERVED127         127


//------------------------------------------------------------------------------
// SECONDARY IRQ DEFINITIONS
//------------------------------------------------------------------------------
#define IRQ_GPIO1_PIN0          (128)
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

#define IRQ_GPIO4_PIN0          (IRQ_GPIO3_PIN31 + 1)
#define IRQ_GPIO4_PIN1          (IRQ_GPIO4_PIN0 + 1)
#define IRQ_GPIO4_PIN2          (IRQ_GPIO4_PIN0 + 2)
#define IRQ_GPIO4_PIN3          (IRQ_GPIO4_PIN0 + 3)
#define IRQ_GPIO4_PIN4          (IRQ_GPIO4_PIN0 + 4)
#define IRQ_GPIO4_PIN5          (IRQ_GPIO4_PIN0 + 5)
#define IRQ_GPIO4_PIN6          (IRQ_GPIO4_PIN0 + 6)
#define IRQ_GPIO4_PIN7          (IRQ_GPIO4_PIN0 + 7)
#define IRQ_GPIO4_PIN8          (IRQ_GPIO4_PIN0 + 8)
#define IRQ_GPIO4_PIN9          (IRQ_GPIO4_PIN0 + 9)
#define IRQ_GPIO4_PIN10         (IRQ_GPIO4_PIN0 + 10)
#define IRQ_GPIO4_PIN11         (IRQ_GPIO4_PIN0 + 11)
#define IRQ_GPIO4_PIN12         (IRQ_GPIO4_PIN0 + 12)
#define IRQ_GPIO4_PIN13         (IRQ_GPIO4_PIN0 + 13)
#define IRQ_GPIO4_PIN14         (IRQ_GPIO4_PIN0 + 14)
#define IRQ_GPIO4_PIN15         (IRQ_GPIO4_PIN0 + 15)
#define IRQ_GPIO4_PIN16         (IRQ_GPIO4_PIN0 + 16)
#define IRQ_GPIO4_PIN17         (IRQ_GPIO4_PIN0 + 17)
#define IRQ_GPIO4_PIN18         (IRQ_GPIO4_PIN0 + 18)
#define IRQ_GPIO4_PIN19         (IRQ_GPIO4_PIN0 + 19)
#define IRQ_GPIO4_PIN20         (IRQ_GPIO4_PIN0 + 20)
#define IRQ_GPIO4_PIN21         (IRQ_GPIO4_PIN0 + 21)
#define IRQ_GPIO4_PIN22         (IRQ_GPIO4_PIN0 + 22)
#define IRQ_GPIO4_PIN23         (IRQ_GPIO4_PIN0 + 23)
#define IRQ_GPIO4_PIN24         (IRQ_GPIO4_PIN0 + 24)
#define IRQ_GPIO4_PIN25         (IRQ_GPIO4_PIN0 + 25)
#define IRQ_GPIO4_PIN26         (IRQ_GPIO4_PIN0 + 26)
#define IRQ_GPIO4_PIN27         (IRQ_GPIO4_PIN0 + 27)
#define IRQ_GPIO4_PIN28         (IRQ_GPIO4_PIN0 + 28)
#define IRQ_GPIO4_PIN29         (IRQ_GPIO4_PIN0 + 29)
#define IRQ_GPIO4_PIN30         (IRQ_GPIO4_PIN0 + 30)
#define IRQ_GPIO4_PIN31         (IRQ_GPIO4_PIN0 + 31)

#define IRQ_SDMA_CH0            (IRQ_GPIO4_PIN31 + 1)
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

// Define number of on-chip IRQs.  Board-level interrupts can start here.
#define IRQ_SOC_MAXIMUM         (IRQ_SDMA_CH31 + 1)

#endif // __MX51_IRQ_H
