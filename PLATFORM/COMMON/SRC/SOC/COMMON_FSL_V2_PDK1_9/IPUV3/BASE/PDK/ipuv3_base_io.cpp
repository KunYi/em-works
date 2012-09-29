//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#pragma warning(pop)

#include "common_macros.h"
#include "ipuv3_base_include.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"
#include "ipuv3_base_priv.h"


//------------------------------------------------------------------------------
// External Functions
extern "C" DWORD CSPIPUGetBaseAddr();

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
    _T("IPU"),
    {
        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Func"),
        TEXT("Info"), TEXT(""), TEXT(""), TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN // ulZoneMask
//0xffff
};
#endif  // DEBUG


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the IPU common module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the PMIC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER((HMODULE)hInstDll);
        DEBUGMSG(ZONE_INIT, (_T("***** DLL PROCESS ATTACH TO IPU_BASE DLL *****\r\n")));
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
// Function:  Init
//
// This function initializes the IPU Base common component.  Called by the
// Device Manager to initialize a device.
//
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD IPU_Init(LPCTSTR pContext, DWORD dwBusContext)
{
    DWORD rc = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(dwBusContext);

    if (!InitIPU())
    {
        DEBUGMSG(ZONE_INIT, (_T("InitIPU failed!")));
        goto cleanUp;
    }

    rc = 1;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  Deinit
//
// This function deinitializes the IPU Base common component.  Called by the
// Device Manager to de-initialize a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL IPU_Deinit(DWORD hDeviceContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);

    DeinitIPU();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: Open
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
DWORD IPU_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    IPU_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    IPU_FUNCTION_EXIT();
    return hDeviceContext;
}


//------------------------------------------------------------------------------
//
// Function: Close
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
BOOL IPU_Close(DWORD hOpenContext)
{
    IPU_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    IPU_FUNCTION_EXIT();
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: IOControl
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
BOOL IPU_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRet = FALSE;
    DWORD *pdwIPUBaseAddr;
    DWORD *pdwVidMemSize;
    DWORD *pdwVidMemBase;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(dwLenIn);
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    UNREFERENCED_PARAMETER(pdwActualOut);

    switch(dwCode)
    {
        case IPU_IOCTL_GET_BASE_ADDR:
            pdwIPUBaseAddr = (DWORD *)pBufOut;
            *pdwIPUBaseAddr = CSPIPUGetBaseAddr();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_GET_BASE_ADDR occurred\r\n")));
            break;

        case IPU_IOCTL_ENABLE_SUBMODULE:
            switch(dwLenIn)
            {
                case IPU_SUBMODULE_IC:
                    EnableIC(*(IPU_DRIVER *)pBufOut);
                    break;
                case IPU_SUBMODULE_IRT:
                    EnableIRT(*(IPU_DRIVER *)pBufOut);
                    break;
                case IPU_SUBMODULE_DP:
                    EnableDP();
                    break;
                case IPU_SUBMODULE_ISP:
                    EnableISP();
                    break;
                case IPU_SUBMODULE_DI0:
                    EnableDI0();
                    break;
                case IPU_SUBMODULE_DI1:
                    EnableDI1();
                    break;
                case IPU_SUBMODULE_DC:
                    EnableDC();
                    break;
                case IPU_SUBMODULE_DMFC:
                    EnableDMFC();
                    break;
                case IPU_SUBMODULE_CSI0:
                    EnableCSI0();
                    break;
                case IPU_SUBMODULE_CSI1:
                    EnableCSI1();
                    break;
                case IPU_SUBMODULE_SMFC:
                    EnableSMFC();
                    break;
                case IPU_SUBMODULE_SISG:
                    EnableSISG();
                    break;
                case IPU_SUBMODULE_VDI:
                    EnableVDI();
                    break;
            }

            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ENABLE_SUBMODULE occurred\r\n")));
            break;

        case IPU_IOCTL_DISABLE_SUBMODULE:
            switch(dwLenIn)
            {
                case IPU_SUBMODULE_IC:
                    DisableIC(*(IPU_DRIVER *)pBufOut);
                    break;
                case IPU_SUBMODULE_IRT:
                    DisableIRT(*(IPU_DRIVER *)pBufOut);
                    break;
                case IPU_SUBMODULE_DP:
                    DisableDP();
                    break;
                case IPU_SUBMODULE_ISP:
                    DisableISP();
                    break;
                case IPU_SUBMODULE_DI0:
                    DisableDI0();
                    break;
                case IPU_SUBMODULE_DI1:
                    DisableDI1();
                    break;
                case IPU_SUBMODULE_DC:
                    DisableDC();
                    break;
                case IPU_SUBMODULE_DMFC:
                    DisableDMFC();
                    break;
                case IPU_SUBMODULE_CSI0:
                    DisableCSI0();
                    break;
                case IPU_SUBMODULE_CSI1:
                    DisableCSI1();
                    break;
                case IPU_SUBMODULE_SMFC:
                    DisableSMFC();
                    break;
                case IPU_SUBMODULE_SISG:
                    DisableSISG();
                    break;
                case IPU_SUBMODULE_VDI:
                    DisableVDI();
                    break;
            }

            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_DISABLE_SUBMODULE occurred\r\n")));
            break;

        case IPU_IOCTL_ENABLE_CLOCKS:
            EnableHSPClock();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ENABLE_CLOCKS occurred\r\n")));
            break;

        case IPU_IOCTL_DISABLE_CLOCKS:
            DisableHSPClock();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_DISABLE_CLOCKS occurred\r\n")));
            break;

        case IPU_IOCTL_GET_VIDEO_MEMORY_SIZE:
            pdwVidMemSize = (DWORD *)pBufOut;
            *pdwVidMemSize = IPUGetVideoMemorySize();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ALLOCATE_BUFFER occurred\r\n")));
            break;

        case IPU_IOCTL_ALLOCATE_BUFFER:
            IPUAllocateBuffer((pIPUBufferInfo)pBufIn,(IpuBuffer *)pBufOut);
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ALLOCATE_BUFFER occurred\r\n")));
            break;

        case IPU_IOCTL_DEALLOCATE_BUFFER:
            IPUDeallocateBuffer((IpuBuffer *)pBufIn);
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ALLOCATE_BUFFER occurred\r\n")));
            break;

        case IPU_IOCTL_GET_VIDEO_MEMORY_BASE:
            pdwVidMemBase = (DWORD *)pBufOut;
            *pdwVidMemBase = IPUGetVideoMemoryBase();
            bRet = TRUE;
            DEBUGMSG(ZONE_INFO, (TEXT("IPU_IOControl: IPU_IOCTL_ALLOCATE_BUFFER occurred\r\n")));
            break;
            
        default:
            DEBUGMSG(ZONE_WARN, (TEXT("IPU_IOControl: No matching IOCTL.\r\n")));
            break;
    }

    return bRet;
}
