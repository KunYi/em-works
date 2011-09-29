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

//------------------------------------------------------------------------------
// Forward declarations
//
UINT16  RegValToDecimal(UINT8);
UINT16  RegValShortToDecimal(UINT16);
UINT8   DecimalToRegVal(UINT16);
UINT16  DecimalToRegValShort(UINT16);

//------------------------------------------------------------------------------
//
//  Function:  Bare_GetRealTime
//
//  This function is called by OEMGetRealTime.
//
BOOL Bare_GetRealTime(LPSYSTEMTIME lpst) 
{
    UINT8   second, minute, hour, wkday, day, month;
    UINT16  year;
    BOOL RetVal = FALSE;
    SH4_RTC_REGS *pRTCRegs = OALPAtoUA(SH4_REG_PA_RTC);

    // Check for invalid parameter
    if (NULL == lpst)
    {
        goto cleanUp;        
    }

    // Clear CIE flag
    OUTREG8(&pRTCRegs->RCR1, (INREG8(&pRTCRegs->RCR1) & (~RTC_RCR1_CIE)));

    do
    {
        // Clear carry flag 
        OUTREG8(&pRTCRegs->RCR1, (INREG8(&pRTCRegs->RCR1) & (~RTC_RCR1_CF)));

        second = INREG8(&pRTCRegs->RSECCNT);
        minute = INREG8(&pRTCRegs->RMINCNT);
        hour   = INREG8(&pRTCRegs->RHRCNT);
        wkday  = INREG8(&pRTCRegs->RWKCNT);
        day    = INREG8(&pRTCRegs->RDAYCNT);
        month  = INREG8(&pRTCRegs->RMONCNT);
        year   = INREG16(&pRTCRegs->RYRCNT);

    } while (INREG8(&pRTCRegs->RCR1) & RTC_RCR1_CF);

    // Convert and populate SYSTEMTIME values
    lpst->wMilliseconds = 0;
    lpst->wSecond       = RegValToDecimal(second);
    lpst->wMinute       = RegValToDecimal(minute);
    lpst->wHour         = RegValToDecimal(hour);
    lpst->wDayOfWeek    = wkday;
    lpst->wDay          = RegValToDecimal(day);
    lpst->wMonth        = RegValToDecimal(month);
    lpst->wYear         = RegValShortToDecimal(year);

    RetVal = TRUE;
cleanUp:
    return RetVal;
}

//------------------------------------------------------------------------------
//
//  Function:  Bare_SetRealTime
//
//  This function is called by OEMSetRealTime.
//
BOOL Bare_SetRealTime(LPSYSTEMTIME lpst) 
{
    UINT8  second, minute, hour, wkday, day, month;
    UINT16 year;
    BOOL RetVal = FALSE;
    SH4_RTC_REGS *pRTCRegs = OALPAtoUA(SH4_REG_PA_RTC);

    // Check for invalid parameter
    if (NULL == lpst)
    {
        goto cleanUp;        
    }
    if (lpst->wYear<2000 || lpst->wYear>=2100)
    {
        goto cleanUp;
    }

    // Convert system time to BCD
    second = DecimalToRegVal(lpst->wSecond);
    minute = DecimalToRegVal(lpst->wMinute);
    hour   = DecimalToRegVal(lpst->wHour);
    wkday  = (UINT8)lpst->wDayOfWeek;
    day    = DecimalToRegVal(lpst->wDay);
    month  = DecimalToRegVal(lpst->wMonth);
    year   = DecimalToRegValShort(lpst->wYear);

    // First stop RTC
    OUTREG8(&pRTCRegs->RCR2, (INREG8(&pRTCRegs->RCR2) & (~RTC_RCR2_START)));

    // Now reset RTC
    OUTREG8(&pRTCRegs->RCR2, (INREG8(&pRTCRegs->RCR2) | RTC_RCR2_RESET));

    // Place converted SYSTEMTIME into RTC registers
    OUTREG8(&pRTCRegs->RSECCNT, second);
    OUTREG8(&pRTCRegs->RMINCNT, minute);
    OUTREG8(&pRTCRegs->RHRCNT,  hour);
    OUTREG8(&pRTCRegs->RWKCNT,  wkday);
    OUTREG8(&pRTCRegs->RDAYCNT, day);
    OUTREG8(&pRTCRegs->RMONCNT, month);
    OUTREG16(&pRTCRegs->RYRCNT, year);

    // Finally, start RTC
    OUTREG8(&pRTCRegs->RCR2, INREG8(&pRTCRegs->RCR2) | RTC_RCR2_START);

    RetVal = TRUE;
cleanUp:
    return RetVal;
}

//------------------------------------------------------------------------------
//
//  Function:   RegValtoDecimal / RegValShortToDecimal
//
//  Convert BCD to Decimal
//
UINT16 RegValToDecimal(UINT8 regVal)
{
    UINT16 tensVal = 0;
    UINT16 onesVal = 0;

    onesVal = regVal & 0x0F;
    tensVal = ((regVal >> 4) & 0x0F) * 10;

    return (tensVal + onesVal);
}
UINT16 RegValShortToDecimal(UINT16 regVal)
{
    UINT16 thousandsVal = 0;
    UINT16 hundredsVal  = 0;

    hundredsVal  = ((regVal >> 8) & 0x0F) * 100;
    thousandsVal = ((regVal >> 12) & 0x0F) * 1000;

    return (thousandsVal + hundredsVal + RegValToDecimal((UINT8)(regVal & 0x00FF)));
}

//------------------------------------------------------------------------------
//
//  Function:   DecimalToRegVal / DecimalToRegValShort
//
//  Convert Decimal to BCD
//
UINT8 DecimalToRegVal(UINT16 decimal)
{
    UINT8 tensVal = 0;
    UINT8 onesVal = 0;

    tensVal = (decimal / 10) << 4;
    onesVal = (decimal % 10);

    return (tensVal | onesVal);
}

UINT16 DecimalToRegValShort(UINT16 decimal)
{
    UINT16 thousandsVal = 0;
    UINT16 hundredsVal  = 0;

    thousandsVal = (decimal / 1000) << 12;
    hundredsVal  = ((decimal % 1000) / 100) << 8;

    return (thousandsVal | hundredsVal | DecimalToRegVal((UINT8)(decimal%100)));
}

