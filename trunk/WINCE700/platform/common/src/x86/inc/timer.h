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

// PIT oscillator is 1.193182 Mhz
#define TIMER_FREQ      1193182

BYTE CMOS_Read( BYTE offset );
void CMOS_Write( BYTE offset, BYTE value );

extern int (*PProfileInterrupt)(void);
extern const DWORD g_dwBSPMsPerIntr;

DWORD IoDelay (DWORD dwUsDelay);
__inline unsigned __int64 __cdecl GetTimeStampCounter(void);
BOOL CalibrateTSC(DWORD dwUsDelay);
DWORD GetCpuIdSignature (void);

#define IPI_QUERY_PERF_COUNTER                      (OEM_FIRST_IPI_COMMAND + 1)