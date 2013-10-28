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
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------


#include "wavemain.h"

void InputDeviceContext::StreamReadyToRender(StreamContext *pStreamContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pStreamContext);
    g_pHWContext->StartInputDMA();
    return;
}

DWORD InputDeviceContext::GetDevCaps(LPVOID pCaps, DWORD dwSize)
{
    static const WAVEINCAPS wc =
    {
        MM_MICROSOFT,        // Manufacturer ID.
        23,                  // Product ID.
        0x0001,              // Driver version.
        TEXT("Audio Input"), // Product name.
                             // Supported formats.
        WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08 | WAVE_FORMAT_4M08 |
        WAVE_FORMAT_1S08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_4S08 |
        WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_4M16 |
        WAVE_FORMAT_1S16 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4S16,

        INCHANNELS,          // See hwctxt.h for the number of input channels.
        0                    // Reserved.
    };

    if(dwSize > sizeof(wc))
    {
        dwSize = sizeof(wc);
        memcpy( pCaps, &wc, dwSize);
    }
    else
        memcpy( pCaps, &wc, sizeof(wc));

    return MMSYSERR_NOERROR;
}

DWORD InputDeviceContext::GetExtDevCaps(LPVOID pCaps, DWORD dwSize)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pCaps);
    UNREFERENCED_PARAMETER(dwSize);
    return MMSYSERR_NOTSUPPORTED;
}

StreamContext *InputDeviceContext::CreateStream(LPWAVEOPENDESC lpWOD)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpWOD);
    return new InputStreamContext;
}