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
//  File: Timer.c

#include <windows.h>
#include <shx.h>
#include <nkintr.h>
#include <oal.h>

//------------------------------------------------------------------------------
// Forward Declarations
VOID OALTimerSetCount(UINT32 count);

//------------------------------------------------------------------------------
//
//  Function:  OALStall
//
//  Wait for time specified in parameter in microseconds (busy wait). This
//  function can be called in hardware/kernel initialization process.
//
//  NOTE:  This function should not be called before the TMU is enabled.
//
//  NOTE:  This function should only be used to delay for short amounts
//         of time.  See comments in the code below.
//
//  NOTE:  This function will wait for a minimum of microSec microseconds.
//         It may wait longer than that if the system is busy handling 
//         many interrupts, or if the owner process gets put to sleep
//         by the kernel scheduler.
//
VOID OALStall(UINT32 microSec)
{
    UINT32 baseTime, curTime, delta;
    UINT64 numWaitCounts, elapsedWaitCounts = 0;
    volatile SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);    

    // First, get current TMU counter value
    baseTime = INREG32(&pTMURegs->TCNT0);   

    // Check for invalid parameters and status
    if (microSec == 0 || g_oalTimer.countsPerMSec == 0)
    {
        return;
    }

    // If called with a large value for the microSec parameter
    // and the timer ticks at a high frequency (i.e., there are a 
    // large number of ticks in each millisecond), it is possible
    // that this calculation will overflow, and thus this function
    // will wait for less time than was requested.
    numWaitCounts = (microSec * g_oalTimer.countsPerMSec) / 1000;

    while ( elapsedWaitCounts < numWaitCounts )
    {
        // To wait for an accurate number of milliseconds, we need to 
        // account for timer underflow.  Thus, we need to calculate
        // the delta seen in the TMU between this iteration of the 
        // while loop and the previous iteration, and use the delta as 
        // the basis for updating the elapsed time.  
        
        curTime = INREG32(&pTMURegs->TCNT0);
        if (curTime < baseTime) // Timer unit has counted down, but no underflow
        {
            delta = (baseTime - curTime);
        }
        else if (baseTime < curTime) // Timer unit has underflowed and rolled over
        {
            delta = g_oalTimer.countsPerMSec - curTime + baseTime + 1;
        }
        else
        {
            delta = 0;
        }

        elapsedWaitCounts += delta;

        // Update the TMU base count for the next iteration
        baseTime = curTime;
    }
}


//------------------------------------------------------------------------------
//
VOID OALTimerInitCount(UINT32 count)
{
    OALTimerSetCompare(count);
    OALTimerSetCount(count);
}

//------------------------------------------------------------------------------
//
UINT32 OALTimerGetCount()
{
    UINT32        retVal   = 0;
    SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);

    // Get current count value
    retVal = INREG32(&pTMURegs->TCNT0) + 1;

    return retVal;
}

//------------------------------------------------------------------------------
//
VOID OALTimerSetCount(UINT32 count)
{
    UINT32        retVal   = 0;
    UINT32        setDelay = 0;
    SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);

    // Stop Timer
    OUTREG8(&pTMURegs->TSTR, INREG8(&pTMURegs->TSTR) & ~TMU_TSTR_STR0);

    // Update the count
    OUTREG32(&pTMURegs->TCNT0, count + setDelay - 1);

    // Start Timer
    OUTREG8(&pTMURegs->TSTR, INREG8(&pTMURegs->TSTR) | TMU_TSTR_STR0);
}

//------------------------------------------------------------------------------
//
UINT32 OALTimerGetCompare()
{
    SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);
    return INREG32(&pTMURegs->TCOR0) + 1;
}


//------------------------------------------------------------------------------
//
VOID OALTimerSetCompare(UINT32 count)
{
    SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);
    OUTREG32(&pTMURegs->TCOR0, count - 1);
}

//------------------------------------------------------------------------------
