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
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"


//-----------------------------------------------------------------------------
// External Functions
extern BOOL ApbhDmaInit(void);
extern BOOL ApbhDmaDeInit(void);
extern BOOL ApbhDmaAlloc(void);
extern BOOL ApbhDmaDealloc(void);

extern BOOL ApbxDmaInit(void);
extern BOOL ApbxDmaDeInit(void);
extern BOOL ApbxDmaAlloc(void);
extern BOOL ApbxDmaDealloc(void);

extern BOOL IomuxAlloc(void);
extern BOOL IomuxDealloc(void);
extern VOID DDKGpioInit();

extern BOOL ClockAlloc(void);
extern BOOL ClockDealloc(void);

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

        if (!ApbhDmaAlloc())
        {
            ERRORMSG(1, (_T("ApbhDmaAlloc failed!")));
            goto cleanUp;
        }
        if (!ApbxDmaAlloc())
        {
            ERRORMSG(1, (_T("ApbxDmaAlloc failed!")));
            goto cleanUp;
        }
        if (!IomuxAlloc())
        {
            ERRORMSG(1, (_T("IomuxAlloc failed!")));
            goto cleanUp;
        }

        if (!DDKPowerAlloc())
        {
            ERRORMSG(1, (_T("DDKPowerAlloc failed!")));
            goto cleanUp;
        }

        if (!ClockAlloc())
        {
            ERRORMSG(1, (_T("ClockAlloc failed!")));
            goto cleanUp;
        }      
    }
    break;

    case DLL_PROCESS_DETACH:
    {
        // DEBUGMSG(ZONE_INIT,(_T("*** DLL_PROCESS_DETACH - Current Process: 0x%x, ID: 0x%x ***\r\n"),
        //    GetCurrentProcess(), GetCurrentProcessId()));

        // Deinitialize contexts for CSPDDK components

        ApbhDmaDealloc();
        ApbxDmaDealloc();
        IomuxDealloc();
        DDKPowerDealloc();
        ClockDealloc();
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
//
// Function:  Init
//
// This function initializes the CSPDDK.  Called by the Device Manager to
// initialize a device.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL Init(void)
{
    BOOL rc = FALSE;

    if (!ApbhDmaInit())
    {
        ERRORMSG(1, (_T("ApbhDmaInit failed!")));
        goto cleanUp;
    }

    if (!ApbxDmaInit())
    {
        ERRORMSG(1, (_T("ApbxDmaInit failed!")));
        goto cleanUp;
    }

    DDKGpioInit();
    
    rc = TRUE;

cleanUp:

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  Deinit
//
// This function deinitializes the SDMA.  Called by the Device Manager to
// de-initialize a device.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL Deinit(void)
{
    BOOL rc = FALSE;

    rc = ApbhDmaDeInit();
    rc = ApbxDmaDeInit();

    return rc;
}
