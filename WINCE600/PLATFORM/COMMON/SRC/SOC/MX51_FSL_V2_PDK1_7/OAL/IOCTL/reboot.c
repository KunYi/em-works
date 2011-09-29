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
//  File:  reboot.c
//
//  This file implements the IOCTL_HAL_REBOOT handler.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4214 4201)
#include <windows.h>
#include <oal.h>
#pragma warning(pop)

#include <csp.h>

// For Watchdog Support
UINT32 WdogInit(UINT32 TimeoutMSec);

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReboot
//
BOOL OALIoCtlHalReboot(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReboot\r\n"));
    
    // Set watchdog timer to timeout in 0.5 sec
    WdogInit(500);

    // Wait for reset...
    //
    for (;;);
    
#if 0    // Should never get to this point...
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalReboot\r\n"));

    return TRUE;
#endif
}
