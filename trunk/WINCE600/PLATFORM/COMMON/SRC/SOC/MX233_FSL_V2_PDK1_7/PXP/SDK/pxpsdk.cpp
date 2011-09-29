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
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pxpsdk.c
//
//  This module provides wrapper functions for accessing
//  the stream interface for the PXP driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#pragma warning(pop)

#include "pxp.h"


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
// Function: PXPOpenHandle
//
// This method creates a handle to the PXP stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to PXP driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE PXPOpenHandle(void)
{
    HANDLE hPXP;

    hPXP = CreateFile(TEXT("PXP1:"),        // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to PXP
    if (hPXP == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(1, (TEXT("CreateFile PXP failed!\r\n")));
    }

    return hPXP;
}


//------------------------------------------------------------------------------
//
// Function: PXPCloseHandle
//
// This method closes a handle to the PXP stream driver.
//
// Parameters:
//      hPXP
//          [in] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPCloseHandle(HANDLE hPXP)
{
    BOOL retVal = TRUE;

    // if we don't have handle to PXP driver
    if (hPXP != NULL)
    {
        if (!CloseHandle(hPXP))
        {
            retVal = FALSE;
        }
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: PXPConfigureGeneral
//
// This method configures the Pixel Pipeline general parameters passed
// in by the caller.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pPpConfigData
//          [in] Pointer to PXP configuration data structure.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPConfigureGeneral(HANDLE hPXP, pPxpParaConfig pParaConfig)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to configure the PXP
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_CONFIG_GENERAL,       // I/O control code
        pParaConfig,              // in buffer
        sizeof(pxpParaConfig),    // in buffer size
        NULL,                     // out buffer
        0,                        // out buffer size
        0,                        // number of bytes returned
        NULL))                    // ignored (=NULL)
    {
        retVal = FALSE;
        RETAILMSG(1, (TEXT("PXP_IOCTL_CONFIG_GENERAL failed!\r\n")));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: PXPStartProcess
//
// This method starts the Pixel Pipeline task.
//
// Parameters:
//      hPxP
//          [in] Handle to PxP driver.
//
//      bWaitForCompletion
//          [in] Indicate whether the caller need to wait for the completion 
//               of PXP operation
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPStartProcess(HANDLE hPxP, BOOL bWaitForCompletion)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to start the PXP task
    if (!DeviceIoControl(hPxP,     // file handle to the driver
        PXP_IOCTL_START_PROCESS,            // I/O control code
        &bWaitForCompletion,                      // in buffer
        sizeof(BOOL),                         // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
        RETAILMSG(1, (TEXT("PXP_IOCTL_START_PROCESS failed!\r\n")));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: PXPSetOutputBuffer1Addr
//
// This method allows the caller to set the output buffer1 address
// for PXP.              
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      PhysBuf
//          [in] PXP output Buffer1 physical address.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPSetOutputBuffer1Addr(HANDLE hPXP, UINT32 PhysBuf)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_OUTPUTBUFFER1_ADDRESS,  // I/O control code
        &PhysBuf,                     // in buffer
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


//------------------------------------------------------------------------------
//
// Function: PXPSetOutputBuffer2Addr
//
// This method allows the caller to set the output buffer1 address
// for PXP.              
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      PhysBuf
//          [in] PXP output Buffer1 physical address.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPSetOutputBuffer2Addr(HANDLE hPXP, UINT32 PhysBuf)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_OUTPUTBUFFER2_ADDRESS,  // I/O control code
        &PhysBuf,                     // in buffer
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


//------------------------------------------------------------------------------
//
// Function: PXPSetS0BufferAddrGroup
//
// This method allows the caller to set S0 buffer address
// for PXP.              
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pAddrGroup
//          [in] PXP S0 buffer address structure.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPSetS0BufferAddrGroup(HANDLE hPXP, pPxpS0BufferAddrGroup pAddrGroup)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_S0BUFFER_ADDRESS,  // I/O control code
        pAddrGroup,                     // in buffer
        sizeof(pxpS0BufferAddrGroup),            // in buffer size
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
// Function: PXPInterruptEnable
//
// This method allows the caller to enable interrupt of Pixel Pipeline driver.
//
// Parameters:
//      hPxP
//          [in] Handle to PXP driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPInterruptEnable(HANDLE hPXP)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_ENABLE_INTERRUPT,    // I/O control code
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
// Function: PXPWaitForCompletion
//
// This method allows the caller to wait for the completion of PXP.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPWaitForCompletion(HANDLE hPXP)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_WAIT_COMPLETION,    // I/O control code
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
// Function: PXPInterruptDisable
//
// This method allows the caller to disable interrupt of Pixel Pipeline driver.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPInterruptDisable(HANDLE hPXP)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_DISABLE_INTERRUPT,    // I/O control code
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
// Function: PXPSetS0BufferOffsetInOutput
//
// This method allows the caller to set PXP S0 buffer offset location within the output frame buffer.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pCoordinate:
//          [in] S0 buffer offset location coordinate
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPSetS0BufferOffsetInOutput(HANDLE hPXP, pPxpCoordinate pCoordinate)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_S0BUFFER_OFFSETINOUTPUT,    // I/O control code
        pCoordinate,                      // in buffer
        sizeof(pxpCoordinate),            // in buffer size
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
// Function: PXPSetS0BufferColorKey
//
// This method allows the caller to set PXP S0 buffer color key.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pPxpColorKey:
//          [in] S0 buffer color key
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPSetS0BufferColorKey(HANDLE hPXP, pPxpColorKey pColorKey)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_S0BUFFER_COLORKEY,    // I/O control code
        pColorKey,                      // in buffer
        sizeof(pxpColorKey),            // in buffer size
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
// Function: PXPSetOverlayBuffersAddr
//
// This method allows the caller to set one of the PXP Overlay buffer address into hardware.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pBufsAddr:
//          [in] Overlay buffer address structure pointer
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPSetOverlayBuffersAddr(HANDLE hPXP, pPxpOverlayBuffersAddr pBufsAddr)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_OVERLAYBUFFERS_ADDRESS,    // I/O control code
        pBufsAddr,                      // in buffer
        sizeof(pxpOverlayBuffersAddr),            // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function:  PXPSetOverlayBuffersPos
//
// This function sets one of the PXP Overlay buffer position into hardware.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pBufsPos
//          [in] Overlay buffer position structure pointer.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PXPSetOverlayBuffersPos(HANDLE hPXP, pPxpOverlayBuffersPos pBufsPos)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_OVERLAYBUFFERS_POSITION,    // I/O control code
        pBufsPos,                      // in buffer
        sizeof(pxpOverlayBuffersPos),            // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function: PXPSetS0BufProperty
//
// This function configures S0 buffer property for pixel pipeline.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pS0Property
//          [in] Pointer to S0 buffer property data structure
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PXPSetS0BufProperty(HANDLE hPXP, pPxpS0Property pS0Property)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_S0BUFFER_PROPERTY,    // I/O control code
        pS0Property,                      // in buffer
        sizeof(pxpS0Property),            // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function:  PXPSetS0BufferSize
//
// This function sets PXP S0 buffer size.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pRectSize
//          [in] S0 buffer size.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PXPSetS0BufferSize(HANDLE hPXP, pPxpRectSize pRectSize)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_S0BUFFER_SIZE,    // I/O control code
        pRectSize,                      // in buffer
        sizeof(pxpRectSize),            // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function: PXPSetOverlayBufsProperty
//
// This function configures overlay property for pixel pipeline.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pPxpOverlayProperty
//          [in] Pointer to overlay property data structure
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PXPSetOverlayBufsProperty(HANDLE hPXP, pPxpOverlayProperty pOverlayProperty)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_OVERLAYBUFFERS_PROPERTY,    // I/O control code
        pOverlayProperty,                      // in buffer
        sizeof(pxpOverlayProperty),            // in buffer size
        NULL,                      // out buffer
        0,                         // out buffer size
        0,                         // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        retVal = FALSE;
    }

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function: PxpSetS0BufProperty
//
// This function configures S0 buffer property for pixel pipeline.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pS0Property
//          [in] Pointer to S0 buffer property data structure
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//-----------------------------------------------------------------------------
BOOL PXPSetOutputProperty(HANDLE hPXP, pPxpOutProperty pOutProperty)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_OUTPUT_PROPERTY,    // I/O control code
        pOutProperty,                      // in buffer
        sizeof(pxpOutProperty),            // in buffer size
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
// Function: PXPSetOverlayColorKey
//
// This method allows the caller to set PXP overlay color key.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
//      pPxpColorKey:
//          [in] Overlay color key
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PXPSetOverlayColorKey(HANDLE hPXP, pPxpColorKey pColorKey)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to get the PXP status register
    if (!DeviceIoControl(hPXP,     // file handle to the driver
        PXP_IOCTL_SET_OVERLAY_COLORKEY,    // I/O control code
        pColorKey,                      // in buffer
        sizeof(pxpColorKey),            // in buffer size
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
// Function: PXPResetDriverStatus
//
// This method allows the caller to reset PXP driver status.
//
// Parameters:
//      hPXP
//          [in] Handle to PXP driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------

BOOL PXPResetDriverStatus(HANDLE hPxP)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to reset the PXP driver status
    if (!DeviceIoControl(hPxP,     // file handle to the driver
        PXP_IOCTL_RESET_DRIVER_STATUS,   // I/O control code
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

