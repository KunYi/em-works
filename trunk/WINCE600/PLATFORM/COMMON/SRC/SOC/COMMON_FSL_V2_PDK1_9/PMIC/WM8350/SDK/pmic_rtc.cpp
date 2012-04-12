//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @file   pmicpdk.c
/// @brief  Platform-specific WM8350 PMIC functions.
///
/// This file contains the PMIC platform-specific functions that provide control
/// over the Power Management IC.
///
/// @version $Id: pmic_rtc.cpp 453 2007-05-02 11:33:48Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//-----------------------------------------------------------------------------
//
//  File:  pmic_rtc.cpp
//
//  This file contains the PMIC rtc SDK interface that is used by applications
//  and other drivers to access registers of the MC13783 RTC.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
//#include "mxarm11_macros.h"
#include "pmic_ioctl.h"
#include "pmic_rtc.h"
#include "pmic_basic_types.h"
#include "pmic_lla.h"
//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
extern "C" HANDLE               hPMI;           // Our global handle to the PMIC driver
extern "C" WM_DEVICE_HANDLE     g_hWMDevice;    // Our global Wolfson device handle

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR            0
#define ZONEID_WARN             1
#define ZONEID_INIT             2
#define ZONEID_FUNC             3
#define ZONEID_INFO             4

// Debug zone masks
#define ZONEMASK_ERROR      (1 << ZONEID_ERROR)
#define ZONEMASK_WARN           (1 << ZONEID_WARN)
#define ZONEMASK_INIT           (1 << ZONEID_INIT)
#define ZONEMASK_FUNC       (1 << ZONEID_FUNC)
#define ZONEMASK_INFO           (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR              DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN               DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT               DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC               DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO               DEBUGZONE(ZONEID_INFO)

extern DBGPARAM dpCurSettings;
// Named event for interrupt registration
static TCHAR *gEventNamePri = TEXT("EVENT_RTC");
static TCHAR *gRTCEventName;
#endif  // DEBUG

#define TRACE_ENTRY()     DEBUGMSG( ZONE_FUNC, (_T("+ %s()\r\n"), _T(__FUNCTION__)))
#define TRACE_EXIT()      DEBUGMSG( ZONE_FUNC, (_T("- %s()\r\n"), _T(__FUNCTION__)))

//------------------------------------------------------------------------------
// Global Variables
static HANDLE hIntrEventtodaPmic;
static HANDLE hPmicTODalarmThread;
//static BOOL bTODalarmTerminate;
static BOOL PmicTodAlarmThreadProc(LPVOID lpParam);
SYSTEMTIME default_time = {2003,1,3,1,0,0,0,0};  //Jan 1, 2003, Wednesday
//-----------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
//
// Function: pmicRTCSetSystemTime
//
// This function sets the current system time and date.
//
// Parameters:
//      Pointer to a SYSTEMTIME structure that contains the current system date and time.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS pmicRTCSetSystemTime(SYSTEMTIME* lpSystemTime)
{
    WM_STATUS   status;
    PMIC_STATUS retval;

    TRACE_ENTRY();

    status = WMPmicSetTime( g_hWMDevice, lpSystemTime );

    if ( WM_SUCCESS( status ) )
        retval = PMIC_SUCCESS;
    else
        retval = PMIC_ERROR;

    TRACE_EXIT();

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: pmicRTCGetSystemTime
//
// This function retrieves the current system time and date.
//
// Parameters:
//      lpSystemTime
//        Pointer to a SYSTEMTIME structure to receive the current system date
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS pmicRTCGetSystemTime(SYSTEMTIME* lpSystemTime)
{
    WM_STATUS   status;
    PMIC_STATUS retval;

    TRACE_ENTRY();

    status = WMPmicGetTime( g_hWMDevice, lpSystemTime );

    if ( WM_SUCCESS( status ) )
        retval = PMIC_SUCCESS;
    else
        retval = PMIC_ERROR;

    TRACE_EXIT();

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: pmicRTCSetAlarmTime
//
// This function sets the alarm time and date
//
// Parameters:
//      lpAlarmTime
//         Pointer to a SYSTEMTIME structure that contains thealarm date and time.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS pmicRTCSetAlarmTime(SYSTEMTIME* lpAlarmTime)
{
    WM_STATUS   status;
    PMIC_STATUS retval;

    TRACE_ENTRY();

    status = WMPmicSetAlarm( g_hWMDevice, lpAlarmTime );

    if ( WM_SUCCESS( status ) )
        retval = PMIC_SUCCESS;
    else
        retval = PMIC_ERROR;

    TRACE_EXIT();

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: pmicRTCGetAlarmTime
//
// This function retrieves the alarm time and date
//
// Parameters:
//      llpAlarmTime
//         Pointer to a SYSTEMTIME structure to receive the alarm date and time.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS pmicRTCGetAlarmTime(SYSTEMTIME* lpAlarmTime)
{
    WM_STATUS   status;
    PMIC_STATUS retval;

    TRACE_ENTRY();

    status = WMPmicGetAlarm( g_hWMDevice, lpAlarmTime );

    if ( WM_SUCCESS( status ) )
        retval = PMIC_SUCCESS;
    else
        retval = PMIC_ERROR;

    TRACE_EXIT();

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: pmicRTCRegisterAlarmCallback
//
// This function registers alarm callback function.
//
// Parameters:
//      alarmCB
//        Alarm callback function.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS pmicRTCRegisterAlarmCallback(RTC_ALARM_CB alarmCB)
{
    PMIC_STATUS status=PMIC_ERROR;

    TRACE_ENTRY();

    // create event for PMIC interrupt signaling
    //      pEventAttributes = NULL (must be NULL)
    //      bManualReset = FALSE => resets automatically to nonsignaled
    //                              state after waiting thread released
    //      bInitialState = FALSE => initial state is non-signaled
    //      lpName = EVENT_RTC_PRI => object created with a name
    hIntrEventtodaPmic = CreateEvent(NULL, FALSE, FALSE, TEXT("EVENT_RTC_PRI"));

    // check if CreateEvent failed
    if (hIntrEventtodaPmic == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s(): CreateEvent failed!\r\n"),
            _T(__FUNCTION__)));
    return status;
    }


 // Register for PMIC ADC done interrupts.
    if (PmicInterruptRegister(WM8350_INT_RTC_SEC, TEXT("EVENT_RTC_PRI"))
        != PMIC_SUCCESS)
    {
        ERRORMSG(1, (_T("Pmic TODAlarm RtcInterruptRegister failed\r\n")));
        return status;
    }

    // Make sure RTC interrupt is unmasked
    if (PmicInterruptEnable(WM8350_INT_RTC_SEC) != PMIC_SUCCESS)
    {
        ERRORMSG(TRUE, (_T("PmicRTCInit:  PmicInterruptEnable failed\r\n")));
        goto cleanUp;
    }


    bTODalarmTerminate = FALSE;

    hPmicTODalarmThread = CreateThread(NULL, 0,
                                  (LPTHREAD_START_ROUTINE)PmicTodAlarmThreadProc,
                                  (LPVOID)alarmCB, 0, NULL);
    if (!hPmicTODalarmThread)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("PmicIsrThreadStart: CreateThread failed\r\n")));
        PmicInterruptDeregister(WM8350_INT_RTC_SEC);
        goto cleanUp;
    }

    status = PMIC_SUCCESS;

cleanUp:
    if (status != PMIC_SUCCESS)PmicInterruptDeregister(WM8350_INT_RTC_SEC);

    TRACE_EXIT();

    return status;
}


//------------------------------------------------------------------------------
//
// Function: pmicRTCCancelAlarm
//
// This function cancels the alarm time and date setting.
//
// Parameters:
//      None
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS pmicRTCCancelAlarm()
{
    TRACE_ENTRY();

    PmicInterruptDeregister(WM8350_INT_RTC_SEC);
    bTODalarmTerminate = TRUE;
    CloseHandle(hPmicTODalarmThread);

    TRACE_EXIT();

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicIsrThreadProc
//
// ISR Thread Process that launches IST loop.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
static BOOL
PmicTodAlarmThreadProc(LPVOID lpParam)
{
   RTC_ALARM_CB alarmFn =  (RTC_ALARM_CB) lpParam;

   SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    while(!bTODalarmTerminate)
    {
        if (WaitForSingleObject(hIntrEventtodaPmic, INFINITE) == WAIT_OBJECT_0)
        {
            alarmFn();
        }
    }

    ExitThread(TRUE);

    return TRUE;
}
