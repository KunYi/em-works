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

//-----------------------------------------------------------------------------
// External Functions
extern PCSP_CRM_REGS g_pCRM;
// For Watchdog Support
UINT32 WdogInit(UINT32 TimeoutMSec);


//-----------------------------------------------------------------------------
// External Variables
extern DWORD g_dwResetCause;


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

    //OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReboot\r\n"));
    OALMSG(1, (L"+OALIoCtlHalReboot\r\n"));

    //WdogInit(500);
    WdogInit(0);
    for(;;);    
}

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlGetResetCause
//
BOOL OALIoCtlGetResetCause(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{    
    BOOL rc = FALSE;

    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);

    if ((pOutBuffer != NULL) && (outSize >= sizeof(DWORD)))
    {
        rc = TRUE;

        *(DWORD*)pOutBuffer = g_dwResetCause;        
        if (pOutSize)
        {
            *pOutSize = sizeof(DWORD);
        }
        
    }

    return rc;
}
