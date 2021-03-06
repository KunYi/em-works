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
;  File:  clearitlbentry.s
;
;
    INCLUDE kxarm.h
    INCLUDE armmacros.s

    TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALClearITLBEntry
;
;  Flush and invalidate an entry in the instruction TLB.
;
    LEAF_ENTRY OALClearITLBEntry

    mrc     p15, 0, r1, c13, c0, 1          ; Read Context ID Register
    orr     r0,  r0, r1                     ; r0 = MVA+ASID
    mcr     p15, 0, r0, c8, c5, 1           ; Clear instruction TLB entry by MVA+ASID

; if ASID in use, also flush any slot 0 aliases
    cmp     r1, #0
    bicne   r0, r0, #0xFE000000
    mcrne   p15, 0, r0, c8, c5, 1           ; clear instruction TLB entry

    RETURN

    END

;-------------------------------------------------------------------------------

