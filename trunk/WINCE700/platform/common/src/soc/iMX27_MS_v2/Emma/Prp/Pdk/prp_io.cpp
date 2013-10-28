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
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: prp_io.cpp
//
// This module provides a stream interface for the Pre-processor (PrP)
// driver.  Client drivers can use the stream interface to
// configure the PP driver and run test programs.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include "csp.h"
#include "prp.h"
#include "PrpClass.h"

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
DBGPARAM dpCurSettings = {
    TEXT("PRP"), {
        TEXT("Init"),           TEXT("Deinit"),
        TEXT("Ioctl"),          TEXT(""),
        TEXT(""),               TEXT(""),
        TEXT(""),               TEXT(""),
        TEXT(""),               TEXT(""),
        TEXT(""),               TEXT("Device"),
        TEXT("Information"),    TEXT("Function"),
        TEXT("Warning"),        TEXT("Error")
    },
    ZONEMASK_WARN | ZONEMASK_ERROR
};
#endif

//------------------------------------------------------------------------------
// Local Variables 

//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: PRP_DllEntry
//
// This is the entry and exit point for the PrP module.  This function
// is called when processed and threads attach and detach from this module.
//
// Parameters:
//      hInstDll
//          [in] The handle to this module.
//
//      dwReason
//          [in] Specifies a flag indicating why the DLL entry-point function
//          is being called.
//
//      lpvReserved
//          [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the everything is OK; FALSE if an error occurred.
//
//------------------------------------------------------------------------------
BOOL WINAPI PRP_DllEntry(HINSTANCE hInstDll, DWORD dwReason, 
    LPVOID lpvReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            // Register debug zones
            DEBUGREGISTER(hInstDll);
            DEBUGMSG(ZONE_INIT, 
                (TEXT("%s: DLL_PROCESS_ATTACH\r\n"), __WFUNCTION__));
            DisableThreadLibraryCalls((HMODULE)hInstDll);
            break;
         
        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_DEINIT, 
                (TEXT("%s: DLL_PROCESS_DETACH\r\n"), __WFUNCTION__));
        break;
    }
    
   // return TRUE for success
   return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PRP_Init
//
// The Device Manager calls this function as a result of a call to the
// ActivateDevice() function.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
//      lpvBusContext
//          [in] Pointer to bus context.
//
// Returns:
//      Returns a handle to the device context created if successful; returns
//      zero if not successful.
//
//------------------------------------------------------------------------------
DWORD PRP_Init(LPCTSTR pContext, LPCVOID lpvBusContext)
{
    PRP_FUNCTION_ENTRY();

    // Construct the Pre-Processor Module Class
    PrpClass *pPrP = new PrpClass();
    if (pPrP == NULL) {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, 
            (TEXT("%s: PrP Class Failed!\r\n"), __WFUNCTION__));
        delete pPrP;
        return NULL;
    }
    
    PRP_FUNCTION_EXIT();

    // Return the created instance
    return (DWORD)pPrP;
}

//------------------------------------------------------------------------------
//
// Function: PRP_Deinit
//
// This function uninitializes a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context returned from PRP_Init.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL PRP_Deinit(DWORD hDeviceContext)
{
    PrpClass *pPrP = (PrpClass *)hDeviceContext;

    PRP_FUNCTION_ENTRY();

    if (pPrP != NULL)
        delete pPrP;

    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PRP_Open
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
DWORD PRP_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    PrpClass *pPrP = (PrpClass *)hDeviceContext;
    
    PRP_FUNCTION_ENTRY();

    // If power up failure, it will release the resource automatically.
    if (!pPrP->PrpPowerUp()) {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: PrP Power Up Failed!\r\n"), __WFUNCTION__));
        return NULL;
    }

    //If there is a module has opened PrP module or PrP enable failure, PrpEnable() return FALSE.
    if (!pPrP->PrpEnable()) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: PrpEnable Failed!\r\n"), __WFUNCTION__));
        pPrP->PrpPowerDown();
        return NULL;
    }
    
    PRP_FUNCTION_EXIT();

    return hDeviceContext;
}

//------------------------------------------------------------------------------
//
// Function: PRP_Close
//
// This function opens a device for reading, writing, or both.
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
BOOL PRP_Close(DWORD hOpenContext)
{
    PrpClass *pPrP = (PrpClass *)hOpenContext;
    
    PRP_FUNCTION_ENTRY();

    pPrP->PrpDisable();

    pPrP->PrpPowerDown();
    
    PRP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PRP_Read
//
// This function reads data from the device identified by the open context.
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
DWORD PRP_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    PRP_FUNCTION_ENTRY();
    PRP_FUNCTION_EXIT();
    return 0;
}

//------------------------------------------------------------------------------
//
// Function: PRP_Write
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
//          [in] Number of bytes to read from the device into pBuffer. 
//
// Returns: 
//      The number of bytes written indicates success. A value of -1
//      indicates failure.
//
//------------------------------------------------------------------------------
DWORD PRP_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD Count)
{
    PRP_FUNCTION_ENTRY();
    PRP_FUNCTION_EXIT();
    return 0;
}

//------------------------------------------------------------------------------
//
// Function: PRP_Seek
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
DWORD PRP_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
    PRP_FUNCTION_ENTRY();
    PRP_FUNCTION_EXIT();
    return (DWORD)-1;
}

//------------------------------------------------------------------------------
//
// Function: PRP_PowerUp
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
void PRP_PowerUp(DWORD hDeviceContext)
{
    PRP_FUNCTION_ENTRY();
    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PRP_PowerDown
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
void PRP_PowerDown(DWORD hDeviceContext)
{
    PRP_FUNCTION_ENTRY();
    PRP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: PRP_IOControl
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
BOOL PRP_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
    DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    prpConfigData *pConfigData;
    pPrpBuffers pBufs;
    UINT8 *pChannel;
    UINT32 *pFrameCount;
    BOOL bRet = FALSE;
    
    // Only allow kernel process to call PRP ioctls.
    if (GetCurrentProcessId() != GetDirectCallerProcessId()) {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: PRP_IOControl: "
            L"PRP_IOControl can be called only from "
            L"device process (caller process id 0x%08x)\r\n",
            GetDirectCallerProcessId()
        ));
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    // hOpenContext is a pointer to PrpClass instance!
    PrpClass *pPrp = (PrpClass *)hOpenContext;

    switch (dwCode) {
        case PRP_IOCTL_CONFIGURE:
//            pConfigData = (pPrpConfigData)pBufIn;
            if (CeOpenCallerBuffer((PVOID*)&pConfigData, (PVOID)pBufIn, sizeof(prpConfigData), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            bRet = pPrp->PrpConfigure(pConfigData);
            if (CeCloseCallerBuffer((PVOID)pConfigData, (PVOID)pBufIn, sizeof(prpConfigData), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PRP_IOCTL_CONFIGURE occurred\r\n"),  
                __WFUNCTION__));
            break;

        case PRP_IOCTL_START:
//            pChannel = (UINT8 *)pBufIn;
            if (CeOpenCallerBuffer((PVOID*)&pChannel, (PVOID)pBufIn, sizeof(UINT8), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            switch (*pChannel) {
                case prpChannel_Viewfinding:
                    bRet = pPrp->PrpStartVfChannel();
                    break;
                    
                case prpChannel_Encoding:
                    bRet = pPrp->PrpStartEncChannel();
                    break;
                    
                default:
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Invalid channel selection for PRP_IOCTL_START\r\n"), 
                        __WFUNCTION__));
                    bRet = FALSE;
            }
            if (CeCloseCallerBuffer((PVOID)pChannel, (PVOID)pBufIn, sizeof(UINT8), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PRP_IOCTL_START occurred\r\n"), 
                __WFUNCTION__));
            break;

        case PRP_IOCTL_STOP:
//            pChannel = (UINT8 *)pBufIn;
            if (CeOpenCallerBuffer((PVOID*)&pChannel, (PVOID)pBufIn, sizeof(UINT8), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            switch (*pChannel) {
                case prpChannel_Viewfinding:
                    bRet = pPrp->PrpStopVfChannel();
                    break;
                    
                case prpChannel_Encoding:
                    bRet = pPrp->PrpStopEncChannel();
                    break;
                    
                default:
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Invalid channel selection for PRP_IOCTL_STOP\r\n"), 
                        __WFUNCTION__));
                    bRet = FALSE;
            }
            if (CeCloseCallerBuffer((PVOID)pChannel, (PVOID)pBufIn, sizeof(UINT8), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PRP_IOCTL_STOP occurred\r\n"), 
                __WFUNCTION__));
            break;

        case PRP_IOCTL_ADD_BUFFERS:
//            pBufs = (pPrpBuffers)pBufIn;
            if (CeOpenCallerBuffer((PVOID*)&pBufs, (PVOID)pBufIn, sizeof(prpBuffers), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            bRet = pPrp->PrpAddBuffers(pBufs);
            if (CeCloseCallerBuffer((PVOID)pBufs, (PVOID)pBufIn, sizeof(prpBuffers), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PRP_IOCTL_ADD_BUFFERS occurred\r\n"), 
                __WFUNCTION__));
            break;

        case PRP_IOCTL_GET_FRAME_COUNT:
//            pChannel = (UINT8 *)pBufIn;
//            pFrameCount = (UINT32 *)pBufOut;
            bRet = TRUE;
            if (CeOpenCallerBuffer((PVOID*)&pChannel, (PVOID)pBufIn, sizeof(UINT8), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            if (CeOpenCallerBuffer((PVOID*)&pFrameCount, (PVOID)pBufOut, sizeof(UINT32), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            switch (*pChannel) {
                case prpChannel_Viewfinding:
                    *pFrameCount = pPrp->PrpGetVfFrameCount();
                    break;
                    
                case prpChannel_Encoding:
                    *pFrameCount = pPrp->PrpGetEncFrameCount();
                    break;
                    
                default:
                    DEBUGMSG(ZONE_ERROR, 
                        (TEXT("%s: Invalid channel selection for PRP_IOCTL_GET_FRAME_COUNT\r\n"), 
                        __WFUNCTION__));
                    bRet = FALSE;
            }
            if (CeCloseCallerBuffer((PVOID)pChannel, (PVOID)pBufIn, sizeof(UINT8), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            if (CeCloseCallerBuffer((PVOID)pFrameCount, (PVOID)pBufOut, sizeof(UINT32), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PRP_IOCTL_GET_FRAME_COUNT occurred\r\n"), 
                __WFUNCTION__));
            break;

        default:
            DEBUGMSG(ZONE_WARN, (TEXT("%s: No matching IOCTL.\r\n")));
            break;
    }

    return bRet;
}
