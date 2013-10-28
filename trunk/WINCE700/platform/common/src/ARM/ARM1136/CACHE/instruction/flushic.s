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
;  File:  flushic.s
;
;
        INCLUDE kxarm.h
        INCLUDE armmacros.s

        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALFlushICache
;
;  Changed to support ARM11 Errata 411920.
;     Invalidate Entire Instruction Cache operation might fail 
;     to invalidate some lines if coincident with linefill


        LEAF_ENTRY OALFlushICache

        mov     r0, #0
        mcr     p15, 0, r0, c7, c5, 0
        mcr     p15, 0, r0, c7, c5, 0
        mcr     p15, 0, r0, c7, c5, 0
        mcr     p15, 0, r0, c7, c5, 0
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop

        RETURN

        END

