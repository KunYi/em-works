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
// File: pp_io.cpp
//
// This module provides a stream interface for the Post-processor (PP)
// driver.  Client drivers can use the stream interface to
// configure the PP driver and run test programs.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include "csp.h"
#include "pp.h"
#include "PpClass.h"

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
    TEXT("PP"), {
        TEXT("Init"),           TEXT("Deinit"),
        TEXT("Ioctl"),          TEXT("Thread"),
        TEXT(""),               TEXT(""),
        TEXT(""),               TEXT(""),
        TEXT(""),               TEXT(""),
        TEXT(""),               TEXT("Registers"),
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
// Function: POP_DllEntry
//
// This is the entry and exit point for the PP module.  This function
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
BOOL WINAPI POP_DllEntry(HINSTANCE hInstDll, DWORD dwReason, 
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
// Function: POP_Init
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
DWORD POP_Init(LPCTSTR pContext, LPCVOID lpvBusContext)
{
    PP_FUNCTION_ENTRY();

    // Construct the Post-Processor Module Class
    PpClass *pPP = new PpClass();
    if (pPP == NULL) {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, 
            (TEXT("%s: PP Class Failed!\r\n"), __WFUNCTION__));
        delete pPP;
        return NULL;
    }
    
    PP_FUNCTION_EXIT();

    // Return the created instance
    return (DWORD)pPP;
}

//------------------------------------------------------------------------------
//
// Function: POP_Deinit
//
// This function uninitializes a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context returned from POP_Init.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL POP_Deinit(DWORD hDeviceContext)
{
    PpClass *pPP = (PpClass *)hDeviceContext;

    PP_FUNCTION_ENTRY();

    if (pPP != NULL)
        delete pPP;

    PP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: POP_Open
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
DWORD POP_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    PpClass *pPP = (PpClass *)hDeviceContext;
    
    PP_FUNCTION_ENTRY();

    if (!pPP->PpEnable()) {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: PpEnable Failed! \r\n"), __WFUNCTION__));
        return NULL;
    }

    PP_FUNCTION_EXIT();

    return hDeviceContext;
}

//------------------------------------------------------------------------------
//
// Function: POP_Close
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
BOOL POP_Close(DWORD hOpenContext)
{
    PpClass *pPP = (PpClass *)hOpenContext;
    
    PP_FUNCTION_ENTRY();

    pPP->PpDisable();

    PP_FUNCTION_EXIT();

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: POP_Read
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
DWORD POP_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    PP_FUNCTION_ENTRY();
    PP_FUNCTION_EXIT();
    return 0;
}

//------------------------------------------------------------------------------
//
// Function: POP_Write
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
DWORD POP_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD Count)
{
    PP_FUNCTION_ENTRY();
    PP_FUNCTION_EXIT();
    return 0;
}

//------------------------------------------------------------------------------
//
// Function: POP_Seek
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
DWORD POP_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
    PP_FUNCTION_ENTRY();
    PP_FUNCTION_EXIT();
    return (DWORD)-1;
}

//------------------------------------------------------------------------------
//
// Function: POP_PowerUp
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
void POP_PowerUp(DWORD hDeviceContext)
{
    PP_FUNCTION_ENTRY();
    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: POP_PowerDown
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
void POP_PowerDown(DWORD hDeviceContext)
{
    PP_FUNCTION_ENTRY();
    PP_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: POP_IOControl
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
BOOL POP_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
    DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    ppConfigData *pConfigData;
    pPpBuffers pBufs;
    UINT32 *pMaxBuffers, *pFrameCount;
    BOOL bRet = FALSE;
    
    // hOpenContext is a pointer to PpClass instance!
    PpClass *pPp = (PpClass *)hOpenContext;

    switch (dwCode) {
        case PP_IOCTL_CONFIGURE:
//            pConfigData = (pPpConfigData)pBufIn;
            if (CeOpenCallerBuffer((PVOID*)&pConfigData, (PVOID)pBufIn, sizeof(ppConfigData), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            bRet = pPp->PpConfigure(pConfigData);
            if (CeCloseCallerBuffer((PVOID)pConfigData, (PVOID)pBufIn, sizeof(ppConfigData), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PP_IOCTL_CONFIGURE occurred\r\n"),  
                __WFUNCTION__));
            break;

        case PP_IOCTL_START:
            bRet = pPp->PpStart();
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PP_IOCTL_START occurred\r\n"), 
                __WFUNCTION__));
            break;

        case PP_IOCTL_STOP:
            bRet = pPp->PpStop();
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PP_IOCTL_STOP occurred\r\n"), 
                __WFUNCTION__));
            break;

        case PP_IOCTL_ENQUEUE_BUFFERS:
//          pBufs = (pPpBuffers)pBufIn;
            // The ppBuffers struct contains embedded pointers, but these are physical addresses, so it shouldn't be a problem
            if (CeOpenCallerBuffer((PVOID*)&pBufs, (PVOID)pBufIn, sizeof(ppBuffers), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            bRet = pPp->PpEnqueueBuffers(pBufs);
            if (CeCloseCallerBuffer((PVOID)pBufs, (PVOID)pBufIn, sizeof(ppBuffers), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PP_IOCTL_ADD_BUFFERS occurred\r\n"), 
                __WFUNCTION__));
            break;

        case PP_IOCTL_CLEAR_BUFFERS:
            pPp->PpClearBuffers();
            bRet = TRUE;
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PP_IOCTL_CLEAR_BUFFERS occurred\r\n"), 
                __WFUNCTION__));
            break;

        case PP_IOCTL_GET_MAX_BUFFERS:
//            pMaxBuffers = (UINT32 *)pBufOut;
            if (CeOpenCallerBuffer((PVOID*)&pMaxBuffers, (PVOID)pBufOut, sizeof(UINT32), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            *pMaxBuffers = pPp->PpGetMaxBuffers();
            bRet = TRUE;
            if (CeCloseCallerBuffer((PVOID)pMaxBuffers, (PVOID)pBufOut, sizeof(UINT32), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PP_IOCTL_GET_MAX_BUFFERS occurred\r\n"), 
                __WFUNCTION__));
            break;

        case PP_IOCTL_GET_FRAME_COUNT:
//            pFrameCount = (UINT32 *)pBufOut;
            if (CeOpenCallerBuffer((PVOID*)&pFrameCount, (PVOID)pBufOut, sizeof(UINT32), ARG_IO_PTR, FALSE) != S_OK) {
               break;
            }
            *pFrameCount = pPp->PpGetFrameCount();
            bRet = TRUE;
            if (CeCloseCallerBuffer((PVOID)pFrameCount, (PVOID)pBufOut, sizeof(UINT32), ARG_IO_PTR) != S_OK) {
                bRet = FALSE;
                break; 
            }
            DEBUGMSG(ZONE_IOCTL, 
                (TEXT("%s: PP_IOCTL_GET_FRAME_COUNT occurred\r\n"), 
                __WFUNCTION__));
            break;

        default:
            DEBUGMSG(ZONE_WARN, (TEXT("%s: No matching IOCTL.\r\n")));
            break;
    }

    return bRet;
}
