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
//------------------------------------------------------------------------------
//
//  File:  init.c
//
//  This file implements OALCacheGlobalsInit function
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <oal.h>

//
//  Defines
//
//L2 cache controller base register address
#define CSP_BASE_REG_PA_L2CC        (0x30000000)

//Register offsets
#define L2CC_TYPE_OFFSET   0x004
#define L2CC_CR_OFFSET     0x100
#define L2CC_AUXCR_OFFSET  0x104

//H bit mask in the cache type register
#define L2CC_CACHE_TYPE_MASK        0x01000000
//Associativity mask of the Auxcr register
#define L2CC_CACHE_ASSOC_MASK   0x1E000

//The cache line length is not dynamically configurable
//for the L2CC, as per Section 14.1.1 of Mx31 processor manual
//the L2 cache line length is 32 bytes and also the cache type register
//does not give proper values.
#define L2CC_CACHE_CACHELEN     32
//The L2 cache size is 128K as per Section 14.1.1 of Mx31 processor manual.
#define L2CC_CACHE_SIZE       128 * 1024

extern VOID OALGetL1CacheInfo();

//
//  Globals
//

//------------------------------------------------------------------------------
//
//  Function:  OALGetL2CacheInfo
//
//  This function initializes globals variables with L2 cache info if L2 cache
//  is enabled in the boot loader.
//
VOID OALGetL2CacheInfo()
{
    DWORD dwAuxCr, dwType, dwCacheInfo;

    //Fill in the L2 Cache details only if it is enabled
    //from the boot loader menu.
    dwCacheInfo = INREG32((OALPAtoUA(CSP_BASE_REG_PA_L2CC+L2CC_CR_OFFSET)));
    if (dwCacheInfo & 0x1)
    {
        dwType = INREG32((OALPAtoUA(CSP_BASE_REG_PA_L2CC+L2CC_TYPE_OFFSET)));
        dwAuxCr = INREG32((OALPAtoUA(CSP_BASE_REG_PA_L2CC+L2CC_AUXCR_OFFSET)));

        //Check if the cache is unified.
        if (!(dwType & L2CC_CACHE_TYPE_MASK))
        {
            g_oalCacheInfo.L2Flags |= CF_UNIFIED;
        }

        g_oalCacheInfo.L2DLineSize  = L2CC_CACHE_CACHELEN;
        g_oalCacheInfo.L2DNumWays   = (dwAuxCr & L2CC_CACHE_ASSOC_MASK) >> 13;
        g_oalCacheInfo.L2DSize      = L2CC_CACHE_SIZE;
        g_oalCacheInfo.L2DSetsPerWay = g_oalCacheInfo.L2DSize/(g_oalCacheInfo.L2DNumWays * g_oalCacheInfo.L2DLineSize);


        //Fill in the same details for I Cache since this is a Unified cache.
        g_oalCacheInfo.L2ILineSize    = g_oalCacheInfo.L2DLineSize;
        g_oalCacheInfo.L2INumWays     = g_oalCacheInfo.L2DNumWays;
        g_oalCacheInfo.L2ISetsPerWay  = g_oalCacheInfo.L2DSetsPerWay;
        g_oalCacheInfo.L2ISize        = g_oalCacheInfo.L2DSize;
    }
}

//------------------------------------------------------------------------------
//
//  Function:  OALCacheGlobalsInit
//
//  This function initializes globals variables which holds cache parameters.
//  It must be called before any other cache/TLB function.
//
VOID OALCacheGlobalsInit()
{
    OALMSG(OAL_CACHE&&OAL_VERBOSE, (L"+OALCacheGlobalsInit()\r\n"));

    //  Initialize kernel cache info struct
    memset( &g_oalCacheInfo, 0 , sizeof(g_oalCacheInfo) );

    //This function gets the cache info from the ARM1136 core
    //and is implemented in the cacheinit.s
    OALGetL1CacheInfo();

    //This function gets the L2 cache info from on chip
    //L2 cache controller of Mx31
    OALGetL2CacheInfo();


    OALMSG(OAL_CACHE&&OAL_VERBOSE,
            (TEXT("L1 cache details:\r\n flags %x\r\nI: %d sets/way, %d ways, %d line size, %d size\r\nD: %d sets/way, %d ways, %d line size, %d size\r\n"),
             g_oalCacheInfo.L1Flags,
             g_oalCacheInfo.L1ISetsPerWay, g_oalCacheInfo.L1INumWays,
             g_oalCacheInfo.L1ILineSize, g_oalCacheInfo.L1ISize,
             g_oalCacheInfo.L1DSetsPerWay, g_oalCacheInfo.L1DNumWays,
             g_oalCacheInfo.L1DLineSize, g_oalCacheInfo.L1DSize));

    OALMSG(OAL_CACHE&&OAL_VERBOSE,
            (TEXT("L2 cache details:\r\n flags %x\r\nI: %d sets/way, %d ways, %d line size, %d size\r\nD: %d sets/way, %d ways, %d line size, %d size\r\n"),
             g_oalCacheInfo.L2Flags,
             g_oalCacheInfo.L2ISetsPerWay, g_oalCacheInfo.L2INumWays,
             g_oalCacheInfo.L2ILineSize, g_oalCacheInfo.L2ISize,
             g_oalCacheInfo.L2DSetsPerWay, g_oalCacheInfo.L2DNumWays,
             g_oalCacheInfo.L2DLineSize, g_oalCacheInfo.L2DSize));


    OALMSG(OAL_CACHE&&OAL_VERBOSE, (L"-OALCacheGlobalsInit\r\n"));
}

//------------------------------------------------------------------------------

