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
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
;
;-------------------------------------------------------------------------------
;
;  File: flushdc.s 
;
;  This file implement OALFlushDCache function. This implementaion should
;  work on most ARM based SoC. Note that newer silicon support more effective
;  test, clean and invalidate DCache function.
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

        ; save registers
        stmfd   sp!, {r4,r5}

        ldr     r0, =g_oalCacheInfo

        ; first we need to get index bit
        ldr     r1, [r0, #L1DNumWays]
        mov     r5, #32
10      movs    r1, r1, lsr #1
        beq     %F20
        sub     r5, r5, #1
        b       %B10
20      

        ldr     r1, [r0, #L1DNumWays]
        sub     r1, r1, #1
        mov     r1, r1, lsl r5
        mov     r2, #1
        mov     r2, r2, lsl r5
        
        ldr     r3, [r0, #L1DLineSize]
30      ldr     r4, [r0, #L1DSetsPerWay]

        mov     r5, r1
        
        ; write back and invalidate the line
40      mcr     p15, 0, r5, c7, c14, 2

        ; add the set index
        add     r5, r5, r3

        ; decrement the set number
        subs    r4, r4, #1
        bgt     %b40

        ; test last index
        cmp     r1, #0
        beq     %f50
        
        ; decrement index
        sub     r1, r1, r2
        b       %b30

        ; drain the write buffer
50      mov     r0, #0
        mcr     p15, 0, r0, c7, c10, 4
    
        ; restore registers
        ldmfd   sp!, {r4,r5}

        RETURN

        END
