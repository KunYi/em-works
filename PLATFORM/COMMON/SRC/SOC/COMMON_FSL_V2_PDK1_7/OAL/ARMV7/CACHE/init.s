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
; Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;-------------------------------------------------------------------------------
;
;  File: init.s
;
;
    INCLUDE kxarm.h
    INCLUDE armmacros.s
    INCLUDE oal_cache.inc

    IMPORT g_oalCacheInfo

    TEXTAREA

    LEAF_ENTRY OALCacheGlobalsInit

    ; Point at global struct to initialise
    ldr     r1, =g_oalCacheInfo

    ; Read number of levels
    mrc     p15, 1, r0, C0, C0, 1   ; Read CLIDR

    ; We assume at least an L1 unified cache
    mov     r3, #0                  ; Select L1D
    mcr     p15, 2, r3, C0, C0, 0   ; Write the Cache Size selection register
    DCD     0xF57FF06F              ; ISB
    mrc     p15, 1, r3, C0, C0, 0   ; Reads current Cache Size ID register

    mov   r12, #0                   ; No flags yet
    tst   r3, #1:shl:31             ; Writethrough supported?
    orrne r12, r12, #2              ; bit1 == WT flag
    tst   r0, #1:shl:2              ; L1 unified?
    orrne r12, r12, #1              ; bit0 == unified flag
    str   r12, [r1, #L1Flags]

    mov   r2, r3, lsl #19           ; Discard bits 31-13
    mov   r2, r2, lsr #22           ; Discard bits 2-0
    add   r2, r2, #1
    str   r2, [r1, #L1DNumWays]
    strne r2, [r1, #L1INumWays]

    mov   r12, r3, lsl #4           ; Discard bits 31-28
    mov   r12, r12, lsr #17         ; Discard bits 12-0
    add   r12, r12, #1
    str   r12, [r1, #L1DSetsPerWay]
    strne r12, [r1, #L1ISetsPerWay]

    mul   r2, r12, r2               ; sets*ways

    and   r12, r3, #&7              ; Extract the line length field
    add   r12, r12, #4              ; log2 line length
    mov   r3, #1
    mov   r3, r3, lsl r12           ; Line length
    str   r3, [r1, #L1DLineSize]
    strne r3, [r1, #L1ILineSize]

    mov   r2, r2, lsl r12           ; Total cache size
    str   r2, [r1, #L1DSize]
    strne r2, [r1, #L1ISize]
    bne   L1Unified

    ; L1 I cache info
    mov   r3, #1                    ; Select L1I
    mcr   p15, 2, r3, C0, C0, 0     ; Write the Cache Size selection register
    mov   r3, #0
    DCD      0xF57FF06F             ; ISB
    mrc   p15, 1, r3, C0, C0, 0     ; Reads current Cache Size ID register

    mov   r2, r3, lsl #19           ; Discard bits 31-13
    mov   r2, r2, lsr #22           ; Discard bits 2-0
    add   r2, r2, #1
    str   r2, [r1, #L1INumWays]

    mov   r12, r3, lsl #4           ; Discard bits 31-28
    mov   r12, r12, lsr #17         ; Discard bits 12-0
    add   r12, r12, #1
    str   r12, [r1, #L1ISetsPerWay]

    mul   r2, r12, r2               ; sets*ways

    and   r12, r3, #&7              ; Extract the line length field
    add   r12, r12, #4              ; log2 line length
    mov   r3, #1
    mov   r3, r3, lsl r12           ; Line length
    str   r3, [r1, #L1ILineSize]

    mov   r2, r2, lsl r12           ; Total cache size
    str   r2, [r1, #L1ISize]
L1Unified
    
    tst   r0, #0x38                 ; Any L2 cache?
    beq   NoL2Cache

    mov   r3, #2                    ; Select L2D
    mcr   p15, 2, r3, c0, c0, 0     ; Write the Cache Size selection register
    DCD   0xF57FF06F                ; ISB
    mrc   p15, 1, r3, c0, c0, 0     ; Reads current Cache Size ID register

    mov   r12, #0                   ; No flags yet
    tst   r3, #1:shl:31             ; Writethrough supported?
    orrne r12, r12, #2              ; bit1 == WT flag
    tst   r0, #1:shl:5              ; L2 unified?
    orrne r12, r12, #1              ; bit0 == unified flag
    str   r12, [r1, #L2Flags]

    mov   r2, r3, lsl #19           ; Discard bits 31-13
    mov   r2, r2, lsr #22           ; Discard bits 2-0
    add   r2, r2, #1
    str   r2, [r1, #L2DNumWays]
    strne r2, [r1, #L2INumWays]

    mov   r12, r3, lsl #4           ; Discard bits 31-28
    mov   r12, r12, lsr #17         ; Discard bits 12-0
    add   r12, r12, #1
    str   r12, [r1, #L2DSetsPerWay]
    strne r12, [r1, #L2ISetsPerWay]

    mul   r2, r12, r2               ; sets*ways

    and   r12, r3, #&7              ; Extract the line length field
    add   r12, r12, #4              ; log2 line length
    mov   r3, #1
    mov   r3, r3, lsl r12           ; Line length
    str   r3, [r1, #L2DLineSize]
    strne r3, [r1, #L2ILineSize]

    mov   r2, r2, lsl r12           ; Total cache size
    str   r2, [r1, #L2DSize]
    strne r2, [r1, #L2ISize]
    bne   L2Unified

    ; L2 I cache info
    mov   r3, #3                    ; Select L2I
    mcr   p15, 2, r3, c0, c0, 0     ; Write the Cache Size selection register
    mov   r3, #0
    DCD   0xF57FF06F                ; ISB
    mrc   p15, 1, r3, c0, c0, 0     ; Reads current Cache Size ID register

    mov   r2, r3, lsl #19           ; Discard bits 31-13
    mov   r2, r2, lsr #22           ; Discard bits 2-0
    add   r2, r2, #1
    str   r2, [r1, #L2INumWays]

    mov   r12, r3, lsl #4           ; Discard bits 31-28
    mov   r12, r12, lsr #17         ; Discard bits 12-0
    add   r12, r12, #1
    str   r12, [r1, #L2ISetsPerWay]

    mul   r2, r12, r2               ; sets*ways

    and   r12, r3, #&7              ; Extract the line length field
    add   r12, r12, #4              ; log2 line length
    mov   r3, #1
    mov   r3, r3, lsl r12           ; Line length
    str   r3, [r1, #L2ILineSize]

    mov   r2, r2, lsl r12           ; Total cache size
    str   r2, [r1, #L2ISize]

L2Unified
NoL2Cache

    RETURN

    END

