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
//  File:  Pf_io.cpp
//
//  This module provides a stream interface for the Post-filter (PF)
//  driver.  Client drivers can use the stream interface to
//  configure the PF driver and run test programs.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipu.h"

#include "pf.h"
#include "PfClass.h"


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
    _T("PF"),
    {
        _T("Init"), _T("DeInit"), _T("Ioctl"), _T("Device"),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    0xffff//ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_FUNCTION | ZONEMASK_INFO | ZONE_INIT | ZONE_DEVICE
//    ZONEMASK_ERROR | ZONEMASK_WARN
};
#endif


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: POF_Init
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
DWORD POF_Init(LPCTSTR pContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);

    PF_FUNCTION_ENTRY();

    // Construct the Post-Processor Module Class
    PfClass* pPF = new PfClass();

    // Managed to create the class?
    if (pPF == NULL)
    {
        DEBUGMSG (ZONE_INIT|ZONE_ERROR, (TEXT("POF_Init: PF Class Failed!\r\n")));
        delete pPF;
        return NULL;
    }

    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("POF_Init - hDev=0x%x\r\n"), pPF));

    PF_FUNCTION_EXIT();

    // Otherwise return the created instance
    return (DWORD) pPF;
}


//------------------------------------------------------------------------------
//
// Function: POF_Deinit
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
BOOL POF_Deinit(DWORD hDeviceContext)
{
    PfClass * pPF = (PfClass*) hDeviceContext;

    PF_FUNCTION_ENTRY();

    if (pPF != NULL)
    {
        delete pPF;
    }

    PF_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: POF_Open
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
DWORD POF_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    PF_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);
    PF_FUNCTION_EXIT();
    return hDeviceContext;
}


//------------------------------------------------------------------------------
//
// Function: POF_Close
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
BOOL POF_Close(DWORD hOpenContext)
{
    PF_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    PF_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: POF_PowerDown
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
void POF_PowerDown(DWORD hDeviceContext)
{
    PF_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    PF_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: POF_PowerUp
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
void POF_PowerUp(void)
{
    PF_FUNCTION_ENTRY();
    PF_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: POF_IOControl
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
BOOL POF_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    pfConfigData  *pConfigData;
    pfStartParams *pStartParms;
    DWORD *pdwResumeReturn;
    UINT32 *pauseRow;
    BOOL bRet = FALSE;
    PVOID pDestMarshalled;      // For WinCE 6.00 pointer marshalling.

    int *size;
    PhysicalAddress phyaddress;
    UINT driverVirtualAddress;
    pfAllocMemoryParams *pMemoryParams;
    pPfSetAttributeExData pSetData;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pdwActualOut);

    // hOpenContext is a pointer to I2CClass instance!
    PfClass* pPF = (PfClass*) hOpenContext;

    switch(dwCode)
    {
    
        case PF_IOCTL_SETATTEX:
            // WinCE 6.00 no longer supports MapCallerPtr().
            // However, the kernel now automatically checks the validity
            // of all pointer arguments such as pBufIn so we can just
            // use it as-is.
            pSetData = (pPfSetAttributeExData)pBufIn;

            bRet = pPF->PfSetAttributeEx(pSetData);
            DEBUGMSG(ZONE_IOCTL, (TEXT("POF_IOControl: PfSetAttributeEx ")
                                  TEXT("occurred\r\n")));
            break;
        
        case PF_IOCTL_CONFIGURE:
            // WinCE 6.00 no longer supports MapCallerPtr().
            // However, the kernel now automatically checks the validity
            // of all pointer arguments such as pBufIn so we can just
            // use it as-is.
            pConfigData = (pPfConfigData)pBufIn;

            bRet = pPF->PfConfigure(pConfigData);
            DEBUGMSG(ZONE_IOCTL, (TEXT("POF_IOControl: PF_IOCTL_CONFIGURE ")
                                  TEXT("occurred\r\n")));
            break;

        case PF_IOCTL_START:
            // WinCE 6.00 no longer supports MapCallerPtr().
            // However, the kernel now automatically checks the validity
            // of all pointer arguments such as pBufIn so we can just
            // use it as-is.
            pStartParms = (pPfStartParams)pBufIn;

            // WinCE 6.00 no longer supports MapCallerPtr().
            // However, we must still check the validity of the "in"
            // and "out" pointers that are embedded within "pStartParms".
            if (FAILED(CeOpenCallerBuffer(&pDestMarshalled,
                                          ((pPfStartParams)pBufIn)->in,
                                          sizeof(pfBuffer),
                                          ARG_I_PTR,
                                          FALSE)))
            {
                ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() for pBufIn->in ")
                                TEXT("failed\r\n")));
                break;
            }

            pStartParms->in = (pPfBuffer)pDestMarshalled;

            if (FAILED(CeOpenCallerBuffer(&pDestMarshalled,
                                          ((pPfStartParams)pBufIn)->out,
                                          sizeof(pfBuffer),
                                          ARG_O_PTR,
                                          FALSE)))
            {
                ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() for ")
                                TEXT("pBufIn->out failed\r\n")));

                // Release previously marshalled input buffer pointer.
                if (FAILED(CeCloseCallerBuffer(pStartParms->in,
                                               ((pPfStartParams)pBufIn)->in,
                                               sizeof(pfBuffer),
                                               ARG_I_PTR)))
                {
                    ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for ")
                                    TEXT("pBufIn->in failed\r\n")));
                }

                break;
            }

            pStartParms->out = (pPfBuffer)pDestMarshalled;

            bRet = pPF->PfStartChannel(pStartParms);

            // For WinCE 6.00 we must release the marshalled buffers here.
            if (FAILED(CeCloseCallerBuffer(pStartParms->out,
                                           ((pPfStartParams)pBufIn)->out,
                                           sizeof(pfBuffer),
                                           ARG_O_PTR)))
            {
                ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for pBufIn->out ")
                                TEXT("failed\r\n")));
                //break;
            }

            if (FAILED(CeCloseCallerBuffer(pStartParms->in,
                                           ((pPfStartParams)pBufIn)->in,
                                           sizeof(pfBuffer),
                                           ARG_I_PTR)))
            {
                ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for pBufIn->in ")
                                TEXT("failed\r\n")));
                break;
            }

            DEBUGMSG(ZONE_IOCTL, (TEXT("POF_IOControl: PF_IOCTL_START ")
                                  TEXT("occurred\r\n")));
            break;

        case PF_IOCTL_START2: //for Direct Render
            // WinCE 6.00 no longer supports MapCallerPtr().
            // However, the kernel now automatically checks the validity
            // of all pointer arguments such as pBufIn so we can just
            // use it as-is.
            pStartParms = (pPfStartParams)pBufIn;

            // WinCE 6.00 no longer supports MapCallerPtr().
            // However, we must still check the validity of the "in"
            // and "out" pointers that are embedded within "pStartParms".
            if (FAILED(CeOpenCallerBuffer(&pDestMarshalled,
                                          ((pPfStartParams)pBufIn)->in,
                                          sizeof(pfBuffer),
                                          ARG_I_PTR,
                                          FALSE)))
            {
                ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() for pBufIn->in ")
                                TEXT("failed\r\n")));
                break;
            }

            pStartParms->in = (pPfBuffer)pDestMarshalled;

            if (FAILED(CeOpenCallerBuffer(&pDestMarshalled,
                                          ((pPfStartParams)pBufIn)->out,
                                          sizeof(pfBuffer),
                                          ARG_O_PTR,
                                          FALSE)))
            {
                ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() for ")
                                TEXT("pBufIn->out failed\r\n")));

                // Release previously marshalled input buffer pointer.
                if (FAILED(CeCloseCallerBuffer(pStartParms->in,
                                               ((pPfStartParams)pBufIn)->in,
                                               sizeof(pfBuffer),
                                               ARG_I_PTR)))
                {
                    ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for ")
                                    TEXT("pBufIn->in failed\r\n")));
                }

                break;
            }

            pStartParms->out = (pPfBuffer)pDestMarshalled;

            bRet = pPF->PfStartChannel2(pStartParms,dwLenOut,*(unsigned int*)pdwActualOut);

            // For WinCE 6.00 we must release the marshalled buffers here.
            if (FAILED(CeCloseCallerBuffer(pStartParms->out,
                                           ((pPfStartParams)pBufIn)->out,
                                           sizeof(pfBuffer),
                                           ARG_O_PTR)))
            {
                ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for pBufIn->out ")
                                TEXT("failed\r\n")));
                //break;
            }

            if (FAILED(CeCloseCallerBuffer(pStartParms->in,
                                           ((pPfStartParams)pBufIn)->in,
                                           sizeof(pfBuffer),
                                           ARG_I_PTR)))
            {
                ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for pBufIn->in ")
                                TEXT("failed\r\n")));
                break;
            }

            DEBUGMSG(ZONE_IOCTL, (TEXT("POF_IOControl: PF_IOCTL_START2 ")
                                  TEXT("occurred\r\n")));
            break;


        case PF_IOCTL_RESUME:
            // WinCE 6.00 no longer supports MapCallerPtr().
            // However, the kernel now automatically checks the validity
            // of all pointer arguments such as pBufOut so we can just
            // use it as-is.
            pdwResumeReturn = (DWORD *)pBufOut;

            // WinCE 6.00 no longer supports MapCallerPtr().
            // However, the kernel now automatically checks the validity
            // of all pointer arguments such as pBufIn so we can just
            // use it as-is.
            pauseRow = (UINT32 *)pBufIn;

            *pdwResumeReturn = pPF->PfResume(*pauseRow);
            if (pdwResumeReturn == PF_SUCCESS)
            {
                bRet = TRUE;
            }
            DEBUGMSG(ZONE_IOCTL, (TEXT("POF_IOControl: PF_IOCTL_RESUME ")
                                  TEXT("occurred\r\n")));
            break;

        case PF_IOCTL_ALLOC_PHY_MEM:
        
            if (FAILED(CeOpenCallerBuffer((LPVOID*)&pMemoryParams, pBufOut, sizeof(pfAllocMemoryParams), ARG_IO_PTR, FALSE)))
            {
                ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() for pBufOut ")
                                TEXT("failed\r\n")));
                break;
            }

            if (FAILED(CeOpenCallerBuffer((LPVOID*)&size, pBufIn, sizeof(int), ARG_IO_PTR,FALSE)))
            {
                ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() for pBufIn ")
                                TEXT("failed\r\n")));
                                
                 // Release previously marshalled input buffer pointer.
                if (FAILED(CeCloseCallerBuffer(pMemoryParams,
                                               (PVOID)pBufOut,
                                               sizeof(pfAllocMemoryParams),
                                               ARG_IO_PTR)))
                {
                    ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for ")
                                    TEXT("pBufOut failed\r\n")));
                }

                break;   
            }
                        
            if ((dwLenIn != sizeof(int)) || (dwLenOut != sizeof(pfAllocMemoryParams)) || (pBufIn == NULL))
            {
                ERRORMSG(TRUE, (TEXT("dwLenIn or dwLenOut or PBufIn -- Invalid PF params\r\n")));
                break;
            }

            pMemoryParams->userVirtAddr = (UINT)pPF->PfAllocPhysMem(*size, &phyaddress, &driverVirtualAddress); 
            pMemoryParams->physAddr = (UINT)phyaddress;
            pMemoryParams->driverVirtAddr = driverVirtualAddress;
            
            if (FAILED(CeCloseCallerBuffer((PVOID)size, (PVOID)pBufIn,sizeof(int),ARG_IO_PTR)))
            {
                ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for pBufIn ")
                                TEXT("failed\r\n")));
                //break;
            }
            
            if (FAILED(CeCloseCallerBuffer((PVOID)pMemoryParams, (PVOID)pBufOut,sizeof(pfAllocMemoryParams),ARG_IO_PTR)))
            { 
                ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for pBufOut ")
                                TEXT("failed\r\n")));
                break;
            }

            if (pMemoryParams->userVirtAddr == NULL)
            {
                ERRORMSG(TRUE, (TEXT("The returned user virtual address is empty!\r\n")));
                break;                   
            }
            else
                bRet = TRUE;

            break;
            
        case PF_IOCTL_FREE_PHY_MEM:
        
            pMemoryParams = (pPfAllocMemoryParams)pBufIn;
            
            if (FAILED(CeOpenCallerBuffer((LPVOID*)&pMemoryParams, pBufIn, sizeof(int), ARG_IO_PTR,FALSE)))
            {
                ERRORMSG(TRUE, (TEXT("CeOpenCallerBuffer() for pBufIn ")
                                TEXT("failed\r\n")));
                break;   
            }
            
            pPF->pfFreePhysMem(pMemoryParams);
            if (FAILED(CeCloseCallerBuffer((PVOID)pMemoryParams, (PVOID)pBufIn,sizeof(int),ARG_IO_PTR)))
            {
                ERRORMSG(TRUE, (TEXT("CeCloseCallerBuffer() for pBufIn->In ")
                                TEXT("failed\r\n")));
                break;
            }
            
            bRet = TRUE;
            break;
            
        default:
            DEBUGMSG(ZONE_WARN, (TEXT("POF_IOControl: No matching IOCTL.\r\n")));
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
         DEBUGMSG(ZONE_INFO, (TEXT("POF_DllEntry: DLL_PROCESS_ATTACH ")
                              TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
         DisableThreadLibraryCalls((HMODULE) hInstDll);
         break;

      case DLL_PROCESS_DETACH:
         DEBUGMSG(ZONE_INFO, (TEXT("POF_DllEntry: DLL_PROCESS_DETACH ")
                              TEXT("lpvReserved(0x%x)\r\n"), lpvReserved));
         break;
   }
   // Return TRUE for success
   return TRUE;
}
