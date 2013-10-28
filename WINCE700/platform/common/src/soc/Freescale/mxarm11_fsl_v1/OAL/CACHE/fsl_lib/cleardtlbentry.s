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
;  File:  cleardtlbentry.s
;
;
        INCLUDE kxarm.h
        INCLUDE armmacros.s

        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALClearDTLBEntry
;
;  Flush and invalidate an entry in the data TLB
;
;  This file was originally copied from the sample OMAP2420 OAL cache
;  handling code.
;
        LEAF_ENTRY OALClearDTLBEntry

        mrc     p15, 0, r1, c13, c0, 1          ; Read Context ID Register
        orr     r0,  r0, r1                     ; r0 = MVA+ASID
        mcr     p15, 0, r0, c8, c6, 1           ; clear data TLB entry by MVA+ASID

        RETURN

        END

;-------------------------------------------------------------------------------
