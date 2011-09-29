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
#include <kxmips.h>
#define __MIPS_ASSEMBLER
#include <oal_cache.h>

//------------------------------------------------------------------------------
//
//  Definition:  KSEG0_BASE, KSEG1_BASE
//
#define KSEG0_BASE      0x80000000
#define KSEG1_BASE      0xA0000000

//------------------------------------------------------------------------------
//
//  Function:  OALCacheInit
//
//  This function initialize caches. It is usally called in boot loader
//  initialization system. It doesn't uses C call stack frame and it should not
//  call any other routine.
//
LEAF_ENTRY(OALCacheInit)

        .set    noreorder
        .set    noat

        mtc0    zero, taglo
        mtc0    zero, taghi
        nop

        mfc0    t2, config
        nop
        srl     t3, t2, 11                      // move CS bit to bit1
        nor     t3, zero
        andi    t3, 2
        srl     t4, t2, 9                       // get I cache size
        andi    t4, 0x0007
        addu    t4, t3
        addu    t4, 10                          // save t4 for later
        li      t5, 1
        sllv    t5, t4                          // cache size in t5

        srl     t2, 5                           // get I cache line size
        andi    t2, 1
        addu    t2, 4
        li      t6, 1
        sllv    t6, t2                          // line size in t6

        li      t0, KSEG0_BASE
10:     cache   INDEX_STORE_TAG_I, 0(t0)
        cache   INDEX_STORE_TAG_I, 1(t0)
        subu    t5, t6
        bne     t5, zero, 10b
        addu    t0, t6
        
        //--------------------------------------------------------------
        // InitializeGet Dcache info
        //--------------------------------------------------------------

        mfc0    t2, config
	nop

        srl     t3, t2, 11                      // move CS bit to bit1
        nor     t3, zero
        andi    t3, 2
        srl     t4, t2, 6                       // get D cache size
        andi    t4, 0x0007
        addu    t4, t3
        addu    t4, 10                          // save t4 for later
        li      t5, 1
        sllv    t5, t4                          // cache size in t5

        srl     t2, 4                           // get D cache line size
        andi    t2, 1
        addu    t2, 4
        li      t6, 1
        sllv    t6, t2                          // line size in t6

        li      t0, KSEG0_BASE
20:     cache   INDEX_STORE_TAG_D, 0(t0)
        cache   INDEX_STORE_TAG_D, 1(t0)
        subu    t5, t6
        subu    t5, t6
        bnez    t5, 20b
        addu    t0, t6

        j       ra
        nop

        .set    at
        .set    reorder
        .end    OALCacheInit

//------------------------------------------------------------------------------
//
//  Function:  OALCacheGlobalsInit
//
//  This function initialize cache global constants.
//
LEAF_ENTRY(OALCacheGlobalsInit)

        .set    noreorder

        // Get cache info address
        la      t7, g_oalCacheInfo

        //--------------------------------------------------------------
        // Get Icache info
        //--------------------------------------------------------------

        mfc0    t2, config
        nop
        srl     t3, t2, 11                      // move CS bit to bit1
        nor     t3, zero
        andi    t3, 2
        srl     t4, t2, 9                       // get I cache size
        andi    t4, 0x0007
        addu    t4, t3
        addu    t4, 10                          // save t4 for later
        li      t1, 1
        sllv    t1, t4
        sw      t1, L1ISize(t7)

        srl     t2, 5                           // get I cache line size
        andi    t2, 1
        addu    t2, 4
        li      t1, 1
        sllv    t1, t2
        sw      t1, L1ILineSize(t7)

        li      t1, 2                           // no other way
        sw      t1, L1INumWays(t7)

        subu    t4, t2                          // get number of sets
        subu    t4, 1
        li      t1, 1
        sllv    t1, t4
        sw      t1, L1ISetsPerWay(t7)

        //--------------------------------------------------------------
        // Get Dcache info
        //--------------------------------------------------------------

        mfc0    t2, config
        srl     t3, t2, 11                      // move CS bit to bit1
        nor     t3, zero
        andi    t3, 2
        srl     t4, t2, 6                       // get D cache size
        andi    t4, 0x0007
        addu    t4, t3
        addu    t4, 10                          // save t4 for later
        li      t1, 1
        sllv    t1, t4
        sw      t1, L1DSize(t7)

        srl     t2, 4                           // get D cache line size
        andi    t2, 1
        addu    t2, 4
        li      t1, 1
        sllv    t1, t2
        sw      t1, L1DLineSize(t7)

        li      t1, 2                           // no other way
        sw      t1, L1DNumWays(t7)

        subu    t4, t2                          // get number of sets
        subu    t4, 1
        li      t1, 1
        sllv    t1, t4
        sw      t1, L1DSetsPerWay(t7)

        //--------------------------------------------------------------
        // There isn't secondary cache
        //--------------------------------------------------------------

        sw      zero, L2ISetsPerWay(t7)
        sw      zero, L2ILineSize(t7)
        sw      zero, L2INumWays(t7)
        sw      zero, L2ISize(t7)
        sw      zero, L2DSetsPerWay(t7)
        sw      zero, L2DLineSize(t7)
        sw      zero, L2DNumWays(t7)
        sw      zero, L2DSize(t7)

        //--------------------------------------------------------------
        // No specific flags
        //--------------------------------------------------------------

        sw      zero, L1Flags(t7)
        sw      zero, L2Flags(t7)

        j       ra
        nop

        .set    reorder
        .end    OALCacheGlobalsInit

//------------------------------------------------------------------------------
