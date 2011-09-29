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
//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------ 
//------------------------------------------------------------------------------
//
//  File: i_stub.c
//
//  This file contains the functions USB Funcation PDD OS layer.

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <celog.h>
#include <oal.h>
#pragma warning(pop)

#pragma warning(disable: 4100)

#include "csp.h"
#include "stub.c"


//-----------------------------------------------------------------------------
//
//  Function: CacheRangeFlush
//
//  This function is to flush a specified range of the cache.
//  Notes:
//        This function is used to replacd CacheSync to improve USB performance on MX51.
//        Since some cache related instruction are reserved on CorTex-A8, CacheSync will
//        have very low performance than CacheRangeFlush(). Because this function is just 
//        used for USB module to avoid linking other cache operation libs, ONLY 
//        CACHE_SYNC_DISCARD is implement. 
//
//  Parameters:
//     pAddr 
//        [in] Starting virtual address where the cache operation is to be performed in.
//             The cache operation is specified by the value in dwFlags.
//
//     dwLength 
//        [in] Specifies the length, in bytes.
//
//     dwFlags 
//        [in] Specifies the operation to be performed. The following table shows 
//             possible values.
//     
//  Returns:
//     NULL
//
//-----------------------------------------------------------------------------
void CacheRangeFlush(
  LPVOID pAddr,
  DWORD dwLength,
  DWORD dwFlags
)
{
    if ((dwFlags & CACHE_SYNC_DISCARD) != 0) {
        // Write back and invalidate the selected portions of the data cache
        if (dwLength == 0) {
            if (pAddr == NULL) OALFlushDCache();
        } else {
            // Normalize address to cache line alignment
            UINT32 mask = g_oalCacheInfo.L1DLineSize - 1;
            UINT32 address = (UINT32)pAddr & ~mask;
            // Adjust size to reflect cache line alignment
            dwLength += (UINT32)pAddr - address;
            // If range is bigger than cache size flush all
            if (dwLength >= g_oalCacheInfo.L1DSize) {
                OALFlushDCache();
            } else {
                // Flush all the indicated cache entries
                OALFlushDCacheLines((VOID*)pAddr, dwLength);
            }
        }
    } 
}
