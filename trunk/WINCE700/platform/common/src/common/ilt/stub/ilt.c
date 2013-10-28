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
//  File:  ilt.c
//
#include <windows.h>
#include <oal.h>

//------------------------------------------------------------------------------
//
// Global:  g_oalILT
//
OAL_ILT_STATE g_oalILT;


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalILTiming
//
//  This function is called to start/stop IL timing. It is called from
//  OEMIoControl for IOCTL_HAL_ILTIMING code.
//
BOOL OALIoCtlHalILTiming( 
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    UNREFERENCED_PARAMETER(code);    
    UNREFERENCED_PARAMETER(pInpBuffer);    
    UNREFERENCED_PARAMETER(inpSize);    
    UNREFERENCED_PARAMETER(pOutBuffer);    
    UNREFERENCED_PARAMETER(outSize);    
    UNREFERENCED_PARAMETER(pOutSize);    

    NKSetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

//------------------------------------------------------------------------------
