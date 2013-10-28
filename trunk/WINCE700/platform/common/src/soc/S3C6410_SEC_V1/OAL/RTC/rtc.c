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
//
//------------------------------------------------------------------------------
//
//  Module: rtc.c
//
//  Real-time clock (RTC) routines for the Samsung S3C6410 processor
//



#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>

#include "s3c6410_base_regs.h"
#include "s3c6410_rtc.h"
#include "s3c6410_vintr.h"


//------------------------------------------------------------------------------
// Defines

#define FROM_BCD(n)            ((((n) >> 4) * 10) + ((n) & 0xf))
#define TO_BCD(n)            ((((n) / 10) << 4) | ((n) % 10))
#define RTC_YEAR_DATUM         1980

//------------------------------------------------------------------------------
//
//  External:  g_oalRtcResetTime
//
//  RTC init time after a RTC reset has occured. It should
//  be defined in platform code.
extern SYSTEMTIME g_oalRtcResetTime;

//Global variable for the Critical section of RTC
extern CRITICAL_SECTION g_oalRTCcs;


//Global variable to hold the Init variable to check whether critical section is initialised
extern unsigned long    g_oalRTCcsInit;


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
        UINT32 code, VOID *pInpBuffer, UINT32 inpSize,
        VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{


    BOOL rc = FALSE;
    BOOL bResetRTC = FALSE;
    SYSTEMTIME SysTime;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalInitRTC: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    // Add static mapping for RTC alarm
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM, IRQ_RTC_ALARM);

    OEMGetRealTime(&SysTime);

    /* RTC Time validity check */
    bResetRTC = (SysTime.wYear  < RTC_YEAR_DATUM    || (SysTime.wYear - RTC_YEAR_DATUM) > 99)   ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wMonth > 12                || SysTime.wMonth < 1)                      ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wDay   > 31                || SysTime.wDay < 1)                        ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wDayOfWeek > 6             || SysTime.wDayOfWeek < 0)                  ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wHour  > 23                || SysTime.wHour < 0)                       ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wMinute > 59               || SysTime.wMinute < 0)                     ? TRUE : bResetRTC;
    bResetRTC = (SysTime.wSecond > 59               || SysTime.wSecond < 0)                     ? TRUE : bResetRTC;

    if(bResetRTC)
    {
        OALMSG(OAL_RTC&&OAL_ERROR,(L"Invalid RTC Time (%d.%d.%d, %d:%d:%d, (%d th day of week)\r\n", \
            SysTime.wYear, SysTime.wMonth,  SysTime.wDay,    \
            SysTime.wHour, SysTime.wMinute, SysTime.wSecond,    \
            SysTime.wDayOfWeek
            ));
    }

    // Set time
    if(bResetRTC)
    {
        rc = OEMSetRealTime(&g_oalRtcResetTime);
    }

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  Reads the current RTC value and returns a system time.
//
BOOL OEMGetRealTime(SYSTEMTIME *pTime)
{
    volatile S3C6410_RTC_REG *pRTCReg;
    UINT16 seconds;
    BOOL rc = FALSE;
    BOOL csEnter;

    /* can get called before IoctlHalInitRTC, if so then don't try to 
       enter (uninitialized) cs (kernel should be single-proc, single-threaded here) */
    if (g_oalRTCcsInit)
    {
        EnterCriticalSection(&g_oalRTCcs);
        csEnter = TRUE;
    }
    else
        csEnter = FALSE;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OEMGetRealTime(pTime = 0x%x)\r\n", pTime));

    if (pTime == NULL) goto cleanUp;

    // Get uncached virtual address
    pRTCReg = (S3C6410_RTC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_RTC, FALSE);

    // Enable RTC control.
    pRTCReg->RTCCON |= (1<<0);

    do
    {
        seconds = FROM_BCD(pRTCReg->BCDSEC & 0x7f);
        pTime->wYear        = FROM_BCD((pRTCReg->BCDYEAR & 0xff)) + RTC_YEAR_DATUM;
        pTime->wMonth       = FROM_BCD(pRTCReg->BCDMON & 0x1f);
        pTime->wDay         = FROM_BCD(pRTCReg->BCDDATE & 0x3f);
        pTime->wDayOfWeek   = pRTCReg->BCDDAY - 1;
        pTime->wHour        = FROM_BCD(pRTCReg->BCDHOUR & 0x3f);
        pTime->wMinute      = FROM_BCD(pRTCReg->BCDMIN & 0x7f);
        pTime->wSecond      = FROM_BCD(pRTCReg->BCDSEC & 0x7f);
        pTime->wMilliseconds= 0;
    } while (pTime->wSecond != seconds);

    // Disable RTC control.
    pRTCReg->RTCCON &= ~(1<<0);

    // Done
    rc = TRUE;

cleanUp:

    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMGetRealTime(rc = %d)\r\n", rc));

    if (csEnter)
        LeaveCriticalSection(&g_oalRTCcs);

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  Updates the RTC with the specified system time.
//
BOOL OEMSetRealTime(LPSYSTEMTIME pTime)
{
    BOOL rc = FALSE;
    volatile S3C6410_RTC_REG *pRTCReg;
    BOOL csEnter;

    /* can get called before IoctlHalInitRTC, if so then don't try to 
       enter (uninitialized) cs (kernel should be single-proc, single-threaded here) */
    if (g_oalRTCcsInit)
    {
        EnterCriticalSection(&g_oalRTCcs);
        csEnter = TRUE;
    }
    else
        csEnter = FALSE;

    if (pTime == NULL) goto cleanUp;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n",
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds));

    OALMSG(TRUE, (
        L"OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n",
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds));

    // The RTC will only support a BCD year value of 0 - 99.  The year datum is
    // 1980, so any dates greater than 2079 will fail unless the datum is
    // adjusted.
    if ((pTime->wYear < RTC_YEAR_DATUM)
        || ((pTime->wYear - RTC_YEAR_DATUM) > 99))
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMSetRealTime: "
            L"RTC cannot support a year greater than %d or less than %d "
            L"(value %d)\r\n", (RTC_YEAR_DATUM + 99), RTC_YEAR_DATUM,
            pTime->wYear));

        goto cleanUp;
    }

    // Get uncached virtual address
    pRTCReg = (S3C6410_RTC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_RTC, FALSE);

    // Enable RTC control.
    pRTCReg->RTCCON |= (1<<0);

    pRTCReg->BCDSEC     = TO_BCD(pTime->wSecond);
    pRTCReg->BCDMIN     = TO_BCD(pTime->wMinute);
    pRTCReg->BCDHOUR    = TO_BCD(pTime->wHour);
    pRTCReg->BCDDATE    = TO_BCD(pTime->wDay);
    pRTCReg->BCDDAY     = pTime->wDayOfWeek + 1;
    pRTCReg->BCDMON     = TO_BCD(pTime->wMonth);
    pRTCReg->BCDYEAR    = TO_BCD(pTime->wYear - RTC_YEAR_DATUM);

    // Disable RTC control.
    pRTCReg->RTCCON &= ~(1<<0);

    // Done
    rc = TRUE;

cleanUp:

    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetRealTime(rc = %d)\r\n", rc));

    if (csEnter)
        LeaveCriticalSection(&g_oalRTCcs);

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
    volatile S3C6410_RTC_REG *pRTCReg;
    UINT32 irq;
    BOOL csEnter;

    /* can get called before IoctlHalInitRTC, if so then don't try to 
       enter (uninitialized) cs (kernel should be single-proc, single-threaded here) */
    if (g_oalRTCcsInit)
    {
        EnterCriticalSection(&g_oalRTCcs);
        csEnter = TRUE;
    }
    else
        csEnter = FALSE;

    if (pTime == NULL) goto cleanUp;

    OALMSG(OAL_RTC&&OAL_FUNC, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n",
        pTime->wMonth, pTime->wDay, pTime->wYear, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds));

    OALMSG(TRUE, (
        L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)\r\n",
        pTime->wMonth, pTime->wDay, pTime->wYear, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds));


    // The RTC will only support a BCD year value of 0 - 99.  The year datum is
    // 1980, so any dates greater than 2079 will fail unless the datum is
    // adjusted.
    if ((pTime->wYear <RTC_YEAR_DATUM)
        || ((pTime->wYear - RTC_YEAR_DATUM) > 99))
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMSetRealTime: "
            L"RTC cannot support a year greater than %d or less than %d "
            L"(value %d)\r\n", (RTC_YEAR_DATUM + 99), RTC_YEAR_DATUM,
            pTime->wYear));

        goto cleanUp;
    }
    // Get uncached virtual address
    pRTCReg = (S3C6410_RTC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_RTC, FALSE);

    // Enable RTC control
    pRTCReg->RTCCON |= (1<<0);

    pRTCReg->ALMSEC     = TO_BCD(pTime->wSecond);
    pRTCReg->ALMMIN     = TO_BCD(pTime->wMinute);
    pRTCReg->ALMHOUR    = TO_BCD(pTime->wHour);
    pRTCReg->ALMDATE    = TO_BCD(pTime->wDay);
    pRTCReg->ALMMON     = TO_BCD(pTime->wMonth);
    pRTCReg->ALMYEAR    = TO_BCD(pTime->wYear - RTC_YEAR_DATUM);

    // Enable the RTC Alarm
    pRTCReg->RTCALM = 0x7f;

    // Disable RTC control.
    pRTCReg->RTCCON  &= ~(1<<0);

    // Clear RTC Alarm Interrupt Pending
    pRTCReg->INTP |= (1<<1);

    // Enable and Clear RTC Alarm Interrupt
    irq = IRQ_RTC_ALARM;
    OALIntrDoneIrqs(1, &irq);

    // Done
    rc = TRUE;

cleanUp:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));

    if (csEnter)
        LeaveCriticalSection(&g_oalRTCcs);

    return rc;
}

//------------------------------------------------------------------------------
