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
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  vdisdk.c
//
//  This module provides wrapper functions for accessing
//  the stream interface for the VDI driver.
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
#include "vdi.h"


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
// Function: VDIOpenHandle
//
// This method creates a handle to the VDI stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to VDI driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE VDIOpenHandle(void)
{
    HANDLE hVDI;

    hVDI = CreateFile(TEXT("VDI1:"),        // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to VDI
    if (hVDI == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(1, (TEXT("%s:  CreateFile VDI failed!\r\n"), __WFUNCTION__));
        hVDI = NULL;
    }

    return hVDI;
}


//------------------------------------------------------------------------------
//
// Function: VDICloseHandle
//
// This method closes a handle to the VDI stream driver.
//
// Parameters:
//      hVDI
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL VDICloseHandle(HANDLE hVDI)
{
    BOOL retVal = TRUE;

    // if we don't have handle to VDI driver
    if (hVDI != NULL)
    {
        if (!CloseHandle(hVDI))
        {
            retVal = FALSE;
        }
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: VDIConfigure
//
// This method configures the Video De-Interlacer using parameters passed
// in by the caller.
//
// Parameters:
//      hVDI
//          [in] Handle to VDI driver.
//
//      pVDIConfigData
//          [in] Pointer to VDI configuration data structure.
//
// Returns:
//      TRUE of FALSE.
//
//------------------------------------------------------------------------------
BOOL VDIConfigure(HANDLE hVDI, pVdiConfigData pConfigData)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to configure the VDI
    if (!DeviceIoControl(hVDI,     // file handle to the driver
        VDI_IOCTL_CONFIGURE,       // I/O control code
        pConfigData,              // in buffer
        sizeof(vdiConfigData),    // in buffer size
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
// Function: VDIStart
//
// This method starts the Video De-Interlacer task.
//
// Parameters:
//      hVDI
//          [in] Handle to VDI driver.
//
//      pInputBuf
//          [in] Pointer to destination buffer for VDI task.
//
// Returns:
//      TRUE of FALSE.
//
//------------------------------------------------------------------------------
BOOL VDIStart(HANDLE hVDI, pVdiStartParams pStartParams)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to start the VDI task
    if (!DeviceIoControl(hVDI,     // file handle to the driver
        VDI_IOCTL_START,           // I/O control code
        pStartParams,              // in buffer
        sizeof(vdiStartParams),    // in buffer size
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
// Function: VDIStop
//
// This method stops the Video De-Interlacer.
//
// Parameters:
//      hVDI
//          [in] Handle to VDI driver.
//
// Returns:
//      TRUE of FALSE.
//
//------------------------------------------------------------------------------
BOOL VDIStop(HANDLE hVDI)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the VDI status register
    if (!DeviceIoControl(hVDI,     // file handle to the driver
        VDI_IOCTL_STOP,             // I/O control code
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
// Function: VDIWaitForNotBusy
//
// This method allows the caller to wait for certain interrupt of
// Video De-Interlacer driver.
//
// Parameters:
//      hVDI
//          [in] Handle to VDI driver.
//
//      timeout
//          [in] Time to wait before returning, in milliseconds.
//
// Returns:
//      TRUE if success.
//      FALSE if failure or if timeout is reached.
//
//------------------------------------------------------------------------------
BOOL VDIWaitForNotBusy(HANDLE hVDI, UINT32 timeout)
{
    BOOL retVal = FALSE;

    // issue the IOCTL to get the VDI status register
    if (!DeviceIoControl(hVDI,     // file handle to the driver
        VDI_IOCTL_WAIT_NOT_BUSY,    // I/O control code
        &timeout,                  // in buffer
        sizeof(UINT8),             // in buffer size
        &retVal,                   // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}
