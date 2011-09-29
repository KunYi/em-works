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

    EXTERN CacheBufferFlag

    TEXTAREA


    LEAF_ENTRY      XScaleFlushDCache
;++
; Routine Description:
;    Clean and invalidate the Data Cache
;
; This code writes back and invalidates the entire data cache by
; allocating lines from a predefined memory range set aside for
; this purpose.
;
; Syntax:
;   void XScaleFlushDCache(DWORD dwCacheBlocks,
;                              DWORD dwLineSize,
;                              DWORD dwCacheFlushMemoryBase);
;
; Arguments:
;   dwCacheBlocks - number of cache lines in the data cache
;   dwLineSize - number of bytes in each cache line
;   dwCacheFlushMemoryBase - pointer to uncached/unbuffered memory reserved
;                            for cache flush operations.
;
; Return Value:
;       -- none --
; r0..r3 junk
; CC flags junk
;--

    ; Save r4.
    ;
    stmdb   sp!, {r4}    
    
    ;
    ;   Check global flag to see which 32K buffer to use. We will alternate from call to call.
    ;     0: 0->32K
    ;     1: 32K->64K
    ;
    ldr     r4, =CacheBufferFlag
    ldr     r3, [r4]
    tst     r3, #1
    addne   r2, r2, #0x8000                         ; use upper range
    eor     r3, r3, #1
    str     r3, [r4]                                ; set/clear flag


LOOP1
    mcr p15, 0, r2, c7, c2, 5                       ; Allocate a line at the virtual address ZBANK_BASE_C_VIRTUAL
                                                    ;  (tossing out a dirty line back to memory)
    add r2, r2, r1
    mcr p15, 0, r2, c7, c2, 5

    add r2, r2, r1
    mcr p15, 0, r2, c7, c2, 5

    add r2, r2, r1
    mcr p15, 0, r2, c7, c2, 5

    add  r2, r2, r1
    subs r0, r0, #4                                 ; Decrement loop count
    bne LOOP1


    ; Deal with Sighting #22271: Drain write buffer may be ignored if no outstanding memory requests
    ;  exist within the core.
    ;
    ;  I will deal with this by doing an arbitrary read from c=b=0 space (i.e. uncached, nonbuffered),
    ;   forcing the drain.
    ;
    ldr r2, =SDRAM_BASE_U_VIRTUAL
    mcr p15, 0, r0, c7, c10, 4  ; drain the write buffer
    ldr r1, [r2]                    

    ; Restore r4.
    ;
    ldmia   sp!, {r4}   

    RETURN

    END

