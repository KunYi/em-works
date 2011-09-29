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
//  Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  owiresdk.c
//
//  This module provides wrapper functions for accessing
//  the stream interface for the OWIRE driver.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201)
#include <windows.h>
#include <Devload.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_owire.h"
#include "owire.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define OWIRE_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define OWIRE_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT       0
#define ZONEID_INFO       12
#define ZONEID_FUNCTION   13
#define ZONEID_WARN       14
#define ZONEID_ERROR      15

// Debug zone masks
#define ZONEMASK_INIT     (1<<ZONEID_INIT)
#define ZONEMASK_INFO     (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN     (1<<ZONEID_WARN)
#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
#endif


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("OWIRE"),
    {
        _T(""), _T(""), _T(""), _T(""),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_INFO
};
#endif


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: OwireOpenHandle
//
// This method creates a handle to the OWIRE stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to OWIRE driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE OwireOpenHandle(void)
{
    HANDLE hOwire;

    OWIRE_FUNCTION_ENTRY();

    hOwire = CreateFile(TEXT("WIR1:"),        // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to OWIRE
    if (hOwire == INVALID_HANDLE_VALUE)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s:  CreateFile OWIRE failed! ErrCode=%d \r\n"), __WFUNCTION__, GetLastError()));
        return hOwire;
    }

    OWIRE_FUNCTION_EXIT();

    return hOwire;
}


//------------------------------------------------------------------------------
//
// Function: OwireCloseHandle
//
// This method closes a handle to the OWIRE stream driver.
//
// Parameters:
//      hOwire
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL OwireCloseHandle(HANDLE hOwire)
{
    OWIRE_FUNCTION_ENTRY();

    // if we don't have handle to OWIRE driver
    if (hOwire != NULL)
    {
        if (!CloseHandle(hOwire))
        {
            DEBUGMSG(ZONE_ERROR,
                (TEXT("%s: CloseHandle failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    OWIRE_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: OwireResetPresencePulse
//
// This function performs a one-wire reset sequence
// with a reset pulse and presence pulse.
//
// Parameters:
//      hOwire
//          [in] Handle to OWIRE driver.
//
// Returns:
//      TRUE if success.  FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL OwireResetPresencePulse(HANDLE hOwire)
{
    OWIRE_FUNCTION_ENTRY();
    
    // issue the IOCTL to issue the reset presence pulse to the OWIRE
    if (!DeviceIoControl(hOwire,     // file handle to the driver
        OWIRE_IOCTL_RESET_PRESENCE_PULSE,     // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: OWIRE_IOCTL_RESET_PRESENCE_PULSE failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    OWIRE_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: OwireRead
//
// This function attempts to read data from the one-wire module.
//
// Parameters:
//      hOwire
//          [in] Handle to OWIRE driver.
//
//      readBuf
//          [in] Pointer to buffer containing bytes read from OWIRE.
//
//      bytesToRead
//          [in] Number of bytes to read from OWIRE.
//
// Returns:
//      TRUE if success.  FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL OwireRead(HANDLE hOwire, BYTE *readBuf, DWORD bytesToRead)
{
    DWORD bytesRead;

    OWIRE_FUNCTION_ENTRY();

    if (!ReadFile(hOwire, readBuf, bytesToRead, (LPDWORD) &bytesRead, NULL))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: OWIRE ReadFile failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    if (bytesToRead != bytesRead)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Did not read the correct number of bytes!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    OWIRE_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: OwireWrite
//
// This function attempts to write data to the one-wire module.
//
// Parameters:
//      hOwire
//          [in] Handle to OWIRE driver.
//
//      writeBuf
//          [in] Pointer to buffer containing bytes to write to OWIRE.
//
//      bytesToWrite
//          [in] Number of bytes to write to OWIRE.
//
// Returns:
//      TRUE if success.  FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL OwireWrite(HANDLE hOwire, BYTE *writeBuf, DWORD bytesToWrite)
{
    DWORD bytesWritten;

    OWIRE_FUNCTION_ENTRY();

    if (!WriteFile(hOwire, writeBuf, bytesToWrite, (LPDWORD) &bytesWritten, NULL))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: OWIRE WriteFile failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    if (bytesToWrite != bytesWritten)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Did not write the correct number of bytes!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    OWIRE_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: OwireLock
//
// This function attempts to lock one-wire bus.
//
// Parameters:
//      hOwire
//          [in] Handle to OWIRE driver.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void OwireBusLock(HANDLE hOwire)
{
    OWIRE_FUNCTION_ENTRY();

    // issue the IOCTL to lock the OWIRE bus
    if (!DeviceIoControl(hOwire,     // file handle to the driver
        OWIRE_IOCTL_BUS_LOCK,     // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: OWIRE_IOCTL_BUS_LOCK failed!\r\n"), __WFUNCTION__));
    }

    OWIRE_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: OwireBusUnLock
//
// This function attempts to unlock one-wire bus.
//
// Parameters:
//      hOwire
//          [in] Handle to OWIRE driver.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void OwireBusUnLock(HANDLE hOwire)
{
    OWIRE_FUNCTION_ENTRY();

    // issue the IOCTL to unlock the OWIRE bus
    if (!DeviceIoControl(hOwire,     // file handle to the driver
        OWIRE_IOCTL_BUS_UNLOCK,     // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: OWIRE_IOCTL_BUS_UNLOCK failed!\r\n"), __WFUNCTION__));
    }

    OWIRE_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the OWIRE SDK module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the OWIRE is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
//extern "C"
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HMODULE)hInstDll);
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS ATTACH TO OWIRE SDK *****\r\n")));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_INIT,
                     (_T("***** DLL PROCESS DETACH FROM OWIRE SDK *****\r\n")));
            break;
    }

    // return TRUE for success
    return TRUE;
}
