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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ppsdk.c
//
//  This module provides wrapper functions for accessing
//  the stream interface for the PP driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#pragma warning(pop)

#include "mxarm11.h"
#include "pp.h"


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
// Function: PPOpenHandle
//
// This method creates a handle to the PP stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to PP driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE PPOpenHandle(void)
{
    HANDLE hPP;

    hPP = CreateFile(TEXT("POP1:"),        // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to PP
    if (hPP == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(1, (TEXT("%s:  CreateFile PP failed!\r\n"), __WFUNCTION__));
    }

    return hPP;
}


//------------------------------------------------------------------------------
//
// Function: PPCloseHandle
//
// This method closes a handle to the PP stream driver.
//
// Parameters:
//      hPP
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPCloseHandle(HANDLE hPP)
{
    BOOL retVal = TRUE;

    // if we don't have handle to PP driver
    if (hPP != NULL)
    {
        if (!CloseHandle(hPP))
        {
            retVal = FALSE;
        }
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: PPConfigure
//
// This method configures the Post-processor using parameters passed
// in by the caller.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      pPpConfigData
//          [in] Pointer to PP configuration data structure.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PPConfigure(HANDLE hPP, pPpConfigData pConfigData)
{
    // issue the IOCTL to configure the PP
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_CONFIGURE,       // I/O control code
        pConfigData,              // in buffer
        sizeof(ppConfigData),    // in buffer size
        NULL,                     // out buffer
        0,                        // out buffer size
        0,                        // number of bytes returned
        NULL))                    // ignored (=NULL)
    {
        ERRORMSG(1, (TEXT("%s: PP_IOCTL_CONFIGURE failed!\r\n"), __WFUNCTION__));
    }
}


//------------------------------------------------------------------------------
//
// Function: PPStart
//
// This method starts the Post-processor task.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      pInputBuf
//          [in] Pointer to destination buffer for PP task.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PPStart(HANDLE hPP)
{
    // issue the IOCTL to start the PP task
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_START,            // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(1, (TEXT("%s: PP_IOCTL_START failed!\r\n"), __WFUNCTION__));
    }
}

//------------------------------------------------------------------------------
//
// Function: PPStop
//
// This method stops the Post-processor.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PPStop(HANDLE hPP)
{
    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_STOP,             // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(1, (TEXT("%s: PP_IOCTL_STOP failed!\r\n"), __WFUNCTION__));
    }

    return;
}


//------------------------------------------------------------------------------
//
// Function: PPAddBuffers
//
// This method allows the caller to add input and output buffers to
// the PP buffer queues.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      pBufs
//          [in] Pointer to structure containing input and output buffers.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPAddBuffers(HANDLE hPP, pPpBuffers pBufs)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_ENQUEUE_BUFFERS,  // I/O control code
        pBufs,                     // in buffer
        sizeof(ppBuffers),         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: PPClearBuffers
//
// This method allows the caller to clear the input and output buffer 
// queues in the Post-processing driver.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPClearBuffers(HANDLE hPP)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_CLEAR_BUFFERS,    // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: PPGetMaxBuffers
//
// This method retrieves the maximum number of buffers that can be
// queued in the Post-processor.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
// Returns:
//      The max buffers for the PP.  -1 if failure.
//
//------------------------------------------------------------------------------
UINT32 PPGetMaxBuffers(HANDLE hPP)
{
    UINT32 maxBufNum = (UINT32) -1;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_GET_MAX_BUFFERS,  // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        &maxBufNum,                // out buffer
        sizeof(UINT32),            // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        ERRORMSG(1, (TEXT("%s: PP_IOCTL_GET_MAX_BUFFERS failed!\r\n"), __WFUNCTION__));
    }

    return maxBufNum;
}

//------------------------------------------------------------------------------
//
// Function: PPPauseDisplay
//
// This method pauses the viewfinding display.  This will only work if
// direct display is enabled to send frames direct to a viewfinding display.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPPauseDisplay(HANDLE hPP)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_PAUSE_VIEWFINDING,    // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: PPGetFrameCount
//
// This method retrieves the frame count (number of frames processed
// since the PPStart was called) for the PP.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPGetFrameCount(HANDLE hPP)
{
    UINT32 frameCnt;
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_GET_FRAME_COUNT,    // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        &frameCnt,                 // out buffer
        sizeof(UINT32),            // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}

