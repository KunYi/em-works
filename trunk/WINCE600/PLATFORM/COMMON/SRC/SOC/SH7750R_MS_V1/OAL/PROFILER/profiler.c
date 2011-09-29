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
//  File: profiler.c
//  
//  This file contains the profiler timer functions implementation
//

#include <windows.h>
#include <shx.h>
#include <nkintr.h>
#include <oal.h>

//------------------------------------------------------------------------------
//
//  Function: ProfilerISR
//
//  This function implements profiler interrupt handler.  It is enabled via
//  a call to the OEMProfileTimerEnable function.
//
UINT32 ProfilerISR(VOID) 
{
    volatile SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);       

    // Clear the interrupt flag
    OUTREG16(&pTMURegs->TCR1, INREG16(&pTMURegs->TCR1) & ~TMU_TCR_UNF);
    
    ProfilerHit(GetEPC());
    return SYSINTR_NOP;
}

//------------------------------------------------------------------------------
//
//  Function: OEMProfileTimerEnable
//
//  This function enables TMU 1 for use in profiling mode.
//
VOID OEMProfileTimerEnable(DWORD interval)
{
    volatile SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);    
    UINT32 ProfilerTicks;

    OALMSG(OAL_FUNC, (L"+OEMProfileTimerEnable(0x%08x)\r\n", 
                      interval));

    if (interval == 0)
    {
        // Use default rate of 1 ms
        interval = 1000;
    }
    else if (interval < 20)
    {
        // Ensure the specified rate is a reasonable minimum (20 us)        
        interval = 20;
    }

    ProfilerTicks = (interval * g_oalTimer.countsPerMSec) / 1000;

    // Initialize TMU1 and enable interrupt

    // Ensure TMU1 is stopped
    OUTREG8(&pTMURegs->TSTR, INREG8(&pTMURegs->TSTR) & ~TMU_TSTR_STR1);

    // Set the input frequency to Peripheral / 16
    OUTREG16(&pTMURegs->TCR1, TMU_TCR_D16);

    // Enable underflow interrupts
    OUTREG16(&pTMURegs->TCR1, INREG16(&pTMURegs->TCR1) | TMU_TCR_UNIE);

    // Clear any pending interrupts
    OUTREG16(&pTMURegs->TCR1, INREG16(&pTMURegs->TCR1) & ~TMU_TCR_UNF);

    // Initialize TMU1 down counter and reload registers
    OUTREG32(&pTMURegs->TCNT1, ProfilerTicks);
    OUTREG32(&pTMURegs->TCOR1, ProfilerTicks);

    // Start Timer
    OUTREG8(&pTMURegs->TSTR, INREG8(&pTMURegs->TSTR) | TMU_TSTR_STR1);

    OALMSG(OAL_FUNC, (L"-OEMProfileTimerEnable\r\n"));    
    return;
}

//------------------------------------------------------------------------------
//
//  Function: OEMProfileTimerDisable
//
//  This function disables TMU 1.
//

VOID OEMProfileTimerDisable(VOID)
{
    volatile SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU); 

    OALMSG(OAL_FUNC, (L"+OEMProfileTimerDisable\r\n"));

    // Stop TMU1
    OUTREG8(&pTMURegs->TSTR, INREG8(&pTMURegs->TSTR) & ~TMU_TSTR_STR1);

    OALMSG(OAL_FUNC, (L"-OEMProfileTimerDisable\r\n"));      
}



