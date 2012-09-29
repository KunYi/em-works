//------------------------------------------------------------------------------
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  PMICrtc.cpp
//
//  This file contains the PMIC platform  RTC functions that provide control
//  over the freescale MC13892 IC.
//
//-------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)
#include "common_macros.h"
#include "pmic.h"

static HANDLE hIntrEventRtc;
static HANDLE hRtcISTThread;

typedef struct {
     DWORD syncTickCount;
     ULONGLONG syncRTC;

     DWORD syncAlarmTickCount;
     ULONGLONG syncAlarnRTC;

     BOOL IsRTCEvent;
     BOOL IsALARMEvent;   
} RTC_QUERY_OUT;


#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)

#endif  // DEBUG

#undef  ORIGINYEAR
#undef  MAXYEAR
#define ORIGINYEAR           1980                  // the begin year
#define MAXYEAR              (ORIGINYEAR + 100)    // the maxium year

#define JAN1WEEKDAY      6                     // Jan 1, 2000 is a Saturday
#define GetDayOfWeek(X) (((X-1)+JAN1WEEKDAY)%7)// Sunday = 0, Monday = 1, etc.

#define SyncTimeOut          180000
#define PMIC_ALL_BITS        0xFFFFFF

SYSTEMTIME default_time = {2003,1,3,1,0,0,0,0};  //Jan 1, 2003, Wednesday
static const UINT8 monthtable[12] = { 31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31
                                    };
static const UINT8 monthtable_leap[12] = { 31, 29, 31, 30, 31, 30,
                                           31, 31, 30, 31, 30, 31
                                         };


//------------------------------------------------------------------------------
#define BCD2BIN(b)            (((b) >> 4)*10 + ((b)&0xF))
#define BIN2BCD(b)            ((((UINT8)(b)/10) << 4)|((UINT8)(b)%10))
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions

WORD ConvertSystemTimeToDays(LPSYSTEMTIME pTime);
void InitRTC(void);

static DWORD CALLBACK PmicRtcIsrThreadProc(LPVOID lpParameter);
extern "C" DWORD BSPGetRtcSetIrq();
extern "C" VOID SetRegister(UINT8 index, UINT32 reg, UINT32 mask);
extern "C" VOID GetRegister(UINT8 index, UINT32* reg);


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
//  Function:  PMICGetRealTime
//
//  Reads the current PMIC RTC value and returns a system time 
//
//  Parameters:
//      pTime
//         [out] pointer to the time construct in which the current time is returned
//
//  Returns:
//      TRUE if successful.
//-----------------------------------------------------------------------------
BOOL PMICGetRealTime(SYSTEMTIME *pTime)
{
    UINT32 days, seconds, secondsCheck;
    UINT8 addr_RTC_TOD = MC13892_RTC_TM_ADDR;
    UINT8 addr_RTC_DAY = MC13892_RTC_DAY_ADDR; 

    do
    {
        GetRegister(addr_RTC_TOD, &seconds);
        GetRegister(addr_RTC_DAY, &days);

        // As documented in the MC13892 Detailed Technical Specifications
        // document, we must now reread the RTC values to make sure that
        // we didn't possibly experience a day counter rollover glitch during
        // the previous read operations.
        GetRegister(addr_RTC_TOD, &secondsCheck);

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

   
    // Convert the count of seconds to the equivalent hours, minutes, and
    // seconds of the day.
    if (!ConvertSeconds(seconds, pTime))
    {
        return FALSE;
    }

    // Convert the count of days to the equivalent year, month and day-of-month
    // values. We need to increment the "days" value by one here because
    // ConvertDays() starts counting from day 1 whereas the RTC register starts
    // counting from day 0.
    if (!ConvertDays(days + 1, pTime))
    {
        return FALSE;
    }
      DEBUGMSG(ZONE_FUNC, (TEXT("Time to be set is INVALID!!!:  %u/%u/%u, %u:%u:%u  Day of week:%u\r\n"),
           pTime->wYear, pTime->wMonth , pTime->wDay, pTime->wHour, pTime->wMinute, pTime->wSecond,pTime->wDayOfWeek));
    return TRUE;
}



//------------------------------------------------------------------------------
//
//  Function:  PMICSetRealTime
//
//  Updates the PMIC RTC with the specified system time.
//
//  Parameters:
//      pTime
//          [in] pointer to the time construct to set the current time
//
//  Returns:
//      TRUE if successful.
//-----------------------------------------------------------------------------
BOOL PMICSetRealTime(LPSYSTEMTIME pTime)
{
    UINT32 days, seconds, secondsCheck;
    UINT8 addr_RTC_TOD =  MC13892_RTC_TM_ADDR  ;
    UINT8 addr_RTC_DAY =  MC13892_RTC_DAY_ADDR ;

    // We cannot set the time to anything that is outside our supported
    // time range.
   

    // calculate time of day in seconds.
    seconds = CalculateSeconds(pTime);
    // Calculate days.
    days = CalculateDays(pTime);

    DEBUGMSG(ZONE_FUNC, (TEXT("RTC SetTime: Date = %02d/%02d/%04d ")
                     TEXT("%02d:%02d:%02d (DD/MM/YYYY HH:MM:SS)\r\n"),
                     pTime->wDay, pTime->wMonth, pTime->wYear,
                     pTime->wHour, pTime->wMinute, pTime->wSecond));
    DEBUGMSG(ZONE_FUNC, (TEXT("        Converted = %d days, %d seconds\r\n"),
                     days, seconds));

    do
    {
    // Write the count of seconds to the 17-bit TimeOftheDay (TOD) register.
    SetRegister(addr_RTC_TOD, seconds,PMIC_ALL_BITS);

    // Write the count of days to the 15-bit DAY register. We decrement
    // the days value by one here because the RTC register counts days
    // from 0 - 32767.
    SetRegister(addr_RTC_DAY, days - 1,PMIC_ALL_BITS);

    // As documented in the MC13892 Detailed Technical Specifications
    // document, we must now read back the RTC values to make sure that
    // we didn't possibly experience a day counter rollover glitch during
    // the previous write operations.
    GetRegister(addr_RTC_TOD, &secondsCheck);

   

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


    return TRUE; 
}


//------------------------------------------------------------------------------
//
//  Function:  PMICSetAlarmTime
//
//  Set the PMIC RTC alarm time.
//
//  Parameters:
//      pTime
//          [in] pointer to the time construct to set RTC alarm time
//
//  Returns:
//      TRUE if successful.
//-----------------------------------------------------------------------------
BOOL PMICSetAlarmTime(LPSYSTEMTIME pTime)
{
    UINT32 days, seconds, secondsCheck;
    UINT8 addr_RTC_TOD = MC13892_RTC_ALM_ADDR;
    UINT8 addr_RTC_DAY = MC13892_RTC_DAY_ALM_ADDR;

    // We cannot set the time to anything that is outside our supported
    // time range.


    // calculate time of day in seconds.
    seconds = CalculateSeconds(pTime);
    // Calculate days.
    days = CalculateDays(pTime);

    DEBUGMSG(ZONE_FUNC, (TEXT("RTC SetTime: Date = %02d/%02d/%04d ")
                     TEXT("%02d:%02d:%02d (DD/MM/YYYY HH:MM:SS)\r\n"),
                     pTime->wDay, pTime->wMonth, pTime->wYear,
                     pTime->wHour, pTime->wMinute, pTime->wSecond));
    DEBUGMSG(ZONE_FUNC, (TEXT("        Converted = %d days, %d seconds\r\n"),
                     days, seconds));

    do
    {
    // Write the count of seconds to the 17-bit TimeOftheDay (TOD) register.
    SetRegister(addr_RTC_TOD, seconds,PMIC_ALL_BITS);

    // Write the count of days to the 15-bit DAY register. We decrement
    // the days value by one here because the RTC register counts days
    // from 0 - 32767.
    SetRegister(addr_RTC_DAY, days - 1,PMIC_ALL_BITS);

    // As documented in the MC13892 Detailed Technical Specifications
    // document, we must now read back the RTC values to make sure that
    // we didn't possibly experience a day counter rollover glitch during
    // the previous write operations.
    GetRegister(addr_RTC_TOD, &secondsCheck);

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

  

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:       InitRTC
//
//  This function is used to initialize the mcu real time clock.
//
//  Parameters:
//               None
//  Returns:
//               None
//
//------------------------------------------------------------------------------
void InitRTC(void)
{    
    UINT8 addr;
    UINT32  data, mask;

    addr = MC13892_RTC_TM_ADDR;
    data = CSP_BITFVAL(MC13892_RTC_TM_RTCCAL, 0xf)|CSP_BITFVAL(MC13892_RTC_TM_RTCCALMODE, 0x3);
    mask = CSP_BITFMASK(MC13892_RTC_TM_RTCCAL)| CSP_BITFMASK(MC13892_RTC_TM_RTCCALMODE);

    SetRegister(addr, data, mask);
      
    return;
}

//-----------------------------------------------------------------------------
//
// Function: InitializePmicRTC
//
// Initializes the Pmic RTC module.
//
// Parameters:
//      None.
//
// Parameters:
//      TRUE if the Pmic is initialized or FALSE if an error occurred during
//      initialization.  The reason for the failure can be retrieved by the
//      GetLastError().
//
//-----------------------------------------------------------------------------
 BOOL
InitializePmicRTC(void)
{

    BOOL initializedPmic = FALSE;
    DWORD irq;
    DWORD dwSysIntrRtc;
    BOOL IsFistRun=FALSE;
    SYSTEMTIME SystemTime;

    DEBUGMSG(ZONE_INIT, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // Create critical sections.
    // InitializeCriticalSection(&rtcCs);

    // Create an event for signalling Pmic¡¡RTC interrupts.
    //
    //      pEventAttributes = NULL (must be NULL)
    //      bManualReset = FALSE => resets automatically to nonsignaled
    //                              state after waiting thread released
    //      bInitialState = FALSE => initial state is non-signaled
    //      lpName = NULL => object created without a name
    //
    hIntrEventRtc = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (hIntrEventRtc == NULL)
    {
        ERRORMSG(TRUE, (TEXT("%s(): CreateEvent() failed.\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

   irq = BSPGetRtcSetIrq(); 

     
    // Call the OAL to translate the IRQ into a SysIntr value.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
                         &dwSysIntrRtc, sizeof(DWORD), NULL))
    {
        ERRORMSG(TRUE, (TEXT("%s(): IOCTL_HAL_REQUEST_SYSINTR failed.\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

    // Register RTC interrupt
    if (!InterruptInitialize(dwSysIntrRtc, hIntrEventRtc, NULL, 0))
    {
        ERRORMSG(TRUE, (TEXT("%s(): InterruptInitialize() failed!\r\n"),
                 __WFUNCTION__));
        goto Error;
    }

  
  

    hRtcISTThread = CreateThread(NULL, 0,
                                  (LPTHREAD_START_ROUTINE)PmicRtcIsrThreadProc,
                                  0, 0, NULL);
    if (!hRtcISTThread)
    {
        ERRORMSG(TRUE, (TEXT("%s(): CreateThread() failed\r\n"),
                 __WFUNCTION__));
        goto Error;
    }
    //Then We init RTC is decided by registry
    if(IsFistRun){
        
        InitRTC();
        SystemTime=default_time;
        PMICSetRealTime(&SystemTime);
    }
    else{

        PMICGetRealTime(&SystemTime);
    }

    if (KernelIoControl(IOCTL_HAL_RTC_INIT, &SystemTime, sizeof(SystemTime), NULL, 0, NULL))
     {
        DEBUGMSG(ZONE_ERROR, (_T("IOCTL_HAL_RTC_INIT  SUCESS!!\r\n")));
            
        }
    else
        DEBUGMSG(ZONE_ERROR, (_T("IOCTL_HAL_RTC_INIT Fail, error %d\r\n"),  GetLastError()));    
      
      

    initializedPmic = TRUE;

Error:

    return initializedPmic;
    
}

//------------------------------------------------------------------------------
//
//  Function:       SyncSysRtc
//
//  This function is used to Sync RTC after suspend
//
//  Parameters:
//               None
//  Returns:
//               None
//
//------------------------------------------------------------------------------
void SyncSysRtc()
{
    SYSTEMTIME SystemTime;
    PMICGetRealTime(&SystemTime);
    
    if (KernelIoControl(IOCTL_HAL_RTC_SYNC, &SystemTime, sizeof(SystemTime), NULL, 0, NULL))
    {
        DEBUGMSG(ZONE_FUNC, (_T("IOCTL_HAL_RTC_SYNC  SUCESS!!\r\n")));
    
    }else
        DEBUGMSG(ZONE_FUNC, (_T("IOCTL_HAL_RTC_SYNC Fail, error %d\r\n"),  GetLastError()));
}

//-----------------------------------------------------------------------------
//
// Function: PmicRtcIsrThreadProc
//
// PmicRtcIsrThreadProc handle for setting RTC and ALARM time and update RTC .
//
// Parameters:
//      LPVOID lpParameter.
//
// Returns:
//      1 if success, 0 if failure.
//
//-----------------------------------------------------------------------------
static DWORD CALLBACK
PmicRtcIsrThreadProc(LPVOID lpParameter)
{
    UNREFERENCED_PARAMETER(lpParameter);
    DWORD dwTimeout = SyncTimeOut;
    RTC_QUERY_OUT out;
    DWORD delta;
    SYSTEMTIME SystemTime;
    ULONGLONG fileTime;
    BOOL retVal = TRUE;
    
    // Set thread priority
    CeSetThreadPriority(GetCurrentThread(), 200);

    while(retVal)
    {
    DWORD dwStatus=WaitForSingleObject(hIntrEventRtc, dwTimeout);
    
        switch(dwStatus) {
        case WAIT_OBJECT_0:                             //We set rtc or Alarm       
            if (KernelIoControl(IOCTL_HAL_RTC_QUERY, NULL, 0, &out, sizeof(out), NULL))
            {
                
                if(out.IsRTCEvent)//rtc setting
                {
                    delta=GetTickCount()-out.syncTickCount;
                    fileTime=out.syncRTC+delta*10000;
                    FileTimeToSystemTime((FILETIME*)&fileTime, &SystemTime);
                    PMICSetRealTime(&SystemTime);
                }
                else if(out.IsALARMEvent)
                {
                    delta=GetTickCount()-out.syncAlarmTickCount;
                    fileTime=out.syncAlarnRTC+delta*10000;
                    FileTimeToSystemTime((FILETIME*)&fileTime, &SystemTime);
                    PMICSetAlarmTime(&SystemTime);    //set alarm time                   
                    
                } 
              
            }
            else
            DEBUGMSG(ZONE_ERROR, (_T("IOCTL_HAL_RTC_QUERY Fail, error %d\r\n"),  GetLastError()));
        break;
        
        case  WAIT_TIMEOUT:  
            PMICGetRealTime(&SystemTime);          
            if (KernelIoControl(IOCTL_HAL_RTC_SYNC, &SystemTime, sizeof(SystemTime), NULL, 0, NULL))
            {
                DEBUGMSG(ZONE_FUNC, (_T("IOCTL_HAL_RTC_SYNC  SUCESS!!\r\n")));
            
            }else
                DEBUGMSG(ZONE_ERROR, (_T("IOCTL_HAL_RTC_SYNC Fail, error %d\r\n"),  GetLastError()));
        break;                      
        
        default:
            DEBUGMSG(ZONE_ERROR, (_T("PmicRtc::PmicRtcIsrThreadProcs: WaitForSingleObject() returned %d, error %d\r\n"), dwStatus, GetLastError()));
        break;
        }
    }

    ExitThread(TRUE);
    
    return 1;
}
