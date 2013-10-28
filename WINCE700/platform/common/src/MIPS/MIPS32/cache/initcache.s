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
#include "kxmips.h"
#define __MIPS_ASSEMBLER
#include "oal_cache.h"

//------------------------------------------------------------------------------
//
//  Definition:  KSEG0_BASE
//
#define KSEG0_BASE      0x80000000

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

        .word   0x40088001              // mfc0 t0, config, 1
        nop

        //--------------------------------------------------------------
        // Get Icache info
        //--------------------------------------------------------------

        srl     t1, t0, 22      
        and     t1, 0x7
        addu    t1, 6
        li      t2, 1
        sll     t2, t1                  // t2 = I sets per way

        srl     t1, t0, 19
        and     t1, 0x7
        addu    t1, 1
        li      t3, 1
        sll     t3, t1                  // t3 = I line size

        sll     t2, t1                  // t2 = I sets per way * I line size

        srl     t1, t0, 16
        and     t1, 0x7
        addu    t1, 1                   // t1 = I ways

        li      t4, 0
10:     addu    t4, t2
        addu    t1, -1
        bnez    t1, 10b
        nop                             // t4 = I size

        li      t5, KSEG0_BASE
20:     cache   INDEX_STORE_TAG_I, 0(t5)
        subu    t4, t3
        bne     t4, zero, 20b
        addu    t5, t3

        //--------------------------------------------------------------
        // Get Dcache info
        //--------------------------------------------------------------

        srl     t1, t0, 13
        and     t1, 0x7
        addu    t1, 6
        li      t2, 1
        sll     t2, t1                  // t2 = D sets per way

        srl     t1, t0, 10
        and     t1, 0x7
        addu    t1, 1
        li      t3, 1
        sll     t3, t1                  // t3 = D line size

        sll     t2, t1                  // t2 = D sets per way * D line size

        srl     t1, t0, 7
        and     t1, 0x7
        addu    t1, 1                   // t1 = D ways

        li      t4, 0
30:     addu    t4, t2
        addu    t1, -1
        bnez    t1, 30b
        nop                             // t4 = D size
        
        li      t5, KSEG0_BASE
40:     cache   INDEX_STORE_TAG_D, 0(t5)
        subu    t4, t3
        bnez    t4, 40b
        addu    t5, t3

        j       ra
        nop

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

        .word   0x40088001      // mfc0    t0, config, 1
        nop

        //--------------------------------------------------------------
        // Get Icache info
        //--------------------------------------------------------------

        srl     t1, t0, 22      
        and     t1, 0x7
        addu    t1, 6
        li      t2, 1
        sll     t2, t1                  // t2 = I sets per way
        sw      t2, L1ISetsPerWay(t7)

        srl     t1, t0, 19
        and     t1, 0x7
        addu    t1, 1
        li      t3, 1
        sll     t3, t1                  // t3 = I line size
        sw      t3, L1ILineSize(t7)

        sll     t2, t1                  // t2 = I sets per way * I line size

        srl     t1, t0, 16
        and     t1, 0x7
        addu    t1, 1                   // t1 = I ways
        sw      t1, L1INumWays(t7)

        li      t3, 0
10:     addu    t3, t2
        addu    t1, -1
        bnez    t1, 10b
        nop
        sw      t3, L1ISize(t7)         // t3 = I size

        //--------------------------------------------------------------
        // Get Dcache info
        //--------------------------------------------------------------

        srl     t1, t0, 13
        and     t1, 0x7
        addu    t1, 6
        li      t2, 1
        sll     t2, t1                  // t2 = D sets per way
        sw      t2, L1DSetsPerWay(t7)

        srl     t1, t0, 10
        and     t1, 0x7
        addu    t1, 1
        li      t3, 1
        sll     t3, t1                  // t3 = D line size
        sw      t3, L1DLineSize(t7)

        sll     t2, t1                  // t2 = D sets per way * D line size

        srl     t1, t0, 7
        and     t1, 0x7
        addu    t1, 1                   // t1 = D ways
        sw      t1, L1DNumWays(t7)

        li      t3, 0
20:     addu    t3, t2
        addu    t1, -1
        bnez    t1, 20b
        nop
        sw      t3, L1DSize(t7)         // t3 = D size
        
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

        .end    OALCacheGlobalsInit

//------------------------------------------------------------------------------
