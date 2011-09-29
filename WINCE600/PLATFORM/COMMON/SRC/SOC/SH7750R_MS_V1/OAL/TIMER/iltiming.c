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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

   iltiming.c

Abstract:

   This module implements the code to support iltiming measurement.

Notes:


--*/

#include <windows.h>
#include <shx.h>
#include <nkintr.h>
#include <oal.h>
#include <pkfuncs.h>
#include <iltiming.h>


//------------------------------------------------------------------------------
//
// Global:  g_oalILT
//
OAL_ILT_STATE g_oalILT;

#define PERIPHERAL_CLOCK_FREQ       60000000
#define TIMER_INPUT_DIVISOR         16
#define TIMER_INPUT_DIVISOR_CODE    0x0001
#define COUNT_FREQUENCY             (PERIPHERAL_CLOCK_FREQ / TIMER_INPUT_DIVISOR)

const DWORD OEMTimerFreq = COUNT_FREQUENCY;		            // timer frequency

DWORD SHxTimer2CountSinceTick (void)
{
    volatile SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);
    return ( INREG32(&pTMURegs->TCOR2) - INREG32(&pTMURegs->TCNT2) );
}

DWORD PerfCountFreq (void)
{
	return OEMTimerFreq;
}

//------------------------------------------------------------------------------
//
// Timer2 - handle iltiming
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Start timer 2 (parameter - interrupt interval in micro-seconds)
//------------------------------------------------------------------------------
void SHxStartTimer2 (DWORD dwUSec)
{
    DWORD dwCount;
    volatile SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);    

    if (!dwUSec) {
        //
        // Use default rate (1 MS)
        //
        dwUSec = 1000;

    } else if (dwUSec < 20) {
        //
        // Rate specified: at least 20us
        //
        dwUSec = 20;
    }
    
    dwCount = (dwUSec * g_oalTimer.countsPerMSec) / 1000;

    //
    // Init timer2 and enable timer2 interrupt
    //

    // make sure timer2 is topped
    OUTREG8(&pTMURegs->TSTR, INREG8(&pTMURegs->TSTR) & ~TMU_TSTR_STR2);

    // initialize timer constant and count register
    OUTREG32(&pTMURegs->TCOR2, dwCount);
    OUTREG32(&pTMURegs->TCNT2, dwCount);

    // enable timer2 interrupts
    // Enable underflow interrupts
    OUTREG16(&pTMURegs->TCR2, INREG16(&pTMURegs->TCR2) | TMU_TCR_UNIE);
    // Clear any pending interrupts
    OUTREG16(&pTMURegs->TCR2, INREG16(&pTMURegs->TCR2) & ~TMU_TCR_UNF);

    // start timer2
    OUTREG8(&pTMURegs->TSTR, INREG8(&pTMURegs->TSTR) | TMU_TSTR_STR2);

}

//------------------------------------------------------------------------------
// Stop Timer 2
//------------------------------------------------------------------------------
void SHxStopTimer2 (void)
{
    volatile SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);    
    //
    // Diable timer2 interrupt
    //
    OUTREG16(&pTMURegs->TCR2, INREG16(&pTMURegs->TCR2) & ~TMU_TCR_UNIE);

}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL SHxIoCtllTiming (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned
)
{
    BOOL fEnable;
    int  nMSec;
    PILTIMING_MESSAGE pITM;

    if(lpBytesReturned)
    {
        *lpBytesReturned = 0;
    }

    if ((nInBufSize != sizeof(ILTIMING_MESSAGE)) || (lpInBuf == NULL))
    {
        RETAILMSG (1, (TEXT("IOCTL_HAL_ILTIMING : BAD PARAMETERS!!!\r\n")));
        NKSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    pITM = (PILTIMING_MESSAGE) lpInBuf;

    switch (pITM->wMsg) {
        
        case ILTIMING_MSG_ENABLE :
        if (!(nMSec = (int) pITM->dwFrequency))
            {
        RETAILMSG (1, (TEXT("ILTiming dwFreqency cannot be 0\r\n")));
        return FALSE;
            }
            fEnable = INTERRUPTS_ENABLE (FALSE);
            g_oalILT.interrupts = 0;
            g_oalILT.isrTime1 = 0xFFFFFFFF;
            SHxStartTimer2 (nMSec * 1000);
            g_oalILT.active = TRUE;
            INTERRUPTS_ENABLE (fEnable);
            RETAILMSG (1, (TEXT("ILTiming Enable (@ every %d MSec)\r\n"), nMSec));
            break;
        
        case ILTIMING_MSG_DISABLE :
            RETAILMSG (1, (TEXT("ILTiming Disable\r\n")));
            fEnable = INTERRUPTS_ENABLE (FALSE);
            SHxStopTimer2();
            g_oalILT.active = FALSE;
            INTERRUPTS_ENABLE (fEnable);
            break;
        
        case ILTIMING_MSG_GET_TIMES :
            pITM->dwIsrTime1 = g_oalILT.isrTime1;
            pITM->dwIsrTime2 = g_oalILT.isrTime2;
            pITM->wNumInterrupts = g_oalILT.interrupts;
            pITM->dwSPC = g_oalILT.savedPC;
            pITM->dwFrequency = PerfCountFreq();
            g_oalILT.interrupts = 0;
            RETAILMSG (1, (TEXT("ILTiming GetTime @ 0x%08X:%08X\r\n"), pITM->dwIsrTime1, pITM->dwIsrTime2));
            break;
        
        case ILTIMING_MSG_GET_PFN :
            pITM->pfnPerfCountSinceTick = (PVOID) SHxTimer2CountSinceTick;
            RETAILMSG (1, (TEXT("ILTiming GetPFN\r\n")));
            break;
        
        default : 
            RETAILMSG (1, (TEXT("IOCTL_HAL_ILTIMING : BAD MESSAGE!!!\r\n")));
            return (FALSE);
    }

    return (TRUE);
}

//------------------------------------------------------------------------------
// ISR for Timer2
//------------------------------------------------------------------------------
DWORD SHxTimer2ISR (void)
{
    volatile SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);    

    g_oalILT.isrTime1 = SHxTimer2CountSinceTick();
    g_oalILT.savedPC = GetEPC();
    g_oalILT.interrupts++;

    //
    // clear underflow flag
    //
    do
    {
        OUTREG16(&pTMURegs->TCR2, INREG16(&pTMURegs->TCR2) & (~TMU_TCR_UNF));
    } while (INREG16(&pTMURegs->TCR2) & TMU_TCR_UNF);

    g_oalILT.interrupts = 0;
    g_oalILT.isrTime2 = SHxTimer2CountSinceTick();
    return SYSINTR_TIMING;

}

