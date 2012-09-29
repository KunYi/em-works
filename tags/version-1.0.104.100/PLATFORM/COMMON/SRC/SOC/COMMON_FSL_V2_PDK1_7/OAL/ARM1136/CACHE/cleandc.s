;-------------------------------------------------------------------------------
; Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;-------------------------------------------------------------------------------
;
;  File: cleandc.s
;
;  This file implements OALCleanDCache function for the Freescale ARM1136.
;
        INCLUDE kxarm.h
        INCLUDE armmacros.s
        INCLUDE oal_cache.inc

        IMPORT g_oalCacheInfo

        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALCleanDCache
;
        LEAF_ENTRY OALCleanDCache

        ; Clean and invalidate entire data cache
        mov     r0, #0
        mcr     p15, 0, r0, c7, c10, 0  

        RETURN

        END

;-------------------------------------------------------------------------------
