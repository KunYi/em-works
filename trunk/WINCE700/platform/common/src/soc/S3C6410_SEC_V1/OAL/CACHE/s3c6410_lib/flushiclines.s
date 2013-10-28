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
;
;  File:  flushiclines.s
;
;
        INCLUDE kxarm.h
        INCLUDE armmacros.s
        INCLUDE oal_cache.inc

        IMPORT g_oalCacheInfo

        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALFlushICacheLines
;
;  The function fix the arm errata to enable the ARM11 instruction cache
;
        LEAF_ENTRY OALFlushICacheLines

        ldr     r2, =g_oalCacheInfo
        ldr     r3, [r2, #L1ILineSize]

10      orr  r2, r0, #0xC0000000
        bic  r2, r2, #1
        mcr  p15, 0, r2, c7, c5, 2    ; invalidate way3
        sub  r2, r2, #0x40000000
        mcr  p15, 0, r2, c7, c5, 2    ; invalidate way2
        sub  r2, r2, #0x40000000
        mcr  p15, 0, r2, c7, c5, 2    ; invalidate way1
        sub  r2, r2, #0x40000000
        mcr  p15, 0, r2, c7, c5, 2    ; invalidate way0
        ;orr  r2, r2, #1
        ;mcr  p15, 0, r2, c7, c5, 2    ; invalidate SmartCache]

        add     r0, r0, r3                      ; move to next
        subs    r1, r1, r3
        bgt     %b10                            ; loop while > 0 bytes left

        RETURN

        END
