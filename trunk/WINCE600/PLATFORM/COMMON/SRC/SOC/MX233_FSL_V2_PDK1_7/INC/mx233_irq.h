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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx233_irq.h
//
//  This file defines names for IRQ. These names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
//  This header contains IRQ definitions for the iMX233 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX233_IRQ_H
#define __MX233_IRQ_H

//------------------------------------------------------------------------------
// DEFINITIONS
//------------------------------------------------------------------------------
// Support of multiple IRQs per one SYSINTR mapping
#define IRQ_PER_SYSINTR         4

// The maximum IRQ number supported by mapping, which should
// accommodate both SoC and board level IRQs.
#define IRQ_MAXIMUM             IRQ_SOC_NUMBER


//------------------------------------------------------------------------------
// MX233-SPECIFIC IRQ DEFINITIONS
//------------------------------------------------------------------------------
#define IRQ_DEBUG_UART          (0)
#define IRQ_COMMS               (1)
#define IRQ_SSP2_ERROR          (2)
#define IRQ_VDD5V               (3)
#define IRQ_HEADPHONE_SHORT     (4)
#define IRQ_DAC_DMA             (5)
#define IRQ_DAC_ERROR           (6)
#define IRQ_ADC_DMA             (7)
#define IRQ_ADC_ERROR           (8)
#define IRQ_SPDIF_DMA           (9)
#define IRQ_SAIF2_DMA           (9)
#define IRQ_SPDIF_ERROR         (10)
#define IRQ_SAIF1               (10)
#define IRQ_SAIF2               (10)
#define IRQ_USB_CTRL            (11)
#define IRQ_USB_WAKEUP          (12)
#define IRQ_GPMI_DMA            (13)
#define IRQ_SSP1_DMA            (14)
#define IRQ_SSP_ERROR           (15)
#define IRQ_GPIO0               (16)
#define IRQ_GPIO1               (17)
#define IRQ_GPIO2               (18)
#define IRQ_SAIF1_DMA           (19)
#define IRQ_SSP2_DMA            (20)
#define IRQ_ECC8                (21)
#define IRQ_RTC_ALARM           (22)
#define IRQ_UARTAPP_TX_DMA      (23)
#define IRQ_UARTAPP_INTERNAL    (24)
#define IRQ_UARTAPP_RX_DMA      (25)
#define IRQ_I2C_DMA             (26)
#define IRQ_I2C_ERROR           (27)
#define IRQ_TIMER0              (28)
#define IRQ_TIMER1              (29)
#define IRQ_TIMER2              (30)
#define IRQ_TIMER3              (31)

#define IRQ_BATT_BRNOUT         (32)
#define IRQ_VDDD_BRNOUT         (33)
#define IRQ_VDDIO_BRNOUT        (34)
#define IRQ_VDD18_BRNOUT        (35)
#define IRQ_TOUCH_DETECT        (36)
#define IRQ_LRADC_CH0           (37)
#define IRQ_LRADC_CH1           (38)
#define IRQ_LRADC_CH2           (39)
#define IRQ_LRADC_CH3           (40)
#define IRQ_LRADC_CH4           (41)
#define IRQ_LRADC_CH5           (42)
#define IRQ_LRADC_CH6           (43)
#define IRQ_LRADC_CH7           (44)
#define IRQ_LCDIF_DMA           (45)
#define IRQ_LCDIF_ERROR         (46)
#define IRQ_DIGCTL_DEBUG_TRAP   (47)
#define IRQ_RTC_1MSEC           (48)
#define IRQ_DRI_DMA             (49)
#define IRQ_DRI_ATTENTION       (50)
#define IRQ_GPMI                (51)
#define IRQ_IR                  (52)
#define IRQ_DCP_VMI             (53)
#define IRQ_DCP                 (54)
#define IRQ_RESERVED_55         (55)
#define IRQ_BCH                 (56)
#define IRQ_PXP                 (57)
#define IRQ_UARTAPP2_TX_DMA     (58)
#define IRQ_UARTAPP2_INTERNAL   (59)
#define IRQ_UARTAPP2_RX_DMA     (60)
#define IRQ_VDAC_DETECT         (61)
#define IRQ_SYDMA               (62)
#define IRQ_SY                  (63)

#define IRQ_VDD5V_DROOP         (64)
#define IRQ_DCDC4P2_BO          (65)
#define IRQ_RESERVED_66         (66)
#define IRQ_RESERVED_67         (67)
#define IRQ_RESERVED_68         (68)
#define IRQ_RESERVED_69         (69)
#define IRQ_RESERVED_70         (70)
#define IRQ_RESERVED_71         (71)
#define IRQ_RESERVED_72         (72)  
#define IRQ_RESERVED_73         (73)
#define IRQ_RESERVED_74         (74)
#define IRQ_RESERVED_75         (75)
#define IRQ_RESERVED_76         (76)
#define IRQ_RESERVED_77         (77)
#define IRQ_RESERVED_78         (78)
#define IRQ_RESERVED_79         (79)
#define IRQ_RESERVED_80         (80)
#define IRQ_RESERVED_81         (81)
#define IRQ_RESERVED_82         (82)
#define IRQ_RESERVED_83         (83)
#define IRQ_RESERVED_84         (84)
#define IRQ_RESERVED_85         (85)
#define IRQ_RESERVED_86         (86)
#define IRQ_RESERVED_87         (87)
#define IRQ_RESERVED_88         (88)
#define IRQ_RESERVED_89         (89)
#define IRQ_RESERVED_90         (90)
#define IRQ_RESERVED_91         (91)
#define IRQ_RESERVED_92         (92)
#define IRQ_RESERVED_93         (93)
#define IRQ_RESERVED_94         (94)
#define IRQ_RESERVED_95         (95)

//------------------------------------------------------------------------------
// SECONDARY IRQ DEFINITIONS
//------------------------------------------------------------------------------
// /WARNING: when you change the virtual IRQ mapping, do not forget to 
// change all the interrupt-related functions (OALIntrDisableIrqs, OALIntrDoneIrqs ...).
//------------------------------------------------------------------------------
#define IRQ_GPIO0_PIN0          (IRQ_RESERVED_95 + 1)
#define IRQ_GPIO0_PIN1          (IRQ_GPIO0_PIN0 + 1)
#define IRQ_GPIO0_PIN2          (IRQ_GPIO0_PIN0 + 2)
#define IRQ_GPIO0_PIN3          (IRQ_GPIO0_PIN0 + 3)
#define IRQ_GPIO0_PIN4          (IRQ_GPIO0_PIN0 + 4)
#define IRQ_GPIO0_PIN5          (IRQ_GPIO0_PIN0 + 5)
#define IRQ_GPIO0_PIN6          (IRQ_GPIO0_PIN0 + 6)
#define IRQ_GPIO0_PIN7          (IRQ_GPIO0_PIN0 + 7)
#define IRQ_GPIO0_PIN8          (IRQ_GPIO0_PIN0 + 8)
#define IRQ_GPIO0_PIN9          (IRQ_GPIO0_PIN0 + 9)
#define IRQ_GPIO0_PIN10         (IRQ_GPIO0_PIN0 + 10)
#define IRQ_GPIO0_PIN11         (IRQ_GPIO0_PIN0 + 11)
#define IRQ_GPIO0_PIN12         (IRQ_GPIO0_PIN0 + 12)
#define IRQ_GPIO0_PIN13         (IRQ_GPIO0_PIN0 + 13)
#define IRQ_GPIO0_PIN14         (IRQ_GPIO0_PIN0 + 14)
#define IRQ_GPIO0_PIN15         (IRQ_GPIO0_PIN0 + 15)
#define IRQ_GPIO0_PIN16         (IRQ_GPIO0_PIN0 + 16)
#define IRQ_GPIO0_PIN17         (IRQ_GPIO0_PIN0 + 17)
#define IRQ_GPIO0_PIN18         (IRQ_GPIO0_PIN0 + 18)
#define IRQ_GPIO0_PIN19         (IRQ_GPIO0_PIN0 + 19)
#define IRQ_GPIO0_PIN20         (IRQ_GPIO0_PIN0 + 20)
#define IRQ_GPIO0_PIN21         (IRQ_GPIO0_PIN0 + 21)
#define IRQ_GPIO0_PIN22         (IRQ_GPIO0_PIN0 + 22)
#define IRQ_GPIO0_PIN23         (IRQ_GPIO0_PIN0 + 23)
#define IRQ_GPIO0_PIN24         (IRQ_GPIO0_PIN0 + 24)
#define IRQ_GPIO0_PIN25         (IRQ_GPIO0_PIN0 + 25)
#define IRQ_GPIO0_PIN26         (IRQ_GPIO0_PIN0 + 26)
#define IRQ_GPIO0_PIN27         (IRQ_GPIO0_PIN0 + 27)
#define IRQ_GPIO0_PIN28         (IRQ_GPIO0_PIN0 + 28)
#define IRQ_GPIO0_PIN29         (IRQ_GPIO0_PIN0 + 29)
#define IRQ_GPIO0_PIN30         (IRQ_GPIO0_PIN0 + 30)
#define IRQ_GPIO0_PIN31         (IRQ_GPIO0_PIN0 + 31)

#define IRQ_GPIO1_PIN0          (IRQ_GPIO0_PIN31 + 1)
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


// The total IRQ number in SoC level
#define IRQ_SOC_NUMBER          (IRQ_GPIO2_PIN31 + 1)


//------------------------------------------------------------------------------

#endif // __MX233_IRQ_H
