//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
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

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.

// @doc EX_TOUCH_DDI INTERNAL DRIVERS MDD TOUCH_PANEL

// Module Name:

// @module tchbasic.c

// Abstract:
//    This module contains the touch panel DLL entry point.<nl>


// Functions:
// TouchPanelDllEntry
// Notes:


#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include    <windows.h>
#include    <types.h>
#include    <memory.h>
#include    <nkintr.h>
#include    <tchddi.h>
#include    <tchddsi.h>
#pragma warning(pop)

PFN_TOUCH_PANEL_CALLBACK v_pfnCgrPointCallback;
PFN_TOUCH_PANEL_CALLBACK v_pfnCgrCallback = NULL;
extern ULONG   culReferenceCount;              //@globalvar ULONG | culReferenceCount | Count of attached threads
extern PFN_TOUCH_PANEL_CALLBACK v_pfnPointCallback;
extern HANDLE hThread;

//    @func BOOL | TouchPanelDllEntry |
//    Dll entry point.

//    @rdesc
//    TRUE if the function succeeds. Otherwise, FALSE.

BOOL
DllEntry(
    HANDLE  hinstDll,    //@parm Process handle.
    DWORD   fdwReason,   //@parm Reason for calling the function.
    LPVOID  lpvReserved  //@parm Reserved, not used.
    )
{

    BOOL ReturnCode = TRUE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch ( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER(hinstDll);
            DEBUGMSG( ZONE_FUNCTION, (TEXT("Dll Process Attach\r\n")) );
            DisableThreadLibraryCalls((HMODULE) hinstDll);

            //
            // Process is attaching.  We allow only 1 process to be attached.
            // If our global counter (maintained by the PDD) is greater than 0,
            //   error.
            //
            if ( DdsiTouchPanelAttach() > 1 )
            {
                DEBUGMSG( ZONE_FUNCTION, (TEXT("DdsiTouchPanelAttach > 1\r\n")) );
                DdsiTouchPanelDetach(); // if a process attach fails, the detach is
                // never called. So adjust the count here.
                ReturnCode = FALSE;
            }

            break;
        case DLL_PROCESS_DETACH:
            DEBUGMSG( ZONE_FUNCTION,
                      (TEXT("Dll Process Detach\r\n")) );

             //
             // Process is detaching.
             // If the detaching process is the process that was allowed
             // to attach, we reset the callback functions,
             // reference count, disable the touch panel, and disconnect from the
             // logical interrupt.
             //
             //
            ASSERT(hThread==NULL);
            DdsiTouchPanelDetach();
            break;
    }
    return ( ReturnCode );
}
