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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File:  mx27_irq.h
//
// This file defines names for IRQ. These names have no other role than make
// code more readable. For SoC where device - IRQ mapping is defined by
// silicon and it can't be changed by software.
//
//-----------------------------------------------------------------------------
#ifndef __MX27_IRQ_H__
#define __MX27_IRQ_H__

//-----------------------------------------------------------------------------
// MX27 IRQ DEFINITIONS
//-----------------------------------------------------------------------------
#define IRQ_RESERVED0       0
#define IRQ_I2C2            1
#define IRQ_GPT6            2
#define IRQ_GPT5            3
#define IRQ_GPT4            4
#define IRQ_RTIC            5
#define IRQ_CSPI3           6
#define IRQ_MSHC            7 
#define IRQ_GPIO            8 
#define IRQ_SDHC3           9 
#define IRQ_SDHC2           10
#define IRQ_SDHC1           11
#define IRQ_I2C1            12
#define IRQ_SSI2            13
#define IRQ_SSI1            14
#define IRQ_CSPI2           15
#define IRQ_CSPI1           16
#define IRQ_UART4           17
#define IRQ_UART3           18
#define IRQ_UART2           19
#define IRQ_UART1           20
#define IRQ_KPP             21
#define IRQ_RTC             22
#define IRQ_PWM             23
#define IRQ_GPT3            24
#define IRQ_GPT2            25
#define IRQ_GPT1            26
#define IRQ_WDOG            27
#define IRQ_PCMCIA          28
#define IRQ_NFC             29
#define IRQ_ATA             30
#define IRQ_CSI             31
#define IRQ_DMACH0          32
#define IRQ_DMACH1          33
#define IRQ_DMACH2          34
#define IRQ_DMACH3          35
#define IRQ_DMACH4          36
#define IRQ_DMACH5          37
#define IRQ_DMACH6          38
#define IRQ_DMACH7          39
#define IRQ_DMACH8          40
#define IRQ_DMACH9          41
#define IRQ_DMACH10         42
#define IRQ_DMACH11         43
#define IRQ_DMACH12         44
#define IRQ_DMACH13         45
#define IRQ_DMACH14         46
#define IRQ_DMACH15         47
#define IRQ_UART6           48 
#define IRQ_UART5           49
#define IRQ_FEC             50
#define IRQ_EMMAPRP         51
#define IRQ_EMMAPP          52
#define IRQ_H264            53
#define IRQ_USBHS1          54
#define IRQ_USBHS2          55
#define IRQ_USBOTG          56
#define IRQ_SMN             57
#define IRQ_SCM             58
#define IRQ_SAHARA          59
#define IRQ_SLCDC           60
#define IRQ_LCDC            61
#define IRQ_IIM             62 
#define IRQ_DPTC            63 

#define CSP_IRQ_AITC_MAX    64

//-----------------------------------------------------------------------------
// Physical interrupt IDs for
//      GPIO PortA interrupts:   64-95
//                              IRQ_GPIO_PA_0 - IRQ_GPIO_PA_31
//      GPIO PortB interrupts:   96-127
//                              IRQ_GPIO_PB_0 - IRQ_GPIO_PB_31
//      GPIO PortC interrupts:   128-159
//                              IRQ_GPIO_PC_0 - IRQ_GPIO_PC_31
//      GPIO PortD interrupts:   160-191
//                              IRQ_GPIO_PD_0 - IRQ_GPIO_PD_31
//      GPIO PortE interrupts:   192-223
//                              IRQ_GPIO_PE_0 - IRQ_GPIO_PE_31
//      GPIO PortF interrupts:   224-255
//                              IRQ_GPIO_PF_0 - IRQ_GPIO_PF_31
//-----------------------------------------------------------------------------
// GPIO PORT A
#define IRQ_GPIO_PA_0           (CSP_IRQ_AITC_MAX + 0) 
#define IRQ_GPIO_PA_1           (CSP_IRQ_AITC_MAX + 1) 
#define IRQ_GPIO_PA_2           (CSP_IRQ_AITC_MAX + 2) 
#define IRQ_GPIO_PA_3           (CSP_IRQ_AITC_MAX + 3) 
#define IRQ_GPIO_PA_4           (CSP_IRQ_AITC_MAX + 4) 
#define IRQ_GPIO_PA_5           (CSP_IRQ_AITC_MAX + 5) 
#define IRQ_GPIO_PA_6           (CSP_IRQ_AITC_MAX + 6) 
#define IRQ_GPIO_PA_7           (CSP_IRQ_AITC_MAX + 7) 
#define IRQ_GPIO_PA_8           (CSP_IRQ_AITC_MAX + 8) 
#define IRQ_GPIO_PA_9           (CSP_IRQ_AITC_MAX + 9)
#define IRQ_GPIO_PA_10          (CSP_IRQ_AITC_MAX + 10)
#define IRQ_GPIO_PA_11          (CSP_IRQ_AITC_MAX + 11)
#define IRQ_GPIO_PA_12          (CSP_IRQ_AITC_MAX + 12)
#define IRQ_GPIO_PA_13          (CSP_IRQ_AITC_MAX + 13)
#define IRQ_GPIO_PA_14          (CSP_IRQ_AITC_MAX + 14)
#define IRQ_GPIO_PA_15          (CSP_IRQ_AITC_MAX + 15)
#define IRQ_GPIO_PA_16          (CSP_IRQ_AITC_MAX + 16)
#define IRQ_GPIO_PA_17          (CSP_IRQ_AITC_MAX + 17)
#define IRQ_GPIO_PA_18          (CSP_IRQ_AITC_MAX + 18)
#define IRQ_GPIO_PA_19          (CSP_IRQ_AITC_MAX + 19)
#define IRQ_GPIO_PA_20          (CSP_IRQ_AITC_MAX + 20)
#define IRQ_GPIO_PA_21          (CSP_IRQ_AITC_MAX + 21)
#define IRQ_GPIO_PA_22          (CSP_IRQ_AITC_MAX + 22)
#define IRQ_GPIO_PA_23          (CSP_IRQ_AITC_MAX + 23)
#define IRQ_GPIO_PA_24          (CSP_IRQ_AITC_MAX + 24)
#define IRQ_GPIO_PA_25          (CSP_IRQ_AITC_MAX + 25)
#define IRQ_GPIO_PA_26          (CSP_IRQ_AITC_MAX + 26)
#define IRQ_GPIO_PA_27          (CSP_IRQ_AITC_MAX + 27)
#define IRQ_GPIO_PA_28          (CSP_IRQ_AITC_MAX + 28)
#define IRQ_GPIO_PA_29          (CSP_IRQ_AITC_MAX + 29)
#define IRQ_GPIO_PA_30          (CSP_IRQ_AITC_MAX + 30)
#define IRQ_GPIO_PA_31          (CSP_IRQ_AITC_MAX + 31)

// GPIO PORT B
#define IRQ_GPIO_PB_0           (CSP_IRQ_AITC_MAX + 32)
#define IRQ_GPIO_PB_1           (CSP_IRQ_AITC_MAX + 33)
#define IRQ_GPIO_PB_2           (CSP_IRQ_AITC_MAX + 34)
#define IRQ_GPIO_PB_3           (CSP_IRQ_AITC_MAX + 35)
#define IRQ_GPIO_PB_4           (CSP_IRQ_AITC_MAX + 36)
#define IRQ_GPIO_PB_5           (CSP_IRQ_AITC_MAX + 37)
#define IRQ_GPIO_PB_6           (CSP_IRQ_AITC_MAX + 38)
#define IRQ_GPIO_PB_7           (CSP_IRQ_AITC_MAX + 39)
#define IRQ_GPIO_PB_8           (CSP_IRQ_AITC_MAX + 40)
#define IRQ_GPIO_PB_9           (CSP_IRQ_AITC_MAX + 41)
#define IRQ_GPIO_PB_10          (CSP_IRQ_AITC_MAX + 42)
#define IRQ_GPIO_PB_11          (CSP_IRQ_AITC_MAX + 43)
#define IRQ_GPIO_PB_12          (CSP_IRQ_AITC_MAX + 44)
#define IRQ_GPIO_PB_13          (CSP_IRQ_AITC_MAX + 45)
#define IRQ_GPIO_PB_14          (CSP_IRQ_AITC_MAX + 46)
#define IRQ_GPIO_PB_15          (CSP_IRQ_AITC_MAX + 47)
#define IRQ_GPIO_PB_16          (CSP_IRQ_AITC_MAX + 48)
#define IRQ_GPIO_PB_17          (CSP_IRQ_AITC_MAX + 49)
#define IRQ_GPIO_PB_18          (CSP_IRQ_AITC_MAX + 50)
#define IRQ_GPIO_PB_19          (CSP_IRQ_AITC_MAX + 51)
#define IRQ_GPIO_PB_20          (CSP_IRQ_AITC_MAX + 52)
#define IRQ_GPIO_PB_21          (CSP_IRQ_AITC_MAX + 53)
#define IRQ_GPIO_PB_22          (CSP_IRQ_AITC_MAX + 54)
#define IRQ_GPIO_PB_23          (CSP_IRQ_AITC_MAX + 55)
#define IRQ_GPIO_PB_24          (CSP_IRQ_AITC_MAX + 56)
#define IRQ_GPIO_PB_25          (CSP_IRQ_AITC_MAX + 57)
#define IRQ_GPIO_PB_26          (CSP_IRQ_AITC_MAX + 58)
#define IRQ_GPIO_PB_27          (CSP_IRQ_AITC_MAX + 59)
#define IRQ_GPIO_PB_28          (CSP_IRQ_AITC_MAX + 60)
#define IRQ_GPIO_PB_29          (CSP_IRQ_AITC_MAX + 61)
#define IRQ_GPIO_PB_30          (CSP_IRQ_AITC_MAX + 62)
#define IRQ_GPIO_PB_31          (CSP_IRQ_AITC_MAX + 63)
                                                     
// GPIO PORT C                                       
#define IRQ_GPIO_PC_0           (CSP_IRQ_AITC_MAX + 64)
#define IRQ_GPIO_PC_1           (CSP_IRQ_AITC_MAX + 65)
#define IRQ_GPIO_PC_2           (CSP_IRQ_AITC_MAX + 66)
#define IRQ_GPIO_PC_3           (CSP_IRQ_AITC_MAX + 67)
#define IRQ_GPIO_PC_4           (CSP_IRQ_AITC_MAX + 68)
#define IRQ_GPIO_PC_5           (CSP_IRQ_AITC_MAX + 69)
#define IRQ_GPIO_PC_6           (CSP_IRQ_AITC_MAX + 70)
#define IRQ_GPIO_PC_7           (CSP_IRQ_AITC_MAX + 71)
#define IRQ_GPIO_PC_8           (CSP_IRQ_AITC_MAX + 72)
#define IRQ_GPIO_PC_9           (CSP_IRQ_AITC_MAX + 73)
#define IRQ_GPIO_PC_10          (CSP_IRQ_AITC_MAX + 74)
#define IRQ_GPIO_PC_11          (CSP_IRQ_AITC_MAX + 75)
#define IRQ_GPIO_PC_12          (CSP_IRQ_AITC_MAX + 76)
#define IRQ_GPIO_PC_13          (CSP_IRQ_AITC_MAX + 77)
#define IRQ_GPIO_PC_14          (CSP_IRQ_AITC_MAX + 78)
#define IRQ_GPIO_PC_15          (CSP_IRQ_AITC_MAX + 79)
#define IRQ_GPIO_PC_16          (CSP_IRQ_AITC_MAX + 80)
#define IRQ_GPIO_PC_17          (CSP_IRQ_AITC_MAX + 81)
#define IRQ_GPIO_PC_18          (CSP_IRQ_AITC_MAX + 82)
#define IRQ_GPIO_PC_19          (CSP_IRQ_AITC_MAX + 83)
#define IRQ_GPIO_PC_20          (CSP_IRQ_AITC_MAX + 84)
#define IRQ_GPIO_PC_21          (CSP_IRQ_AITC_MAX + 85)
#define IRQ_GPIO_PC_22          (CSP_IRQ_AITC_MAX + 86)
#define IRQ_GPIO_PC_23          (CSP_IRQ_AITC_MAX + 87)
#define IRQ_GPIO_PC_24          (CSP_IRQ_AITC_MAX + 88)
#define IRQ_GPIO_PC_25          (CSP_IRQ_AITC_MAX + 89)
#define IRQ_GPIO_PC_26          (CSP_IRQ_AITC_MAX + 90)
#define IRQ_GPIO_PC_27          (CSP_IRQ_AITC_MAX + 91)
#define IRQ_GPIO_PC_28          (CSP_IRQ_AITC_MAX + 92)
#define IRQ_GPIO_PC_29          (CSP_IRQ_AITC_MAX + 93)
#define IRQ_GPIO_PC_30          (CSP_IRQ_AITC_MAX + 94)
#define IRQ_GPIO_PC_31          (CSP_IRQ_AITC_MAX + 95)

// GPIO PORT D
#define IRQ_GPIO_PD_0           (CSP_IRQ_AITC_MAX + 96)
#define IRQ_GPIO_PD_1           (CSP_IRQ_AITC_MAX + 97)
#define IRQ_GPIO_PD_2           (CSP_IRQ_AITC_MAX + 98)
#define IRQ_GPIO_PD_3           (CSP_IRQ_AITC_MAX + 99)
#define IRQ_GPIO_PD_4           (CSP_IRQ_AITC_MAX + 100)
#define IRQ_GPIO_PD_5           (CSP_IRQ_AITC_MAX + 101)
#define IRQ_GPIO_PD_6           (CSP_IRQ_AITC_MAX + 102)
#define IRQ_GPIO_PD_7           (CSP_IRQ_AITC_MAX + 103)
#define IRQ_GPIO_PD_8           (CSP_IRQ_AITC_MAX + 104)
#define IRQ_GPIO_PD_9           (CSP_IRQ_AITC_MAX + 105)
#define IRQ_GPIO_PD_10          (CSP_IRQ_AITC_MAX + 106)
#define IRQ_GPIO_PD_11          (CSP_IRQ_AITC_MAX + 107)
#define IRQ_GPIO_PD_12          (CSP_IRQ_AITC_MAX + 108)
#define IRQ_GPIO_PD_13          (CSP_IRQ_AITC_MAX + 109)
#define IRQ_GPIO_PD_14          (CSP_IRQ_AITC_MAX + 110)
#define IRQ_GPIO_PD_15          (CSP_IRQ_AITC_MAX + 111)
#define IRQ_GPIO_PD_16          (CSP_IRQ_AITC_MAX + 112)
#define IRQ_GPIO_PD_17          (CSP_IRQ_AITC_MAX + 113)
#define IRQ_GPIO_PD_18          (CSP_IRQ_AITC_MAX + 114)
#define IRQ_GPIO_PD_19          (CSP_IRQ_AITC_MAX + 115)
#define IRQ_GPIO_PD_20          (CSP_IRQ_AITC_MAX + 116)
#define IRQ_GPIO_PD_21          (CSP_IRQ_AITC_MAX + 117)
#define IRQ_GPIO_PD_22          (CSP_IRQ_AITC_MAX + 118)
#define IRQ_GPIO_PD_23          (CSP_IRQ_AITC_MAX + 119)
#define IRQ_GPIO_PD_24          (CSP_IRQ_AITC_MAX + 120)
#define IRQ_GPIO_PD_25          (CSP_IRQ_AITC_MAX + 121)
#define IRQ_GPIO_PD_26          (CSP_IRQ_AITC_MAX + 122)
#define IRQ_GPIO_PD_27          (CSP_IRQ_AITC_MAX + 123)
#define IRQ_GPIO_PD_28          (CSP_IRQ_AITC_MAX + 124)
#define IRQ_GPIO_PD_29          (CSP_IRQ_AITC_MAX + 125)
#define IRQ_GPIO_PD_30          (CSP_IRQ_AITC_MAX + 126)
#define IRQ_GPIO_PD_31          (CSP_IRQ_AITC_MAX + 127)
                                                      
// GPIO PORT E
#define IRQ_GPIO_PE_0           (CSP_IRQ_AITC_MAX + 128)
#define IRQ_GPIO_PE_1           (CSP_IRQ_AITC_MAX + 129)
#define IRQ_GPIO_PE_2           (CSP_IRQ_AITC_MAX + 130)
#define IRQ_GPIO_PE_3           (CSP_IRQ_AITC_MAX + 131)
#define IRQ_GPIO_PE_4           (CSP_IRQ_AITC_MAX + 132)
#define IRQ_GPIO_PE_5           (CSP_IRQ_AITC_MAX + 133)
#define IRQ_GPIO_PE_6           (CSP_IRQ_AITC_MAX + 134)
#define IRQ_GPIO_PE_7           (CSP_IRQ_AITC_MAX + 135)
#define IRQ_GPIO_PE_8           (CSP_IRQ_AITC_MAX + 136)
#define IRQ_GPIO_PE_9           (CSP_IRQ_AITC_MAX + 137)
#define IRQ_GPIO_PE_10          (CSP_IRQ_AITC_MAX + 138)
#define IRQ_GPIO_PE_11          (CSP_IRQ_AITC_MAX + 139)
#define IRQ_GPIO_PE_12          (CSP_IRQ_AITC_MAX + 140)
#define IRQ_GPIO_PE_13          (CSP_IRQ_AITC_MAX + 141)
#define IRQ_GPIO_PE_14          (CSP_IRQ_AITC_MAX + 142)
#define IRQ_GPIO_PE_15          (CSP_IRQ_AITC_MAX + 143)
#define IRQ_GPIO_PE_16          (CSP_IRQ_AITC_MAX + 144)
#define IRQ_GPIO_PE_17          (CSP_IRQ_AITC_MAX + 145)
#define IRQ_GPIO_PE_18          (CSP_IRQ_AITC_MAX + 146)
#define IRQ_GPIO_PE_19          (CSP_IRQ_AITC_MAX + 147)
#define IRQ_GPIO_PE_20          (CSP_IRQ_AITC_MAX + 148)
#define IRQ_GPIO_PE_21          (CSP_IRQ_AITC_MAX + 149)
#define IRQ_GPIO_PE_22          (CSP_IRQ_AITC_MAX + 150)
#define IRQ_GPIO_PE_23          (CSP_IRQ_AITC_MAX + 151)
#define IRQ_GPIO_PE_24          (CSP_IRQ_AITC_MAX + 152)
#define IRQ_GPIO_PE_25          (CSP_IRQ_AITC_MAX + 153)
#define IRQ_GPIO_PE_26          (CSP_IRQ_AITC_MAX + 154)
#define IRQ_GPIO_PE_27          (CSP_IRQ_AITC_MAX + 155)
#define IRQ_GPIO_PE_28          (CSP_IRQ_AITC_MAX + 156)
#define IRQ_GPIO_PE_29          (CSP_IRQ_AITC_MAX + 157)
#define IRQ_GPIO_PE_30          (CSP_IRQ_AITC_MAX + 158)
#define IRQ_GPIO_PE_31          (CSP_IRQ_AITC_MAX + 159)
                                                      
// GPIO PORT F
#define IRQ_GPIO_PF_0           (CSP_IRQ_AITC_MAX + 160)
#define IRQ_GPIO_PF_1           (CSP_IRQ_AITC_MAX + 161)
#define IRQ_GPIO_PF_2           (CSP_IRQ_AITC_MAX + 162)
#define IRQ_GPIO_PF_3           (CSP_IRQ_AITC_MAX + 163)
#define IRQ_GPIO_PF_4           (CSP_IRQ_AITC_MAX + 164)
#define IRQ_GPIO_PF_5           (CSP_IRQ_AITC_MAX + 165)
#define IRQ_GPIO_PF_6           (CSP_IRQ_AITC_MAX + 166)
#define IRQ_GPIO_PF_7           (CSP_IRQ_AITC_MAX + 167)
#define IRQ_GPIO_PF_8           (CSP_IRQ_AITC_MAX + 168)
#define IRQ_GPIO_PF_9           (CSP_IRQ_AITC_MAX + 169)
#define IRQ_GPIO_PF_10          (CSP_IRQ_AITC_MAX + 170)
#define IRQ_GPIO_PF_11          (CSP_IRQ_AITC_MAX + 171)
#define IRQ_GPIO_PF_12          (CSP_IRQ_AITC_MAX + 172)
#define IRQ_GPIO_PF_13          (CSP_IRQ_AITC_MAX + 173)
#define IRQ_GPIO_PF_14          (CSP_IRQ_AITC_MAX + 174)
#define IRQ_GPIO_PF_15          (CSP_IRQ_AITC_MAX + 175)
#define IRQ_GPIO_PF_16          (CSP_IRQ_AITC_MAX + 176)
#define IRQ_GPIO_PF_17          (CSP_IRQ_AITC_MAX + 177)
#define IRQ_GPIO_PF_18          (CSP_IRQ_AITC_MAX + 178)
#define IRQ_GPIO_PF_19          (CSP_IRQ_AITC_MAX + 179)
#define IRQ_GPIO_PF_20          (CSP_IRQ_AITC_MAX + 180)
#define IRQ_GPIO_PF_21          (CSP_IRQ_AITC_MAX + 181)
#define IRQ_GPIO_PF_22          (CSP_IRQ_AITC_MAX + 182)
#define IRQ_GPIO_PF_23          (CSP_IRQ_AITC_MAX + 183)
#define IRQ_GPIO_PF_24          (CSP_IRQ_AITC_MAX + 184)
#define IRQ_GPIO_PF_25          (CSP_IRQ_AITC_MAX + 185)
#define IRQ_GPIO_PF_26          (CSP_IRQ_AITC_MAX + 186)
#define IRQ_GPIO_PF_27          (CSP_IRQ_AITC_MAX + 187)
#define IRQ_GPIO_PF_28          (CSP_IRQ_AITC_MAX + 188)
#define IRQ_GPIO_PF_29          (CSP_IRQ_AITC_MAX + 189)
#define IRQ_GPIO_PF_30          (CSP_IRQ_AITC_MAX + 190)
#define IRQ_GPIO_PF_31          (CSP_IRQ_AITC_MAX + 191)
                                                      
// Min and max interrupt IDs for GPIO related
#define CSP_IRQ_GPIO_MIN        IRQ_GPIO_PA_0
#define CSP_IRQ_GPIO_MAX        IRQ_GPIO_PF_31

// Min and max interrupt IDs for MX27 CSP
#define CSP_IRQ_MX27_MIN        0
#define CSP_IRQ_MX27_MAX        CSP_IRQ_GPIO_MAX

#endif // __MX27_IRQ_H__
