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
//  File:  ioctl.c
//
//  File implements OEMIoControl function.
//
#include <windows.h>
#include <oal.h>

//------------------------------------------------------------------------------
//
//  Global:  g_ioctlState;
//
//  This state variable contains critical section used to serialize IOCTL
//  calls.
//
static struct {
    BOOL postInit;
    CRITICAL_SECTION cs;
} g_ioctlState = { FALSE };


__declspec(noinline) BOOL DoOEMIoControlWithCS (
    DWORD dwIndex,
    DWORD code, VOID *pInBuffer, DWORD inSize, VOID *pOutBuffer, DWORD outSize,
    DWORD *pOutSize
) {
    BOOL rc = FALSE;
    // Take critical section            
    EnterCriticalSection(&g_ioctlState.cs);
    __try {
        // Execute the handler
        rc = g_oalIoCtlTable[dwIndex].pfnHandler(
            code, pInBuffer, inSize, pOutBuffer, outSize, (UINT32 *)pOutSize
        );
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        rc = FALSE;
        NKSetLastError(ERROR_INVALID_PARAMETER);
    }
    // Release critical section            
    LeaveCriticalSection(&g_ioctlState.cs);
    return rc;

}

//------------------------------------------------------------------------------
//
//  Function:  OEMIoControl
//
//  The function is called by kernel a device driver or application calls 
//  KernelIoControl. The system is fully preemtible when this function is 
//  called. The kernel does no processing of this API. It is provided to 
//  allow an OEM device driver to communicate with kernel mode code.
//
BOOL OEMIoControl(
    DWORD code, VOID *pInBuffer, DWORD inSize, VOID *pOutBuffer, DWORD outSize,
    DWORD *pOutSize
) {
    BOOL rc = FALSE;
    UINT32 i;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (
        L"+OEMIoControl(0x%x, 0x%x, %d, 0x%x, %d, 0x%x)\r\n", 
        code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
    ));

    //Initialize g_ioctlState.cs when IOCTL_HAL_POSTINIT is called. By this time, 
    //the kernel is up and ready to handle the critical section initialization.
    if (!g_ioctlState.postInit && code == IOCTL_HAL_POSTINIT) {
        // Initialize critical section
        InitializeCriticalSection(&g_ioctlState.cs);
        g_ioctlState.postInit = TRUE;
    } 

    // Search the IOCTL table for the requested code.
    for (i = 0; g_oalIoCtlTable[i].pfnHandler != NULL; i++) {
        if (g_oalIoCtlTable[i].code == code) break;
    }

    // Indicate unsupported code
    if (g_oalIoCtlTable[i].pfnHandler == NULL) {
        NKSetLastError(ERROR_NOT_SUPPORTED);
        OALMSG(OAL_IOCTL, (
            L"OEMIoControl: Unsupported Code 0x%x - device 0x%04x func %d\r\n", 
            code, code >> 16, (code >> 2)&0x0FFF
        ));
        goto cleanUp;
    }        

    // Take critical section if required (after postinit & no flag)
    if (
        g_ioctlState.postInit && 
        (g_oalIoCtlTable[i].flags & OAL_IOCTL_FLAG_NOCS) == 0
    ) {
        rc = DoOEMIoControlWithCS(i,code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
    } 
    else {
        rc = g_oalIoCtlTable[i].pfnHandler(
            code, pInBuffer, inSize, pOutBuffer, outSize, (UINT32 *)pOutSize
        );
    }

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OEMIoControl(rc = %d)\r\n", rc ));
    return rc;
}

//------------------------------------------------------------------------------
