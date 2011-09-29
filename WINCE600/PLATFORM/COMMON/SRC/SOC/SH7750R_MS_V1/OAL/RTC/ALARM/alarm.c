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
//  File:  rtc.c
//
#include <windows.h>
#include <oal.h>
#include <nkintr.h>
#include "shx.h"

BOOL Bare_SetAlarmTime(LPSYSTEMTIME lpst);

//------------------------------------------------------------------------------
// Utility routines for converting RTC register values into decimal values
//
UINT16  RegValToDecimal(UINT8);
UINT16  RegValShortToDecimal(UINT16);
UINT8   DecimalToRegVal(UINT16);
UINT16  DecimalToRegValShort(UINT16);

//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  This function is called by the kernel to set the real-time clock alarm.
//
BOOL OEMSetAlarmTime(LPSYSTEMTIME lpst) 
{
    BOOL RetVal, PrevState;

    PrevState = INTERRUPTS_ENABLE(FALSE);
    RetVal = Bare_SetAlarmTime(lpst);
    INTERRUPTS_ENABLE(PrevState);

    return RetVal;
}

//------------------------------------------------------------------------------
//
//  Function:  Bare_SetAlarmTime
//
//  This function is called by OEMSetAlarmTime.
//
BOOL Bare_SetAlarmTime(LPSYSTEMTIME lpst)
{
    SH4_RTC_REGS    *pRTCRegs   = OALPAtoUA(SH4_REG_PA_RTC);
    SH4_RTC_REGS_EX *pRTCRegsEx = OALPAtoUA(SH4_REG_PA_RTC_EX);
    SH4_INTC_REGS   *pINTCRegs  = OALPAtoUA(SH4_REG_PA_INTC);
    BOOL            retVal      = FALSE;

    OALMSG(OAL_FUNC, (L"+OEMSetAlarmTime()\r\n"));

    if (NULL == lpst)
    {
        goto cleanUp;
    }

    // Disable RTC interrupts
    OUTREG8(&pRTCRegs->RCR1, (INREG8(&pRTCRegs->RCR1) & ~(RTC_RCR1_AIE)));

    // Convert and populate RTC registers with alarm values
    OUTREG8(&pRTCRegs->RSECAR,   (DecimalToRegVal(lpst->wSecond)   | RTC_RSECAR_ENB));
    OUTREG8(&pRTCRegs->RMINAR,   (DecimalToRegVal(lpst->wMinute)   | RTC_RMINAR_ENB));
    OUTREG8(&pRTCRegs->RHRAR,    (DecimalToRegVal(lpst->wHour)     | RTC_RHRAR_ENB));
    OUTREG8(&pRTCRegs->RWKAR,    0);
    OUTREG8(&pRTCRegs->RDAYAR,   (DecimalToRegVal(lpst->wDay)      | RTC_RDAYAR_ENB));
    OUTREG8(&pRTCRegs->RMONAR,   (DecimalToRegVal(lpst->wMonth)    | RTC_RMONAR_ENB));
    OUTREG16(&pRTCRegsEx->RYRAR, DecimalToRegValShort(lpst->wYear));
    OUTREG8(&pRTCRegsEx->RCR3,   RTC_RYRAR_ENB);

    // Clear alarm flag and enable alarm interrupt
    OUTREG8(&pRTCRegs->RCR1, (INREG8(&pRTCRegs->RCR1) & ~RTC_RCR1_AF) | RTC_RCR1_AIE);

    retVal = TRUE;

cleanUp:
    OALMSG(OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", retVal));
    return retVal;
}

//------------------------------------------------------------------------------
//
int AlarmISR()
{
    SH4_RTC_REGS *pRTCRegs = OALPAtoUA(SH4_REG_PA_RTC);

    // Clear alarm flag and disable the interrupt
    OUTREG8(&pRTCRegs->RCR1, (INREG8(&pRTCRegs->RCR1) & ~(RTC_RCR1_AF | RTC_RCR1_AIE)));

    // Do not reenable the interrupt here. It will be set up in the next call
    // to OEMSetAlarmTime. All we need to do is let the system know that 
    // this was an alarm interrupt that fired.

    return SYSINTR_RTC_ALARM;
}
