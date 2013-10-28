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
;
;-------------------------------------------------------------------------------
.486p

include mpstart.inc

OFFSET32        EQU     <OFFSET FLAT:>

;
; bit fields of CR0
;
CR0_PE          EQU     1                       ; protected mode enable
CR0_PG          EQU     80000000h               ; paging enable
CR0_CD          EQU     40000000h               ; cache disable
CR0_NW          EQU     20000000h               ; not write-through
CR0_WP          EQU     00010000h               ; write protect (k mode can't write to r/o pages in user mode)

;
; instruction encoding
;
MODE_PREFIX     EQU     066h
FAR_JUMP        EQU     0eah


;
; All codes in this file is copied to SIPI vector page,
; physical at (SIPI_VECTOR << VM_PAGE_SHIFT). 
;
; Note: (1) Identity map to SIPI Vector page is created in the
;           page directory, passed in the MpStartParam field, before
;           AP entering this code, so it's safe to enable paging
;           once we loaded CR3 with the Page Directory passed in.
;       (2) The destination field of the last far jump instruction 
;           is modified to to MpStart_PM when the page is setup. i.e.
;           the far jump will jump to MpStartPM
;       (3) NO STACK OPERATION ALLOWED IN THIS FILE - WE DON'T HAVE A STACK DURING MP STARTUP.
;
;
;+========================================================+ <- SIPI vecotr page (physical at (SIPI_VECTOR << VM_PAGE_SHIFT))
;| Jmp instruction to end of MpStart_RM                   | 
;+--------------------------------------------------------+ 
;|                                                        | 
;| Remainder of MPStartParam fields                       | 
;|                                                        | 
;+--------------------------------------------------------+ <-- copy of MpStart_RM
;|                                                        | 
;| 16-bit real-mode code from MpStart_RM (mpstart.asm)    |
;|                                                        |
;| Enters 32-bit protected mode (no paging) and performs  |
;| a far jump to MpStart_PM.                              |
;|                                                        | 
;+--------------------------------------------------------+ <-- copy of MpStart_PM
;|                                                        | 
;| 32-bit protected-mode code from MpStart_PM.            |
;|                                                        |
;| Enable paging, jump to actual virtual address to       |
;| perform the rest of AP initialization.                 |
;|                                                        |
;|                                                        |
;+--------------------------------------------------------+    

;
; the "temporary GDT" contains 3 entries,
;
;       NULL (all 0)    - unused
;       RING_0_CODE     - for ring 0 code (for CS)
;       RING_0_DATA     - for ring 0 data (for DS, SS)
;
KGDT_R0_CODE    EQU     08h
KGDT_R0_DATA    EQU     10h


_TEXT16 SEGMENT DWORD PUBLIC USE16 'CODE'

        public  _MpStart_RM_Length
        public  _PMAddr

;
;
; _MPStart_RM - MP startup code in Real-Mode
;
; The code is copied to the SIPI vecotr address in RAM
;
;
_MpStart_RM PROC NEAR PUBLIC

        jmp     short ActualStart

        align   4

        db      (size MPStartParam - 4) dup (?)

        align   4
ActualStart:
        cli
        ; convert segment (cs) to flat address
        xor     eax, eax
        mov     ax, cs
        mov     ds, ax
        shl     eax, 4

        mov     edi, eax                ; (edi) = ptr (flat) to the page
                                        ;       == ptr to MPStartParam when switched protected mode


        ; load the temporary GDT
        db      MODE_PREFIX             ; 32-bit instruction
        lgdt    fword ptr ds:[tmpGDTBase]

        ; switch to protected mode
        mov     eax, cr0                ; Get the current CR0
        or      al,  CR0_PE             ; Set the PE bit to enable protected mode
        mov     cr0, eax                ; NOW WE'RE IN PMODE!

        ;
        ; Load CS by performing a far jump to the protected mode target
        ; address
        ;
        db      MODE_PREFIX             ; 32-bit instruction
        db      FAR_JUMP                ; Far jump forces a selector lookup
_PMAddr dd      0                       ; will be filled with flat phys address of _MpStart_PM
        dw      KGDT_R0_CODE

        ;
        ; should never get here
        ;
        hlt

        align   4
_MpStart_RM_Length:
        dd      $ - _MpStart_RM
_MpStart_RM ENDP

_TEXT16 ENDS 


_TEXT SEGMENT PARA PUBLIC 'CODE'

        PUBLIC  _MpStart_PM_Length
        EXTRN   _MpPagingEnabled:NEAR

;
;
; _MPStart_RM - MP startup code in Protected mode
;
; The code is copied to after the Real-Mode startup code
; in the SIPI vecotr address in RAM.
;
; At entrance:
;       (edi) = ptr to MPStartParam (flat, physical address)
;
;
_MpStart_PM PROC NEAR PUBLIC

        ;
        ; setup segment registers
        ;
        mov     eax, KGDT_R0_DATA
        mov     ds, ax
        mov     es, ax
        mov     ss, ax

        ;
        ; get page directory address from param block and load cr3
        ;
        mov     ebx, dword ptr [edi+PageDirPhysAddr]    ; (ebx) = page directory (physical address)
        mov     cr3, ebx                                ; load cr3

        ;
        ; Get the Virtual address to continue after cache enabled
        ;
        mov     ecx, OFFSET32 _MpPagingEnabled          ; (ecx) = VA of MpPagingEnabled

.586p
        ; update CR4 (enable large page mapping)
        mov     eax, dword ptr [edi+Cr4Value]
        mov     cr4, eax
.486p

        ;
        ; enable paging
        ;
        
        ; load current cr0
        mov     eax, cr0

        ; set "Paging Enable" and "Write Protect" bit
        or      eax, CR0_WP OR CR0_PG

        align   16
        ; write to cr0 to enable paging
        mov     cr0, eax

        ; jump to MpPagingEnabled to continue initialization
        jmp     ecx

        cli
        hlt


_MpStart_PM_Length:
        dd      $ - _MpStart_PM
_MpStart_PM ENDP


_GetCR3 PROC NEAR PUBLIC
        mov     eax, cr3
        ret
_GetCR3 ENDP

_GetCR4 PROC NEAR PUBLIC
.586p
        mov     eax, cr4
        ret
.486p
_GetCR4 ENDP

_TEXT ENDS 

        END
        

