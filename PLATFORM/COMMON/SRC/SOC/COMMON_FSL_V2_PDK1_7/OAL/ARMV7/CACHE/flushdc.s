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
;  File: flushdc.s
;
;  This file implements the OALFlushDCache function.
;
    INCLUDE kxarm.h
    INCLUDE armmacros.s
    INCLUDE oal_cache.inc

ARM_AUXCR_L2EN      EQU     2

    TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALFlushDCache
;
    LEAF_ENTRY OALFlushDCache

    ; Save registers
    stmfd   sp!, {r4,r5,r7,r9,r10,r11}

    mrc     p15, 1, r0, c0, c0, 1   ; Read CLIDR
    ands    r3, r0, #&7000000       ; Isolate level of coherency
    mov     r3, r3, lsr #23         ; Cache level value (naturally aligned)
    beq     Finished
    mov     r10, #0
Loop1
    add     r2, r10, r10, lsr #1    ; Work out cache level
    mov     r1, r0, lsr r2          ; R0 bottom 3 bits = Cache Type for this level
    and     r1, r1, #7              ; Get those 3 bits alone
    cmp     r1, #2
    blt     Skip                    ; No cache or only instruction cache at this level
    mcr     p15, 2, r10, c0, c0, 0  ; Write the Cache Size selection register
    mov     r1, #0
    DCD     0xF57FF06F              ; ISB
    mrc     p15, 1, r1, c0, c0, 0   ; Reads current Cache Size ID register
    and     r2, r1, #&7             ; Extract the line length field
    add     r2, r2, #4              ; Add 4 for the line length offset (log2 16 bytes)
    ldr     r4, =0x3FF
    ands    r4, r4, r1, lsr #3      ; R4 is the max number on the way size (right aligned)
    clz     r5, r4                  ; R5 is the bit position of the way size increment
    ldr     r7, =0x00007FFF
    ands    r7, r7, r1, lsr #13     ; R7 is the max number of the index size (right aligned)
Loop2
    mov     r9, r4                  ; R9 working copy of the max way size (right aligned)
Loop3
    orr     r11, r10, r9, lsl r5    ; Factor in the way number and cache number into R11
    orr     r11, r11, r7, lsl r2    ; Factor in the index number
    mcr     p15, 0, r11, c7, c14, 2 ; Clean and invalidate by set/way
    subs    r9, r9, #1              ; Decrement the way number
    bge     Loop3
    subs    r7, r7, #1              ; Decrement the index
    bge     Loop2
Skip
    mrc  p15, 0, r1, c1, c0,1       ; Read aux control register
    tst  r1, #ARM_AUXCR_L2EN
    beq  Finished                   ; L2 not enabled

    add  r10, r10, #2               ; Increment the cache number
    cmp  r3, r10
    bgt  Loop1
    
Finished

    ; Restore registers
    ldmfd sp!, {r4,r5,r7,r9,r10,r11}

    RETURN

    END
