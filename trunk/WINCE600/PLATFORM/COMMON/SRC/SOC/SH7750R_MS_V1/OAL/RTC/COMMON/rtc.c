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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  rtc.c
//
#include <windows.h>
#include <oal.h>
#include <nkintr.h>
#include "shx.h"

BOOL Bare_SetRealTime(LPSYSTEMTIME lpst);
BOOL Bare_GetRealTime(LPSYSTEMTIME lpst);

//------------------------------------------------------------------------------
//
//  Function:  OEMGetRealTime
//
//  This function is called by the kernel to retrieve the time from
//  the real-time clock.
//
BOOL OEMGetRealTime(LPSYSTEMTIME lpst)
{
    BOOL RetVal, PrevState;

    PrevState = INTERRUPTS_ENABLE(FALSE);
    RetVal = Bare_GetRealTime(lpst);
    INTERRUPTS_ENABLE(PrevState);

    return RetVal;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMSetRealTime
//
//  This function is called by the kernel to set the time for
//  the real-time clock.
//
BOOL OEMSetRealTime(LPSYSTEMTIME lpst)
{
    BOOL RetVal, PrevState;

    PrevState = INTERRUPTS_ENABLE(FALSE);
    RetVal = Bare_SetRealTime(lpst);
    INTERRUPTS_ENABLE(PrevState);

    return RetVal;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalInitRTC
//
//  This function is called by WinCE OS to initialize the time after boot. 
//  Input buffer contains SYSTEMTIME structure with default time value. If
//  hardware has persistent real time clock it will ignore this value
//  (or all call).
//
BOOL OALIoCtlHalInitRTC(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
) {
    SYSTEMTIME *lpst   = NULL;
    BOOL       retVal  = FALSE;

    OALMSG(OAL_RTC&&OAL_FUNC, (L"+OALIoCtlHalInitRTC()\r\n"));

    if(pOutSize)
    { 
        *pOutSize = 0;
    }

    // Validity checks
    if((code       != IOCTL_HAL_INIT_RTC) ||
       (pInpBuffer == NULL)               ||
       (inpSize    != sizeof(SYSTEMTIME)))
    {
        OALMSG(OAL_ERROR, (L"ERROR: Invalid calling parameters...returning\r\n"));
        retVal = FALSE;
        goto initExit;
    }

    // Call OEMSetRealTime
    lpst    = (SYSTEMTIME *)pInpBuffer;
    retVal  = OEMSetRealTime(lpst); 

initExit:
    OALMSG(OAL_RTC&&OAL_FUNC, (L"-OALIoCtlHalInitRTC(rc = %d)\r\n", retVal));
    return retVal;
}

