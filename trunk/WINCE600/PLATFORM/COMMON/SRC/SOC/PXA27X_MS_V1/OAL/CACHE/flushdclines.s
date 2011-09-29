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
    INCLUDE xscalecsp.inc
    OPT     1       ; reenable listing
    OPT     128     ; disable listing of macro expansions
    INCLUDE armmacros.s

; interrupt-related constants.
;
NoIntsMask       EQU    0x000000C0
IrqFiqEnable     EQU    0xFFFFFF3F


    TEXTAREA

    LEAF_ENTRY  XScaleFlushDCacheLines
;++
; Routine Description:
;    Clean and invalidate some number of Data Cache lines
;
; Syntax:
;   void XScaleFlushDCacheLines(PVOID pAddr, DWORD dwLength, DWORD dwLineLength);
;
; Arguments:
;   pAddr -- virtual address at which to start flushing, on dwLineLength-byte 
;            alignment
;   dwLength -- number of bytes to invalidate, must be > zero
;   dwLineLength -- number of bytes in a cache line
;
; Return Value:
;   -- none --
; r0..r3 junk
; CC flags junk 
;--

    ; invalidate the range of lines
10
    ; disable IRQ, FIQ to avoid potential data corruption.
    ; NOTE: since this routine can be called with interrupts off, we need to save the interrupt state and restore it later.
    ;
    mrs     r3, cpsr    
    stmdb   sp!, {r3}    
    orr     r3, r3, #NoIntsMask             ; Set IRQ and FIQ bits (disable).
    msr     cpsr_c, r3

    mcr p15, 0, r0, c7, c10, 1              ; clean a single DCache line. NOTE: line is still valid and will therefore continue to hit
                                            ;  (tossing out a dirty line back to memory) 
    mcr p15, 0, r0, c7, c6, 1               ; invalidate same line... preemption considerations: if line is dirtied during
                                            ;  data corrutpion will occur.  Wrapping atomically is a perf. degradation, but safe.                                             
    ; restore cpsr to previous state.
    ldmia   sp!, {r3}   
    msr     cpsr_c, r3                      ; Control the IRQ/FIQ
                                      
    add     r0, r0, r2                      ; on to the next line
    subs    r1, r1, r2                      ; reduce the number of bytes left
    bgt     %b10                            ; loop while > 0 bytes left
    
    ; Deal with Sighting #22271: Drain write buffer may be ignored if no outstanding memory requests
    ;  exist within the core.
    ;
    ;  I will deal with this by doing an arbitrary read from c=b=0 space (i.e. uncached, nonbuffered),
    ;   forcing the drain.
    ;
    ldr     r2, =SDRAM_BASE_U_VIRTUAL
    mcr     p15, 0, r0, c7, c10, 4  ; drain the write buffer
    ldr     r1, [r2]                    
   
    RETURN

    END
