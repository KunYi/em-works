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
//  Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Module: alarm.c
//
//  Real-time clock (RTC) routines for the Freescale MX31 processor
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"

//------------------------------------------------------------------------------
// External Functions
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, 
    DDK_CLOCK_GATE_MODE mode);
extern BOOL CheckRealTime(LPSYSTEMTIME pTime);
extern WORD ConvertSystemTimeToDays(LPSYSTEMTIME pTime);



//------------------------------------------------------------------------------
// Defines
#define RTC_DEBUG_MSG       0

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot.
//  Input buffer contains SYSTEMTIME structure with default time value.
//  If hardware has persistent real time clock it will ignore this value
//  (or all call).
//
BOOL OALIoCtlHalInitRTC(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize
) {
    BOOL rc = FALSE;
    SYSTEMTIME *pTime = (SYSTEMTIME*)pInpBuffer;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(1, (L"+OALIoCtlHalInitRTC(...)\r\n"));

    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalInitRTC: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    // Add static mapping for RTC alarm
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM, IRQ_RTC);

    // Enable RTC clocks
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_RTC, 
        DDK_CLOCK_GATE_MODE_ENABLED_ALL);

    // Set time
    rc = OEMSetRealTime(pTime);

cleanUp:
    OALMSG(1, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  Set the RTC alarm time.
//
BOOL OEMSetAlarmTime(SYSTEMTIME *pTime)
{
    BOOL rc = FALSE;
    WORD sec, min, hour, day;
    pRTCRegisters_t pRtc;
    UINT32 irq;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n",
        pTime->wMonth, pTime->wDay, pTime->wYear, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    if (pTime == NULL) goto cleanUp;

    // Check if the input time is valid or not
    if (CheckRealTime(pTime) == FALSE)
    {
        OALMSG(OAL_ERROR, (TEXT("Alarm to be set INVALID!!!:  %u/%u/%u, %u:%u:%u  Day of week:%u \r\n"),
               pTime->wYear, pTime->wMonth , pTime->wDay, pTime->wHour, pTime->wMinute, pTime->wSecond,pTime->wDayOfWeek));
        goto cleanUp;
    }

    OALMSG(OAL_RTC&&OAL_INFO, (TEXT("Alarm to be set is OK:  %u/%u/%u, %u:%u:%u  Day of week:%u \r\n"),
           pTime->wYear, pTime->wMonth , pTime->wDay, pTime->wHour, pTime->wMinute, pTime->wSecond,pTime->wDayOfWeek));

    // Get uncached virtual addresses for RTC
    pRtc = (pRTCRegisters_t) OALPAtoUA(CSP_BASE_REG_PA_RTC);
    if (pRtc == NULL)
    {
        OALMSG(OAL_ERROR, (L"OEMSetAlarmTime:  RTC null pointer!\r\n"));
        goto cleanUp;
    }

    // Disable and clear alarm IRQ
    OUTREG32(&pRtc->RTCIntEnable, (pRtc->RTCIntEnable & ~RTC_INT_EN_ALARM));
    OUTREG32(&pRtc->RTCIntStatus, (pRtc->RTCIntStatus | RTC_INT_STAT_ALARM));

    DEBUGMSG(RTC_DEBUG_MSG, (TEXT("RTC Alarm Interrupt disabled.\r\n")));

    sec = pTime->wSecond;
    min = pTime->wMinute;
    hour = pTime->wHour;
    day = ConvertSystemTimeToDays(pTime);

    //Set seconds in RTC seconds alarm register
    OUTREG32(&pRtc->RTCSecAlarm, sec);

    //Set hours and minutes in RTC hours and minutes alarm register
    OUTREG32(&pRtc->RTCHMAlarm, (hour << RTC_HOUR_OFFSET) | min);

    //Set day in RTC day alarm register
    OUTREG32(&pRtc->RTCDayAlarm, day);

    // Enable alarm IRQ
    OUTREG32(&pRtc->RTCIntEnable, (pRtc->RTCIntEnable | RTC_INT_EN_ALARM));

    OALMSG(OAL_RTC&&OAL_INFO, (TEXT("RTC Alarm Interrupt enabled.\r\n")));

    // Enable/clear RTC interrupt
    irq = IRQ_RTC;
    OALIntrDoneIrqs(1, &irq);

    // Done
    rc = TRUE;

cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
