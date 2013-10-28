;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
;
;-------------------------------------------------------------------------------
; Copyright (C) 2007, Freescale Semiconductor, Inc. All Rights Reserved.
; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;-------------------------------------------------------------------------------
;
;  File: flushl2.s
;
;  This file implements L2 cache flushing for the Freescale ARM1136 CPU.
;
;-------------------------------------------------------------------------------
        INCLUDE kxarm.h
        INCLUDE armmacros.s
        INCLUDE oal_cache.inc

L2CC_SYNC_OFFSET            EQU     0x730
L2CC_INVWAY_OFFSET          EQU     0x77C
L2CC_CLEANINVWAY_OFFSET     EQU     0x7FC
L2CC_DBGCR_OFFSET           EQU     0xF40
L2CC_DBGCR_DWB              EQU     (1 << 1)

        IMPORT g_pL2CC

        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALFlushL2Cache
;
        LEAF_ENTRY OALFlushL2Cache

        ldr     r1, =g_pL2CC
        ldr     r1, [r1]

        ;
        ; Check if we are in forced write-through mode
        ;
        ldr     r0, [r1, #L2CC_DBGCR_OFFSET]
        ands    r0, r0, #L2CC_DBGCR_DWB
        beq     L2_WB

        ;
        ; When L2 is configured for write-through mode, we only need to
        ; drain the write buffer and invalidate all of the ways.  The write
        ; buffer is automatically drained during any background cache 
        ; maintenance operation.
        ;

        ;
        ; Invalidate L2 cache memory
        ;
        ldr     r0, =0x000000FF
        str     r0, [r1, #L2CC_INVWAY_OFFSET]

        ;
        ; Wait for invalidation to complete
        ;
L2_invalidate
        ldr     r0, [r1, #L2CC_INVWAY_OFFSET]
        cmp     r0, #0
        bne     L2_invalidate

        RETURN

L2_WB
        ;
        ; When L2 is configured for write-back mode, we need to clean and 
        ; invalidate all of the ways.
        ;

        ;
        ; Apply software workaround for L2CC errata:  "Starting a background 
        ; maintenance operation while a line is sitting in the write-allocate
        ; buffer can prevent that line from being allocated".  Perform
        ; cache maintenance operations one way at at time with a Cache Sync 
        ; preceeding each way operation.
        
        mov     r3, #0      ; r3 holds write value for cache sync
        mov     r2, #1      ; r2 holds current way bitmask

L2_way_loop

        ; Issue two Cache Sync operations to make certain all write
        ; buffers have been drained
        str     r3, [r1, #L2CC_SYNC_OFFSET]
        str     r3, [r1, #L2CC_SYNC_OFFSET]

        ; 
        ; Clean and invalidate a single way in L2 cache memory
        ;
        str     r2, [r1, #L2CC_CLEANINVWAY_OFFSET]

        ;
        ; Wait for clean and invalidation to complete
        ;
L2_clean_invalidate
        ldr     r0, [r1, #L2CC_CLEANINVWAY_OFFSET]
        cmp     r0, #0
        bne     L2_clean_invalidate

        ;
        ; Advance to next way by shifting way bitmask.  We are done
        ; if all 8 ways have been flushed.
        ;
        mov     r2, r2, lsl #1
        cmp     r2, #0x100
        blt     L2_way_loop     

        RETURN

        END

;-------------------------------------------------------------------------------
