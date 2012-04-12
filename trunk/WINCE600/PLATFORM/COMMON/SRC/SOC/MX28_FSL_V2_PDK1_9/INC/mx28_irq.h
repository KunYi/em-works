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
//  File:  mx28_irq.h
//
//  This file defines names for IRQ. These names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
//  This header contains IRQ definitions for the iMX28 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX28_IRQ_H
#define __MX28_IRQ_H

//------------------------------------------------------------------------------
// DEFINITIONS
//------------------------------------------------------------------------------
// Support of multiple IRQs per one SYSINTR mapping
#define IRQ_PER_SYSINTR         4

// The maximum IRQ number supported by mapping, which should
// accommodate both SoC and board level IRQs.
#define IRQ_MAXIMUM             IRQ_SOC_NUMBER


//------------------------------------------------------------------------------
// MX28-SPECIFIC IRQ DEFINITIONS
//------------------------------------------------------------------------------
#define IRQ_BATT_BROWNOUT       (0)
#define IRQ_VDDD_BROWNOUT       (1)
#define IRQ_VDDIO_BROWNOUT      (2)
#define IRQ_VDDA_BROWNOUT       (3)
#define IRQ_VDD5V_DROOP         (4)
#define IRQ_DCDC4P2_BROWNOUT    (5)
#define IRQ_VDD5V               (6)
#define IRQ_RESERVED_7          (7)
#define IRQ_CAN0                (8)
#define IRQ_CAN1                (9)
#define IRQ_LRADC_TOUCH         (10)
#define IRQ_RESERVED_11         (11)
#define IRQ_RESERVED_12         (12)
#define IRQ_HSADC               (13)
#define IRQ_LRADC_THRESH0       (14)
#define IRQ_LRADC_THRESH1       (15)
#define IRQ_LRADC_CH0           (16)
#define IRQ_LRADC_CH1           (17)
#define IRQ_LRADC_CH2           (18)
#define IRQ_LRADC_CH3           (19)
#define IRQ_LRADC_CH4           (20)
#define IRQ_LRADC_CH5           (21)
#define IRQ_LRADC_CH6           (22)
#define IRQ_LRADC_CH7           (23)
#define IRQ_LRADC_BUTTON0       (24)
#define IRQ_LRADC_BUTTON1       (25)
#define IRQ_RESERVED_26         (26)
#define IRQ_BUS_PERFMON         (27)
#define IRQ_RTC_1MSEC           (28)
#define IRQ_RTC_ALARM           (29)
#define IRQ_RESERVED_30         (30)
#define IRQ_COMMS               (31)
#define IRQ_EMI_ERROR           (32)
#define IRQ_RESERVED_33         (33)
#define IRQ_RESERVED_34         (34)
#define IRQ_RESERVED_35         (35)
#define IRQ_RESERVED_36         (36)
#define IRQ_RESERVED_37         (37)
#define IRQ_LCDIF               (38)
#define IRQ_PXP                 (39)
#define IRQ_RESERVED_40         (40)
#define IRQ_BCH                 (41)
#define IRQ_GPMI                (42)
#define IRQ_RESERVED_43         (43)
#define IRQ_RESERVED_44         (44)
#define IRQ_SPDIF_ERROR         (45)
#define IRQ_RESERVED_46         (46)
#define IRQ_DBG_UART            (47)
#define IRQ_TIMER0              (48)
#define IRQ_TIMER1              (49)
#define IRQ_TIMER2              (50)
#define IRQ_TIMER3              (51)
#define IRQ_DCP_VMI             (52)
#define IRQ_DCP                 (53)
#define IRQ_DCP_SECURE          (54)
#define IRQ_RESERVED_55         (55)
#define IRQ_RESERVED_56         (56)
#define IRQ_RESERVED_57         (57)
#define IRQ_SAIF1               (58)
#define IRQ_SAIF0               (59)
#define SW_IRQ_60               (60)
#define SW_IRQ_61               (61)
#define SW_IRQ_62               (62)
#define SW_IRQ_63               (63)
#define IRQ_RESERVED_64         (64)
#define IRQ_RESERVED_65         (65)
#define IRQ_SPDIF_DMA           (66)
#define IRQ_RESERVED_67         (67)
#define IRQ_I2C0_DMA            (68)
#define IRQ_I2C1_DMA            (69)
#define IRQ_AUART0_RX_DMA       (70)
#define IRQ_AUART0_TX_DMA       (71)
#define IRQ_AUART1_RX_DMA       (72)
#define IRQ_AUART1_TX_DMA       (73)
#define IRQ_AUART2_RX_DMA       (74)
#define IRQ_AUART2_TX_DMA       (75)
#define IRQ_AUART3_RX_DMA       (76)
#define IRQ_AUART3_TX_DMA       (77)
#define IRQ_AUART4_RX_DMA       (78)
#define IRQ_AUART4_TX_DMA       (79)
#define IRQ_SAIF0_DMA           (80)
#define IRQ_SAIF1_DMA           (81)
#define IRQ_SSP0_DMA            (82)
#define IRQ_SSP1_DMA            (83)
#define IRQ_SSP2_DMA            (84)
#define IRQ_SSP3_DMA            (85)
#define IRQ_LCDIF_DMA           (86)
#define IRQ_HSADC_DMA           (87)
#define IRQ_GPMI_DMA            (88)
#define IRQ_DIGCTL_DEBUG_TRAP   (89)
#define IRQ_RESERVED_90         (90)
#define IRQ_RESERVED_91         (91)
#define IRQ_USB1                (92)
#define IRQ_USB0                (93)
#define IRQ_USB1_WAKEUP         (94)
#define IRQ_USB0_WAKEUP         (95)
#define IRQ_SSP0_ERROR          (96)
#define IRQ_SSP1_ERROR          (97)
#define IRQ_SSP2_ERROR          (98)
#define IRQ_SSP3_ERROR          (99)
#define IRQ_ENET_SWI            (100)
#define IRQ_ENET_MAC0           (101)
#define IRQ_ENET_MAC1           (102)
#define IRQ_ENET_MAC0_1588      (103)
#define IRQ_ENET_MAC1_1588      (104)
#define IRQ_RESERVED_105        (105)
#define IRQ_RESERVED_106        (106)
#define IRQ_RESERVED_107        (107)
#define IRQ_RESERVED_108        (108)
#define IRQ_RESERVED_109        (109)
#define IRQ_I2C1_ERROR          (110)
#define IRQ_I2C0_ERROR          (111)
#define IRQ_APP_UART0           (112)
#define IRQ_APP_UART1           (113)
#define IRQ_APP_UART2           (114)
#define IRQ_APP_UART3           (115)
#define IRQ_APP_UART4           (116)
#define IRQ_RESERVED_117        (117)
#define IRQ_RESERVED_118        (118)
#define IRQ_RESERVED_119        (119)
#define IRQ_RESERVED_120        (120)
#define IRQ_RESERVED_121        (121)
#define IRQ_RESERVED_122        (122)
#define IRQ_GPIO4               (123)
#define IRQ_GPIO3               (124)
#define IRQ_GPIO2               (125)
#define IRQ_GPIO1               (126)
#define IRQ_GPIO0               (127)

//------------------------------------------------------------------------------
// SECONDARY IRQ DEFINITIONS
//------------------------------------------------------------------------------
// /WARNING: when you change the virtual IRQ mapping, do not forget to 
// change all the interrupt-related functions (OALIntrDisableIrqs, OALIntrDoneIrqs ...).
//------------------------------------------------------------------------------

//GPIO0  Only Pin0-Pin7,Pin16-Pin28 are valid.
#define IRQ_GPIO0_PIN0          (IRQ_GPIO0 + 1)
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

//GPIO1 Pin0-Pin31 are valid
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

//GPIO2 Only Pin0-Pin10 Pin12-Pin21,Pin24-Pin27 are valid.
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

//GPIO3 Only Pin0-Pin18,Pin20-Pin30 are valid.
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

//GPIO4 Only Pin0-Pin16,Pin20 are valid.
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


// The total IRQ number in SoC level
#define IRQ_SOC_NUMBER          (IRQ_GPIO4_PIN31 + 1)   //288 total in Mx28


//------------------------------------------------------------------------------

#endif // __MX28_IRQ_H
