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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  Debugging support.
//

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>

#ifndef SHIP_BUILD

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings =
{
    L"WAVEDEV",
        {
        L"HWBridge", L"IOCTL",   L"Verbose",     L"Irq",
        L"WODM",     L"WIDM",    L"PDD",         L"MDD",
        L"Mixer",    L"Ril",     L"Unused",      L"Modem",
        L"Power",    L"Function",L"Warning",     L"Error"
        },
    0xC000
};

#endif