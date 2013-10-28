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
//  File:  pfsdk.c
//
//  This module provides wrapper functions for accessing
//  the stream interface for the PF driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "mxarm11.h"
#include "pf.h"


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
// Function: PFOpenHandle
//
// This method creates a handle to the PF stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to PF driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE PFOpenHandle(void)
{
    HANDLE hPF;

    hPF = CreateFile(TEXT("POF1:"),        // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to PF
    if (hPF == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(1, (TEXT("%s:  CreateFile PF failed!\r\n"), __WFUNCTION__));
    }

    return hPF;
}


//------------------------------------------------------------------------------
//
// Function: PFCloseHandle
//
// This method closes a handle to the PF stream driver.
//
// Parameters:
//      hPF
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PFCloseHandle(HANDLE hPF)
{
    BOOL retVal = TRUE;

    // if we don't have handle to PF driver
    if (hPF != NULL)
    {
        if (!CloseHandle(hPF))
        {
            retVal = FALSE;
        }
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: PFConfigure
//
// This method configures the Post-filter driver using parameters passed
// in by the caller.
//
// Parameters:
//      hPF
//          [in] Handle to PF driver.
//
//      pPfConfigData
//          [in] Pointer to PF configuration data structure.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PFConfigure(HANDLE hPF, pPfConfigData pConfigData)
{
    // issue the IOCTL to configure the PF
    if (!DeviceIoControl(hPF,     // file handle to the driver
        PF_IOCTL_CONFIGURE,       // I/O control code
        pConfigData,              // in buffer
        sizeof(pfConfigData),     // in buffer size
        NULL,                     // out buffer
        0,                        // out buffer size
        0,                        // number of bytes returned
        NULL))                    // ignored (=NULL)
    {
        ERRORMSG(1, (TEXT("%s: PF_IOCTL_CONFIGURE failed!\r\n"), __WFUNCTION__));
    }
}


//------------------------------------------------------------------------------
//
// Function: PFStart
//
// This method starts the Post-filter task.
//
// Parameters:
//      hPF
//          [in] Handle to PF driver.
//
//      pStartParms
//          [in] Pointer to structure containing input and output buffer data
//          for the Post-filter operation.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PFStart(HANDLE hPF, pPfStartParams pStartParms)
{
    // issue the IOCTL to start the PF task
    if (!DeviceIoControl(hPF,     // file handle to the driver
        PF_IOCTL_START,            // I/O control code
        pStartParms,               // in buffer
        sizeof(pfStartParams),     // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(1, (TEXT("%s: PF_IOCTL_START failed!\r\n"), __WFUNCTION__));
    }
}

// for Direct Render
//------------------------------------------------------------------------------
//
// Function: PFStart2
//
// This method starts the Post-filter task.
//
// Parameters:
//      hPF
//          [in] Handle to PF driver.
//
//      pStartParms
//          [in] Pointer to structure containing input and output buffer data
//          for the Post-filter operation.
//
//      VirtualFlag
//          [in] Flag to indicate if virtual memory pointer
//
//      yOffset
//          [in] offset to the pointer, in byte
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PFStart2(HANDLE hPF, pPfStartParams pStartParms,unsigned int VirtualFlag,unsigned int yOffset)
{
    // issue the IOCTL to start the PF task
    if (!DeviceIoControl(hPF,     // file handle to the driver
        PF_IOCTL_START2,            // I/O control code
        (LPVOID)pStartParms,               // in buffer
        sizeof(pfStartParams),     // in buffer size
        (LPVOID)&VirtualFlag,                      // dummy out buffer
        VirtualFlag,                         // out buffer size
        (PDWORD)&yOffset,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(1, (TEXT("%s: PF_IOCTL_START failed!\r\n"), __WFUNCTION__));
    }
}

//------------------------------------------------------------------------------
//
// Function: PFSetAttributeEx
//
// This method call VirtualSetAttributesEx() to set the virtual memory attributes.
//
// Parameters:
//      hPF
//          [in] Handle to PF driver.
//
//      pData
//          [in] Pointer to VirtualSetAttributesEx data structure
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
BOOL PFSetAttributeEx(HANDLE hPF, pPfSetAttributeExData pData)
{

    // issue the IOCTL to start the PF task
    if (!DeviceIoControl(hPF,     // file handle to the driver
            PF_IOCTL_SETATTEX,            // I/O control code
            (LPVOID)pData,               // in buffer
            sizeof(pfSetAttributeExData),     // in buffer size
            NULL,                     // out buffer
            0,                        // out buffer size
            0,                        // number of bytes returned
            NULL))                    // ignored (=NULL)
    {
        DEBUGMSG(1, (TEXT("%s: PFSetAttributeEx failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;
}
    

//------------------------------------------------------------------------------
//
// Function: PFResume
//
// This method resumes an H.264 deblocking operation that was previously 
// started with a pause row specified.  A new pause row may be specified
//
// Parameters:
//      hPF
//          [in] Handle to PF driver.
//
//      h264_pause_row
//          [in] Integer indicating the Y row at which to pause the operation.
//          Should be set to 0 to disable an additional pause.
//
// Returns:
//      If success, PF_SUCCESS.
//      If failure, one of the following:
//          PF_ERR_NOT_RUNNING
//          PF_ERR_PAUSE_NOT_ENABLED
//          PF_ERR_INVALID_PARAMETER
//
//------------------------------------------------------------------------------
DWORD PFResume(HANDLE hPF, UINT32 h264_pause_row)
{
    DWORD retVal;

    // issue the IOCTL to resume the PF task
    if (!DeviceIoControl(hPF,      // file handle to the driver
        PF_IOCTL_RESUME,           // I/O control code
        &h264_pause_row,           // in buffer
        sizeof(UINT32),            // in buffer size
        &retVal,                   // out buffer
        sizeof(DWORD),             // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        DEBUGMSG(1, (TEXT("%s: PF_IOCTL_RESUME failed!\r\n"), __WFUNCTION__));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
//  Function:  PFAllocPhysMem
//
//  This function allocates physically contiguous memory
//  
//  Parameters: 
//      hPF
//          [in] a file handle to the PF driver 
//      size   
//          [in] Number of bytes to allocate
//
//      pBitsStreamBufMemParams
//          [out] Pointer to a pPfAllocMemoryParams that stores 
//                the memory paramters of the memory allocation.
//
//  Returns: If the allocation is sucessful, return TRUE, otherwise, return FALSE. 
// 
//------------------------------------------------------------------------------
BOOL PFAllocPhysMem(HANDLE hPF, UINT32 size,  pPfAllocMemoryParams pBitsStreamBufMemParams)
{
    BOOL bResult = FALSE;
    pfAllocMemoryParams bufMemParams = {0, 0, 0};

    // issue the IOCTL to allocate a physical memory
    bResult = DeviceIoControl(hPF,           // file handle to the driver
        PF_IOCTL_ALLOC_PHY_MEM,         // I/O control code
        &size,                          // in buffer
        sizeof(DWORD),                  // in buffer size
        &bufMemParams,                  // out buffer
        sizeof(pfAllocMemoryParams),    // out buffer size
        0,                              // number of bytes returned
        NULL);                          // ignored (=NULL)
    
    if (!bResult)
    {
        DEBUGMSG(1, (TEXT("%s: PF_IOCTL_ALLOC_PHY_MEM failed!\r\n"), __WFUNCTION__));
    }
    
    *pBitsStreamBufMemParams = bufMemParams;
    
    return bResult;
}

//------------------------------------------------------------------------------
//
//  Function:  PFFreePhysMem
//
//  This function releases physical memory back to the system
//  
//  Parameters:
//      hPF
//          [in] a file handle to the PF driver 
//
//      bitsStreamBufMemParams 
//          [in] Virtual memory address parameters returned from PFAllocPhysMem
//
//  Return Values:
//      If the allocation failed, then FALSE is returned; 
//      otherwise, TRUE is returned.
//
//------------------------------------------------------------------------------

BOOL PFFreePhysMem(HANDLE hPF, pfAllocMemoryParams bitsStreamBufMemParams)
{
    BOOL bResult = FALSE;
    
    // issue the IOCTL to free a physical memory
    bResult = DeviceIoControl(hPF,      // file handle to the driver
        PF_IOCTL_FREE_PHY_MEM,          // I/O control code
        &bitsStreamBufMemParams,        // in buffer
        sizeof(pfAllocMemoryParams),    // in buffer size
        0,                              // out buffer
        0,                              // out buffer size
        0,                              // number of bytes returned
        NULL);                          // ignored (=NULL)
    
    if (!bResult)
    {
        DEBUGMSG(1, (TEXT("%s: PF_IOCTL_FREE_PHY_MEM failed!\r\n"), __WFUNCTION__));
    }
    
    return bResult;
}


