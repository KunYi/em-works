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
//  File:  reginit.c
//
//  This file implements the IOCTL_HAL_INITREGISTRY handler.
//
#include <windows.h>
#include <oal.h>

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRegistry
//
//  Implements the IOCTL_HAL_INITREGISTRY handler.

BOOL OALIoCtlHalInitRegistry( 
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    // Default implementation doesn't do anything

    // KITL registry initialization is done in kitl.dll's IOCTL_HAL_INITREGISTRY handler
    
    UNREFERENCED_PARAMETER(code);    
    UNREFERENCED_PARAMETER(pInpBuffer);    
    UNREFERENCED_PARAMETER(inpSize);    
    UNREFERENCED_PARAMETER(pOutBuffer);    
    UNREFERENCED_PARAMETER(outSize);    
    UNREFERENCED_PARAMETER(pOutSize);    

    return TRUE;
}

//------------------------------------------------------------------------------
