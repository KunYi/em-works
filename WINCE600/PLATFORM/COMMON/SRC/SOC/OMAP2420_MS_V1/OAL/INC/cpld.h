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
//  File:  cpld.h
//
//  This file contains specification for H2/P2 debug board registers and
//  related constants. 
//
#ifndef __CPLD_H
#define __CPLD_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

typedef struct 
{
    UINT16   RESERVED1[8];       //reserved 0x00-0x0E
    UINT16   CPLD;				//offset 0x10
	UINT16   BOARD;				//offset 0x12
	UINT16   GPIO;		        //offset 0x14
    UINT16   LED;		        //offset 0x16
    UINT16   MISCINPUT;		    //offset 0x18
    UINT16   LANSTAT;	        //offset 0x1A
    UINT16   LANCTL;            //offset 0x1C
    UINT16   QUARTCTLSTAT;      //offset 0x1E
	UINT16   HIDTxRxDATA;	    //offset 0x20
	UINT16   HIDRxCTLSTAT;      //offset 0x22
	UINT16   QUARTENBLE;	    //offset 0x24
	UINT16   RESERVED2[5];	    //reserved offset 0x26-0x30
	UINT16   QUARTREG[8];       //offset 0x30-0x40
	UINT16   RESERVED3[352];     //reserved offset 0x40-0x300
	UINT16   LANBASE;			//offset 0x300
}
CPLD_REGS;

//------------------------------------------------------------------------------

#define CPLD_LANCTL_RST         (1 << 0)

#define CPLD_LANSTAT_RDY        (1 << 0)

#define CPLD_GPO7               (1 << 15)
#define CPLD_GPO6               (1 << 14)
#define CPLD_GPO5               (1 << 13)
#define CPLD_GPIO4              (1 << 12)
#define CPLD_GPIO3              (1 << 11)
#define CPLD_GPIO2              (1 << 10)
#define CPLD_GPI1               (1 << 9)
#define CPLD_GPI0               (1 << 8)

#define CPLD_GPIO_DIROUT4       (1 << 4)
#define CPLD_GPIO_DIROUT3       (1 << 3)
#define CPLD_GPIO_DIROUT2       (1 << 2)

#define CPLD_LED_BIG_SLEEP      (1 << 0)
#define CPLD_LED_IDLE           (1 << 1)
#define CPLD_LED_TIMER_SPIN     (1 << 2)
#define CPLD_LED_TIMER_SPIN_MASK (0xff << 2)
#define CPLD_LED_FREE_TO_USE_0  (1 << 10)
#define CPLD_LED_FREE_TO_USE_1  (1 << 11)
#define CPLD_LED_FREE_TO_USE_2  (1 << 12)
#define CPLD_LED_FREE_TO_USE_3  (1 << 13)
#define CPLD_LED_FREE_TO_USE_4  (1 << 14)
#define CPLD_LED_FREE_TO_USE_5  (1 << 15)

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
