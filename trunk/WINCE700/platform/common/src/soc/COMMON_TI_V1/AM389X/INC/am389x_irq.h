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
/*
================================================================================
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  am389x_irq.h
//
//  This file defines names for IRQ. This names have no other role than make
//  code more readable. For SoC where device - IRQ mapping is defined by
//  silicon and it can't be changed by software.
//
#ifndef __AM389X_IRQ_H
#define __AM389X_IRQ_H


//------------------------------------------------------------------------------
//
//  Global: Flag for fake OEMIdle busy loop

extern volatile BOOL fInterruptFlag;


//------------------------------------------------------------------------------

#define AM389X_IRQ_MAXIMUM                192
#define AM389X_IRQ_PER_SYSINTR            3
#define AM389X_GPIO_BANK_COUNT            2

//------------------------------------------------------------------------------
//
//  Physical IRQs on the AM389X
//

#define IRQ_EMUINT                      0   //
#define IRQ_COMMRX                      1   //
#define IRQ_COMMTX                      2   //
#define IRQ_BENCH                       3   //
#define IRQ_ELM                         4   //
#define IRQ_SSM_WFI                     5   //
#define IRQ_SSM                         6   //
#define IRQ_NMI                         7   // External source (active low ????)
#define IRQ_SEC_EVNT                    8   // Firewalls
#define IRQ_L3DEBUG                     9   //
#define IRQ_L3APPINT                    10  // 
//#define IRQ_PRCM_MPU                    11  
#define IRQ_EDMACOMPINIT                12  //
#define IRQ_EDMAMPERR                   13  //
#define IRQ_EDMAERRINT                  14  //
//#define IRQ_PBISTINT                    15  
#define IRQ_SATAINT                     16  //
#define IRQ_USBSSINT                    17  //
#define IRQ_USBINT0                     18  //
#define IRQ_USBINT1                     19  //
// 20-33
#define IRQ_USBWAKEUP                   34  //
#define IRQ_PCIWAKEUP                   35  //
#define IRQ_DSSINT                      36  //
#define IRQ_GFXINT                      37  //
#define IRQ_HDMIINT                     38  //
// 39
#define IRQ_MACRXTHR0                   40  //
#define IRQ_MACRXINT0                   41  //
#define IRQ_MACTXINT0                   42  //
#define IRQ_MACMISC0                    43  //
#define IRQ_MACRXTHR1                   44  //
#define IRQ_MACRXINT1                   45  //
#define IRQ_MACTXINT1                   46  //
#define IRQ_MACMISC1                    47  //
#define IRQ_PCIINT0                     48  //
#define IRQ_PCIINT1                     49  //
#define IRQ_PCIINT2                     50  //
#define IRQ_PCIINT3                     51  //
// 52-63
#define IRQ_SDINT                       64  //
#define IRQ_SPIINT                      65  //
#define IRQ_TIMER0                      66  //
#define IRQ_TIMER1                      67  //
#define IRQ_TIMER2                      68  //
#define IRQ_TIMER3                      69  //
#define IRQ_I2CINT0                     70  //
#define IRQ_I2CINT1                     71  //
#define IRQ_UART0INT                    72  //
#define IRQ_UART1INT                    73  //
#define IRQ_UART2INT                    74  //
#define IRQ_RTCINT                      75  //
#define IRQ_RTCALARM                    76  //
#define IRQ_MBINT                       77  // Mailbox
// 78-79
#define IRQ_MCA0TX                      80  //
#define IRQ_MCA0RX                      81  //
#define IRQ_MCA1TX                      82  //
#define IRQ_MCA1RX                      83  //
#define IRQ_MCA2TX                      84  //
#define IRQ_MCA2RX                      85  //
#define IRQ_MCBSP                       86  //
// 87-90
#define IRQ_WDT                         91  //
#define IRQ_TIMER4                      92  //
#define IRQ_TIMER5                      93  //
#define IRQ_TIMER6                      94  //
#define IRQ_TIMER7                      95  // 
#define IRQ_GPIO0A                      96  //
#define IRQ_GPIO0B                      97  //
#define IRQ_GPIO1A                      98  //
#define IRQ_GPIO1B                      99  //
#define IRQ_CPMC                       100  // 
#define IRQ_DDRERR0                    101  //
#define IRQ_DDRERR1                    102  //
// 103-111
#define IRQ_TCERR0                     112  //
#define IRQ_TCERR1                     113  //
#define IRQ_TCERR2                     114  //
#define IRQ_TCERR3                     115  //
// 116-119
#define IRQ_SMRFLX0                    120  //
#define IRQ_SMRFLX1                    121  //
#define IRQ_SYSMMU                     122  //
#define IRQ_MCMMU                      123  //
#define IRQ_DMM                        124  //
#define MAX_IRQ_COUNT                  125  // Total number of IRQs 


//------------------------------------------------------------------------------
//
//  Software IRQs for kernel/driver interaction
//

#define IRQ_SW_RTC_QUERY                125         // Query to OAL RTC
#define IRQ_SW_RESERVED_1               126
#define IRQ_SW_RESERVED_2               127
#define IRQ_SW_RESERVED_MAX             128


//------------------------------------------------------------------------------
//
//  GPIO virtual IRQs for flattening out the 2 level GPIO interrupts
//

//GPIO Bank 1
#define IRQ_GPIO_0                      128         // GPIO1 bit 0
#define IRQ_GPIO_1                      129         // GPIO1 bit 1
#define IRQ_GPIO_2                      130         // GPIO1 bit 2
#define IRQ_GPIO_3                      131         // GPIO1 bit 3
#define IRQ_GPIO_4                      132         // GPIO1 bit 4
#define IRQ_GPIO_5                      133         // GPIO1 bit 5
#define IRQ_GPIO_6                      134         // GPIO1 bit 6
#define IRQ_GPIO_7                      135         // GPIO1 bit 7
#define IRQ_GPIO_8                      136         // GPIO1 bit 8
#define IRQ_GPIO_9                      137         // GPIO1 bit 9
#define IRQ_GPIO_10                     138         // GPIO1 bit 10
#define IRQ_GPIO_11                     139         // GPIO1 bit 11
#define IRQ_GPIO_12                     140         // GPIO1 bit 12
#define IRQ_GPIO_13                     141         // GPIO1 bit 13
#define IRQ_GPIO_14                     142         // GPIO1 bit 14
#define IRQ_GPIO_15                     143         // GPIO1 bit 15
#define IRQ_GPIO_16                     144         // GPIO1 bit 16
#define IRQ_GPIO_17                     145         // GPIO1 bit 17
#define IRQ_GPIO_18                     146         // GPIO1 bit 18
#define IRQ_GPIO_19                     147         // GPIO1 bit 19
#define IRQ_GPIO_20                     148         // GPIO1 bit 20
#define IRQ_GPIO_21                     149         // GPIO1 bit 21
#define IRQ_GPIO_22                     150         // GPIO1 bit 22
#define IRQ_GPIO_23                     151         // GPIO1 bit 23
#define IRQ_GPIO_24                     152         // GPIO1 bit 24
#define IRQ_GPIO_25                     153         // GPIO1 bit 25
#define IRQ_GPIO_26                     154         // GPIO1 bit 26
#define IRQ_GPIO_27                     155         // GPIO1 bit 27
#define IRQ_GPIO_28                     156         // GPIO1 bit 28
#define IRQ_GPIO_29                     157         // GPIO1 bit 29
#define IRQ_GPIO_30                     158         // GPIO1 bit 30
#define IRQ_GPIO_31                     159         // GPIO1 bit 31

//GPIO Bank 2
#define IRQ_GPIO_32                     160         // GPIO2 bit 0
#define IRQ_GPIO_33                     161         // GPIO2 bit 1
#define IRQ_GPIO_34                     162         // GPIO2 bit 2
#define IRQ_GPIO_35                     163         // GPIO2 bit 3
#define IRQ_GPIO_36                     164         // GPIO2 bit 4
#define IRQ_GPIO_37                     165         // GPIO2 bit 5
#define IRQ_GPIO_38                     166         // GPIO2 bit 6
#define IRQ_GPIO_39                     167         // GPIO2 bit 7
#define IRQ_GPIO_40                     168         // GPIO2 bit 8
#define IRQ_GPIO_41                     169         // GPIO2 bit 9
#define IRQ_GPIO_42                     170         // GPIO2 bit 10
#define IRQ_GPIO_43                     171         // GPIO2 bit 11
#define IRQ_GPIO_44                     172         // GPIO2 bit 12
#define IRQ_GPIO_45                     173         // GPIO2 bit 13
#define IRQ_GPIO_46                     174         // GPIO2 bit 14
#define IRQ_GPIO_47                     175         // GPIO2 bit 15
#define IRQ_GPIO_48                     176         // GPIO2 bit 16
#define IRQ_GPIO_49                     177         // GPIO2 bit 17
#define IRQ_GPIO_50                     178         // GPIO2 bit 18
#define IRQ_GPIO_51                     179         // GPIO2 bit 19
#define IRQ_GPIO_52                     180         // GPIO2 bit 20
#define IRQ_GPIO_53                     181         // GPIO2 bit 21
#define IRQ_GPIO_54                     182         // GPIO2 bit 22
#define IRQ_GPIO_55                     183         // GPIO2 bit 23
#define IRQ_GPIO_56                     184         // GPIO2 bit 24
#define IRQ_GPIO_57                     185         // GPIO2 bit 25
#define IRQ_GPIO_58                     186         // GPIO2 bit 26
#define IRQ_GPIO_59                     187         // GPIO2 bit 27
#define IRQ_GPIO_60                     188         // GPIO2 bit 28
#define IRQ_GPIO_61                     189         // GPIO2 bit 29
#define IRQ_GPIO_62                     190         // GPIO2 bit 30
#define IRQ_GPIO_63                     191         // GPIO2 bit 31
//------------------------------------------------------------------------------
#endif // __AM389X_IRQ_H

