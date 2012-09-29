//------------------------------------------------------------------------------
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
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bspkeypad.cpp
//
//  This module provides a stream interface for the KeyPad driver
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 6001 6385)
#include <windows.h>
#pragma warning(pop)

#include "keypad.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
//#ifdef DEBUG
//DBGPARAM dpCurSettings = {
//    _T("KPD"),
//    {
//        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Func"),
//        TEXT("Info"), TEXT(""), TEXT(""), TEXT(""),
//        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
//        TEXT(""),TEXT(""),TEXT(""),TEXT("")
//    },
//    ZONEMASK_ERROR | ZONEMASK_WARN // ulZoneMask
////0xffff
//};
//#endif  // DEBUG
//-----------------------------------------------------------------------------
// Local Variables
BSP_KEYPAD      *KeyPadDriver = NULL;
//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
//  Function: DllMain
//
//  This function called when the DLL is loaded.
//
//  Parameters:
//
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
extern BOOL WINAPI KEY_DllEntry(
    HANDLE hInstDll, DWORD dwReason, VOID* lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        //DEBUGREGISTER((HMODULE)hInstDll);
        DEBUGMSG(1 /*ZONE_INIT*/, (_T("***** DLL PROCESS ATTACH TO KEYPAD DLL *****\r\n")));
        DisableThreadLibraryCalls((HMODULE) hInstDll);
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    // return TRUE for success
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: KPD_Init
//
// The Device Manager calls this function as a result of a call to the
//      ActivateDevice() function.
//
//  Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//                active key for the stream interface driver.
//
//  Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD KPD_Init(LPCTSTR pContext,DWORD dwContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    DEBUGMSG(1,(TEXT("KPD_Init ++\r\n")));

    KeyPadDriver =(BSP_KEYPAD *) new BSP_KEYPAD;

    if(KeyPadDriver == NULL)
    {
        ERRORMSG(1, (TEXT("KEY_Init alloc fail\r\n")));
        goto KEY_InitFail;
    }
    if(KeyPadDriver->Init(dwContext) == FALSE)
    {
        ERRORMSG(1, (TEXT("KEY_Init Create\r\n")));
        KeyPadDriver->DeInit();
    }

    DEBUGMSG(1, (TEXT("KeyPad Initial Successed\r\n")));

KEY_InitFail:

    DEBUGMSG(0,(TEXT("KPD_Init --\r\n")));

    return (DWORD) KeyPadDriver;
}

//-----------------------------------------------------------------------------
//
//  Function: KPD_Deinit
//
// This function uninitializes a device.
//
//  Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL KPD_Deinit(DWORD hDeviceContext)
{
    BOOL bRetVal = TRUE;
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);

    return bRetVal;
}

//-----------------------------------------------------------------------------
//
//  Function: KPD_Open
//
// This function opens a device for reading, writing, or both.
//
//  Parameters:
//      hDeviceContext
//          [in] Handle to the device context. The XXX_Init function creates
//                and returns this handle.
//      AccessCode
//          [in] Access code for the device. The access is a combination of
//                read and write access from CreateFile.
//      ShareMode
//          [in] File share mode of the device. The share mode is a
//                combination of read and write access sharing from CreateFile.
//
//  Returns:
//      This function returns a handle that identifies the open context of
//      the device to the calling application.
//
//-----------------------------------------------------------------------------
DWORD KPD_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DWORD dwRetVal = 1;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);
    
    return dwRetVal;
}
//-----------------------------------------------------------------------------
//
//  Function: KPD_Close
//
// This function opens a device for reading, writing, or both.
//
//  Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to identify
//                the open context of the device.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL KPD_Close(DWORD hOpenContext )
{
    BOOL bRetVal = TRUE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    DEBUGMSG(0,(TEXT("KPD_Close ++\r\n")));
    DEBUGMSG(0,(TEXT("KPD_Close --\r\n")));

    return bRetVal;
}
//-----------------------------------------------------------------------------
//
// Function: KPD_PowerDown
//
// This function suspends power to the device. It is useful only with
//      devices that can power down under software control.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void KPD_PowerDown(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    // Not implemented!
}


//-----------------------------------------------------------------------------
//
// Function: KPD_PowerUp
//
// This function restores power to a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void KPD_PowerUp(void)
{
    // Not implemented!
}

//-----------------------------------------------------------------------------
//
//  Function: KPD_IOControl
//
// This function sends a command to a device.
//
//  Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//                function creates and returns this identifier.
//      dwCode
//          [in] I/O control operation to perform. These codes are
//                device-specific and are usually exposed to developers through
//                a header file.
//      pBufIn
//          [in] Pointer to the buffer containing data to transfer to the
//                device.
//      dwLenIn
//         [in] Number of bytes of data in the buffer specified for pBufIn.
//
//      pBufOut
//          [out] Pointer to the buffer used to transfer the output data
//                  from the device.
//      dwLenOut
//          [in] Maximum number of bytes in the buffer specified by pBufOut.
//
//      pdwActualOut
//          [out] Pointer to the DWORD buffer that this function uses to
//                  return the actual number of bytes received from the device.
//
//  Returns:
//      The new data pointer for the device indicates success. A value of -1
//      indicates failure.
//
//-----------------------------------------------------------------------------
BOOL KPD_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRetVal = TRUE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(dwCode);
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);

   
    bRetVal = KeyPadDriver->IOControl(hOpenContext,
                                      dwCode,
                                      pBufIn,
                                      dwLenIn,
                                      pBufOut,
                                      dwLenOut,
                                      pdwActualOut
                                      );

    return bRetVal;
}
