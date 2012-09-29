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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  owiremdd.c
//
//  This module provides a stream interface for the One-Wire
//  driver.  Client drivers can use the stream interface to
//  configure the OWIRE driver and run test programs.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201)
#include <windows.h>
#include <Devload.h>
#pragma warning(pop)

#include "common_owire.h"
#include "common_macros.h"
#include "owire.h"
#include "owire_priv.h"


//-----------------------------------------------------------------------------
// External Functions
extern BOOL BSPOwireSetClockGatingMode(BOOL);


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("OWIRE"),
    {
        _T("Init"), _T("DeInit"), _T("Ioctl"), _T("ReadWrite"),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN
};
#endif


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: WIR_Init
//
// The Device Manager calls this function as a result of a call to the
// ActivateDevice() function.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD WIR_Init(LPCTSTR pContext)
{
    DWORD ret = TRUE;
    UNREFERENCED_PARAMETER(pContext);

    OWIRE_FUNCTION_ENTRY();

    BSPOwireSetClockGatingMode(TRUE);

    if (!OwireInitialize())
    {
        ret = FALSE;
    }

    BSPOwireSetClockGatingMode(FALSE);

    OWIRE_FUNCTION_EXIT();

    return ret;
}


//------------------------------------------------------------------------------
//
// Function: WIR_Deinit
//
// This function uninitializes a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL WIR_Deinit(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER(hDeviceContext);
    
    OWIRE_FUNCTION_ENTRY();

    OwireDeinit();

    OWIRE_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: WIR_Open
//
// This function opens a device for reading, writing, or both.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context. The XXX_Init function creates
//          and returns this handle.
//
//      AccessCode
//          [in] Access code for the device. The access is a combination of
//          read and write access from CreateFile.
//
//      ShareMode
//          [in] File share mode of the device. The share mode is a
//          combination of read and write access sharing from CreateFile.
//
// Returns:
//      This function returns a handle that identifies the open context of
//      the device to the calling application.
//
//------------------------------------------------------------------------------
DWORD WIR_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);
    
    OWIRE_FUNCTION_ENTRY();
    OWIRE_FUNCTION_EXIT();
    return hDeviceContext;
}


//------------------------------------------------------------------------------
//
// Function: WIR_Close
//
// This function closes the OWIRE for reading and writing.
//
// Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to identify
//          the open context of the device.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL WIR_Close(DWORD hOpenContext)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    
    OWIRE_FUNCTION_ENTRY();
    OWIRE_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: WIR_PowerDown
//
// This function suspends power to the device. It is useful only with
// devices that can power down under software control.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void WIR_PowerDown(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER(hDeviceContext);
    
    OWIRE_FUNCTION_ENTRY();
    OWIRE_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: WIR_PowerUp
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
//------------------------------------------------------------------------------
void WIR_PowerUp(void)
{
    OWIRE_FUNCTION_ENTRY();
    OWIRE_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: WIR_Read
//
// This function reads data from the device identified by the open
// context.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      pBuffer
//          [out] Pointer to the buffer that stores the data read from the
//          device. This buffer should be at least Count bytes long.
//
//      Count
//          [in] Number of bytes to read from the device into pBuffer.
//
// Returns:
//      Returns zero to indicate end-of-file. Returns -1 to indicate an
//      error. Returns the number of bytes read to indicate success.
//
//------------------------------------------------------------------------------
DWORD WIR_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    BYTE* readBuf;
    DWORD ret = Count;

    UNREFERENCED_PARAMETER(hOpenContext);

    OWIRE_FUNCTION_ENTRY();

    BSPOwireSetClockGatingMode(TRUE);
    
    if (!OwireReadData((BYTE *)pBuffer, Count)) 
        ret = (DWORD)-1;

    readBuf = (BYTE *)pBuffer;

#if DEBUG
    {
        DWORD i;
        for (i = 0; i < Count; i++)
            DEBUGMSG(ZONE_READWRITE, (TEXT("BYTE #%x read: %x\r\n"), i, readBuf[i]));
    }
#endif

    BSPOwireSetClockGatingMode(FALSE);

    OWIRE_FUNCTION_EXIT();

    return ret;
}


//------------------------------------------------------------------------------
//
// Function: WIR_Write
//
// This function writes data to the device.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      pBuffer
//          [out] Pointer to the buffer that contains the data to write.
//
//      Count
//          [in] Number of bytes to write from the pBuffer buffer into the
//          device.
//
// Returns:
//      The number of bytes written indicates success. A value of -1 indicates
//      failure.
//
//------------------------------------------------------------------------------
DWORD WIR_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    DWORD ret = dwNumBytes;

    UNREFERENCED_PARAMETER(Handle);
    
    OWIRE_FUNCTION_ENTRY();

    BSPOwireSetClockGatingMode(TRUE);
    
    if (!OwireWriteData((BYTE *)pBuffer, dwNumBytes))
        ret = (DWORD)-1;

    BSPOwireSetClockGatingMode(FALSE);
    
    OWIRE_FUNCTION_EXIT();

    return ret;
}


//------------------------------------------------------------------------------
//
// Function: WIR_Seek
//
// This function moves the data pointer in the device.
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      Amount
//          [in] Number of bytes to move the data pointer in the device.
//          A positive value moves the data pointer toward the end of the
//          file, and a negative value moves it toward the beginning.
//
//      Type
//          [in] Starting point for the data pointer.
//
// Returns:
//      The new data pointer for the device indicates success. A value of -1
//      indicates failure.
//
//------------------------------------------------------------------------------
DWORD WIR_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(Amount);
    UNREFERENCED_PARAMETER(Type);
    
    OWIRE_FUNCTION_ENTRY();
    OWIRE_FUNCTION_EXIT();
    return (DWORD)-1;
}


//------------------------------------------------------------------------------
//
// Function: WIR_IOControl
//
// This function sends a command to the OWIRE device.
// IOCTL Codes:
//     OWIRE_IOCTL_RESET_PRESENCE_PULSE
//         IOCTL to perform the reset sequence with reset 
//         pulse and presence pulse
//
//     OWIRE_IOCTL_BUS_LOCK
//         IOCTL to perform a bus lock on the OWIRE
//
//     OWIRE_IOCTL_BUS_UNLOCK
//         IOCTL to perform a bus unlock on the OWIRE
//
// Parameters:
//      hOpenContext
//          [in] Handle to the open context of the device. The XXX_Open
//          function creates and returns this identifier.
//
//      dwCode
//          [in] I/O control operation to perform. These codes are
//          device-specific and are usually exposed to developers through
//          a header file.
//
//      pBufIn
//          [in] Pointer to the buffer containing data to transfer to the
//          device.
//
//      dwLenIn
//          [in] Number of bytes of data in the buffer specified for pBufIn.
//
//      pBufOut
//          [out] Pointer to the buffer used to transfer the output data
//          from the device.
//
//      dwLenOut
//          [in] Maximum number of bytes in the buffer specified by pBufOut.
//
//      pdwActualOut
//          [out] Pointer to the DWORD buffer that this function uses to
//          return the actual number of bytes received from the device.
//
// Returns:
//      The new data pointer for the device indicates success. A value of -1
//      indicates failure.
//
//------------------------------------------------------------------------------
BOOL WIR_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRet = TRUE;

    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);
    
    OWIRE_FUNCTION_ENTRY();

    switch(dwCode)
    {
    case OWIRE_IOCTL_RESET_PRESENCE_PULSE:
        DEBUGMSG(ZONE_IOCTL, (TEXT("WIR_IOControl: OWIRE_IOCTL_RESET_PRESENCE_PULSE occurred\r\n")));
        BSPOwireSetClockGatingMode(TRUE);
        if (!OwireSetResetPresencePulse())
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("Owire Device not present!\r\n")));
            bRet = FALSE;
        }
        BSPOwireSetClockGatingMode(FALSE);
        break;
    case OWIRE_IOCTL_BUS_LOCK:
        OwireLock();
        break;
    case OWIRE_IOCTL_BUS_UNLOCK:
        OwireUnLock();
        break;
    default:
        DEBUGMSG(ZONE_WARN, (TEXT("WIR_IOControl: No matching IOCTL.\r\n")));
        bRet = FALSE;
        break;
    }

    OWIRE_FUNCTION_EXIT();
    return bRet;
}

BOOL WINAPI WIR_DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);
    
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            //Register Debug Zones
            DEBUGREGISTER((HINSTANCE) hInstDll);
            //DEBUGMSG(ZONE_INFO, (TEXT("WIR_DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;

        case DLL_PROCESS_DETACH:
            //DEBUGMSG(ZONE_INFO, (TEXT("WIR_DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
    // Return TRUE for success
    return TRUE;
}
