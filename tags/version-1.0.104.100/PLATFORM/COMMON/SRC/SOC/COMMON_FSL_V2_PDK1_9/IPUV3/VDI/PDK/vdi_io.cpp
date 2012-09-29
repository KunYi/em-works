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
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  vdi_io.cpp
//
//  This module provides a stream interface for the Video De-Interlacer (VDI)
//  driver.  Client drivers can use the stream interface to
//  configure the VDI driver and run test programs.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"

#include "IpuBuffer.h"
#include "ipu_base.h"
#include "ipu_common.h"
#include "vdi.h"
#include "vdi_priv.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("PP"),
    {
        _T("Init"), _T("DeInit"), _T("Ioctl"), _T("Device"),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN
};
#endif

static BOOL gAlreadyOpened;
//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: VDI_Init
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
//      Returns TRUE if successful. Returns
//      zero if not successful.
//
//------------------------------------------------------------------------------
DWORD VDI_Init(LPCTSTR pContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    DWORD dwRet = 0;

    VDI_FUNCTION_ENTRY();

    // Initialize VDI driver
    dwRet = VDIInit();
    if (!dwRet)
    {
        DEBUGMSG (ZONE_INIT|ZONE_ERROR, (TEXT("VDI_Init: VDI initialization failed!\r\n")));
        return NULL;
    }

    gAlreadyOpened = FALSE;

    VDI_FUNCTION_EXIT();

    // Otherwise return the created instance
    return dwRet;
}


//------------------------------------------------------------------------------
//
// Function: VDI_Deinit
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
BOOL VDI_Deinit(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);

    VDI_FUNCTION_ENTRY();

    VDIDeinit();

    VDI_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: VDI_Open
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
DWORD VDI_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    VDI_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    if(gAlreadyOpened)
    {
        return 0;
    }
    else
    {
        gAlreadyOpened = TRUE;
    }

    VDI_FUNCTION_EXIT();
    return hDeviceContext;
}


//------------------------------------------------------------------------------
//
// Function: VDI_Close
//
// This function closes the VDI for reading and writing.
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
BOOL VDI_Close(DWORD hOpenContext)
{
    VDI_FUNCTION_ENTRY();

    gAlreadyOpened = FALSE;
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    VDI_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: VDI_PowerDown
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
void VDI_PowerDown(DWORD hDeviceContext)
{
    VDI_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    VDI_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: VDI_PowerUp
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
void VDI_PowerUp(void)
{
    VDI_FUNCTION_ENTRY();
    VDI_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: VDI_IOControl
//
// This function sends a command to a device.
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
BOOL VDI_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    vdiConfigData *pConfigData;
    vdiStartParams *pStartParams;
    BOOL bRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pdwActualOut);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pBufOut);

    switch(dwCode)
    {
        case VDI_IOCTL_CONFIGURE:
            pConfigData = (pVdiConfigData)pBufIn;

            bRet = VDIConfigure(pConfigData);
            DEBUGMSG(ZONE_IOCTL, (TEXT("VDI_IOControl: VDI_IOCTL_CONFIGURE ")
                                  TEXT("occurred\r\n")));
            break;

        case VDI_IOCTL_START:
            pStartParams = (pVdiStartParams)pBufIn;

            bRet = VDIStartChannel(pStartParams);
            DEBUGMSG(ZONE_IOCTL, (TEXT("VDI_IOControl: VDI_IOCTL_START ")
                                  TEXT("occurred\r\n")));
            break;
            
        case VDI_IOCTL_STOP:
            bRet = VDIStopChannel();
            DEBUGMSG(ZONE_IOCTL, (TEXT("VDI_IOControl: VDI_IOCTL_STOP ")
                                  TEXT("occurred\r\n")));
            break;
            
        case VDI_IOCTL_WAIT_NOT_BUSY:
            bRet = VDIWaitForEOF((DWORD)*pBufIn);
            DEBUGMSG(ZONE_IOCTL, (TEXT("VDI_IOControl: VDI_IOCTL_WAIT_NOT_BUSY ")
                                  TEXT("occurred\r\n")));
            break;

        default:
            DEBUGMSG(ZONE_WARN, (TEXT("VDI_IOControl: No matching IOCTL.\r\n")));
            break;
    }

    return bRet;
}

extern "C"
BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
         //Register Debug Zones
         DEBUGREGISTER((HINSTANCE) hInstDll);
         DEBUGMSG(ZONE_INFO, (TEXT("VDI_DllEntry: DLL_PROCESS_ATTACH ")
                              TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
         DisableThreadLibraryCalls((HMODULE) hInstDll);
         break;

      case DLL_PROCESS_DETACH:
         DEBUGMSG(ZONE_INFO, (TEXT("VDI_DllEntry: DLL_PROCESS_DETACH ")
                              TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
         break;
   }
   // Return TRUE for success
   return TRUE;
}
