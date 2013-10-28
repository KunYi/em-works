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

        INCLUDE kxarm.h

        IMPORT  pTOC
        IMPORT  BootMain
        IMPORT  BootMapTable

        STARTUPTEXT

;-------------------------------------------------------------------------------
;
;  Function:  BootMMUStart
;
;
        LEAF_ENTRY BootMMUStart

        ;---------------------------------------------------------------
        ; Initialize 1st level page table
        ;---------------------------------------------------------------

        mov     r11, r0                 ; Save map table physical address

        ; Get 1st level page PA
        ldr     r0, =PTs                ; 1st level page table VA
        mov     r1, r11                 ; (r1) - map table address
        bl      CAtoPA
        mov     r10, r0                 ; (r10) - 1st level page table 16k aligned

; r10 = TTB physical base address
; r11 = Memory Map Table

        ; Set the TTB.
        mov     r9, r10
        ldr     r0, =0xFFFFC000                   ;
        and     r9, r9, r0                        ; Mask off TTB[31:0] (must be 0's).
        mcr     p15, 0, r9, c2, c0, 0             ; Set the TTB.

        ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ; MAP CACHED and BUFFERED SECTION DESCRIPTORS
        ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        mov     r0, #0x0E                         ; Section (1MB) descriptor; (C=B=1: write-back, read-allocate).
        orr     r0, r0, #0x400                    ; Set AP.
blMapStart        
        mov     r1, r11                           ; Pointer to OEMAddressTable.

        ; Start Crunching through the OEMAddressTable[]:
        ;
        ; r2 temporarily holds OEMAddressTable[VA]
        ; r3 temporarily holds OEMAddressTable[PHY]
        ; r4 temporarily holds OEMAddressTable[#MB]
        ;
blMapSection        
        ldr     r2, [r1], #4                       ; Virtual (cached) address to map physical address to.
        ldr     r3, [r1], #4                       ; Physical address to map from.
        ldr     r4, [r1], #4                       ; Number of MB to map.

        cmp     r4, #0                             ; End of table?
        beq     blMapEnd

        ; r2 holds the descriptor address (virtual address)
        ; r0 holds the actual section descriptor

        ; Create descriptor address.
        ldr     r6, =0xFFF00000
        and     r2, r2, r6             ; Only VA[31:20] are valid.
        orr     r2, r9, r2, LSR #18    ; Build the descriptor address:  r2 = (TTB[31:14} | VA[31:20] >> 18)

        ; Create the descriptor.
        ldr     r6, =0xFFF00000
        and     r3, r3, r6             ; Only PA[31:20] are valid for the descriptor and the rest will be static.
        orr     r0, r3, r0             ; Build the descriptor: r0 = (PA[31:20] | the rest of the descriptor)

        ; Store the descriptor at the proper (physical) address
blMapMeg
        str     r0, [r2], #4
        add     r0, r0, #0x00100000    ; Section descriptor for the next 1MB mapping (just add 1MB).
        sub     r4, r4, #1             ; Decrement number of MB left.
        cmp     r4, #0                 ; Done?
        bne     blMapMeg               ; No - map next MB.

        bic     r0, r0, #0xF0000000    ; Clear section base address field.
        bic     r0, r0, #0x0FF00000    ; Clear section base address field.
        b       blMapSection           ; Get and process the next OEMAddressTable element.

        ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ; MAP UNCACHED and UNBUFFERED SECTION DESCRIPTORS
        ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
blMapEnd
        tst     r0, #8             
        bic     r0, r0, #0x0C          ; Clear cached and buffered bits in the descriptor (clear C&B bits).
        add     r9, r9, #0x0800        ; Pointer to the first PTE for "unmapped uncached space" (0x2000 0000 + V_U_Adx).
        bne     blMapStart             ; Repeat the descriptor setup for uncached space (map C=B=0 space).

        ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ; ACTIVATE MMU
        ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ; The 1st Level Section Descriptors are setup. Initialize the MMU and turn it on.
        mov     r1, #1
        mcr     p15, 0, r1, c3, c0, 0   ; Set up access to domain 0.
        mcr     p15, 0, r0, c8, c7, 0   ; Flush the instruction and data TLBs.
        mcr     p15, 0, r1, c7, c10, 4  ; Drain the write and fill buffers.

        mov     r1, #0x78               ; Bits [6:3] must be written as 1's.
        orr     r1, r1, #0x1            ; Enable MMU.
        orr     r1, r1, #0x1000         ; Enable IC.
        orr     r1, r1, #0x0800         ; Enable BTB.
        orr     r1, r1, #0x4            ; Enable DC.
        ldr     r0, =blVirtualStart     ; Get virtual address of 'VirtualStart' label.
        cmp     r0, #0                  ; Make sure no stall on "mov pc,r2" below.

        ; Enable the MMU
        mcr     p15, 0, r1, c1, c0, 0   ; All memory accesses are now virtual.

        ; Jump to the virtual address of the 'blVirtualStart' label.
        mov     pc, r0   
        nop
        nop
        nop

    ALIGN
blVirtualStart
        ;---------------------------------------------------------------
        ; Initialize stack pointer on top of RAM
        ;---------------------------------------------------------------
        ldr     sp, =pTOC
        ldr     sp, [sp]
        ldr     sp, [sp, #0x001C]       ; ulRAMEnd

        ;---------------------------------------------------------------
        ; Jump to BootMain code
        ;---------------------------------------------------------------
        b       BootMain
        nop
        nop
        nop
blDeath
        b      blDeath

        ENTRY_END

;-------------------------------------------------------------------------------
;
;  Function:  BootJumpTo
;
;  This function switch off MMU and jump to address passed in (r0).
;
;
        LEAF_ENTRY BootJumpTo
        
        mov     r11, r0
        
        ldr     r0, =PhysicalStart
        ldr     r1, =BootMapTable
        bl      CAtoPA

        mcr     p15, 0, r15, c7, c14, 3 ; Clean and invalidate data cache

        mov     r1, #0                  ; Clean write buffer
        mcr     p15, 0, r1, c7, c10, 4

        mov     r1, #0                  ; Invalidate instruction cache
        mcr     p15, 0, r1, c7, c5, 0

        mrc     p15, 0, r1, c1, c0, 0   ; Disable MMU
        bic     r1, r1, #(1 :SHL: 0)    
        mcr     p15, 0, r1, c1, c0, 0

        ; branch must be in the next three instructions (prefecth) since virtual memory had just gone away
        nop
        mov     pc, r0                  ; Jump to PhysicalStart
        nop

PhysicalStart
        mov     r0, #0                  ; Flush the TLB
        mcr     p15, 0, r0, c8, c7, 0
        mov     pc, r11                 ; Jump to final address

        ENTRY_END
        
;-------------------------------------------------------------------------------
;
;  Function:  PAtoCA
;
;  This function translates physical address (r0) to virtual cached address
;  (returned in r0). Translastion is based on table passed in r1.
;  Uses r2, r3, r12.


        LEAF_ENTRY PAtoCA

        mov     r12, r0                 ; (r12) = PA
        mov     r0, #0                  ; (r0) = 0 (invalid VA)

10      ldr     r3, [r1, #8]            ; (r3) = region size
        ldr     r2, [r1, #4]            ; (r2) = region PA
        cmp     r3, #0                  ; EndOfTable?
        moveq   pc, lr                  ; PA not found if end of table
        
        cmp     r12, r2                 ; Is PA >= region PA?
        blt     %f20                    ; Go to next entry if not

        add     r2, r2, r3, LSL #20     ; region PA + size
        cmp     r12, r2                 ; Is PA <  region PA + size?
        blt     %f30                    ; Found if it's true

20      add     r1, r1, #12             ; Move to next entry
        b       %b10

30      ldr     r0, [r1]                ; Region VA
        ldr     r3, [r1, #4]            ; Region PA
        mov     r0, r0, LSR #20         ; (r0) >>= 20
        add     r0, r12, r0, LSL #20    ; (r0) = PA + (r0 << 20)
        sub     r0, r0, r3              ; (r0) -= region PA
        
        mov     pc, lr                  ; return

        ENTRY_END

;-------------------------------------------------------------------------------
;
;  Function:  CAtoPA
;
;  This function translates virtual address (r0) to physical address
;  (returned in r0). Translastion is based on table passed in r1.
;  Uses r2, r3, r12.

        LEAF_ENTRY CAtoPA

        mov     r12, r0                 ; (r12) = VA
        mov     r0, #-1                 ; (r0) = -1 (invalid PA)

10      ldr     r3, [r1]                ; (r3) = region VA
        mov     r2, r3, LSR #20         ; make 1MB aligned
        mov     r3, r2, LSL #20

        cmp     r3, #0                  ; EndOfTable?
        moveq   pc, lr                  ; VA not found if end of table
        
        cmp     r12, r3                 ; Is VA >= region VA?
        blt     %f20                    ; go to next entry if not

        ldr     r2, [r1, #8]            ; (r2) = region size
        add     r2, r3, r2, LSL #20     ; (r2) = region VA + size
        cmp     r12, r2                 ; Is VA < region VA + size?
        blt     %f30                    ; Found if it's true

20      add     r1, r1, #12             ; Move to next entry
        b       %b10

30      ldr     r0, [r1, #4]            ; (r0) = region PA
        mov     r0, r0, LSR #20         ; (r0) >>= 20
        add     r0, r12, r0, LSL #20    ; (r0) = VA + (r0 << 20)
        sub     r0, r0, r3              ; (r0) -= region VA
        
        mov     pc, lr                  ; Return
        
        ENTRY_END

;-------------------------------------------------------------------------------

        AREA |.KDATA|,DATA,NOINIT
        ALIGN 16384 ; page tables must be start on a 16k boundary
PTs     %       0x4000                  ; Space for first-level page table

;-------------------------------------------------------------------------------

        END

