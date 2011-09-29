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
//  File:  omap2420_irq.h
//
//  This file defines names for IRQ. This names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
#ifndef __OMAP2420_IRQ_H
#define __OMAP2420_IRQ_H

//------------------------------------------------------------------------------

#define OMAP2420_IRQ_MAXIMUM         IRQ_GPIO_128
#define OMAP2420_IRQ_PER_SYSINTR     5
#define OMAP2420_SYSINTR_PER_IRQ     8            // IRQ Sharing

//------------------------------------------------------------------------------

#define IRQ_EMUINT                   0    //MPU Emulation
#define IRQ_COMMRX                   1	  //MPU Emulation
#define IRQ_COMMTX                   2	  //MPU Emulation
#define IRQ_BENCH                    3	  //MPU Emulation
#define IRQ_XTI                      4	  //XTI Module
#define IRQ_XTI_WKUP                 5	  //XTI Module
#define IRQ_SSM_ABORT                6	  //MPU Subsystem secure state machine
                                          //abort (internally generated)
#define IRQ_SYS_NIRQ                 7	  //External interrupt (active low)
#define IRQ_RESERVED_8               8	  //Reserved
#define IRQ_RESERVED_9				 9	  //Reserved
#define IRQ_L3                       10	  //L3 interconnect(transaction error)
#define IRQ_PRCM_MPU                 11   //PRCM Module
#define IRQ_SDMA_0                   12   //System DMA intr request 0
#define IRQ_SDMA_1                   13   //System DMA intr request 1
#define IRQ_SDMA_2                   14   //System DMA intr request 2
#define IRQ_SDMA_3                   15   //System DMA intr request 3
#define IRQ_RESERVED_16              16   //Reserved
#define IRQ_RESERVED_17              17   //Reserved
#define IRQ_RESERVED_18              18   //Reserved
#define IRQ_RESERVED_19              19   //Reserved
#define IRQ_GPMC                     20   //GPMC module
#define IRQ_GUFFAW                   21   //2D/3D graphics module
#define IRQ_IVA                      22   //IVA subsystem 
#define IRQ_EAC                      23   //Audio Controller (Also mapped on DSP
                                          //subsystem INTC)
#define IRQ_CAM_0                    24   //Camera Interface request 0
#define IRQ_DSS                      25   //Display Subsystem module
#define IRQ_MAIL_U0_MPU              26   //Mailbox user 0 interrupt request
#define IRQ_DSP_UMA                  27   //DSP subsystem UMA core software interrupt
#define IRQ_DSP_MMU                  28   //DSP Subsystem MMU
#define IRQ_GPIO1_MPU                29   //GPIO module 1
#define IRQ_GPIO2_MPU                30   //GPIO module 2
#define IRQ_GPIO3_MPU                31   //GPIO module 3
#define IRQ_GPIO4_MPU                32   //GPIO module 4
#define IRQ_RESERVED_33              33   //Reserved
#define IRQ_MAIL_U3_MPU              34   //Mailbox user3 interrupt request
#define IRQ_WDT3                     35   //Watchdog timer module 3 overflow
#define IRQ_WDT4                     36   //Watchdog timer module 4 overflow
#define IRQ_GPT1                     37   //GP-Timer 1 module
#define IRQ_GPT2                     38   //GP-Timer 2 module 
#define IRQ_GPT3                     39   //GP-Timer 3 module
#define IRQ_GPT4                     40   //GP-Timer 4 module 
#define IRQ_GPT5                     41   //GP-Timer 5 module
#define IRQ_GPT6                     42   //GP-Timer 6 module
#define IRQ_GPT7                     43   //GP-Timer 7 module
#define IRQ_GPT8                     44   //GP-Timer 8 module
#define IRQ_GPT9                     45   //GP-Timer 9 module
#define IRQ_GPT10                    46   //GP-Timer 10 module
#define IRQ_GPT11                    47   //GP-Timer 11 module
#define IRQ_GPT12                    48   //GP-Timer 12 module
#define IRQ_RESERVED_49              49   //Reserved
#define IRQ_PKA                      50   //PKA crypto-accelerator
#define IRQ_SHA1MD5                  51   //SHA1/MD5 crypto-accelerator
#define IRQ_RNG                      52   //RNG module
#define IRQ_MG                       53   //MG function
#define IRQ_RESERVED_54              54   //Reserved
#define IRQ_RESERVED_55              55   //Reserved
#define IRQ_I2C1                     56   //I2C Module 1
#define IRQ_I2C2                     57   //I2C Module 2
#define IRQ_HDQ                      58   //HDQ/1-wire
#define IRQ_McBSP1_TX                59   //McBSP module 1 transmit
#define IRQ_McBSP1_RX                60   //McBSP module 1 receive
#define IRQ_RESERVED_61              61   //Reserved
#define IRQ_McBSP2_TX                62   //McBSP module 2 transmit
#define IRQ_McBSP2_RX                63   //McBSP module 2 receive
#define IRQ_RESERVED_64              64   //Reserved
#define IRQ_SPI1                     65   //McSPI module 1
#define IRQ_SPI2                     66   //McSPI module 2
#define IRQ_SSI_P1_MPU_0             67   //dual SSI port 1 intr request 0
#define IRQ_SSI_P1_MPU_1             68   //dual SSI port 1 intr request 1
#define IRQ_SSI_P2_MPU_0             69   //dual SSI port 2 intr request 0
#define IRQ_SSI_P2_MPU_1             70   //dual SSI port 2 intr request 1
#define IRQ_SSI_GDD_MPU              71   //dual SSI GDD
#define IRQ_UART1                    72   //UART module 1
#define IRQ_UART2                    73   //UART module 2
#define IRQ_UART3                    74   //UART module 3
#define IRQ_USB_GEN                  75   //USB device generic interrupt
#define IRQ_USB_NISO                 76   //USB device non-ISO
#define IRQ_USB_ISO                  77   //USB device ISO
#define IRQ_USB_HGEN                 78   //USB host general interrupt
#define IRQ_USB_HSOF                 79   //USB host start frame
#define IRQ_USB_OTG                  80   //USB OTG
#define IRQ_VLYNQ                    81   //VLYNQ module
#define IRQ_RESERVED_82              82   //Reserved
#define IRQ_MMC                      83   //MMC/SD module
#define IRQ_MS                       84   //MS-PRO module
#define IRQ_FAC                      85   //FAC module
#define IRQ_RESERVED_86              86   //Reserved
#define IRQ_RESERVED_87              87   //Reserved
#define IRQ_RESERVED_88              88   //Reserved
#define IRQ_RESERVED_89              89   //Reserved
#define IRQ_RESERVED_90              90   //Reserved
#define IRQ_RESERVED_91              91   //Reserved
#define IRQ_RESERVED_92              92   //Reserved
#define IRQ_RESERVED_93              93   //Reserved
#define IRQ_RESERVED_94              94   //Reserved
#define IRQ_RESERVED_95              95   //Reserved

// MENELAUS 1 Power Management Chipset
#define IRQ_MENELAUS_CD1            100         // SLOT1 INSERT/REMOVAL EVENT
#define IRQ_MENELAUS_CD2            101         // SLOT2 INSERT/REMOVAL EVENT
#define IRQ_MENELAUS_DL1            102         // SLOT1 DATA1 LOW EVENT
#define IRQ_MENELAUS_DL2            103         // SLOT2 DATA1 LOW EVENT
#define IRQ_MENELAUS_LOWBAT         104         // LOW BATTERY EVENT
#define IRQ_MENELAUS_HOTDIE         105         // HOT DIE EVENT
#define IRQ_MENELAUS_UVLO           106         // UVLO EVENT
#define IRQ_MENELAUS_TSHUT			107			// THERMAL SHUTDOWN EVENT
#define IRQ_MENELAUS_RTCTMR         108         // RTC TIMER EVENT
#define IRQ_MENELAUS_RTCALM         109         // RTC ALARM EVENT
#define IRQ_MENELAUS_RCERR          110         // RTC ERROR EVENT
#define IRQ_MENELAUS_PSHBTN			111			// PUSH BUTTON EVENT

#define IRQ_GPIO_0                  160         // GPIO1 bit 0
#define IRQ_GPIO_32                 192         // GPIO2 bit 0
#define IRQ_GPIO_64                 224         // GPIO3 bit 0
#define IRQ_GPIO_96                 256         // GPIO4 bit 0
#define IRQ_GPIO_128                288         // Limit on GPIO interrupts
//------------------------------------------------------------------------------

#endif // __OMAP2420_IRQ_H
