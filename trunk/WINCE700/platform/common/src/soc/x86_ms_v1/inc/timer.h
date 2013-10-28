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
// Define the frequency of the system tick.
#define SYSTEM_TICK_MS  1      // time in milliseconds

// PIT oscillator is 1.19318 Mhz
#define TIMER_FREQ      1193180

// PIT granularity is then 1/1.19318, or 0.838097
#define TIMER_BASE      838

// Timer count value for 1 ms
#define OEM_COUNT_1MS (TIMER_FREQ / 1000)

// Timer count value for rescheduler period (1193 for 1 ms system tick)
#define TIMER_COUNT     ((SYSTEM_TICK_MS * TIMER_FREQ) / 1000)

BYTE CMOS_Read( BYTE offset );
void CMOS_Write( BYTE offset, BYTE value );
