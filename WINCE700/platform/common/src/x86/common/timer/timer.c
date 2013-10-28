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
//------------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  
//------------------------------------------------------------------------------
#include <windows.h>
#include <pc.h>
#include <nkintr.h>
#include <timer.h>
#include <oal.h>


DWORD  g_dwOALTimerCount;

static DWORD  dwOALTicksPerMs;

volatile ULARGE_INTEGER CurTicks = { 0, 0 };
//
// Kernel global variables used by GetIdleTime() to determine CPU utilization.
//

//
// Profiling
//
DWORD dwReschedCount;
DWORD dwProfilingMultiple;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SetTimer0(
               WORD wVal
               )
{
    __asm {
        // configure counter for correct mode
        mov     al, 00110100b           ; counter 0, 16-bit, mode 2, binary
        out     043h, al
        jmp     short $+2
        // load the timer with correct count value
        mov     ax, wVal
        out     040h, al
        jmp     short $+2
        mov     al,ah
        out     040h, al
    }
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int ProfileInterrupt(void) 
{
    ProfilerHit(GetEPC());
    dwReschedCount++;
    
    if (dwReschedCount < dwProfilingMultiple)
        return SYSINTR_NOP;

    dwReschedCount=0;
    return SYSINTR_RESCHED;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void OEMProfileTimerEnable(
                           DWORD dwUSec
                           )
{
    DWORD dwUSecAdjusted;

    if (dwUSec) {
        //
        // Rate specified (limit boundary)
        //
        if (dwUSec < 50) {
            dwUSec = 50;
        }
        if (dwUSec > 1000) {
            dwUSec = 1000;
        }
    } else {
        //
        // Use default rate (100 uS)
        //
        dwUSec = 100;
    }
    //
    // We can only program the timer such that 1ms is a direct multiple
    //
    dwProfilingMultiple = 1000 * g_dwBSPMsPerIntr / dwUSec;
    dwUSecAdjusted = 1000 * g_dwBSPMsPerIntr / dwProfilingMultiple;

    OALMSG(OAL_TIMER, (TEXT("Starting profile timer at %d uS rate\r\n"), dwUSecAdjusted));

    dwReschedCount = 0;
    PProfileInterrupt = ProfileInterrupt;
    
    SetTimer0((WORD) (g_dwOALTimerCount / dwProfilingMultiple));
    
    PICEnableInterrupt(INTR_TIMER0, TRUE);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void OEMProfileTimerDisable(void) 
{
    PProfileInterrupt = NULL;
    SetTimer0 ((WORD) g_dwOALTimerCount);
    PICEnableInterrupt(INTR_TIMER0, TRUE);
}

extern BOOL x86InitPerfCounter();

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void InitClock() 
#pragma warning(suppress:4740) //flow in or out of inline asm code suppresses global optimization
{
    BYTE cData;

    //
    // Set up translation constant for GetIdleTime() (1 ms units).
    // Note: Since curridlehigh, curridlelow is counting in ms, and GetIdleTime( )
    // reports in ms, the conversion ratio is one.  If curridlehigh, curridlelow
    // were using other units (like ticks), then the conversion would be calculated
    // from the clock frequency.
    //
    idleconv = 1;
    
    // Give the kernel access to the profiling functions
    g_pOemGlobal->pfnProfileTimerEnable  = OEMProfileTimerEnable;
    g_pOemGlobal->pfnProfileTimerDisable = OEMProfileTimerDisable;

    //
    // Setup Timer0 to fire every TICK_RATE mS and generate interrupt
    //
    g_dwOALTimerCount = (g_dwBSPMsPerIntr * TIMER_FREQ) / 1000;
    dwOALTicksPerMs   = TIMER_FREQ / 1000;
    
    SetTimer0 ((WORD) g_dwOALTimerCount);
    PICEnableInterrupt(INTR_TIMER0, TRUE);

    //
    // Set up Timer2 to use its full range for kcall profiling
    //
    __asm {
        // configure counter for correct mode
        mov     al, 10110100b           ; counter 2, 16-bit, mode 2, binary
        out     043h, al
        jmp     short $+2
        // Start counter at highest value, it's a countdown.
        // It's confusing, but 0 is largest initial value.  Read the manual.
        xor     eax, eax    ; 0x00
        out     042h, al
        jmp     short $+2
        out     042h, al
        // Enable bit 0 of Port B to start counter2 counting
        in      al, 61h     ;Read Current value of Port B
        or      al, 00000001b     ;Set bit 0 to enable gate
        out     61h, al     ;Store new value to Port B
    }

    do {
        cData = CMOS_Read( RTC_STATUS_A);
    } while ( cData & RTC_SRA_UIP );
    cData = CMOS_Read( RTC_STATUS_B );
    CMOS_Write( RTC_STATUS_B, (BYTE)(cData|RTC_SRB_24HR) );
    cData = CMOS_Read( (BYTE)(RTC_STATUS_B) );
    OALMSG(OAL_TIMER, (TEXT("RTC - Status Reg B - 0x%2.2X\r\n"), cData));
    PICEnableInterrupt(INTR_RTC, TRUE);

    // Set the RTC rollover interval to 100 years
    g_pOemGlobal->dwYearsRTCRollover = 100;

    x86InitPerfCounter ();
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CPUEnterIdle(
                  DWORD dwIdleParam
                  )
{
    UNREFERENCED_PARAMETER(dwIdleParam);

    _asm {
        sti
        hlt
    }
}


//------------------------------------------------------------------------------
//
//  This routine is called by the kernel when there are no threads ready to
//  run. The CPU should be put into a reduced power mode and halted. It is 
//  important to be able to resume execution quickly upon receiving an interrupt.
//
// In general the PIT (8254) does not support immediate reprogramming of the counter
// (the 8254 waits until the current countdown cycle is finished before reloading the
// counter with the new value) so OEMIdle never re-program the counter.  If this
// OAL is being adapted to a system with a reprogrammable countdown timer or a
// free running counter with compare registers, see other platforms for example
//------------------------------------------------------------------------------
void OEMIdle(
             DWORD dwIdleParam
             )
{
    DWORD dwIdleMs = dwReschedTime - CurMSec;

    if ((int) dwIdleMs > 0) {

        // Use for 64-bit math
        ULARGE_INTEGER currIdle;
        DWORD dwPrevMSec = CurMSec;
        currIdle.HighPart = curridlehigh;
        currIdle.LowPart = curridlelow;

        // max idle time is the system tick
        if (dwIdleMs > g_dwBSPMsPerIntr)
            dwIdleMs = g_dwBSPMsPerIntr;

        // enter idle
        CPUEnterIdle (dwIdleParam);

        INTERRUPTS_OFF ();

        if (dwPrevMSec != CurMSec) 
        {
            // waked up by timer interrupt, update global idle time
            currIdle.QuadPart += dwIdleMs;
            curridlelow = currIdle.LowPart;
            curridlehigh = currIdle.HighPart;
        }
        
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD OEMGetTickCount() 
{
    return CurMSec;
}
