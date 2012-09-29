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
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  Pxp_io.cpp
//
//  This module provides a stream interface for the Pixel Pipeline (PXP)
//  driver.  Client drivers can use the stream interface to
//  configure the PXP driver and run test programs.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#include <winddi.h>
#include <ddgpe.h>
#pragma warning(pop)


#include "common_pxp.h"
#include "PxpClass.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("PXP"),
    {
        _T("Init"), _T("DeInit"), _T("Ioctl"), _T("Device"),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    0xffff//ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_FUNCTION | ZONEMASK_INFO | ZONE_INIT | ZONE_DEVICE
    //ZONEMASK_ERROR | ZONEMASK_WARN
};
#endif

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
static BOOL gAlreadyOpened;
//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: PXP_Init
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
//------------------------------------------------------------------------------
DWORD PXP_Init(LPCTSTR pContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    PXP_FUNCTION_ENTRY();

    // Construct the Post-Processor Module Class
    PxpClass* pPXP = new PxpClass();

    // Managed to create the class?
    if (pPXP == NULL)
    {
        RETAILMSG (1, (TEXT("PXP_Init: PXP Class Failed!\r\n")));
        delete pPXP;
        return NULL;
    }

    gAlreadyOpened = FALSE;
    RETAILMSG (1, (TEXT("PXP_Init - hDev=0x%x\r\n"), pPXP));

    PXP_FUNCTION_EXIT();

    // Otherwise return the created instance
    return (DWORD) pPXP;
}


//------------------------------------------------------------------------------
//
// Function: PXP_Deinit
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
BOOL PXP_Deinit(DWORD hDeviceContext)
{
    PxpClass * pPXP = (PxpClass*) hDeviceContext;

    PXP_FUNCTION_ENTRY();

    if (pPXP != NULL)
    {
        delete pPXP;
    }

    PXP_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: PXP_Open
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
DWORD PXP_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    PXP_FUNCTION_ENTRY();

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

    PxpClass* pPxp = (PxpClass*) hDeviceContext;
    pPxp->PxpOpen();

    PXP_FUNCTION_EXIT();
    return hDeviceContext;
}


//------------------------------------------------------------------------------
//
// Function: PXP_Close
//
// This function closes the PXP for reading and writing.
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
BOOL PXP_Close(DWORD hOpenContext)
{
    PXP_FUNCTION_ENTRY();

    gAlreadyOpened = FALSE;

    PxpClass* pPxp = (PxpClass*) hOpenContext;
    pPxp->PxpClose();

    PXP_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: PXP_PowerDown
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
void PXP_PowerDown(DWORD hDeviceContext)
{
    PXP_FUNCTION_ENTRY();
    PxpClass* pPxp = (PxpClass*) hDeviceContext;
    pPxp->PxpPowerDown();
    PXP_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: PXP_PowerUp
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
void PXP_PowerUp(DWORD hDeviceContext)
{
    PXP_FUNCTION_ENTRY();
    PxpClass* pPxp = (PxpClass*) hDeviceContext;
    pPxp->PxpPowerUp();
    PXP_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: PXP_IOControl
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
BOOL PXP_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    UINT32 *pBuf;
    BOOL bRet = TRUE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);

    // hOpenContext is a pointer to PPClass instance!
    PxpClass* pPxp = (PxpClass*) hOpenContext;

    switch(dwCode)
    {
    case PXP_IOCTL_ENABLE:
        pPxp->PxpEnable();
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_ENABLE ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_DISABLE:
        pPxp->PxpDisable();
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_DISABLE ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_START_PROCESS:
        pPxp->PxpStartProcess(*((BOOL *)pBufIn));
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_START_PROCESS ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_ENABLE_INTERRUPT:
        pPxp->PxpIntrEnable(TRUE);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_ENABLE_INTERRUPT ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_DISABLE_INTERRUPT:
        pPxp->PxpIntrEnable(FALSE);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_DISABLE_INTERRUPT ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_OUTPUTBUFFER1_ADDRESS:
        pBuf = (UINT32 *) (*((UINT32 *)pBufIn));
        pPxp->PxpSetOutputBuffer1Addr(pBuf);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_OUTPUTBUFFER1_ADDRESS ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_OUTPUTBUFFER2_ADDRESS:
        pBuf = (UINT32 *) (*((UINT32 *)pBufIn));
        pPxp->PxpSetOutputBuffer2Addr(pBuf);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_OUTPUTBUFFER2_ADDRESS ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_WAIT_COMPLETION:
        pPxp->PxpWaitForCompletion();
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_WAIT_NOT_BUSY ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_S0BUFFER_ADDRESS:
        pPxp->PxpSetS0BufferAddrGroup((pPxpS0BufferAddrGroup)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_S0BUFFER_ADDRESS ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_S0BUFFER_OFFSETINOUTPUT:
        pPxp->PxpSetS0BufferOffsetInOutput((pPxpCoordinate)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_S0BUFFER_OFFSETINOUTPUT ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_S0BUFFER_SIZE:
        pPxp->PxpSetS0BufferSize((pPxpRectSize)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_S0BUFFER_SIZE ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_S0BUFFER_COLORKEY:
        pPxp->PxpSetS0BufferColorKey((pPxpColorKey)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_S0BUFFER_COLORKEY ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_OVERLAYBUFFERS_ADDRESS:
        pPxp->PxpSetOverlayBuffersAddr((pPxpOverlayBuffersAddr)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_OVERLAYBUFFERS_ADDRESS ")
           TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_OVERLAYBUFFERS_POSITION:
        pPxp->PxpSetOverlayBuffersPos((pPxpOverlayBuffersPos)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_OVERLAYBUFFERS_POSITION ")
           TEXT("occurred\r\n")));
        break;
    
    case PXP_IOCTL_CONFIG_GENERAL:
        pPxp->PxpConfigureGeneral((pPxpParaConfig)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_CONFIG_GENERAL ")
            TEXT("occurred\r\n")));
        break;
  
    case PXP_IOCTL_SET_S0BUFFER_PROPERTY:
        pPxp->PxpSetS0BufProperty((pPxpS0Property)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_S0BUFFER_PROPERTY ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_OUTPUT_PROPERTY:
        pPxp->PxpSetOutputProperty((pPxpOutProperty)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_OUTPUT_PROPERTY ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_OVERLAYBUFFERS_PROPERTY:
        pPxp->PxpSetOverlayBufsProperty((pPxpOverlayProperty)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_OVERLAYBUFFERS_PROPERTY ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_OVERLAY_COLORKEY:
        pPxp->PxpSetOverlayColorKey((pPxpColorKey)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_OVERLAY_COLORKEY ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_RESET_DRIVER_STATUS:
        pPxp->PxpResetDriverStatus();
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_RESET_DRIVER_STATUS ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_SET_BLOCK_SIZE:
        pPxp->PxpSetBlockSize((UINT32 *)pBufIn);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_SET_BLOCK_SIZE ")
            TEXT("occurred\r\n")));
        break;

    case PXP_IOCTL_GET_DOWNSCALE_EXPONENT:
        pPxp->PxpGetDownScaleFactorExponent((UINT32 *)pBufOut);
        DEBUGMSG(1, (TEXT("PXP_IOControl: PXP_IOCTL_GET_DOWNSCALE_EXPONENT ")
            TEXT("occurred\r\n")));
        break;

    default:
        DEBUGMSG(1, (TEXT("PXP_IOControl: No matching IOCTL.\r\n")));
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
        RETAILMSG(1, (TEXT("PXP_DllEntry: DLL_PROCESS_ATTACH ")
            TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
        DisableThreadLibraryCalls((HMODULE) hInstDll);
        break;

    case DLL_PROCESS_DETACH:
        RETAILMSG(1, (TEXT("PXP_DllEntry: DLL_PROCESS_DETACH ")
            TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
        break;
    }
    // Return TRUE for success
    return TRUE;
}
