;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;-------------------------------------------------------------------------------
;
; Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;-------------------------------------------------------------------------------
;
;  File: flushdc.s
;
;  This file implements the OALFlushDCache function.
;
    INCLUDE kxarm.h
    INCLUDE armmacros.s
    INCLUDE oal_cache.inc

    IMPORT g_oalCacheInfo

    TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALFlushDCache
;
    LEAF_ENTRY OALFlushDCache

    ; Save registers
    stmfd   sp!, {r4}

    ldr     r0, =g_oalCacheInfo
    ldr     r1, [r0, #L1DNumWays]
    sub     r1, r1, #1              ; R1 = way index (unshifted)
    ldr     r4, [r0, #L1DSetsPerWay]
    sub     r4, r4, #1              ; R4 = set index (unshifted)
    mov     r3, #(0 << 1)           ; R3 = cache level index (shifted) = L1
FlushLoopWayL1
    mov     r2, r4                  ; R2 = set index (unshifted)
FlushLoopSetL1
    orr     r0, r3, r1, lsl #30     ; Factor in the way and cache level
    orr     r0, r0, r2, lsl #6      ; Factor in the set index
    mcr     p15, 0, r0, c7, c14, 2  ; Clean and invalidate by set/way
    subs    r2, r2, #1              ; Decrement the index
    bge     FlushLoopSetL1
    subs    r1, r1, #1              ; Decrement the way number
    bge     FlushLoopWayL1

    ; Restore registers
    ldmfd sp!, {r4}

    RETURN

    END
