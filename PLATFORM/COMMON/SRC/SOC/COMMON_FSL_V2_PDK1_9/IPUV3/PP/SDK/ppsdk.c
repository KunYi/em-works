//-----------------------------------------------------------------------------
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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "common_macros.h"
#include "ipu_common.h"
#include "tpm.h"
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
        ERRORMSG(1, (TEXT("%s:  CreateFile PP failed!It is used by other module.\r\n"), __WFUNCTION__));
        ASSERT(FALSE);        
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
//      TRUE of FALSE.
//
//------------------------------------------------------------------------------
BOOL PPConfigure(HANDLE hPP, pPpConfigData pConfigData)
{
    BOOL retVal = TRUE;
    
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
        retVal = FALSE;
    }

    return retVal;

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
//      TRUE of FALSE.
//
//------------------------------------------------------------------------------
BOOL PPStart(HANDLE hPP)
{
    BOOL retVal = TRUE;
    
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
        retVal = FALSE;
    }

    return retVal;

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
//      TRUE of FALSE.
//
//------------------------------------------------------------------------------
BOOL PPStop(HANDLE hPP)
{
    BOOL retVal = TRUE;
    
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
        retVal = FALSE;
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: PPAddInputBuffer
//
// This method allows the caller to add input buffers to
// the PP buffer queues.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      PhysBuf
//          [in] Pointer to input Buffer physical address.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPAddInputBuffer(HANDLE hPP, UINT32 PhysBuf)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_ADD_INPUT_BUFFER,  // I/O control code
        &PhysBuf,                     // in buffer
        sizeof(UINT32),         // in buffer size
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
// Function: PPAddInputCombBuffer
//
// This method allows the caller to add second input buffers to
// the PP buffer queues for combining.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      PhysBuf
//          [in] Pointer to input Buffer physical address.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPAddInputCombBuffer(HANDLE hPP, UINT32 PhysBuf)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_ADD_INPUT_COMBBUFFER,  // I/O control code
        &PhysBuf,                     // in buffer
        sizeof(UINT32),         // in buffer size
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
// Function: PPAddOutputBuffer
//
// This method allows the caller to add output buffers to
// the PP buffer queues.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      PhysBuf
//          [in] Pointer to output Buffer physical address.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPAddOutputBuffer(HANDLE hPP, UINT32 PhysBuf)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_ADD_OUTPUT_BUFFER,  // I/O control code
        &PhysBuf,                     // in buffer
        sizeof(UINT32),         // in buffer size
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
// Function: PPInterruptEnable
//
// This method allows the caller to enable certain interrupt of Post-processing driver.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      IntType
//          [in] A flag structure for interrupt type.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPInterruptEnable(HANDLE hPP, UINT8 IntType)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_ENABLE_INTERRUPT,    // I/O control code
        &IntType,                      // in buffer
        sizeof(UINT8),                         // in buffer size
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
// Function: PPWaitForNotBusy
//
// This method allows the caller to wait for certain interrupt of Post-processing driver.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      IntType
//          [in] A flag structure for interrupt type.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPWaitForNotBusy(HANDLE hPP, UINT8 IntType)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_WAIT_NOT_BUSY,    // I/O control code
        &IntType,                      // in buffer
        sizeof(UINT8),                         // in buffer size
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
// Function: PPInterruptDisable
//
// This method allows the caller to disable certain interrupt of Post-processing driver.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      IntType
//          [in] A flag structure for interrupt type.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPInterruptDisable(HANDLE hPP, UINT8 IntType)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_DISABLE_INTERRUPT,    // I/O control code
        &IntType,                      // in buffer
        sizeof(UINT8),                         // in buffer size
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
// Function: PPSetWindowPos
//
// This method allows the caller to set the active window position when mask channel is enabled.
//
// Parameters:
//      hPP
//          [in] Handle to PP driver.
//
//      pPos:
//          [in] the active window in mask buffer
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PPSetWindowPos(HANDLE hPP, RECT * pPos)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PP status register
    if (!DeviceIoControl(hPP,     // file handle to the driver
        PP_IOCTL_SET_WINDOW_POS,    // I/O control code
        pPos,                      // in buffer
        sizeof(UINT32),            // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}

