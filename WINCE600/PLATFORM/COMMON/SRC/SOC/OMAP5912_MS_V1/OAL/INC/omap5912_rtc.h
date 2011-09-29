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
//  File:  omap5912_rtc.h
//
#ifndef __OMAP5912_RTC_H
#define __OMAP5912_RTC_H

//------------------------------------------------------------------------------

#define		RTC_MEMORY_ALIGNMENT							4

typedef volatile struct {

    UINT8 SECS;             // 0000
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0000[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 MINS;             // 0001
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0001[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 HOURS;            // 0002
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0002[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 DAY;              // 0003   
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0003[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 MONTH;            // 0004
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0004[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 YEAR;             // 0005
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0005[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 WEEKDAY;          // 0006
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0006[RTC_MEMORY_ALIGNMENT-1];
										#endif
										#ifndef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0007;       // 0007
										#else
										    UINT8 UNUSED0007[RTC_MEMORY_ALIGNMENT];
										#endif
    UINT8 ALARM_SECS;       // 0008
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0008[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 ALARM_MINS;       // 0009
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0009[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 ALARM_HOURS;      // 000A
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED000A[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 ALARM_DAY;        // 000B
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED000B[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 ALARM_MONTH;      // 000C
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED000C[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 ALARM_YEAR;       // 000D
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED000D[RTC_MEMORY_ALIGNMENT-1];
										#endif
										#ifndef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED000E;       // 000E
										#else
										    UINT8 UNUSED000E[RTC_MEMORY_ALIGNMENT];
										#endif
										#ifndef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED000F;       // 000F
										#else
										    UINT8 UNUSED000F[RTC_MEMORY_ALIGNMENT];
										#endif
    UINT8 CTRL;             // 0010
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0010[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 STAT;             // 0011
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0011[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 INTR;             // 0012
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0012[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 COMP_LSB;         // 0013
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0013[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 COMP_MSB;         // 0014
										#ifdef RTC_MEMORY_ALIGNMENT
										    UINT8 UNUSED0014[RTC_MEMORY_ALIGNMENT-1];
										#endif
    UINT8 OSC;              // 0015
} OMAP5912_RTC_REGS;

//------------------------------------------------------------------------------

#define RTC_STAT_BUSY       (1 << 0)
#define RTC_STAT_RUN        (1 << 1)
#define RTC_STAT_ALARM      (1 << 6)
#define RTC_STAT_RESET      (1 << 7)

#define RTC_CTRL_INIT       0
#define RTC_CTRL_RUN        (1 << 0)

#define RTC_INTR_ALARM      (1 << 3)

//------------------------------------------------------------------------------

#define		RTC_REG(RegisterNumber)     ((UINT8 volatile *) OALPAtoUA(OMAP5912_RTC_REGS_PA + RegisterNumber * RTC_MEMORY_ALIGNMENT))


#define	RTC_SECS				0x0000
#define	RTC_MINS				0x0001
#define	RTC_HOURS				0x0002
#define	RTC_DAY					0x0003
#define	RTC_MONTH				0x0004
#define	RTC_YEAR				0x0005
#define	RTC_WEEKDAY				0x0006
#define	RTC_ALARM_SECS			0x0008
#define	RTC_ALARM_MINS			0x0009
#define	RTC_ALARM_HOURS			0x000A
#define	RTC_ALARM_DAY			0x000B
#define	RTC_ALARM_MONTH			0x000C
#define	RTC_ALARM_YEAR			0x000D
#define	RTC_CTRL				0x0010
#define	RTC_STAT				0x0011
#define	RTC_INTR				0x0012
#define	RTC_COMP_LSB			0x0013
#define	RTC_COMP_MSB			0x0014
#define	RTC_OSC					0x0015

#endif // __OMAP5912_RTC_H
