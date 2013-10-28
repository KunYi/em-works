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

//------------------------------------------------------------------------------
// Defines

#define GetDayOfWeek(X) (((X-1)+JAN1WEEK)%7)
#define RTC_DEBUG_MSG       0
#define RTC_DEBUG_LEAP_MSG  0
#define RTC_CLK_FREQ        RTC_CTL_CLK32768HZ

//--------------------------------------------------------------------------------
// Extern Variables
extern BOOL g_oalPostInit;

//------------------------------------------------------------------------------
// Global Variables
static const UINT8 monthtable[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const UINT8 monthtable_leap[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Protect RTC registers with a critical section to avoid simultaneous
// get and set on the RTC clock
CRITICAL_SECTION g_csRTC;
//Critical section Initialization status flag
BOOL g_bcsRTCInitStatus = FALSE;

SYSTEMTIME default_time = {2003,1,3,1,0,0,0,0};  //Jan 1, 2003, Wednesday

//------------------------------------------------------------------------------
// Local Functions
BOOL CheckRealTime(LPSYSTEMTIME pTime);
WORD ConvertSystemTimeToDays(LPSYSTEMTIME pTime);
static BOOL IsLeapYear(int Year);


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
//-----------------------------------------------------------------------------
BOOL OEMGetRealTime(SYSTEMTIME *pTime)
{
    BOOL rc = FALSE;
    WORD ms, secs[2], mins[2], hours[2], days[2], dayofweek, month, year;
    BOOL isleap;
    UINT8 *month_tab;
    pRTCRegisters_t pRtc;
    int numOfLeap=0;
    int i = 0;
    UINT32 sCnt, hmCnt, dCnt;

    OALMSG(OAL_IOCTL && OAL_FUNC, (L"+OEMGetRealTime(pTime = 0x%x)\r\n", pTime));

    if (pTime == NULL) goto cleanUp;

    // Get uncached virtual addresses for RTC
    pRtc = (pRTCRegisters_t) OALPAtoUA(CSP_BASE_REG_PA_RTC);
    if (pRtc == NULL)
    {
        OALMSG(OAL_ERROR, (L"OEMGetRealTime:  RTC null pointer!\r\n"));
        goto cleanUp;
    }

    //The value of millisecond is set to 0
    ms = 0;

    // Do not enter if critical section is not initialized
    // as this may get called before OALIoCtlHalPostInit
    if (g_oalPostInit)
    {
        if (g_bcsRTCInitStatus == FALSE)
        {
                InitializeCriticalSection(&g_csRTC);
                g_bcsRTCInitStatus = TRUE;
        }

        EnterCriticalSection(&g_csRTC);
    }

    for (i =0; i<2; i++)
    {
        sCnt = INREG32(&pRtc->RTCSecCnt);
        hmCnt = INREG32(&pRtc->RTCHMCnt);
        dCnt = INREG32(&pRtc->RTCDayCnt);

        secs[i] = (WORD)(sCnt & RTC_SECOND_MASK);

        //Get value of minute from RTC register
        mins[i] = (WORD)(hmCnt & RTC_MINUTE_MASK);

        //Get value of hour from RTC register
        hours[i] = (WORD)(hmCnt & RTC_HOUR_MASK) >> RTC_HOUR_OFFSET;

        //Get value of day from RTC register
        days[i] = (WORD)(dCnt & RTC_DAY_MASK);
    }

    if ( (secs[0] != secs[1] ) ||
         (mins[0] != mins[1] ) ||
         (hours[0] != hours[1] ) ||
         (days[0] != days[1] ) )
    {
        sCnt = INREG32(&pRtc->RTCSecCnt);
        hmCnt = INREG32(&pRtc->RTCHMCnt);
        dCnt = INREG32(&pRtc->RTCDayCnt);

        secs[0] = (WORD)(sCnt & RTC_SECOND_MASK);

        //Get value of minute from RTC register
        mins[0] = (WORD)(hmCnt & RTC_MINUTE_MASK);

        //Get value of hour from RTC register
        hours[0] = (WORD)(hmCnt & RTC_HOUR_MASK) >> RTC_HOUR_OFFSET;

        //Get value of day from RTC register
        days[0] = (WORD)(dCnt & RTC_DAY_MASK);

    }

    //Calculate current day of the week
    dayofweek = GetDayOfWeek(days[0]);

    //Calculate current year, month, and day use the value stored in RTC day counter register
    OALMSG(OAL_RTC&&OAL_INFO, (TEXT("RTCDayCnt=%d\r\n"),days[0]));
    year = ORIGINYEAR;
    while (days[0] > 365)
    {
        if (IsLeapYear(year))
        {
            numOfLeap++;
            if (days[0] > 366)
            {
                OALMSG(OAL_RTC&&OAL_INFO, (TEXT("Leap Year:    %u"),year));
                days[0] -= 366;
                year += 1;
                OALMSG(OAL_RTC&&OAL_INFO, (TEXT(", Days left: %u\r\n"),days[0]));
            }
            else
            {
                break;
            }
        }
        else
        {
            OALMSG(OAL_RTC&&OAL_INFO, (TEXT("Not Leap Year: %u"),year));
            days[0] -= 365;
            year += 1;
            OALMSG(OAL_RTC&&OAL_INFO, (TEXT(", Days left: %u\r\n"),days[0]));
        }
    }

    OALMSG(OAL_RTC&&OAL_INFO, (TEXT("Number of leap years before this year(%u)\r\n"),numOfLeap));

    // Determine whether it is a leap year
    isleap = IsLeapYear(year);
    month_tab = (UINT8 *)((isleap)? monthtable_leap : monthtable);

    for (month=0; month<12; month++)
    {
        if (days[0] <= month_tab[month])
            break;
        days[0] = days[0] - month_tab[month];
    }
    month +=1;


    //Save all the value to passed in pointer
    pTime->wMilliseconds = ms;
    pTime->wSecond = secs[0];
    pTime->wMinute = mins[0];
    pTime->wHour = hours[0];
    pTime->wDay = days[0];
    pTime->wDayOfWeek = dayofweek;
    pTime->wMonth = month;
    pTime->wYear = year;

    // Done
    rc = TRUE;

cleanUp:
    // Do not enter if critical section is not initialized
    // as this may get called before OALIoCtlHalPostInit
    if (g_oalPostInit)
    {
        LeaveCriticalSection(&g_csRTC);
    }
    OALMSG(OAL_IOCTL && OAL_FUNC, (L"-OEMGetRealTime(rc = %d)\r\n", rc));
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
//-----------------------------------------------------------------------------
BOOL OEMSetRealTime(LPSYSTEMTIME pTime)
{
    BOOL rc = FALSE;
    WORD sec, min, hour, day;
    pRTCRegisters_t pRtc;

    OALMSG(OAL_IOCTL && OAL_FUNC, ( L"+OEMSetRealTime(%d/%d/%d %d:%d:%d.%03d)\r\n",
        pTime->wYear, pTime->wMonth, pTime->wDay, pTime->wHour, pTime->wMinute,
        pTime->wSecond, pTime->wMilliseconds
    ));

    // Do not enter if critical section is not initialized
    // as this may get called before OALIoCtlHalPostInit
    if (g_oalPostInit)
    {
        if (g_bcsRTCInitStatus == FALSE)
        {
                InitializeCriticalSection(&g_csRTC);
                g_bcsRTCInitStatus = TRUE;
        }

        EnterCriticalSection(&g_csRTC);
    }

    if (pTime == NULL) goto cleanUp;

    // Check if the input time is valid or not
    if (CheckRealTime(pTime) == FALSE)
    {
        OALMSG(OAL_ERROR, (TEXT("Time to be set is INVALID!!!:  %u/%u/%u, %u:%u:%u  Day of week:%u\r\n"),
               pTime->wYear, pTime->wMonth , pTime->wDay, pTime->wHour, pTime->wMinute, pTime->wSecond,pTime->wDayOfWeek));
        goto cleanUp;
    }else
        OALMSG(OAL_RTC&&OAL_INFO, (TEXT("Time to be set is valid:  %u/%u/%u, %u:%u:%u  Day of week:%u\r\n"),
               pTime->wYear, pTime->wMonth , pTime->wDay, pTime->wHour, pTime->wMinute, pTime->wSecond,pTime->wDayOfWeek));

    sec = pTime->wSecond;
    min = pTime->wMinute;
    hour = pTime->wHour;
    day = ConvertSystemTimeToDays(pTime);

    // Get uncached virtual addresses for RTC
    pRtc = (pRTCRegisters_t) OALPAtoUA(CSP_BASE_REG_PA_RTC);
    if (pRtc == NULL)
    {
        OALMSG(OAL_ERROR, (L"OEMSetRealTime:  RTC null pointer!\r\n"));
        goto cleanUp;
    }

    //Set seconds in RTC register
    OUTREG32(&pRtc->RTCSecCnt, sec);

    //Set hours and minutes in RTC register
    OUTREG32(&pRtc->RTCHMCnt, (hour << RTC_HOUR_OFFSET) | min);

    //Set day in RTC register
    OUTREG32(&pRtc->RTCDayCnt, day);

    // Done
    rc = TRUE;

cleanUp:
    // Do not enter if critical section is not initialized
    // as this may get called before OALIoCtlHalPostInit
    if (g_oalPostInit)
    {
        LeaveCriticalSection(&g_csRTC);
    }
    OALMSG(OAL_IOCTL && OAL_FUNC, (L"-OEMSetRealTime(rc = %d)\r\n", rc));
    return rc;
}


/*********************************************************************
 *
 *  FUNCTION:       InitRTC
 *
 *  DESCRIPTION:    This function is used to initialize the real time clock.
 *
 *  PARAMETERS:
 *                  None
 *
 *  RETURNS:
 *                  None
 *
  ********************************************************************/
void InitRTC(void)
{
    pRTCRegisters_t pRtc;

    // Get uncached virtual addresses for RTC
    pRtc = (pRTCRegisters_t) OALPAtoUA(CSP_BASE_REG_PA_RTC);
    if (pRtc == NULL)
    {
        OALMSG(OAL_ERROR, (L"InitRTC:  RTC null pointer!\r\n"));

        return;
    }

    OUTREG32(&pRtc->RTCControl, RTC_CTL_SW_RESET);

    //Set date and time
    if  (!OEMSetRealTime(&default_time))
    {
        DEBUGMSG(RTC_DEBUG_MSG, (TEXT("ERROR: rtcInit failed\r\n")));

        OUTREG32(&pRtc->RTCControl, RTC_CTL_SW_RESET);

        return;
    }

    //Set the Control register and Enable RTC
    OUTREG32(&pRtc->RTCControl, (RTC_CLK_FREQ | RTC_CTL_RTC_EN));

    return;
}


/******************************************************************************
 * PRIVATE FUNCTIONS
 *****************************************************************************/

/*********************************************************************
 *
 *  FUNCTION:       CheckRealTime
 *
 *  DESCRIPTION:    This function is used to check if the input
 *                  time is valid.
 *
 *  PARAMETERS:
 *                  lpst -
 *                      Long pointer to the buffer containing
 *                      the time to be checked in SYSTEMTIME format.
 *
 *  RETURNS:
 *                  TRUE - If time is valid.
 *
 *                  FALSE - If time is invalid.
 *
 *********************************************************************/
BOOL CheckRealTime(LPSYSTEMTIME lpst)
{
    BOOL isleap;
    UINT8 *month_tab;

    isleap = IsLeapYear(lpst->wYear);
    month_tab = (UINT8 *)(isleap? monthtable_leap : monthtable);

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


/*********************************************************************
 *
 *  FUNCTION:       ConvertSystemTimeToDays
 *
 *  DESCRIPTION:    This function is used to convert the year, month
 *                  and days of SYSTEMTIME to number of days which
 *                  is compatible with the RTC.
 *
 *  PARAMETERS:
 *                  lpst -
 *                      Long pointer to the buffer containing
 *                      the time to be checked in SYSTEMTIME format.
 *
 *  RETURNS:
 *                  Number of days from January 1, ORIGINYEAR to date specified
 *                  in input parameter.
 *
 *********************************************************************/
WORD ConvertSystemTimeToDays(LPSYSTEMTIME lpst)
{
    WORD day, month, year;
    BOOL isleap;
    UINT8 *month_tab;
    int i, numOfLeap=0;

    day = lpst->wDay;
    month = lpst->wMonth;
    year = lpst->wYear;

    //Calculate whole number of day    from orginal year
    isleap = IsLeapYear(year);

    month_tab = (UINT8 *)(isleap? monthtable_leap : monthtable);

    for (i=0; i<month-1; i++)
    {
        day = day + month_tab[i];
    }

    for (i=ORIGINYEAR; i<year; i++)
    {
        if (IsLeapYear(i))
        {
            day += 366;
            numOfLeap++;
        }
        else
            day += 365;
    }

    DEBUGMSG(RTC_DEBUG_LEAP_MSG, (TEXT("Total days from year %u: %u  Number of Leap years: %u\r\n "),year, day, numOfLeap));

    return day;
}


/*********************************************************************
 *
 *  FUNCTION:       IsLeapYear
 *
 *  DESCRIPTION:    This function determines if the year input is a
 *                  leap year.
 *
 *  PARAMETERS:
 *                  Year -
 *                      Year to be checked
 *
 *  RETURNS:
 *                  1 - If Year is a leap year
 *                  0 - If Year is not a leap year
 *
 *********************************************************************/
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
