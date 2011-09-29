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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
#pragma warning(disable: 4115 4201 4204 4214 )
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#pragma warning(pop)
#include "csp.h"


//------------------------------------------------------------------------------
// Defines
#define ORIGINYEAR       1980                  // the begin year
#ifdef MAXYEAR
#undef MAXYEAR
#define MAXYEAR          (ORIGINYEAR + 100)    // the maxium year
#endif
//------------------------------------------------------------------------------
// Global Variables
static const UINT8 monthtable[12]      = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const UINT8 monthtable_leap[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define IRQ_EXT_IRQ            IRQ_RESERVED5

BOOL CheckRealTime(LPSYSTEMTIME lpst);

//------------------------------------------------------------------------------

#define BCD2BIN(b)              (((b) >> 4)*10 + ((b)&0xF))
#define BIN2BCD(b)              ((((UINT8)(b)/10) << 4)|((UINT8)(b)%10))

//------------------------------------------------------------------------------

typedef struct {

    DWORD syncTickCount;
    ULONGLONG syncRTC;

    DWORD syncAlarmTickCount;
    ULONGLONG syncAlarnRTC;
    BOOL IsRTCEvent;
    BOOL IsALARMEvent;
       
} RTC_QUERY_OUT;


static struct {

    BOOL initialized;           // Is RTC subsystem intialized
    CRITICAL_SECTION cs;
  
    ULONGLONG baseRTC;          // RTC for hardware time
       
    DWORD syncTickCount;        // Tick count for synchronization
    ULONGLONG syncRTC;
    
    DWORD syncAlarmTickCount;        // Tick count of Alarm  for synchronization
    ULONGLONG syncAlarnRTC;
    
    BOOL IsRTCEvent;           // Is RTC subsystem Event
    BOOL IsALARMEvent;           // Is ALARM subsystem Event
    
    RTC_QUERY_OUT query;

} s_rtc = { FALSE };

//------------------------------------------------------------------------------
//
//  Static:  s_rtcUpperBound
//
//  This is maximal real time supported by RTC code (currently 2964/01/17 
//  14:20:00), but it can be almost any time supported by FILETIME structure.
//
static const FILETIME s_rtcUpperBound = { 0x6302b800, 0x05f825a0 };
//------------------------------------------------------------------------------

UINT32
SC_GetTickCount(
    )
    {        
        return CurMSec;
    }

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot. 
//  Input buffer contains SYSTEMTIME structure with default time value.
//------------------------------------------------------------------------------
BOOL
OALIoCtlHalInitRTC(
    UINT32 code, 
    VOID *pInBuffer, 
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInBuffer);
    UNREFERENCED_PARAMETER(inSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    // Add static mapping for RTC alarm
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM, IRQ_RTC);
    
    return FALSE;
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
BOOL
OEMGetRealTime(
    SYSTEMTIME *pSystemTime
    ) 
{
    DWORD count;
    ULONGLONG delta, time;

    if (!s_rtc.initialized)
        {
            // Return zero time if RTC isn't initialized
            pSystemTime->wYear = 1601;
            pSystemTime->wMonth = 1;
            pSystemTime->wDayOfWeek = 0;
            pSystemTime->wDay = 1;
            pSystemTime->wHour = 0;
            pSystemTime->wMinute = 0;
            pSystemTime->wSecond = 0;
            pSystemTime->wMilliseconds = 0;
        }
    else
        {
                   
            EnterCriticalSection(&s_rtc.cs);
            count = SC_GetTickCount();
            delta = count - s_rtc.syncTickCount;
            time = ((s_rtc.syncRTC +delta * 10000)/10000000)*10000000;
            if (time > *(ULONGLONG*)&s_rtcUpperBound)
            {
                time = *(ULONGLONG*)&s_rtcUpperBound;
            }
            NKFileTimeToSystemTime((FILETIME*)&time, pSystemTime);
            LeaveCriticalSection(&s_rtc.cs);
        }

    return TRUE;
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
//------------------------------------------------------------------------------
BOOL
OEMSetRealTime(
    SYSTEMTIME *pSystemTime
    ) 
{
    BOOL rc = FALSE;
    DWORD sysIntr;
    
    if (pSystemTime == NULL) return FALSE;
    // Check if the input time is valid or not
    if (CheckRealTime(pSystemTime) == FALSE)
    {
        OALMSG(OAL_FUNC, (L"OEMSetRealTime is INVALID!!!:  %u/%u/%u, %u:%u:%u  Day of week:%u\r\n",pSystemTime->wYear, pSystemTime->wMonth , pSystemTime->wDay, pSystemTime->wHour, pSystemTime->wMinute, pSystemTime->wSecond,pSystemTime->wDayOfWeek));
        return FALSE;
    }else
        OALMSG(OAL_FUNC, (TEXT("OEMSetRealTime set is valid:  %u/%u/%u, %u:%u:%u  Day of week:%u\r\n"), pSystemTime->wYear, pSystemTime->wMonth , pSystemTime->wDay, pSystemTime->wHour, pSystemTime->wMinute, pSystemTime->wSecond,pSystemTime->wDayOfWeek));   

    // Save time to global structure
    EnterCriticalSection(&s_rtc.cs);

    // Convert to base time
    if (NKSystemTimeToFileTime(pSystemTime, (FILETIME*)&s_rtc.syncRTC) &&
        (s_rtc.syncRTC <= *(ULONGLONG*)&s_rtcUpperBound))
        {
        
        s_rtc.syncTickCount=SC_GetTickCount();
        s_rtc.IsRTCEvent=TRUE;
        LeaveCriticalSection(&s_rtc.cs);
  
        // Get SYSINTR for Triton chip
        sysIntr = OALIntrTranslateIrq(IRQ_EXT_IRQ);
        //send a interrupt event to MCU Driver
        if (sysIntr != KITL_SYSINTR_NOINTR) 
            NKSetInterruptEvent(sysIntr);    
                    
        // Done
        rc = TRUE;
    }
    
    

    LeaveCriticalSection(&s_rtc.cs);
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetAlarmTime
//
//  Updates the ALARM time with the specified system time.
//
//  Parameters:
//      pTime
//          [in] pointer to the time construct to set the current time
//
//  Returns:
//      TRUE if successful.
//
//------------------------------------------------------------------------------

BOOL
OEMSetAlarmTime(
    SYSTEMTIME *pSystemTime
    ) 
{
    BOOL rc = FALSE;
    UINT32 sysIntr;
    
     if (pSystemTime == NULL) return FALSE;
    // Check if the input time is valid or not
    if (CheckRealTime(pSystemTime) == FALSE)
    {
        OALMSG(OAL_FUNC, (L"OEMSetAlarmTime is INVALID!!!:  %u/%u/%u, %u:%u:%u  Day of week:%u\r\n",pSystemTime->wYear, pSystemTime->wMonth , pSystemTime->wDay, pSystemTime->wHour, pSystemTime->wMinute, pSystemTime->wSecond,pSystemTime->wDayOfWeek));
        return FALSE;
    }else
        OALMSG(OAL_FUNC, (TEXT("OEMSetAlarmTime set is valid:  %u/%u/%u, %u:%u:%u  Day of week:%u\r\n"), pSystemTime->wYear, pSystemTime->wMonth , pSystemTime->wDay, pSystemTime->wHour, pSystemTime->wMinute, pSystemTime->wSecond,pSystemTime->wDayOfWeek));   

    // Save time to global structure
    if (NKSystemTimeToFileTime(pSystemTime, (FILETIME*)&s_rtc.syncAlarnRTC) &&
        (s_rtc.syncAlarnRTC <= *(ULONGLONG*)&s_rtcUpperBound))
        {
        EnterCriticalSection(&s_rtc.cs);

        s_rtc.syncAlarmTickCount=SC_GetTickCount();

        s_rtc.IsALARMEvent=TRUE;
        LeaveCriticalSection(&s_rtc.cs);
        
        // Get SYSINTR for Triton chip
        sysIntr = OALIntrTranslateIrq(IRQ_EXT_IRQ);
        if (sysIntr != KITL_SYSINTR_NOINTR) 
            NKSetInterruptEvent(sysIntr);

        // Done
        rc = TRUE;
        }    

    return rc;
}




//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by MCU Driver to initialize the time after boot.
//  Input buffer contains SYSTEMTIME structure with default time value.
//  
//
//------------------------------------------------------------------------------
BOOL OALIoCtlInitRTC( UINT32 code, VOID *pInpBuffer, UINT32 inpSize,
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

    NKSystemTimeToFileTime(pTime, (FILETIME*)&s_rtc.baseRTC);
    s_rtc.syncRTC = s_rtc.baseRTC;
    s_rtc.syncTickCount=SC_GetTickCount();
    InitializeCriticalSection(&s_rtc.cs);
    s_rtc.initialized=TRUE;

        // Done 
    rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL && OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcUpdate
//
//  This function is called periodically by MCU driver to Sync
//  RTC value.
//------------------------------------------------------------------------------
BOOL
OALIoCtlHalRtcSync(
    UINT32 code, 
    VOID *pInBuffer, 
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL rc = FALSE;
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    // Check basic parameters
    if ((pInBuffer == NULL) || (inSize < sizeof(SYSTEMTIME)))
        {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        goto cleanUp;
        }

    EnterCriticalSection(&s_rtc.cs);

    NKSystemTimeToFileTime((LPSYSTEMTIME)pInBuffer, (FILETIME*)&s_rtc.syncRTC);
    s_rtc.syncTickCount=SC_GetTickCount();

    LeaveCriticalSection(&s_rtc.cs);

    // Done 
    rc = TRUE;

cleanUp:
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcAlarm
//
//  This function is called by RTC driver when alarm interrupt occurs.
//------------------------------------------------------------------------------
BOOL
OALIoCtlHalRtcAlarm(
    UINT32 code, 
    VOID *pInBuffer, 
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInBuffer);
    UNREFERENCED_PARAMETER(inSize);
    UNREFERENCED_PARAMETER(pOutSize);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    
    NKSetInterruptEvent(SYSINTR_RTC_ALARM);
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalRtcQuery
//
//  This function is called by RTC driver before  OEMSetAlarmTime and OEMSetRealTime to get
// current syncRTC and syncTickCount
//------------------------------------------------------------------------------
BOOL
OALIoCtlHalRtcQuery(
    UINT32 code, 
    VOID *pInBuffer, 
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL rc = FALSE;
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInBuffer);
    UNREFERENCED_PARAMETER(inSize);

    if (pOutSize != NULL) *pOutSize = sizeof(RTC_QUERY_OUT);

    if ((pOutBuffer == NULL) || 
        (outSize < sizeof(RTC_QUERY_OUT)))
        {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        goto cleanUp;
        }

    // Copy and clear info...
    EnterCriticalSection(&s_rtc.cs);

    s_rtc.query.syncTickCount=SC_GetTickCount();
    s_rtc.query.syncAlarmTickCount=SC_GetTickCount();
    s_rtc.query.syncRTC=s_rtc.syncRTC;
    s_rtc.query.syncAlarnRTC=s_rtc.syncAlarnRTC;
    s_rtc.query.IsRTCEvent=s_rtc.IsRTCEvent;
    s_rtc.query.IsALARMEvent=s_rtc.IsALARMEvent;

    memcpy(pOutBuffer, &s_rtc.query, sizeof(s_rtc.query));
    memset(&s_rtc.query, 0, sizeof(s_rtc.query));
    LeaveCriticalSection(&s_rtc.cs);
    s_rtc.IsRTCEvent=FALSE;
    s_rtc.IsALARMEvent=FALSE;
    // Done
    rc = TRUE;

cleanUp:    
    return rc;
}



//------------------------------------------------------------------------------
//
//  Function: IsLeapYear
//
//  This function determines if the year input is a
//  leap year.
//
//  Parameters:
//             Year 
//             [in]  Year to be checked
//  Parameters:
//             1 - If Year is a leap year
//             0 - If Year is not a leap year
//
//------------------------------------------------------------------------------
static BOOL IsLeapYear(int Year)
{
    int Leap;

    Leap = 0;
    if ((Year % 4) == 0) {
        Leap = 1;
        if ((Year % 100) == 0) {
            Leap = (Year%400) ? 0 : 1;
        }
    }

    return (Leap==1)?TRUE:FALSE;
}


//------------------------------------------------------------------------------
//
//  Function:       CheckRealTime
//
//  This function is used to check if the input
//   time is valid.
//
//  Parameters:
//           lpst
//           [in] pointer to to the buffer containing the time to 
//                be checked in SYSTEMTIME format
//  Returns:
//           TRUE - If time is valid.
//           FALSE - If time is invalid.
//
//------------------------------------------------------------------------------
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




