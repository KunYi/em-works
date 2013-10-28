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
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  hwentropy.c
//
//  This file implements the IOCTL_HAL_GET_HWENTROPY handler.
//
#include <windows.h>
#include <oal.h>
#include <csp.h>


//------------------------------------------------------------------------------
// External Functions
extern VOID OALClockEnableIIM(BOOL bClockEnable);

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetHWEntropy
//
//  Implements the IOCTL_HAL_GET_HWENTROPY handler. This function creates a
//  64-bit value which is unique to the hardware.  This value never changes.
//
BOOL OALIoCtlHalGetHWEntropy( 
    UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    UINT32 *SiliconUID = OALPAtoUA(CSP_BASE_REG_PA_IIM + 0x0814);
    UINT32 SiID[2] ={0,0};
    BOOL rc = FALSE;
    int i;

    // Check buffer size
    if (lpBytesReturned != NULL) 
    {
        *lpBytesReturned = sizeof(SiID);
    }

    if (lpOutBuf == NULL || nOutBufSize < sizeof(SiID))
    {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetHWEntropy: Buffer too small\r\n"));
    }
    else
    {
        // Turn on IIM module clocks so we can read fuse bank
        OALClockEnableIIM(TRUE);
        
        // Get Silicon ID
        for(i=0; i<3; i++)
        {
            SiID[0] <<= 8;
            SiID[0] |= INREG32(&SiliconUID[i]);
            SiID[1] <<= 8;
            SiID[1] |= INREG32(&SiliconUID[i+3]);
        }

        // Disable clocks to save power
        OALClockEnableIIM(FALSE);

        // Copy pattern to output buffer
        memcpy(lpOutBuf, SiID, sizeof(SiID));

        // We are done
        rc = TRUE;
    }

    // Indicate status
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetHWEntropy(rc = %d)\r\n", rc));
    return rc;
}

