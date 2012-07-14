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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
/* File: oaliocontrol.cpp
 *
 * Purpose: OAL updatabale module which will be called from kernel for user
 * mode threads. OEMs can update the list of ioctls supported by user mode
 * threads by updating the IOControl function in this module.
 *
 * OEMs can do the following updates:
 *
 * a) Change this dll name: In which case make sure to update oalioctl.h
 * file in public\common\oak\inc to update OALIOCTL_DLL symbol to relfect the
 * new name for this dll.
 *
 * b) Change the entry point name from IOControl to something else: In this case
 * make sure to update OALIOCTL_DLL_IOCONTROL symbol in public\common \
 * oak\inc\oalioctl.h file to reflect the new name of the entry point
 *
 * c) Add/Delete from the list of ioctls in IOControl function below: This entry point
 * is called whenever a user mode thread calls into kernel via KernelIoControl or
 * via KernelLibIoControl (with KMOD_OAL). For kernel mode threads, all the
 * handling for KernelIoControl and KernelLibIoControl happens inside kernel and
 * this module is not involved.
 *
 */
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <oalioctl.h>
#include <oal.h>
#include "ioctl_cfg.h"
#include "bsp_drivers.h"		// CS&ZHL MAY-26-2012: supporting EM9280 IOCTL
#pragma warning(pop)

PFN_Ioctl g_pfnExtOALIoctl;

//------------------------------------------------------------------------------
// Function: IOControl
//
// Arguments: Same signature as KernelIoControl
//    DWORD dwIoControlCode: io control code
//    PBYTE pInBuf: pointer to input buffer
//    DWORD nInBufSize: length of input buffer in bytes
//    PBYTE pOutBuf: pointer to out buffer
//    DWORD nOutBufSize: length of output buffer in bytes
//    PDWORD pBytesReturned: number of bytes returned in output buffer
//
// Return Values:
// If the function call is successful, TRUE is returned from this API call.
// If the function call is not successful, FALSE is returned from this API
// call and the last error is set to:
// a) ERROR_INVALID_PARAMETER: any of the input arguments are invalid
// b) ERROR_NOT_SUPPORTED: given ioctl is not supported
// c) any other ioctl set by OAL code
//
// Abstract:
// This is called by kernel whenever a user mode thread makes a call to
// KernelIoControl or KernelLibIoControl with io control code being an OAL
// io control code. OEMs can override what ioctls a user mode thread can call
// by enabling or disabling ioctl codes in this function.
//
//------------------------------------------------------------------------------
EXTERN_C
BOOL
IOControl(
    DWORD dwIoControlCode,
    PBYTE pInBuf,
    DWORD nInBufSize,
    PBYTE pOutBuf,
    DWORD nOutBufSize,
    PDWORD pBytesReturned
    )
{
    BOOL fRet = FALSE;

    //
    // By default the following ioctls are supported for user mode threads.
    // If a new ioctl is being added to this list, make sure the corresponding
    // data associated with that ioctl is marshalled properly to the OAL
    // ioctl implementation. In normal cases, one doesn't need any
    // marshaling as first level user specified buffers are already validated
    // by kernel that:
    // -- the buffers are within the user process space
    // Check out IsValidUsrPtr() function in vmlayout.h for details on kernel
    // validation of user specified buffers. Kernel doesn't validate that the
    // buffers are accessible; it only checks that the buffer start and end
    // addresses are within the user process space.
    //
    switch (dwIoControlCode) {
		//  MSFT Standard kernel IOCTLs
		case IOCTL_HAL_GET_CACHE_INFO:
		case IOCTL_HAL_GET_DEVICE_INFO:
		case IOCTL_HAL_GET_DEVICEID:
		case IOCTL_HAL_GET_UUID:
		case IOCTL_PROCESSOR_INFORMATION:
		case IOCTL_HAL_REBOOT:
		case IOCTL_HAL_POWER_OFF_ENABLE:
		case IOCTL_HAL_QUERY_BOOT_MODE:
		case IOCTL_HAL_SET_BOOT_SOURCE:
		case IOCTL_HAL_QUERY_UPDATE_SIG:
		case IOCTL_HAL_SET_UPDATE_SIG:
		case IOCTL_HAL_CPU_INFO_READ:			// read iMX copyright info
//#ifdef	EM9280
		//
		// CS&ZHL APR-06-2012: add more kernel IOcontrol
		//
		case IOCTL_HAL_BOARDINFO_READ:
		case IOCTL_HAL_TIMESTAMP_READ:
		case IOCTL_HAL_VENDOR_ID_READ:
		case IOCTL_HAL_CUSTOMER_ID_READ:
		case IOCTL_HAL_BOARD_STATE_READ:
//#endif	//EM9280
        // request is to service the ioctl - forward the call to OAL code
        // OAL code will set the last error if there is a failure
        fRet = (*g_pfnExtOALIoctl)(dwIoControlCode, pInBuf, nInBufSize, pOutBuf, nOutBufSize, pBytesReturned);
        break;
    default:
        SetLastError(ERROR_NOT_SUPPORTED);
        break;
    }

    return fRet;
}

BOOL
WINAPI
DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls((HINSTANCE)hDll);
        g_pfnExtOALIoctl = (PFN_Ioctl) lpReserved;
        break;
    case DLL_PROCESS_DETACH:
    default:
        break;
    }

    return TRUE;
}

