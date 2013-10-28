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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File:  gptsdk.c
//
// This module provides wrapper functions for accessing
// the stream interface for the GPT driver.
//
//-----------------------------------------------------------------------------
#include <windows.h>
#include <Devload.h>
#include "csp.h"
#include "gpt.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define GPT_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define GPT_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))



#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INFO       12
#define ZONEID_FUNCTION   13
#define ZONEID_WARN       14
#define ZONEID_ERROR      15

// Debug zone masks
#define ZONEMASK_INFO     (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN     (1<<ZONEID_WARN)
#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
#endif


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

// If other drivers use this library, they must define the global dpCurSettings
#ifdef DEBUG
extern DBGPARAM dpCurSettings;
#endif


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: GptOpenHandle
//
// This method creates a handle to the GPT stream driver.
//
// Parameters:
//      GptNumber[in] - Specify which GPT to open. The
//                      value ranges from 1 to 5.
//
// Returns:
//      Handle to GPT driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE GptOpenHandle(UINT8 GptNumber)
{
    HANDLE hGpt;
    WCHAR deviceName[5];
    WCHAR deviceNumber[2];

    GPT_FUNCTION_ENTRY();

    if((GptNumber < GPT_MINIMUM_NUMBER) || (GptNumber > GPT_MAXIMUM_NUMBER))
    {
    DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  GptNumber is out of range!\r\n"), __WFUNCTION__));
        return INVALID_HANDLE_VALUE;
    }

    // get the device name correspond to the GptNumber
    _itow_s(GptNumber, deviceNumber, _countof(deviceNumber), 10);
    wcscpy_s(deviceName, _countof(deviceNumber), TEXT("GPT"));
    wcscat_s(deviceName, _countof(deviceNumber), deviceNumber);
    wcscat_s(deviceName, _countof(deviceNumber), TEXT(":"));

    DEBUGMSG(ZONE_INFO,
            (TEXT("%s:  The device name for the GPT you want to open is %s.\r\n"), __WFUNCTION__, deviceName));

    hGpt = CreateFile(deviceName,        // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to GPT
    if (hGpt == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  CreateFile GPT failed!\r\n"), __WFUNCTION__));
        return hGpt;
    }

    GPT_FUNCTION_EXIT();

    return hGpt;
}


//------------------------------------------------------------------------------
//
// Function: GptCloseHandle
//
// This method closes a handle to the GPT stream driver.
//
// Parameters:
//      hGpt
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL GptCloseHandle(HANDLE hGpt)
{
    GPT_FUNCTION_ENTRY();

    // if we don't have handle to GPT bus driver
    if (hGpt != NULL)
    {
        if (!CloseHandle(hGpt))
        {
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: CloseHandle failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    GPT_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: GptCreateTimerEvent
//
// This method returns a handle triggered
// when the GPT timer period has elapsed.
//
// Parameters:
//      hGpt
//          [in] Handle to GPT driver.
//
//      eventName
//          [in] String identifying timer event.
//
// Returns:
//      Timer event handle created.  Handle is NULL if failure.
//
//------------------------------------------------------------------------------
HANDLE GptCreateTimerEvent(HANDLE hGpt, LPTSTR eventName)
{
    HANDLE hTimerEvent;

    GPT_FUNCTION_ENTRY();

    hTimerEvent = CreateEvent(NULL, FALSE, FALSE, eventName);
    if (hTimerEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: CreateEvent failed\r\n"), __WFUNCTION__));
        goto cleanup;
    }

    // issue the IOCTL to create the GPT timer event
    if (!DeviceIoControl(hGpt,     // file handle to the driver
        GPT_IOCTL_TIMER_CREATE_EVENT,     // I/O control code
        eventName,                      // in buffer
        wcslen(eventName) * sizeof(WCHAR),                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: GPT_IOCTL_TIMER_CREATE_EVENT failed!\r\n"), __WFUNCTION__));
        goto cleanup;
    }

    GPT_FUNCTION_EXIT();

    return hTimerEvent;

cleanup:
    CloseHandle(hTimerEvent);
    return NULL;
}


//------------------------------------------------------------------------------
//
// Function: GptReleaseTimerEvent
//
// This method closes a handle to the GPT timer event.
//
// Parameters:
//      hGpt
//          [in] Handle to GPT driver.
//
//      eventName
//          [in] String identifying timer event.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL GptReleaseTimerEvent(HANDLE hGpt, LPTSTR eventName)
{
    // issue the IOCTL to close the handle to the GPT timer event
    if (!DeviceIoControl(hGpt,     // file handle to the driver
        GPT_IOCTL_TIMER_RELEASE_EVENT,     // I/O control code
        eventName,                      // in buffer
        wcslen(eventName) * sizeof(WCHAR),                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: GPT_IOCTL_TIMER_RELEASE_EVENT failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptSetTimer
//
// This method sets the timer period for the GPT.
//
// Parameters:
//      hGpt
//          [in] Handle to GPT driver.
//
//      gptTimerDelayPkt
//          [in] Packet containing information for setting GPT timer delay.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL GptSetTimer(HANDLE hGpt, PGPT_TIMER_SET_PKT pGptTimerDelayPkt)
{
    // issue the IOCTL to set GPT timer delay
    if (!DeviceIoControl(hGpt,     // file handle to the driver
        GPT_IOCTL_TIMER_SET_DELAY, // I/O control code
        pGptTimerDelayPkt,         // in buffer
        sizeof(PGPT_TIMER_SET_PKT),  // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: GPT_IOCTL_TIMER_SET_DELAY failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptStart
//
// This method enables the GPT interrupt and starts the GPT timer.
//
// Parameters:
//      hGpt
//          [in] Handle to GPT driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL GptStart(HANDLE hGpt)
{
    // issue the IOCTL to request GPT interrupt enable
    if (!DeviceIoControl(hGpt,     // file handle to the driver
        GPT_IOCTL_INT_ENABLE,     // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: GPT_IOCTL_INT_ENABLE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // issue the IOCTL to start GPT timer
    if (!DeviceIoControl(hGpt,     // file handle to the driver
        GPT_IOCTL_TIMER_START,     // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: GPT_IOCTL_TIMER_START failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: GptStop
//
// This method disables the GPT interrupt and stops the GPT timer.
//
// Parameters:
//      hGpt
//          [in] Handle to GPT driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL GptStop(HANDLE hGpt)
{
    // issue the IOCTL to stop GPT timer
    if (!DeviceIoControl(hGpt,     // file handle to the driver
        GPT_IOCTL_TIMER_STOP,     // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: GPT_IOCTL_TIMER_START failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // issue the IOCTL to request GPT interrupt disable
    if (!DeviceIoControl(hGpt,     // file handle to the driver
        GPT_IOCTL_INT_DISABLE,     // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: GPT_IOCTL_INT_ENABLE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    return TRUE;
}
