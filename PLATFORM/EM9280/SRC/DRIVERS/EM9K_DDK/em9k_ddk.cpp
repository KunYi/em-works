//------------------------------------------------------------------------------
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
//  Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  cspddk.c
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <types.h>
#include <Devload.h>
#include <windev.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"
#include "em9k_ddk.h"
#include "em9k_ddk_class.h"


//-----------------------------------------------------------------------------
// Global Variable
Em9kDDKClass* g_pEM9280DDK = NULL;

//-----------------------------------------------------------------------------
//
// Function:  DllEntry
//
// This function is an optional method of entry into a DLL. If the function
// is used, it is called by the system when processes and threads are
// initialized and terminated, or on calls to the LoadLibrary and
// FreeLibrary functions.
//
// Parameters:
//      hinstDLL
//          [in] Handle to the DLL. The value is the base address of the DLL.
//
//      dwReason
//          [in] Specifies a flag indicating why the DLL entry-point function
//          is being called.
//
//      lpvReserved
//          [in] Specifies further aspects of DLL initialization and cleanup.
//          If dwReason is DLL_PROCESS_ATTACH, lpvReserved is NULL for
//          dynamic loads and nonnull for static loads. If dwReason is
//          DLL_PROCESS_DETACH, lpvReserved is NULL if DllMain is called
//          by using FreeLibrary and nonnull if DllMain is called during
//          process termination.
//
// Returns:
//      When the system calls the DllMain function with the
//      DLL_PROCESS_ATTACH value, the function returns TRUE if it
//      succeeds or FALSE if initialization fails.
//
//      If the return value is FALSE when DllMain is called because the
//      process uses the LoadLibrary function, LoadLibrary returns NULL.
//
//      If the return value is FALSE when DllMain is called during
//      process initialization, the process terminates with an error.
//
//      When the system calls the DllMain function with a value other
//      than DLL_PROCESS_ATTACH, the return value is ignored.
//
//-----------------------------------------------------------------------------
BOOL WINAPI DllEntry(HINSTANCE hDllHandle, DWORD dwReason,
                     LPVOID lpreserved)
{
    BOOL rc = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpreserved);

    // RETAILMSG(1, (_T("CSPDDK DllEntry:  dwReason = %d"), dwReason));

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
		{
			// DEBUGREGISTER(hDllHandle);
			// DEBUGMSG(ZONE_INIT,(_T("*** DLL_PROCESS_ATTACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),
			//    GetCurrentProcess(), GetCurrentProcessId()));
			DisableThreadLibraryCalls((HMODULE) hDllHandle);

			// Perform context initialization for CSPDDK components
			g_pEM9280DDK = new Em9kDDKClass();
			if (!g_pEM9280DDK)
			{
				ERRORMSG(1, (_T("Create EM9280 DDK instance failed!\r\n")));
				goto cleanUp;
			}
			RETAILMSG(1, (_T("EM9280 DDK instance created.\r\n")));
		}
		break;

    case DLL_PROCESS_DETACH:
		{
			// DEBUGMSG(ZONE_INIT,(_T("*** DLL_PROCESS_DETACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),
			//    GetCurrentProcess(), GetCurrentProcessId()));

			// Deinitialize contexts for EM9K_DDK components
			if(g_pEM9280DDK)
			{
				delete g_pEM9280DDK;
				g_pEM9280DDK = NULL;
				RETAILMSG(1, (_T("EM9280 DDK instance deleted.\r\n")));
			}
		}
		break;

    default:
        break;
    }

    rc = TRUE;

cleanUp:

    return rc;
}


//-----------------------------------------------------------------------------
// Export Functions
//-----------------------------------------------------------------------------
BOOL DDKGpioSpiRead(DWORD dwCS, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength)
{
	BOOL bRet = FALSE;

	if(g_pEM9280DDK)
	{
		bRet = g_pEM9280DDK->DDKGpioSpiRead(dwCS, dwCmdAddr, pBuf, dwLength);
	}

	return bRet;
}

BOOL DDKGpioSpiWrite(DWORD dwCS, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength)
{
	BOOL bRet = FALSE;

	if(g_pEM9280DDK)
	{
		bRet = g_pEM9280DDK->DDKGpioSpiWrite(dwCS, dwCmdAddr, pBuf, dwLength);
	}

	return bRet;
}

BOOL DDKGpioI2cWrite(DWORD dwDeviceID, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength)
{
	BOOL bRet = FALSE;

	if(g_pEM9280DDK)
	{
		bRet = g_pEM9280DDK->DDKGpioI2cWrite(dwDeviceID, dwCmdAddr, pBuf, dwLength);
	}

	return bRet;
}

BOOL DDKGpioI2cRead(DWORD dwDeviceID, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength)
{
	BOOL bRet = FALSE;

	if(g_pEM9280DDK)
	{
		bRet = g_pEM9280DDK->DDKGpioI2cRead(dwDeviceID, dwCmdAddr, pBuf, dwLength);
	}

	return bRet;
}

BOOL DDKGpioxPinOutEn(DDK_GPIOX_PIN gpiox_pin)
{
	BOOL bRet = FALSE;

	if(g_pEM9280DDK)
	{
		bRet = g_pEM9280DDK->DDKGpioxPinOutEn(gpiox_pin);
	}

	return bRet;
}

BOOL DDKGpioxPinOutDis(DDK_GPIOX_PIN gpiox_pin)
{
	BOOL bRet = FALSE;

	if(g_pEM9280DDK)
	{
		bRet = g_pEM9280DDK->DDKGpioxPinOutDis(gpiox_pin);
	}

	return bRet;
}

BOOL DDKGpioxPinSet(DDK_GPIOX_PIN gpiox_pin)
{
	BOOL bRet = FALSE;

	if(g_pEM9280DDK)
	{
		bRet = g_pEM9280DDK->DDKGpioxPinSet(gpiox_pin);
	}

	return bRet;
}

BOOL DDKGpioxPinClear(DDK_GPIOX_PIN gpiox_pin)
{
	BOOL bRet = FALSE;

	if(g_pEM9280DDK)
	{
		bRet = g_pEM9280DDK->DDKGpioxPinClear(gpiox_pin);
	}

	return bRet;
}

BOOL DDKGpioxPinState(DDK_GPIOX_PIN gpiox_pin, PDWORD pRetState)
{
	BOOL bRet = FALSE;

	if(g_pEM9280DDK)
	{
		bRet = g_pEM9280DDK->DDKGpioxPinState(gpiox_pin, pRetState);
	}

	return bRet;
}
