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
        LEAF_ENTRY OALFlushICacheLines
 
        ldr     r2, =g_oalCacheInfo
        ldr     r3, [r2, #L1ILineSize]

;       Workaround for ARM1136JS processor bug -
;       Invalidate Instruction Cache operation can fail -
;       Refer ARM1136 Errata #371025.
;       mcr     p15, 0, r0, c7, c5, 1           ; invalidate entry


10      orr     r0, r0, #0xC0000000
        bic     r0, r0, #1              
        mcr     p15, 0, r0, c7, c5, 2   ;invalidate way
        sub     r0, r0, #0x40000000     
        mcr     p15, 0, r0, c7, c5, 2   ;invalidate way
        sub     r0, r0, #0x40000000     
        mcr     p15, 0, r0, c7, c5, 2   ;invalidate way
        sub     r0, r0, #0x40000000     
        mcr     p15, 0, r0, c7, c5, 2   ;invalidate way
        add     r0, r0, r3              ; move to next
        subs    r1, r1, r3
        bgt     %b10                   ; loop while > 0 bytes left
        RETURN

        END
