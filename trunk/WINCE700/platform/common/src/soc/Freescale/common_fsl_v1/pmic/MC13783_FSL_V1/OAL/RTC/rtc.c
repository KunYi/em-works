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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Module: rtc.c
//
//  PQOAL Real-time clock (RTC) routines for the MC13783 PMIC RTC.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4214)
#include <windows.h>
#include <oal.h>
#include "nkintr.h"
#pragma warning(pop)

#include "regs.h"
#include "regs_rtc.h"

#include "pmic_ioctl.h"
#include "regs_regulator.h"
#include "pmic_basic_types.h"
#include "pmic_lla.h"


//-----------------------------------------------------------------------------
// External Functions

extern BOOL OALPmicRead(UINT32 addr, PUINT32 pData);
extern BOOL OALPmicWrite(UINT32 addr, UINT32 data);
extern BOOL OALPmicWriteMasked(UINT32 addr, UINT32 mask, UINT32 data);
extern BOOL OALIoCtlHalUnforceIrq(UINT32 code, VOID *pInpBuffer,
                                  UINT32 inpSize, VOID *pOutBuffer,
                                  UINT32 outSize, UINT32 *pOutSize);

//-----------------------------------------------------------------------------
// Global Variables

extern UINT32 g_IRQ_RTC;

//------------------------------------------------------------------------------
// Global Variables

// These macro define some default RTC information:
//
//     ORIGINYEAR    This is the year that we chose as our device's "beginning
//                   of time". The value is somewhat arbitrary but it should
//                   encompass the expected operating lifetime of the device
//                   in conjunction with the MAXYEAR value.
//     MAXYEAR       This is the "end of time" for our device. In the case of
//                   the MC13783 PMIC, the days counter is implemented as a
//                   15-bit counter. Therefore, the maximum number of days is
//                   32768 which translates into only about 88 years. However,
//                   this means that we will fail the CETK Real-Time Functions
//                   test 1260 since that test requires a minimum interval of
//                   100 years between ORIGINYEAR and MAXYEAR. But this test
//                   failure is simply due to a hardware limitation and not a
//                   software or device driver problem.
//
#define ORIGINYEAR       2000                  // the beginning year
#define MAXYEAR          (ORIGINYEAR + 88)     // the maximum year

#define JAN1WEEKDAY      6                     // Jan 1, 2000 is a Saturday
#define GetDayOfWeek(X) (((X-1)+JAN1WEEKDAY)%7)// Sunday = 0, Monday = 1, etc.

#define TYPE_TIME        0
#define TYPE_ALRM        1

static const UINT8 monthtable[12] = { 31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31
                                    };
static const UINT8 monthtable_leap[12] = { 31, 29, 31, 30, 31, 30,
                                           31, 31, 30, 31, 30, 31
                                         };

static BOOL g_RTCInitialized = FALSE;
static CRITICAL_SECTION g_CritSecRTC; // Used to ensure that SetTime() and
                                      // GetTime() are both reentrant.

static BOOL InitializeRTC(LPSYSTEMTIME pTime);
BOOL OEMSetRealTime(LPSYSTEMTIME lpst);

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot.
//  Input buffer contains SYSTEMTIME structure with default time value. If
//  hardware has persistent real time clock it will ignore this value
//  (or all call).
//
//------------------------------------------------------------------------------
BOOL OALIoCtlHalInitRTC( UINT32 code, VOID *pInpBuffer, UINT32 inpSize,
                         VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    LPSYSTEMTIME pTime = (LPSYSTEMTIME)pInpBuffer;

    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_IOCTL && OAL_FUNC, (L"+OALIoCtlHalInitRTC(...)\r\n"));

    // Validate inputs
    if (pInpBuffer == NULL || inpSize < sizeof(SYSTEMTIME)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIoCtlHalInitRTC: Invalid parameter\r\n"
        ));
        goto cleanUp;
    }

    if (!g_RTCInitialized)
    {
        InitializeRTC(pTime);
    }

cleanUp:
    OALMSG(OAL_IOCTL && OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: InitializeRTC
//
// Local helper function to reinitialize the RTC registers if required.
//
// Parameters:
//     pTime - current system time
//
// Returns:
//     TRUE  - RTC successfully initialized
//     FALSE - RTC initialization failed
//
//------------------------------------------------------------------------------
static BOOL InitializeRTC(LPSYSTEMTIME pTime)
{
    BOOL rc = TRUE;
    UINT32 tempISR1;

    InitializeCriticalSection(&g_CritSecRTC);

    EnterCriticalSection(&g_CritSecRTC);

    // Add static mapping for RTC alarm
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM, g_IRQ_RTC);

    OALPmicRead(MC13783_INT_STAT1_ADDR, &tempISR1);

    // Check if we need to reinitialize the timer. Otherwise, don't
    // do anything since the RTC is still valid.
    //
    // Note that the RTC RESET (RTCRSTI) interrupt will be asserted
    // following an RTC reset and we must then reinitialize the RTC
    // registers and clear the interrupt.
    //
    if ( tempISR1 & MC13783_RTCRSTI_MASK )
    {
        // We must force g_RTCInitialized to be TRUE here so that the call to
        // OEMSetRealTime() will not just return a failure due to the driver
        // state checking that is done there.
        g_RTCInitialized = TRUE;

        // Set the RTC time to match the current system time.
        rc = OEMSetRealTime(pTime);

        if (rc)
        {
            // Clear the status bit now that the RTC has been initialized.
            OALPmicWriteMasked(MC13783_INT_STAT1_ADDR,MC13783_RTCRSTI_MASK,
                               tempISR1);
        }
    }
    else
    {
        OALMSG(OAL_RTC, (TEXT("InitializeRTC: The MC13783 RTCRSTI is not ")
                         TEXT("asserted, no RTC updates required\r\n")));
    }

    // We can now set the true value of the global flag to indicate
    // whether or not we were successful in initializing the RTC hardware.
    g_RTCInitialized = rc;

    // The RTC supports alarms with 1 second resolution.
    g_pOemGlobal->dwAlarmResolution = 1000;

    LeaveCriticalSection(&g_CritSecRTC);

    return rc;
}


//------------------------------------------------------------------------------
//
// Function: IsLeapYear
//
// Local helper function checks if the year is a leap year
//
// Parameters:
//     Year - the year to be checked
//
// Returns:
//     TRUE if the given year is a leap year
//     FALSE otherwise
//
//------------------------------------------------------------------------------
static BOOL IsLeapYear(int Year)
{
    BOOL bIsLeap = FALSE;

    if ((Year % 4) == 0)
    {
        // Leap years are all evenly divisible by 4, except ...
        bIsLeap = TRUE;

        if ((Year % 100) == 0)
        {
            // Centuries are not leap years unless they are also evenly
            // divisible by 400.
            bIsLeap = ((Year % 400) == 0);
        }
    }

    return (bIsLeap);
}

//------------------------------------------------------------------------------
//
//  FUNCTION:       CheckRealTime
//
//  DESCRIPTION:    Helper function is used to check if the input
//                  time is valid.
//
//  PARAMETERS:
//                  lpst -
//                      Long pointer to the buffer containing
//                      the time to be checked in SYSTEMTIME format.
//
//  RETURNS:
//                  TRUE - If time is valid.
//
//                  FALSE - If time is invalid.
//
//------------------------------------------------------------------------------
static BOOL CheckRealTime(LPSYSTEMTIME lpst)
{
    BOOL isleap;
    UINT8 *month_tab;
    BOOL  rc = TRUE;

    if (lpst == NULL)
        return FALSE;

    isleap = IsLeapYear(lpst->wYear);
    month_tab = (UINT8 *)(isleap ? monthtable_leap : monthtable);

    if ((lpst->wYear < ORIGINYEAR) || (lpst->wYear > MAXYEAR))
        return FALSE;

    if ((lpst->wMonth < 1) || (lpst->wMonth > 12))
        return FALSE;

    if((lpst->wDay < 1) || (lpst->wDay > month_tab[lpst->wMonth-1]))
        return FALSE;

    if ((lpst->wHour > 23) || (lpst->wMinute > 59) || (lpst->wSecond > 59))
         return FALSE;

    if ((lpst->wHour < 0) || (lpst->wMinute < 0) || (lpst->wSecond < 0))
         return FALSE;

    return rc;
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
static UINT32 CalculateDays(LPSYSTEMTIME lpTime)
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
static UINT32 CalculateSeconds(LPSYSTEMTIME lpTime)
{
    return ((UINT32)(lpTime->wHour) * 60 * 60 +
            (UINT32)(lpTime->wMinute) * 60    +
            (UINT32)(lpTime->wSecond));
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
    int   dayofweek, month, year;
    UINT  daysInAYear;
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
// Function: ConvertSeconds
//
// Local helper function that converts time of day in seconds to hour,
// minute and seconds.
//
// Parameters:
//      seconds - the number of seconds to be converted (starts from 0)
//      lpTime  - pointer to the SYSTEMTIME structure to be filled in
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
static BOOL ConvertSeconds(UINT32 seconds, LPSYSTEMTIME lpTime)
{
    int minutes = 0, hours = 0;

    // The number of seconds should be less than 24 hours (86400 seconds).
    if(seconds < 86400)
    {
        if (seconds >= 60)
        {
            minutes = (int) seconds / 60;
            seconds -= (minutes * 60);
            if (minutes >= 60)
            {
                hours = (int) minutes / 60;
                minutes -= (hours * 60);
            }
        }
    }
    else
    {
        ERRORMSG(TRUE, (_T("TOD in sec is wrong(seconds > 86399) %d"),
                        seconds));
        return FALSE;
    }

    lpTime->wMilliseconds = 0;
    lpTime->wHour         = (WORD)hours;
    lpTime->wMinute       = (WORD)minutes;
    lpTime->wSecond       = (WORD)seconds;

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: SetTime
//
// This function sets the given time & day into the register pair indicated
// by type.
//
// Parameters:
//     type   - either TYPE_TIME or TYPE_ALRM
//     lpTime - pointer to the SYSTEMTIME structure with the desired RTC
//              settings
//
// Returns:
//     Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
static BOOL SetTime(UINT32 type, LPSYSTEMTIME lpTime)
{
    UINT32 days, seconds, secondsCheck;
    UINT32 addr_RTC_TOD = (type == TYPE_TIME) ? MC13783_RTC_TM_ADDR :
                                                MC13783_RTC_ALM_ADDR;
    UINT32 addr_RTC_DAY = (type == TYPE_TIME) ? MC13783_RTC_DAY_ADDR :
                                                MC13783_RTC_DAY_ALM_ADDR;

    // We cannot set the time to anything that is outside our supported
    // time range.
    if (!(g_RTCInitialized && CheckRealTime(lpTime)))
    {
        return FALSE;
    }

    // calculate time of day in seconds.
    seconds = CalculateSeconds(lpTime);
    // Calculate days.
    days = CalculateDays(lpTime);

    OALMSG(OAL_RTC, (TEXT("RTC SetTime: Date = %02d/%02d/%04d ")
                     TEXT("%02d:%02d:%02d (DD/MM/YYYY HH:MM:SS)\r\n"),
                     lpTime->wDay, lpTime->wMonth, lpTime->wYear,
                     lpTime->wHour, lpTime->wMinute, lpTime->wSecond));
    OALMSG(OAL_RTC, (TEXT("        Converted = %d days, %d seconds\r\n"),
                     days, seconds));

    EnterCriticalSection(&g_CritSecRTC);

    do
    {
        // Write the count of seconds to the 17-bit TimeOftheDay (TOD) register.
        OALPmicWrite(addr_RTC_TOD, seconds);
        
        // Write the count of days to the 15-bit DAY register. We decrement
        // the days value by one here because the RTC register counts days
        // from 0 - 32767.
        OALPmicWrite(addr_RTC_DAY, days - 1);

        // As documented in the MC13783 Detailed Technical Specifications
        // document, we must now read back the RTC values to make sure that
        // we didn't possibly experience a day counter rollover glitch during
        // the previous write operations.
        OALPmicRead(addr_RTC_TOD, &secondsCheck);

        // Repeat the RTC write operation if we possibly experienced a glitch.
        // Note that a glitch is only possible if the seconds count was
        // incremented in between the two register write operations above.
        //
        // This glitch can occur if the TOD register just happens to roll over
        // in between the first two RTC write statements. So instead of getting
        // the correct day = 5, seconds = 86399, we would actually get day = 5,
        // seconds = 0 which would put us about a day behind.
        //
        // Note that the TOD counter is driven by a 32 KHz clock that is
        // independent of the SPI clock. The two clocks are not synchronized
        // in hardware so we must handle any possible glitches in software.

    } while (secondsCheck != seconds);

    LeaveCriticalSection(&g_CritSecRTC);

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GetTime
//
// This function gets the time and day from the register pair indicated by type.
//
// Parameters:
//     type   - either TYPE_TIME or TYPE_ALRM
//     lpTime - pointer to the SYSTEMTIME structure that is to be filled in
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
static BOOL GetTime(UINT32 type, LPSYSTEMTIME lpTime)
{
    UINT32 days, seconds, secondsCheck;
    UINT32 addr_RTC_TOD = (type == TYPE_TIME) ? MC13783_RTC_TM_ADDR :
                                                MC13783_RTC_ALM_ADDR;
    UINT32 addr_RTC_DAY = (type == TYPE_TIME) ? MC13783_RTC_DAY_ADDR :
                                                MC13783_RTC_DAY_ALM_ADDR;

    if (!g_RTCInitialized || (lpTime == NULL))
    {
        return FALSE;
    }

    EnterCriticalSection(&g_CritSecRTC);

    do
    {
        OALPmicRead(addr_RTC_TOD, &seconds);
        OALPmicRead(addr_RTC_DAY, &days);

        // As documented in the MC13783 Detailed Technical Specifications
        // document, we must now reread the RTC values to make sure that
        // we didn't possibly experience a day counter rollover glitch during
        // the previous read operations.
        OALPmicRead(addr_RTC_TOD, &secondsCheck);

        // Repeat the RTC read operation if we possibly experienced a glitch.
        // Note that a glitch is only possible if the seconds count was
        // incremented in between the two register read operations above.
        //
        // This glitch can occur if the TOD register just happens to roll over
        // in between the first two RTC read statements. So instead of getting
        // the correct day = 5, seconds = 0, we would actually get day = 5,
        // seconds = 86399 which would put us about a day ahead of the actual
        // time.
        //
        // Note that the TOD counter is driven by a 32 KHz clock that is
        // independent of the SPI clock. The two clocks are not synchronized
        // in hardware so we must handle any possible glitches in software.

    } while (secondsCheck != seconds);

    LeaveCriticalSection(&g_CritSecRTC);

    // Convert the count of seconds to the equivalent hours, minutes, and
    // seconds of the day.
    if (!ConvertSeconds(seconds, lpTime))
    {
        return FALSE;
    }

    // Convert the count of days to the equivalent year, month and day-of-month
    // values. We need to increment the "days" value by one here because
    // ConvertDays() starts counting from day 1 whereas the RTC register starts
    // counting from day 0.
    if (!ConvertDays(days + 1, lpTime))
    {
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  This function is called by the kernel to retrieve the time from
//  the real-time clock.
//
//------------------------------------------------------------------------------
BOOL OEMGetRealTime(LPSYSTEMTIME lpst)
{
    return GetTime(TYPE_TIME, lpst);
}


//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  This function is called by the kernel to set the real-time clock.
//
//------------------------------------------------------------------------------
BOOL OEMSetRealTime(LPSYSTEMTIME lpst)
{
    return SetTime(TYPE_TIME, lpst);

}


//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  Set the RTC alarm time.
//
//------------------------------------------------------------------------------
BOOL OEMSetAlarmTime(LPSYSTEMTIME pTime)
{
    BOOL rc = FALSE;
    UINT32 irq;

    OALMSG(OAL_RTC && OAL_FUNC, ( L"+OEMSetAlarmTime(%d/%d/%d %d:%d:%d.%03d)"
                                  L"\r\n",
                                  pTime->wMonth, pTime->wDay, pTime->wYear,
                                  pTime->wHour, pTime->wMinute, pTime->wSecond,
                                  pTime->wMilliseconds ));

    if (pTime == NULL)
        goto cleanUp;

    // The call to InitializeRTC() was added here just to match the sample
    // code that Microsoft has provided for the OMAP2420 RTC. In our case
    // we always expect the RTC to be properly initialized during boot by
    // calling OALIoCtlHalInitRTC(). However, adding this additional bit of
    // code here just ensures that we are consistent with the sample code
    // that Microsoft has provided.
    //
    if (!g_RTCInitialized && !InitializeRTC(pTime))
        goto cleanUp;

    // Set seconds, minutes, hours and day in RTC day alarm register.
    if (!SetTime(TYPE_ALRM, pTime))
        goto cleanUp;

    // Enable alarm IRQ.
    OALMSG(OAL_RTC && OAL_INFO, (TEXT("RTC Alarm Interrupt enabled.\r\n")));

    // Enable/clear RTC interrupt
    irq = g_IRQ_RTC;
    OALIoCtlHalUnforceIrq(0, &irq, sizeof(irq), NULL, 0, NULL);
    OALIntrDoneIrqs(1, &irq);

    // Done
    rc = TRUE;

cleanUp:
    OALMSG(OAL_RTC && OAL_FUNC, (L"-OEMSetAlarmTime(rc = %d)\r\n", rc));
    return rc;
}
