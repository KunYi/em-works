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

    TEXTAREA

    LEAF_ENTRY  XScaleFlushICacheLines
;++
; Routine Description:
;    Invalidate some number of Instruction Cache lines
;
; Syntax:
;   void XScaleFlushICacheLines(PVOID pAddr, DWORD dwLength, DWORD dwLineLength);
;
; Arguments:
;   pAddr -- virtual address at which to start invalidation, on dwLineLength-byte 
;            alignment
;   dwLength -- number of bytes to invalidate, must be > zero
;   dwLineLength -- number of bytes in a cache line
;
; Return Value:
;   -- none --
; r0..r2 junk
; CC flags junk 
;--

    ; invalidate the range of lines
10
    mcr     p15, 0, r0, c7, c5, 1   ; invalidate each entry
    add     r0, r0, r2              ; on to the next line
    subs    r1, r1, r2              ; reduce the number of bytes left
    bgt     %b10                    ; loop while > 0 bytes left

    ; invalidate the branch target buffer
    mcr     p15, 0, r0, c7, c5, 6

    RETURN

    END
