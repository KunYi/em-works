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
//  File:  Pp_io.cpp
//
//  This module provides a stream interface for the Post-processor (PP)
//  driver.  Client drivers can use the stream interface to
//  configure the PP driver and run test programs.
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


//#include "IpuModuleInterfaceClass.h"
#include "IpuBuffer.h"
#include "IPU_base.h"
#include "IPU_common.h"
#include "tpm.h"
#include "pp.h"
#include "PpClass.h"
#include "idmac.h"
#include "cpmem.h"
#include "dp.h"
#include "dmfc.h"


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
//    0xffff//ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_FUNCTION | ZONEMASK_INFO | ZONE_INIT | ZONE_DEVICE
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
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//------------------------------------------------------------------------------
DWORD POP_Init(LPCTSTR pContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    PP_FUNCTION_ENTRY();

    // Construct the Post-Processor Module Class
    PpClass* pPP = new PpClass();

    // Managed to create the class?
    if (pPP == NULL)
    {
        DEBUGMSG (ZONE_INIT|ZONE_ERROR, (TEXT("POP_Init: PP Class Failed!\r\n")));
        delete pPP;
        return NULL;
    }
    
    gAlreadyOpened = FALSE;
    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("POP_Init - hDev=0x%x\r\n"), pPP));

    PP_FUNCTION_EXIT();

    // Otherwise return the created instance
    return (DWORD) pPP;
}


//------------------------------------------------------------------------------
//
// Function: POP_Deinit
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
BOOL POP_Deinit(DWORD hDeviceContext)
{
    PpClass * pPP = (PpClass*) hDeviceContext;

    PP_FUNCTION_ENTRY();

    if (pPP != NULL)
    {
        delete pPP;
    }

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
    PP_FUNCTION_ENTRY();

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

    PP_FUNCTION_EXIT();
    return hDeviceContext;
}


//------------------------------------------------------------------------------
//
// Function: POP_Close
//
// This function closes the PP for reading and writing.
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
    PP_FUNCTION_ENTRY();

    gAlreadyOpened = FALSE;
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    PP_FUNCTION_EXIT();
    return TRUE;
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
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    PP_FUNCTION_EXIT();
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
void POP_PowerUp(void)
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
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    ppConfigData *pConfigData;
    UINT32 *pBuf;
    UINT8 IntType;
    RECT * pRect;
    BOOL bRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pdwActualOut);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pBufOut);

    // hOpenContext is a pointer to PPClass instance!
    PpClass* pPp = (PpClass*) hOpenContext;

    switch(dwCode)
    {
        case PP_IOCTL_CONFIGURE:
            pConfigData = (pPpConfigData)pBufIn;

            bRet = pPp->PpConfigure(pConfigData);
            DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_CONFIGURE ")
                                  TEXT("occurred\r\n")));
            break;

        case PP_IOCTL_START:
            bRet = pPp->PpStartChannel();
            DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_START ")
                                  TEXT("occurred\r\n")));
            break;

        case PP_IOCTL_STOP:
            bRet = pPp->PpStopChannel();
            DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_STOP ")
                                  TEXT("occurred\r\n")));
            break;

        case PP_IOCTL_ADD_INPUT_BUFFER:
            pBuf = (UINT32 *) (*((UINT32 *)pBufIn));

            bRet = pPp->PpAddInputBuffer(pBuf);

            DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_ADD_INTPUT_BUFFERS ")
                                  TEXT("occurred\r\n")));

            break;
            
        case PP_IOCTL_ADD_OUTPUT_BUFFER:
            pBuf = (UINT32 *) (*((UINT32 *)pBufIn));

            bRet = pPp->PpAddOutputBuffer(pBuf);

            DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_ADD_OUTTPUT_BUFFERS ")
                                  TEXT("occurred\r\n")));

            break;

        case PP_IOCTL_CLEAR_BUFFERS:
            pPp->PpClearBuffers();
            bRet = TRUE;
            DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_CLEAR_BUFFERS ")
                                  TEXT("occurred\r\n")));
            break;
            
        case PP_IOCTL_ENABLE_INTERRUPT:
            IntType = (UINT8)*pBufIn;
            pPp->PpIntrEnable(IntType,TRUE);
            bRet = TRUE;
            DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_ENABLE_INTERRUPT ")
                                  TEXT("occurred\r\n")));
            break;
            
       case PP_IOCTL_DISABLE_INTERRUPT:
            IntType = (UINT8)*pBufIn;
            pPp->PpIntrEnable(IntType,FALSE);
            bRet = TRUE;
            DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_DISABLE_INTERRUPT ")
                                  TEXT("occurred\r\n")));
            break;
            
        case PP_IOCTL_WAIT_NOT_BUSY:
             IntType = (UINT8)*pBufIn;
             pPp->PpWaitForNotBusy(IntType);
             bRet = TRUE;
             DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_WAIT_NOT_BUSY ")
                                   TEXT("occurred\r\n")));
             break;
        case PP_IOCTL_ADD_INPUT_COMBBUFFER:
            pBuf = (UINT32 *) (*((UINT32 *)pBufIn));

            bRet = pPp->PpAddInputCombBuffer(pBuf);

            DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_ADD_INPUT_COMBBUFFER ")
                                  TEXT("occurred\r\n")));

            break;             
         case PP_IOCTL_SET_WINDOW_POS:
              pRect = (RECT *)pBufIn;
              bRet = pPp->PpChangeMaskPos(pRect);
              DEBUGMSG(ZONE_IOCTL, (TEXT("POP_IOControl: PP_IOCTL_SET_WINDOW_POS ")
                                    TEXT("occurred\r\n")));
              break;

        default:
            DEBUGMSG(ZONE_WARN, (TEXT("POP_IOControl: No matching IOCTL.\r\n")));
            break;
    }

    return bRet;
}

//------------------------------------------------------------------------------
//
// Function: DllEntry
//
// This function is the driver DLL entry.
//
// Parameters:
//      hInstDll
//          [in] The handle of the DLL.
//      dwReason
//          [in] The action wanted this DLLEntry to do.
//      lpvReserved
//          [in] Reserved, Not use.
//
// Returns:
//      Return TRUE for success
//------------------------------------------------------------------------------
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
         DEBUGMSG(ZONE_INFO, (TEXT("POP_DllEntry: DLL_PROCESS_ATTACH ")
                              TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
         DisableThreadLibraryCalls((HMODULE) hInstDll);
         break;

      case DLL_PROCESS_DETACH:
         DEBUGMSG(ZONE_INFO, (TEXT("POP_DllEntry: DLL_PROCESS_DETACH ")
                              TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
         break;
   }
   // Return TRUE for success
   return TRUE;
}
