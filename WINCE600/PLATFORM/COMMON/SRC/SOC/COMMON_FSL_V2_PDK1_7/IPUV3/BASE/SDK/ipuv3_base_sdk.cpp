//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ipuv3_base_sdk.c
//
//  This module provides wrapper functions for accessing
//  the stream interface for the IPU_BASE driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#pragma warning(pop)

#include "common_macros.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "ipuv3_base_priv.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: IPUV3BaseOpenHandle
//
// This method creates a handle to the PP stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to PP driver, which is set in this method.
//      Returns INVALID_HANDLE_VALUE if failure.
//
//------------------------------------------------------------------------------
HANDLE IPUV3BaseOpenHandle(void)
{
    HANDLE hIPUBase;

    hIPUBase = CreateFile(TEXT("IPU1:"),        // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to PP
    if (hIPUBase == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(1, (TEXT("%s:  CreateFile IPU_BASE failed!\r\n"), __WFUNCTION__));
    }

    return hIPUBase;
}


//------------------------------------------------------------------------------
//
// Function: IPUV3BaseCloseHandle
//
// This method closes a handle to the IPUBase stream driver.
//
// Parameters:
//      hIPUBase
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL IPUV3BaseCloseHandle(HANDLE hIPUBase)
{
    BOOL retVal = TRUE;

    // if we don't have handle to PP driver
    if (hIPUBase != NULL)
    {
        if (!CloseHandle(hIPUBase))
        {
            retVal = FALSE;
        }
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: IPUV3BaseGetBaseAddr
//
// This method retrieves the offset from the IPUv3 base
// driver offset in memory.
//
// Parameters:
//      None
//
// Returns:
//      The IPUv3 base address.  -1 if failure.
//
//------------------------------------------------------------------------------
DWORD IPUV3BaseGetBaseAddr(HANDLE hIPUBase)
{
    DWORD dwIPUBaseAddr = (UINT32) -1;

    // issue the IOCTL to get IPUv3 base addr
    if (!DeviceIoControl(hIPUBase,     // file handle to the driver
        IPU_IOCTL_GET_BASE_ADDR,    // I/O control code
        NULL,                     // in buffer
        0,                        // in buffer size
        &dwIPUBaseAddr,           // out buffer
        sizeof(DWORD),            // out buffer size
        0,                        // number of bytes returned
        NULL))                    // ignored (=NULL)
    {
        ERRORMSG(1, (TEXT("%s: IPU_IOCTL_GET_BASE_ADDR failed!\r\n"), __WFUNCTION__));
    }

    return dwIPUBaseAddr;
}

//------------------------------------------------------------------------------
//
// Function: IPUV3EnableModule
//
// This method enables the specified IPUv3 submodule.
//
// Parameters:
//      hIPUBase
//          [in] Handle to IPU Base driver.
//
//      submodule
//          [in] IPUv3 submodule to enable.
//
//      driver
//          [in] For IPU_SUBMODULE_IC or IPU_SUBMODULE_IRT, this
//          parameter selects the driver that is calling to enable/disable
//          the IC or IRT.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL IPUV3EnableModule(HANDLE hIPUBase, IPU_SUBMODULE submodule, IPU_DRIVER driver)
{
    BOOL retVal = TRUE;

    if ((submodule >= maxNumSubmodules) || (submodule < 0))
    {
        ERRORMSG(1, (TEXT("%s: Invalid submodule parameter!  Aborting.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // issue the IOCTL to enable IPU module
    if (!DeviceIoControl(hIPUBase,     // file handle to the driver
        IPU_IOCTL_ENABLE_SUBMODULE,    // I/O control code
        NULL,                     // in buffer
        submodule,                // in buffer size
        &driver,                  // out buffer
        sizeof(IPU_DRIVER),       // out buffer size
        0,                        // number of bytes returned
        NULL))                    // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("%s: IPU_IOCTL_ENABLE_SUBMODULE failed!\r\n"), __WFUNCTION__));
    }

    return retVal;
}

//------------------------------------------------------------------------------
//
// Function: IPUV3DisableModule
//
// This method disables the specified IPUv3 submodule.
//
// Parameters:
//      hIPUBase
//          [in] Handle to IPU Base driver.
//
//      submodule
//          [in] IPUv3 submodule to disable.
//
//      driver
//          [in] For IPU_SUBMODULE_IC or IPU_SUBMODULE_IRT, this
//          parameter selects the driver that is calling to enable/disable
//          the IC or IRT.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL IPUV3DisableModule(HANDLE hIPUBase, IPU_SUBMODULE submodule, IPU_DRIVER driver)
{
    BOOL retVal = TRUE;

    if ((submodule >= maxNumSubmodules) || (submodule < 0))
    {
        ERRORMSG(1, (TEXT("%s: Invalid submodule parameter!  Aborting.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    // issue the IOCTL to disable IPU module
    if (!DeviceIoControl(hIPUBase,     // file handle to the driver
        IPU_IOCTL_DISABLE_SUBMODULE,    // I/O control code
        NULL,                     // in buffer
        submodule,                // in buffer size
        &driver,                  // out buffer
        sizeof(IPU_DRIVER),       // out buffer size
        0,                        // number of bytes returned
        NULL))                    // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("%s: IPU_IOCTL_DISABLE_SUBMODULE failed!\r\n"), __WFUNCTION__));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: IPUV3EnableClocks
//
// This method enables the IPUv3 HSP clock, used for the following purposes in the IPU:
//      - General clock to control IPU operation (DMA, processing modules, etc.)
//      - Clock controls access to IPU registers
//      - May be used to provide a pixel clock for a display panel.
//
// Note: IPU clocks are automatically enabled whenever any IPU submodule is enabled,
// so this function may not need to be called if IPU submodules are enabled.  The
// primary purpose of this function is to provide a means for enabling IPU clocks
// so that the IPU registers can be written, without having to enable IPU submodules.
//
// Parameters:
//      hIPUBase
//          [in] Handle to IPU Base driver.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL IPUV3EnableClocks(HANDLE hIPUBase)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to enable IPU clocks
    if (!DeviceIoControl(hIPUBase,     // file handle to the driver
        IPU_IOCTL_ENABLE_CLOCKS,    // I/O control code
        NULL,                     // in buffer
        0,                        // in buffer size
        NULL,                     // out buffer
        0,                        // out buffer size
        0,                        // number of bytes returned
        NULL))                    // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("%s: IPU_IOCTL_ENABLE_CLOCKS failed!\r\n"), __WFUNCTION__));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: IPUV3DisableClocks
//
// This method disables the IPUv3 HSP clock, used for the following purposes in the IPU:
//      - General clock to control IPU operation (DMA, processing modules, etc.)
//      - Clock controls access to IPU registers
//      - May be used to provide a pixel clock for a display panel.
//
// Note: IPU clocks are automatically enabled whenever any IPU submodule is enabled,
// so this function may not need to be called if IPU submodules are enabled.  The
// primary purpose of this function is to provide a means for enabling IPU clocks
// so that the IPU registers can be written, without having to enable IPU submodules.
//
// Parameters:
//      hIPUBase
//          [in] Handle to IPU Base driver.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL IPUV3DisableClocks(HANDLE hIPUBase)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to enable IPU clocks
    if (!DeviceIoControl(hIPUBase,     // file handle to the driver
        IPU_IOCTL_DISABLE_CLOCKS,    // I/O control code
        NULL,                     // in buffer
        0,                        // in buffer size
        NULL,                     // out buffer
        0,                        // out buffer size
        0,                        // number of bytes returned
        NULL))                    // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("%s: IPU_IOCTL_DISABLE_CLOCKS failed!\r\n"), __WFUNCTION__));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: IPUV3GetVideoMemorySize
//
// This method requests the size of the video memory region.
//
// Parameters:
//      hIPUBase
//          [in] Handle to IPU Base driver.
//
// Returns:
//      Size of video memory.  0 if failure.
//
//------------------------------------------------------------------------------
DWORD IPUV3GetVideoMemorySize(HANDLE hIPUBase)
{
    DWORD dwVidMemSize = 0;

    // issue the IOCTL to get video memory size
    if (!DeviceIoControl(hIPUBase,     // file handle to the driver
        IPU_IOCTL_GET_VIDEO_MEMORY_SIZE,    // I/O control code
        NULL,                          // in buffer
        0,                             // in buffer size
        &dwVidMemSize,                 // out buffer
        sizeof(DWORD),                 // out buffer size
        0,                             // number of bytes returned
        NULL))                         // ignored (=NULL)
    {
        dwVidMemSize = 0;
        ERRORMSG(1, (TEXT("%s: IPU_IOCTL_GET_VIDEO_MEMORY_SIZE failed!\r\n"), __WFUNCTION__));
    }

    return dwVidMemSize;
}


//------------------------------------------------------------------------------
//
// Function: IPUV3GetVideoMemoryBase
//
// This method requests the video memory base.
//
// Parameters:
//      hIPUBase
//          [in] Handle to IPU Base driver.
//
// Returns:
//      Video memory base.  0 if failure.
//
//------------------------------------------------------------------------------
DWORD IPUV3GetVideoMemoryBase(HANDLE hIPUBase)
{
    DWORD dwVidMemBase = 0;

    // issue the IOCTL to get video memory size
    if (!DeviceIoControl(hIPUBase,     // file handle to the driver
        IPU_IOCTL_GET_VIDEO_MEMORY_BASE,    // I/O control code
        NULL,                          // in buffer
        0,                             // in buffer size
        &dwVidMemBase,                 // out buffer
        sizeof(DWORD),                 // out buffer size
        0,                             // number of bytes returned
        NULL))                         // ignored (=NULL)
    {
        dwVidMemBase = 0;
        ERRORMSG(1, (TEXT("%s: IPU_IOCTL_GET_VIDEO_MEMORY_SIZE failed!\r\n"), __WFUNCTION__));
    }

    return dwVidMemBase;
}


//------------------------------------------------------------------------------
//
// Function: IPUV3AllocateBuffer
//
// This method requests an IPU buffer from the IPU buffer manager.
//
// Parameters:
//      hIPUBase
//          [in] Handle to IPU Base driver.
//
//      pBufferInfo
//          [in] Pointer to an IPUBufferInfo object holding info needed
//          to allocate an IPU buffer.
//          value.
//
//      pIpuBuffer
//          [out] Pointer to an IpuBuffer object to hold return
//          value.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL IPUV3AllocateBuffer(HANDLE hIPUBase, IPUBufferInfo *pBufferInfo, IpuBuffer *pIpuBuffer)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to allocate IPU buffer
    if (!DeviceIoControl(hIPUBase,     // file handle to the driver
        IPU_IOCTL_ALLOCATE_BUFFER,    // I/O control code
        pBufferInfo,                     // in buffer
        sizeof(IPUBufferInfo),          // in buffer size
        pIpuBuffer,                     // out buffer
        sizeof(IpuBuffer),              // out buffer size
        0,                        // number of bytes returned
        NULL))                    // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("%s: IPU_IOCTL_ALLOCATE_BUFFER failed!\r\n"), __WFUNCTION__));
    }

    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: IPUV3DeallocateBuffer
//
// This method calls to the IPU buffer manager to delete an IPU Buffer object.
//
// Parameters:
//      hIPUBase
//          [in] Handle to IPU Base driver.
//
//      pIpuBuffer
//          [out] Pointer to an IpuBuffer object to hold return
//          value.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL IPUV3DeallocateBuffer(HANDLE hIPUBase, IpuBuffer *pIpuBuffer)
{
    BOOL retVal = TRUE;

    // issue the IOCTL to allocate IPU buffer
    if (!DeviceIoControl(hIPUBase,     // file handle to the driver
        IPU_IOCTL_DEALLOCATE_BUFFER,    // I/O control code
        pIpuBuffer,                     // in buffer
        sizeof(IpuBuffer),              // in buffer size
        NULL,                           // in buffer
        0,                              // in buffer size
        0,                              // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        retVal = FALSE;
        ERRORMSG(1, (TEXT("%s: IPU_IOCTL_DEALLOCATE_BUFFER failed!\r\n"), __WFUNCTION__));
    }

    return retVal;
}
