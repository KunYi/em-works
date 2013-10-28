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
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File: prpsdk.c
//
// This module provides wrapper functions for accessing the stream interface
// for the PrP driver.
//
//-----------------------------------------------------------------------------
#include <windows.h>
#include <Devload.h>
#include "csp.h"
#include "prp.h"
#include "emmadbg.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: PRPOpenHandle
//
// This method creates a handle to the PrP stream driver.
//
// Parameters:
//      None.
//
// Returns:
//      Handle to PrP driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE PRPOpenHandle(void)
{
    HANDLE hPrP;

    hPrP = CreateFile(TEXT("PRP1:"),        // name of device
        GENERIC_READ | GENERIC_WRITE,       // desired access
        FILE_SHARE_READ | FILE_SHARE_WRITE, // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // If we failed to get handle to PrP
    if (hPrP == INVALID_HANDLE_VALUE)
        ERRORMSG(TRUE, (TEXT("%s:  CreateFile PrP failed!\r\n"), __WFUNCTION__));

    return hPrP;
}

//------------------------------------------------------------------------------
//
// Function: PRPCloseHandle
//
// This method closes a handle to the PrP stream driver.
//
// Parameters:
//      hPrP
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PRPCloseHandle(HANDLE hPrP)
{
    BOOL retVal = TRUE;

    // If we don't have handle to PrP driver
    if (hPrP != NULL)
        if (!CloseHandle(hPrP))
            retVal = FALSE;

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: PRPConfigure
//
// This method configures the Pre-processor using parameters passed
// in by the caller.
//
// Parameters:
//      hPrP
//          [in] Handle to PrP driver.
//
//      pPrpConfigData
//          [in] Pointer to PrP configuration data structure.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PRPConfigure(HANDLE hPrP, pPrpConfigData pConfigData)
{
    BOOL retVal = TRUE;

    // Issue the IOCTL to configure the PrP
    if (!DeviceIoControl(hPrP,      // file handle to the driver
        PRP_IOCTL_CONFIGURE,        // I/O control code
        pConfigData,                // in buffer
        sizeof(prpConfigData),      // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        0,                          // number of bytes returned
        NULL)) {                    // ignored (=NULL)
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PRP_IOCTL_CONFIGURE failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: PRPStart
//
// This method starts the Pre-processor processing.
//
// Parameters:
//      hPrP
//          [in] Handle to PrP driver.
//
//      channel
//          [in] Channel selection.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PRPStart(HANDLE hPrP, prpChannel channel)
{
    BOOL retVal = TRUE;
    UINT8 nChannel = channel;

    // Issue the IOCTL to start the PrP processing
    if (!DeviceIoControl(hPrP,      // file handle to the driver
        PRP_IOCTL_START,            // I/O control code
        &nChannel,                  // in buffer
        sizeof(UINT8),              // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        0,                          // number of bytes returned
        NULL)) {                    // ignored (=NULL)
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PRP_IOCTL_START failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: PRPStop
//
// This method stops the Pre-processor processing.
//
// Parameters:
//      hPrP
//          [in] Handle to PrP driver.
//
//      channel
//          [in] Channel selection.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PRPStop(HANDLE hPrP, prpChannel channel)
{
    BOOL retVal = TRUE;
    UINT8 nChannel = channel;

    // Issue the IOCTL to stop the PrP processing
    if (!DeviceIoControl(hPrP,      // file handle to the driver
        PRP_IOCTL_STOP,             // I/O control code
        &nChannel,                  // in buffer
        sizeof(UINT8),              // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        0,                          // number of bytes returned
        NULL)) {                    // ignored (=NULL)
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PRP_IOCTL_STOP failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: PRPAddBuffers
//
// This method allows the caller to add work buffers to the PrP buffer queues.
//
// Parameters:
//      hPrP
//          [in] Handle to PrP driver.
//
//      pBufs
//          [in] Pointer to structure containing input and output buffers.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PRPAddBuffers(HANDLE hPrP, pPrpBuffers pBufs)
{
    BOOL retVal = TRUE;

    // Issue the IOCTL to add work buffers
    if (!DeviceIoControl(hPrP,      // file handle to the driver
        PRP_IOCTL_ADD_BUFFERS,      // I/O control code
        pBufs,                      // in buffer
        sizeof(prpBuffers),         // in buffer size
        NULL,                       // out buffer
        0,                          // out buffer size
        0,                          // number of bytes returned
        NULL)) {                    // ignored (=NULL)
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PRP_IOCTL_ADD_BUFFERS failed!\r\n"),
            __WFUNCTION__));
        retVal = FALSE;
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: PRPGetFrameCount
//
// This method retrieves the frame count (number of frames processed
// since the PRPStart was called) for the PrP.
//
// Parameters:
//      hPrP
//          [in] Handle to PrP driver.
//
//      channel
//          [in] Channel selection.
//
// Returns:
//      The frame count, -1 if failure.
//
//------------------------------------------------------------------------------
BOOL PRPGetFrameCount(HANDLE hPrP, prpChannel channel)
{
    UINT32 frameCnt = -1;
    UINT8 nChannel = channel;

    // Issue the IOCTL to get number of frames processed
    if (!DeviceIoControl(hPrP,      // file handle to the driver
        PRP_IOCTL_GET_FRAME_COUNT,  // I/O control code
        &nChannel,                  // in buffer
        sizeof(UINT8),              // in buffer size
        &frameCnt,                  // out buffer
        sizeof(UINT32),             // out buffer size
        0,                          // number of bytes returned
        NULL))                      // ignored (=NULL)
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: PRP_IOCTL_GET_FRAME_COUNT failed!\r\n"),
            __WFUNCTION__));

    return frameCnt;
}

