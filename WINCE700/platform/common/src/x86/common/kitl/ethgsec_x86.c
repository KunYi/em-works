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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <oal_kitl.h>

extern BOOL IsAfterPostInit();

DWORD OEMKitlGetSecs()
{
    SYSTEMTIME st;
    DWORD dwRet;
    static DWORD dwLastTime;  // For making sure we aren't running backward
    static DWORD dwBias;

    // after post-init, it's possible that someone is holding the RTC CS, and we cannot safely call OEMGetRealTime while in KITL.
    // So we use CurMSec directly after post init.
    // NOTE: if this function is called while interrupts are off, the time will not advance.
    // 
    if (!IsAfterPostInit()) 
    {
        OEMGetRealTime( &st );
        dwRet = ((60UL * (60UL * (24UL * (31UL * st.wMonth + st.wDay) + st.wHour) + st.wMinute)) + st.wSecond);
        dwBias = dwRet;
    } 
    else 
    {
        dwRet = (CurMSec/1000) + dwBias;
    }

    if (dwRet < dwLastTime) 
    {
        KITLOutputDebugString ("! Time went backwards (or wrapped): cur: %u, last %u\n",
                              dwRet,dwLastTime);
    }

    dwLastTime = dwRet;
    return (dwRet);
}


UINT32 OALGetTickCount()
{
    return OEMKitlGetSecs () * 1000;
}


