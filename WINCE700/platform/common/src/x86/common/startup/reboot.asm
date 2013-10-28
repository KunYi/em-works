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
;   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
;   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
;   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
;   PARTICULAR PURPOSE.
;
;-------------------------------------------------------------------------------
.486p

PAGE_SIZE EQU 1000h
PAGE_ATTRIB_RW      EQU 000001100011b
PAGE_ATTRIB_RW_NOCACHE  EQU 000001111011b
PAGE_ATTRIB_RO_USER EQU 000000100101b
PAGE_ATTRIB_RW_USER EQU 000001100111b

PageRange MACRO Start, NumPages, Attrib
        LOCAL CurPhys
        .ERRNZ Start AND (PAGE_SIZE-1)
        CurPhys = Start
        REPEAT NumPages
            dd  CurPhys OR Attrib
            CurPhys = CurPhys + PAGE_SIZE
        ENDM
        ENDM

LIN_TO_PHYS_OFFSET EQU 80000000h

OFFSET32 EQU <OFFSET FLAT:>
BOOTPAGE                EQU     801DC000h


;vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
_DATA SEGMENT DWORD USE32 PUBLIC 'DATA'
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

; This GDT is only used by _RebootHandler to set the correct GDT during the warm reboot path

GDT_Data LABEL DWORD
    db         0,    0,   0,   0,   0,         0,         0,   0    ; Unused
    CS_FLAT_SEL EQU ($-GDT_Data)
    db      0FFh, 0FFh, 00h, 00h, 00h, 10011010b, 11001111b, 00h    ; Code (4GB)
    CS_64K_SEL EQU ($-GDT_Data)
    db      0FFh, 0FFh, 00h, 00h, 00h, 10011010b, 00000000b, 00h    ; Code (64KB)
    DS_FLAT_SEL EQU ($-GDT_Data)
    db      0FFh, 0FFh, 00h, 00h, 00h, 10010010b, 11001111b, 00h    ; Data (4GB)
    DS_64K_SEL EQU ($-GDT_Data)
    db      0FFh, 0FFh, 00h, 00h, 00h, 10010010b, 00000000b, 00h    ; Data (64KB)
GDT_TABLE_SIZE = $ - OFFSET GDT_Data

GDTPtr LABEL FWORD
        dw      GDT_TABLE_SIZE - 1  ; Limit of 0 = 1 byte
        dd      BOOTPAGE-LIN_TO_PHYS_OFFSET
_DATA ENDS

        ASSUME  CS: FLAT, DS: FLAT, SS: FLAT



;vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
_TEXT SEGMENT para public 'TEXT'
;^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------

LIN_TO_PHYS_MASK        EQU     7FFFFFFFh
PG_MASK                 EQU     80000000h
CD_MASK                 EQU     40000000h


extrn _dwRebootAddress:near

;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
_RebootHandler PROC NEAR PUBLIC

        cli

        ; make a copy of GDT to boot page, such that it won't be overwritten by bootloader
        mov     edi, BOOTPAGE
        lea     esi, GDT_Data

        ; copy 5 quad-words, plus a dword for GDTPtr
        mov     eax, dword ptr [esi]
        mov     dword ptr [edi], eax
        mov     eax, dword ptr [esi+4]
        mov     dword ptr [edi+4], eax
        
        mov     eax, dword ptr [esi+8]
        mov     dword ptr [edi+8], eax
        mov     eax, dword ptr [esi+12]
        mov     dword ptr [edi+12], eax
        
        mov     eax, dword ptr [esi+16]
        mov     dword ptr [edi+16], eax
        mov     eax, dword ptr [esi+20]
        mov     dword ptr [edi+20], eax
        
        mov     eax, dword ptr [esi+24]
        mov     dword ptr [edi+24], eax
        mov     eax, dword ptr [esi+28]
        mov     dword ptr [edi+28], eax
        
        mov     eax, dword ptr [esi+32]
        mov     dword ptr [edi+32], eax
        mov     eax, dword ptr [esi+36]
        mov     dword ptr [edi+36], eax
        
        mov     eax, dword ptr [esi+40]
        mov     dword ptr [edi+40], eax

        
        ;
        ; Get the linear address of the page directory.
        ;
        mov     ebx, cr3
        or      ebx, LIN_TO_PHYS_OFFSET

        ;
        ; Create the identity-mapped addresses (physical = linear)
        ;

        mov     ecx, OFFSET32 IdentityMapped
        mov     edx, ecx
        and     edx, LIN_TO_PHYS_MASK
        ;
        ; ECX = current linear addr, EDX = current physical addr.
        ;
        shr     ecx, 22                 ; Which 4MB?
        shl     ecx, 2                  ; DWORD index
        shr     edx, 22                 ; Which 4MB?
        shl     edx, 2                  ; DWORD index

        ;
        ; Copy the page directory entry for the current linear
        ; address to the physical address.
        ;
        mov     eax, DWORD PTR [ebx][ecx]
        mov     DWORD PTR [ebx][edx], eax

        wbinvd                          ; Flush the cache

        mov     ebx, cr3
        mov     cr3, ebx                ; Flush the TLB

        ;
        ; Move execution to the identity-mapped physical address.
        ;
        mov     edx, OFFSET32 IdentityMapped
        and     edx, LIN_TO_PHYS_MASK
        jmp     edx

IdentityMapped:
        mov     edx, OFFSET32 PagingDisabled
        and     edx, LIN_TO_PHYS_MASK

        mov     eax, cr0
        and     eax, not PG_MASK        ; Disable paging
        or      eax, CD_MASK            ; Disable cache
        mov     cr0, eax                ; Apply new system settings

        wbinvd                          ; Flush the cache

        mov     ebx, cr3
        mov     cr3, ebx                ; Flush the TLB

        and     eax, not CD_MASK        ; Re-enable cache
        mov     cr0, eax                ; Apply new system settings

        jmp     edx                     ; Far jump to purge any prefetch state.

        align   4
PagingDisabled:
        ;
        ; Move the stack pointer to the physical address.
        ;
        ;
        ; switch to boot GDT
        ;
        mov     eax, BOOTPAGE
        and     eax, LIN_TO_PHYS_MASK
        lea     ecx, GDTPtr
        and     ecx, LIN_TO_PHYS_MASK
        
        lea     esp, [eax-4092]

        lgdt    FWORD PTR [ecx]         ; Load the GDTR

        mov     eax, DS_FLAT_SEL
        mov     ds, ax
        mov     es, ax
        mov     ss, ax


INITIAL_CR0     EQU     00000011h
INITIAL_CR3     EQU     00000000h
INITIAL_CR4     EQU     00000000h
INITIAL_EFLAGS  EQU     00007046h

        mov     eax, INITIAL_CR0
        mov     cr0, eax
        mov     eax, INITIAL_CR3
        mov     cr3, eax
.586p
        mov     eax, INITIAL_CR4
        mov     cr4, eax
.486p
        mov     eax, INITIAL_EFLAGS


        push    eax
        popfd

        ;
        ; Read from RAM the address we are jumping to.
        ;
        mov     edx, _dwRebootAddress
        and     edx, LIN_TO_PHYS_MASK
        mov     ebx, dword ptr [edx]
        and     ebx, LIN_TO_PHYS_MASK
        ;
        ; Jump to the first instruction.
        ;
        jmp     ebx


_RebootHandler ENDP

;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
_CallBios32 PROC NEAR PUBLIC
RegisterArray   equ  8
CallingAddress  equ  12
        push    ebp
        mov     ebp,esp
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi
        ; Load Register.
        mov     ebx,DWORD PTR RegisterArray[ebp]
        mov     eax,DWORD PTR [ebx]
        mov     ecx,DWORD PTR [ebx+8]
        mov     edx,DWORD PTR [ebx+12]
        mov     esi,DWORD PTR [ebx+16]
        mov     edi,DWORD PTR [ebx+20]
        mov     ebx,DWORD PTR [ebx+4]
        ; Calling into BIOS32
        push    ebp
        mov     ebp,DWORD PTR CallingAddress[ebp]
        push    cs
        call    ebp
        pop     ebp
        ;Save Register
        push    ebx
        mov     ebx,DWORD PTR RegisterArray[ebp]
        mov     DWORD PTR [ebx],eax
        mov     DWORD PTR [ebx+8],ecx
        mov     DWORD PTR [ebx+12],edx
        mov     DWORD PTR [ebx+16],esi
        mov     DWORD PTR [ebx+20],edi
        pop     eax
        mov     DWORD PTR [ebx+4],eax
        ; returning
        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx

        pop     ebp
        jc      CallBios32_Fails
        xor     eax,eax
        inc     eax
        ret     0
CallBios32_Fails:
        xor     eax,eax
        ret     0

_CallBios32 ENDP


;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
_GetDS PROC NEAR PUBLIC
        push ds
        pop  eax
        ret 0
_GetDS ENDP

_TEXT ENDS


        END

