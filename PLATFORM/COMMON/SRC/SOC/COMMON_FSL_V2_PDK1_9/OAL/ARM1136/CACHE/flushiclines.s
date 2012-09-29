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
; Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;-------------------------------------------------------------------------------
;
;  File:  flushiclines.s
;
;  This file was originally copied from the sample OMAP2420 OAL cache
;  handling code.
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

        ; The following block of code is the recommended workaround for
        ; ARM1136 erratum 411920.  This code sequence is also a documented
        ; workaround for other ARM1136 instruction cache errata (328429, 
        ; 371025, 405875).  By avoiding invalidate-icache-line-by-MVA operations,
        ; we also eliminate one of the conditions necessary for erratum 325157
        ; during instruction cache line flushes.

        mov     r0, #0
        mrs     r1, cpsr
        cpsid   ifa
        mcr     p15, 0, r0, c7, c5, 0
        mcr     p15, 0, r0, c7, c5, 0
        mcr     p15, 0, r0, c7, c5, 0
        mcr     p15, 0, r0, c7, c5, 0
        msr     cpsr_cx, r1
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
