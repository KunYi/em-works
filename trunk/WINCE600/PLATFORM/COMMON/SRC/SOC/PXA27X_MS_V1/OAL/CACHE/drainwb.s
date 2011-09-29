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

    OPT     2       ; disable listing
    INCLUDE kxarm.h
    OPT     1       ; reenable listing
    OPT     128     ; disable listing of macro expansions
    INCLUDE armmacros.s
    INCLUDE xscalecsp.inc

    TEXTAREA

    LEAF_ENTRY      XScaleDrainWriteBuffer
;++
; Routine Description:
;    Stall the CPU until write operations in progress have completed.
;
; Syntax:
;       void XScaleDrainWriteBuffer
;
; Arguments:
;       -- none --
;
; Return Value:
;       -- none --
;--
    ;
    ; Deal with Sighting #22271: Drain write buffer may be ignored if no outstanding memory requests
    ;  exist within the core.
    ;
    ;  I will deal with this by doing an arbitrary read from c=b=0 space (i.e. uncached, nonbuffered),
    ;   forcing the drain.
    ;
    ldr r2, =SDRAM_BASE_U_VIRTUAL
    mcr     p15, 0, r0, c7, c10, 4  ; drain the write buffer
    ldr r1, [r2]                    

    RETURN

    END
