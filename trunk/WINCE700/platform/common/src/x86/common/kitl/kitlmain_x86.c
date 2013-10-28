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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

Abstract:  

Functions:


Notes: 

--*/
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <x86boot.h>
#include <oal_kitl.h>
#include <x86kitl.h>

PX86BootInfo g_pX86Info;

//------------------------------------------------------------------------------
//
// OEMKitlStartup
//
// First entry point to OAL when Kitldll is loaded. 
// Called by the kernel after the BSP calls KITLIoctl( IOCTL_KITL_STARTUP ) in OEMInit()
//
BOOL OEMKitlStartup()
{
    g_pX86Info     = g_pOemGlobal->pKitlInfo;
    g_pOemGlobal->pfnKDIoctl = OEMKDIoControl;
    // start KITL
    OALKitlStart ();

    return TRUE;
}

