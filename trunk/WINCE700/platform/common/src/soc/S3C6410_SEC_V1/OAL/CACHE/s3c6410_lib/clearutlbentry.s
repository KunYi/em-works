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
;  File:  clearutlbentry.s
;
;
        INCLUDE kxarm.h
        INCLUDE armmacros.s

        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALClearUTLBEntry
;
;  Flush and invalidate an entry in the unified TLB
;
        LEAF_ENTRY OALClearUTLBEntry

        mrc     p15, 0, r1, c13, c0, 1          ; Read Context ID Register
        orr     r0,  r0, r1                     ; r0 = MVA+ASID
        mcr     p15, 0, r0, c8, c7, 1           ; clear unified TLB entry by MVA+ASID

        RETURN

        END

;-------------------------------------------------------------------------------

