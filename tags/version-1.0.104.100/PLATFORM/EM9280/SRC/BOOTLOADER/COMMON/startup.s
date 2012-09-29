;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;------------------------------------------------------------------------------
;
;  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
;  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;  File:  startup.s
;
;  Defines configuration parameters used to create the NK and Bootloader
;  program images.
;
;------------------------------------------------------------------------------

    GBLL    BOOTLOADER
BOOTLOADER   SETL    {TRUE}

    INCLUDE ..\\..\\..\\..\\src\\oal\\oallib\\startup.s
    INCLUDE image_cfg.inc

    IMPORT  main
    IMPORT  OALVAtoPA
    IMPORT  OALPAtoVA

;-------------------------------------------------------------------------------
;
; KernelStart: OEM bootloader startup code.  This routine will:
;
; * Copy the image to RAM if it's not already running there.
;
; * Set up the MMU and Dcache for the bootloader.
;
; * Initialize the first-level page table based up the contents
;   of the MemoryMap array and enable the MMU and caches.
;
; Inputs: None.
; 
; On return: N/A.
;
; Register used:
;
;-------------------------------------------------------------------------------
;
    ALIGN
    
    LEAF_ENTRY KernelStart

    ;[TODO]if boot from flash copy code from flash to RAM
            
CODEINRAM
    ; Now that we're running out of RAM, construct the first-level Section descriptors
    ; to create 1MB mapped regions from the addresses defined in the OEMAddressTable.
    ; This will allow us to enable the MMU and use a virtual address space that matches
    ; the mapping used by the OS image.
    ;
    ; We'll create two different mappings from the addresses specified:
    ;     [8000 0000 --> 9FFF FFFF] = Cacheable, Bufferable
    ;     [A000 0000 --> BFFF FFFF] = NonCacheable, nonBufferable
    ;
BUILDTTB

    add     r11, pc, #g_oalAddressTable-(.+8)    ; Pointer to OEMAddressTable.
    
    ; Set the TTB.
    ;
    ldr     r9, =IMAGE_BOOT_BOOTPT_RAM_PA_START
    ldr     r0, =0xFFFFC000                   ;
    and     r9, r9, r0                        ; Mask off TTB to be on 16K boundary.
    mcr     p15, 0, r9, c2, c0, 0             ; Set the TTB.
    mov     r10, r9                           ; Save TTB address

    ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ; ~~~~~~~~~~ MAP CACHED and BUFFERED SECTION DESCRIPTORS ~~~~~~~~~~~~~~~~~~
    ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    mov     r0, #0x0A                         ; Section (1MB) descriptor; (C=1 B=0: cached write-through).
    orr     r0, r0, #0x400                    ; Set AP.
20  mov     r1, r11                           ; Pointer to OEMAddressTable.

    ; Start Crunching through the OEMAddressTable[]:
    ;
    ; r2 temporarily holds OEMAddressTable[VA]
    ; r3 temporarily holds OEMAddressTable[PHY]
    ; r4 temporarily holds OEMAddressTable[#MB]
    ;
25  ldr     r2, [r1], #4                       ; Virtual (cached) address to map physical address to.
    ldr     r3, [r1], #4                       ; Physical address to map from.
    ldr     r4, [r1], #4                       ; Number of MB to map.

    cmp     r4, #0                             ; End of table?
    beq     %F29

    ; r2 holds the descriptor address (virtual address)
    ; r0 holds the actual section descriptor
    ;

    ; Create descriptor address.
    ;
    ldr     r6, =0xFFF00000
    and     r2, r2, r6             ; Only VA[31:20] are valid.
    orr     r2, r9, r2, LSR #18    ; Build the descriptor address:  r2 = (TTB[31:14} | VA[31:20] >> 18)

    ; Create the descriptor.
    ;
    ldr     r6, =0xFFF00000
    and     r3, r3, r6             ; Only PA[31:20] are valid for the descriptor and the rest will be static.
    orr     r0, r3, r0             ; Build the descriptor: r0 = (PA[31:20] | the rest of the descriptor)

    ; Store the descriptor at the proper (physical) address
    ;
28  str     r0, [r2], #4
    add     r0, r0, #0x00100000    ; Section descriptor for the next 1MB mapping (just add 1MB).
    sub     r4, r4, #1             ; Decrement number of MB left.
    cmp     r4, #0                 ; Done?
    bne     %B28                   ; No - map next MB.

    bic     r0, r0, #0xF0000000    ; Clear section base address field.
    bic     r0, r0, #0x0FF00000    ; Clear section base address field.
    b       %B25                   ; Get and process the next OEMAddressTable element.

    ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ; ~~~~~~~~~~ MAP UNCACHED and UNBUFFERED SECTION DESCRIPTORS ~~~~~~~~~~~~~~
    ; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

29  tst     r0, #8                 ; Test for 'C' bit set (means we just used 
                                   ; above loop structure to map cached and buffered space).
    bic     r0, r0, #0x0C          ; Clear cached and buffered bits in the descriptor (clear C&B bits).
    add     r9, r9, #0x0800        ; Pointer to the first PTE for "unmapped uncached space" (0x2000 0000 + V_U_Adx).
    bne     %B20                   ; Repeat the descriptor setup for uncached space (map C=B=0 space).

    ; Create descriptor for 1M identity mapping to allow MMU enable transition.
    ; NOTE:  code assume EBOOT does not span 1M boundary from current execution point.
    and     r1, pc, r6
    orr     r0, r1, #(2 << 0)       ; L1D[1:0) = 2 = 1MB Section
    orr     r0, r0, #(1 << 10)      ; L1D[11:10] = 1 = AP is R/W privileged

    orr     r2, r10, r1, LSR #18    ; &L1D = TTB + (VA[31:20] >> 18)
    ldr     r3, [r2]                ; Save existing mapping
    str     r0, [r2]                ; Create identity mapping

ACTIVATEMMU
    ; The 1st Level Section Descriptors are setup. Initialize the MMU and turn it on.
    ;
    mov     r1, #1
    mcr     p15, 0, r1, c3, c0, 0   ; Set up access to domain 0.
    mov     r0, #0
    mcr     p15, 0, r0, c8, c7, 0   ; Flush the instruction and data TLBs.
    mcr     p15, 0, r1, c7, c10, 4  ; Drain the write and fill buffers.

    ;mov     r1, #0x78               ; Bits [6:3] must be written as 1's. Reason?
    orr     r1, r1, #0x1            ; Enable MMU.
    orr     r1, r1, #0x1000         ; Enable IC.
    orr     r1, r1, #0x4            ; Enable DC.

    ldr     r4, =VirtualStart       ; Get virtual address of 'VirtualStart' label.
    cmp     r4, #0                  ; Make sure no stall on "mov pc,r4" below.
    mcr     p15, 0, r1, c1, c0, 0   ; Enable MMU and caches
    mov     pc, r4                  ; Jump to virtual address
    nop

    ; We are now running virtual address space.
VirtualStart
    str     r3, [r2]                ; Restore mapping overwritten by identity
                                    ; mapping
    mcr     p15, 0, r2, c8, c7, 0   ; Flush the I&D TLBs

    ; Set up EBOOT stack
    ldr     sp, =(IMAGE_BOOT_STACK_RAM_CA_START)

    ; Jump to the C entrypoint.
    ;    
    bl      main                              ; Jump to main.c::main(), never to return...
    nop
    nop
    nop
    
STALL2
    b      STALL2 



;-------------------------------------------------------------------------------
;
; void Launch(UINT32 pFunc): This function launches the program at pFunc (pFunc
;                            is a physical address).  The MMU is disabled just
;                            before jumping to specified address.
;
; Inputs: pFunc (r0) - Physical address of program to Launch.
; 
; On return: None - the launched program never returns.
;
; Register used:
;
;-------------------------------------------------------------------------------
;
    ALIGN
    LEAF_ENTRY Launch
   
    mov     r4, r0                  ; Save jump address

    mrc     p15, 0, r0, c2, c0, 0   ; Get the TTB into r0
    ldr     r1, =0xFFFFC000
    and     r0, r0, r1              ; Mask off TTB to be on 16K boundary.
    mov     r1, #0                  ; Cached parameter of OALPAtoVA = FALSE
    bl      OALPAtoVA               ; Translate TTB PA into UA
    mov     r5, r0                  ; Save TTB virtual address in r5
    
    ; Create descriptor for 1M identity mapping to allow MMU disable transition.
    ; NOTE:  code assume EBOOT does not span 1M boundary from current execution point.
    mov     r0, pc
    bl      OALVAtoPA               ; Translate PC VA into PA
    ldr     r1, =0xFFF00000
    and     r1, r0, r1
    orr     r0, r1, #(2 << 0)       ; L1D[1:0) = 2 = 1MB Section
    orr     r0, r0, #(1 << 10)      ; L1D[11:10] = 1 = AP is R/W privileged

    orr     r2, r5, r1, LSR #18     ; &L1D = TTB + (VA[31:20] >> 18)
    str     r0, [r2]                ; Create identity mapping
    mcr     p15, 0, r2, c8, c7, 0   ; Flush the I&D TLBs
    mcr     p15, 0, r1, c7, c10, 4  ; Drain the write and fill buffers.
    
    ldr     r0, =VirtualEnd
    bl      OALVAtoPA               ; r0 contains physical address of code
    mov     pc, r0                  ; Jump to physical before MMU disable
   
VirtualEnd

    ; Next, we disable the MMU, and I&D caches.
    ;
    mrc     p15, 0, r1, c1, c0, 0   ; Read control register
    bic     r1, r1, #0x1            ; Disable MMU.
    bic     r1, r1, #0x1000         ; Disable IC.
    bic     r1, r1, #0x4            ; Disable DC.
    mcr     p15, 0, r1, c1, c0, 0   ; MMU OFF:  All memory accesses are now virtual.

    ; Flush the I&D TLBs.
    ;
    mcr     p15, 0, r2, c8, c7, 0   ; Flush the I&D TLBs

    ; Jump to the physical launch address.  This should never return...
    ;    
    mov     pc, r4
    nop
    nop
    nop
    nop
    nop
    nop
       
    END

    
