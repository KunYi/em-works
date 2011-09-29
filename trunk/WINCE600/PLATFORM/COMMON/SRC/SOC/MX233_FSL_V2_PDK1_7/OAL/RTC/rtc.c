//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Module: rtc.c
//
//  Real-time clock (RTC) routines.
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
#include "rtc_persistent.h"

extern CSP_RTC_REGS *pRtcReg;

//------------------------------------------------------------------------------
// Defines

#define ORIGINYEAR       2000                  // the beginning year
#define MAXYEAR          (ORIGINYEAR + 135)    // the maximum year supported by mx233 rtc
#define JAN1WEEKDAY      6                     // Jan 1, 2000 is a Saturday

#define GetDayOfWeek(X) (((X-1)+JAN1WEEKDAY)%7)

#define RTC_DEBUG_MSG       0
#define RTC_DEBUG_LEAP_MSG  0
#define RTC_CLK_FREQ        RTC_CTL_CLK32000HZ

//------------------------------------------------------------------------------
// Global Variables
static const UINT8 monthtable[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const UINT8 monthtable_leap[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static BOOL g_RTCInitialized = FALSE;
static CRITICAL_SECTION g_CritSecRTC;

//------------------------------------------------------------------------------
// Local Functions
BOOL CheckRealTime(LPSYSTEMTIME pTime);
WORD ConvertSystemTimeToDays(LPSYSTEMTIME pTime);
static BOOL IsLeapYear(int Year);

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot.
//  Input buffer contains SYSTEMTIME structure with default time value.
//  If hardware has persistent real time clock it will ignore this value
//  (or all call).
//
BOOL OALIoCtlHalInitRTC(UINT32 code,VOID *pInpBuffer,UINT32 inpSize,
                        VOID *pOutBuffer, UINT32 outSize,UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    SYSTEMTIME *lpst = (SYSTEMTIME*)pInpBuffer;
    SYSTEMTIME lpstget = *((SYSTEMTIME*)pInpBuffer);

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

    if (pOutSize) {
        *pOutSize = 0;
    }

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

    if (!g_RTCInitialized)
    {
        InitializeCriticalSection(&g_CritSecRTC);
        g_RTCInitialized = TRUE;
    }

    OEMGetRealTime(&lpstget);
    if(lpstget.wYear < 2006)
        rc = OEMSetRealTime(lpst);
    
cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
// PRIVATE FUNCTIONS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function:  CheckRealTime
//
//  This function is used to check if the input time is valid.
//
//  Parameters:
//      lpst
//          [in] Long pointer to the buffer containing the time to be
//          checked in SYSTEMTIME format.
//
//  Returns:
//      TRUE - If time is valid.
//      FALSE - If time is invalid.
//
//------------------------------------------------------------------------------
BOOL CheckRealTime(LPSYSTEMTIME lpst)
{
    BOOL isleap;
    UINT8 *month_tab;

    if (lpst == NULL)
        return FALSE;

    isleap = IsLeapYear(lpst->wYear);
    month_tab = (UINT8 *)(isleap ? monthtable_leap : monthtable);

    if ((lpst->wYear < ORIGINYEAR) || (lpst->wYear > MAXYEAR))
        return FALSE;

    if ((lpst->wMonth < 1) ||(lpst->wMonth > 12))
        return FALSE;

    if((lpst->wDay < 1) ||(lpst->wDay > month_tab[lpst->wMonth-1]))
        return FALSE;

    if ((lpst->wHour > 23) ||(lpst->wMinute > 59) ||(lpst->wSecond > 59))
        return FALSE;

    return TRUE;
}
//------------------------------------------------------------------------------
//
// Function: CalculateSeconds
//
// Local helper function that calculates the number of seconds in lpTime since
// the beginning of the day.
//
// Parameters:
//      lpTime - pointer to the SYSTEMTIME structure that is to be converted
//
// Returns:
//      seconds
//
//------------------------------------------------------------------------------
UINT32 CalculateSeconds(LPSYSTEMTIME lpTime)
{
    return ((UINT32)(lpTime->wHour) * 60 * 60 +
            (UINT32)(lpTime->wMinute) * 60    +
            (UINT32)(lpTime->wSecond));
}
//------------------------------------------------------------------------------
//
// Function: CalculateDays
//
// Local helper function to calculate the total number of days in the lpTime
// structure since Jan 1, ORIGINYEAR. Note that the contents of the lpTime
// structure must have already been checked for validity by calling the
// CheckRealTime() function.
//
// Parameters:
//      lpTime - pointer to the SYSTEMTIME structure that is to be converted
//
// Returns:
//      days - total days since Jan 1, ORIGINYEAR
//
//------------------------------------------------------------------------------
UINT32 CalculateDays(LPSYSTEMTIME lpTime)
{
    UINT8 *month_tab;
    int days, year, month;
    int i;

    days  = lpTime->wDay;
    month = lpTime->wMonth;
    year  = lpTime->wYear;

    // Calculate number of days spent so far from beginning of this year
    month_tab = (UINT8 *)(IsLeapYear(year) ? monthtable_leap : monthtable);

    for (i = 0; i < month - 1; i++)
    {
        days += month_tab[i];
    }

    // calculate the number of days in the previous years
    for (i = ORIGINYEAR; i < year; i++)
    {
        days += (IsLeapYear(i) ? 366 : 365);
    }

    return days;
}
//------------------------------------------------------------------------------
//
// Function: ConvertDays
//
// Local helper function that converts the total days since Jan 1, ORIGINYEAR
// into the equivalent year, month and day values that are suitable for filling
// in the SYSTEMTIME structure.
//
// Parameters:
//      days   - the number of days to be converted (starts from 1)
//      lpTime - pointer to the SYSTEMTIME structure to be filled in
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
static BOOL ConvertDays(UINT32 days, LPSYSTEMTIME lpTime)
{
    int dayofweek, month, year;
    UINT daysInAYear;
    UINT8 *month_tab;

    //Calculate current day of the week
    dayofweek = GetDayOfWeek(days);

    year = ORIGINYEAR;

    // Keep looping here as long as we have enough days left to add at
    // least one more year.
    daysInAYear = IsLeapYear(year) ? 366 : 365;
    while (days > daysInAYear)
    {
        // Yes, we have enough days to add one full year.
        days -= daysInAYear;
        year++;

        // Check again if we have enough days still left for another year.
        daysInAYear = IsLeapYear(year) ? 366 : 365;
    }

    // We now have the correct year plus possibly some leftover days. So we
    // need to check again to see if we are truly in a leap year in order to
    // use the correct month table.
    month_tab = (UINT8 *)((IsLeapYear(year)) ? monthtable_leap : monthtable);

    // Next, find out which day of the month we actually have.
    for (month = 0; month < 12; month++)
    {
        if (days <= month_tab[month])
        {
            break;
        }

        days -= month_tab[month];
    }

    // We now also have the correct day within a specific month. But we
    // must make sure that month=1 for January (and not zero).
    month++;

    // Finally, fill in the lpTime structure with the appropriate values.
    lpTime->wDay       = (WORD)days;
    lpTime->wDayOfWeek = (WORD)dayofweek;
    lpTime->wMonth     = (WORD)month;
    lpTime->wYear      = (WORD)year;

    return TRUE;
}
//------------------------------------------------------------------------------
//
//  Function:  IsLeapYear
//
//  This function determines if the year input is a leap year.
//
//  Parameters:
//      Year
//          [in] Year to be checked.
//
//  Returns:
//      1 - If Year is a leap year
//      0 - If Year is not a leap year
//
//------------------------------------------------------------------------------
static int IsLeapYear(int Year)
{
    int Leap;

    Leap = 0;
    if ((Year % 4) == 0) {
        Leap = 1;
        if ((Year % 100) == 0) {
            Leap = (Year%400) ? 0 : 1;
        }
    }

    return (Leap);
}
//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  Reads the current RTC value and returns a system time.
//
//  Parameters:
//      pTime
//         [out] pointer to the time construct in which the current time is returned
//
//  Returns:
//      TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL OEMGetRealTime(SYSTEMTIME *pTime)
{
    BOOL rc = FALSE;
    UINT32 seconds,TempMinute,TempHour,Tempdays;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OEMGetRealTime\r\n"));

    if (!g_RTCInitialized || (pTime == NULL))
    {
        return rc;
    }

    EnterCriticalSection(&g_CritSecRTC);

    do
    {
        seconds =INREG32(&pRtcReg->SECONDS);
    } while (seconds != INREG32(&pRtcReg->SECONDS));

    LeaveCriticalSection(&g_CritSecRTC);

    // Calcuate the seconds
    pTime->wSecond       = (WORD)(seconds % 60);

    // convert to minutes
    TempMinute                       = seconds/60;
    pTime->wMinute       = (WORD)(TempMinute%60);

    // convert to Hours
    TempHour                         = TempMinute/60;
    pTime->wHour             = (WORD)(TempHour%24);
    pTime->wMilliseconds = 0;

    // convert to days
    Tempdays = TempHour/24;

    // Convert the count of days to the equivalent year, month and day-of-month
    // values.

    if (!ConvertDays(Tempdays, pTime))
    {
        OALMSG(OAL_ERROR, (L" ERROR [0.1]OEMGetRealTime\r\n"));
        return rc;
    }

    OALMSG(OAL_RTC&&OAL_FUNC, (L"OEMGetRealTime(%d/%d/%d %d:%d:%d.%d)\r\n",
                               pTime->wYear,
                               pTime->wMonth,
                               pTime->wDay,
                               pTime->wHour,
                               pTime->wMinute,
                               pTime->wSecond,
                               pTime->wMilliseconds));
    
    rc = TRUE;
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  Updates the RTC with the specified system time.
//
//  Parameters:
//      pTime
//          [in] pointer to the time construct to set the current time
//
//  Returns:
//      TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL OEMSetRealTime(LPSYSTEMTIME pTime)
{
    BOOL rc = FALSE;
    UINT32 days, seconds,Totalseconds;

    if (!(g_RTCInitialized && CheckRealTime(pTime)))
    {
        return rc;
    }

    // calculate time of day in seconds.
    seconds = CalculateSeconds(pTime);

    // Calculate days.
    days = CalculateDays(pTime);

    // convert days to seconds
    Totalseconds = (days * 86400) + seconds;

    OALMSG(OAL_RTC&&OAL_FUNC, (TEXT(" ++RTC SetTime: Date = %02d/%02d/%04d ")
                               TEXT("%02d:%02d:%02d (DD/MM/YYYY HH:MM:SS)\r\n"),
                               pTime->wDay, pTime->wMonth, pTime->wYear,
                               pTime->wHour, pTime->wMinute, pTime->wSecond));

    OALMSG(OAL_RTC&&OAL_FUNC, (TEXT("       Converted = %d days, %d seconds, %d TotalSeconds\r\n"),
                               days, seconds,Totalseconds));

    EnterCriticalSection(&g_CritSecRTC);

    // Begin a write to a shadow register
    while((INREG32(&pRtcReg->STAT) & BM_RTC_STAT_NEW_REGS)) ;

    // write
    OUTREG32(&pRtcReg->SECONDS,Totalseconds);

    // end shadow write
    while((INREG32(&pRtcReg->STAT) & BM_RTC_STAT_NEW_REGS)) ;
    OUTREG32(&pRtcReg->CTRL[SETREG],BM_RTC_CTRL_FORCE_UPDATE);
    OUTREG32(&pRtcReg->CTRL[CLRREG],BM_RTC_CTRL_FORCE_UPDATE);
    while((INREG32(&pRtcReg->STAT) & BM_RTC_STAT_STALE_REGS)) ;

    LeaveCriticalSection(&g_CritSecRTC);

    rc = TRUE;
    return rc;
}

//------------------------------------------------------------------------------
