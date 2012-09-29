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
;  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

    ;--------------------------------------------------------
    ; Enable only necessary clock in clock gating controller
    ;--------------------------------------------------------
    ;   The EMI clock used by the XLDR in Clock Gating Control Register 0
    ;   is not turned off. We only need RAM and NandFlash access so only EMI
    ;   controller must be enable.
    ;       Enable AHB hclk_emi             = 0x00080000
    ;       Enable AHB fec                  = 0x00800000
    ;       Enable AHB ESDHC1               = 0x00200000
    ;       Enable PER UART                 = 0x00008000
    ;       Enable PER NFC                  = 0x00000100
    ;       Enable PER EPIT                 = 0x00000002
    ;       Enable PER ESDHC1               = 0x00000008
    ;                                       ------------
    ;                                         0x00A8810A

    ldr     r1, =CSP_BASE_REG_PA_CRM
    ldr     r0, =0x00A8810A
    str     r0, [r1, #CRM_CGCR0_OFFSET]
   
    ;   Clock Gating Control Register 1
    ;       Enable clk EPIT1 (1<<10)        = 0x00000400
    ;       Enable clk CSPI1 (1<<5)         = 0x00000020
    ;                                       ------------
    ;                                       = 0x00000420
    ldr     r0, =0x00000420
    str     r0, [r1, #CRM_CGCR1_OFFSET]

    ;   Clock Gating Control Register 2
    ;       Enable clk UART1 (1<<46)        = 0x00004000
    ;                                       ------------
    ;                                       = 0x00004000
    ldr     r0, =0x00004010
    str     r0, [r1, #CRM_CGCR2_OFFSET]


    ; Copy the bootloader image from flash to RAM.  The image is configured
    ; to run in RAM, but is stored in flash.  Absolute address references
    ; should be avoided until the image has been relocated and the MMU enabled.
    ;
    ; NOTE: The destination (RAM) address must match the address in the
    ; bootloader's .bib file.  The latter dictates the code fix-up addresses.
    ;
    
    ; No NOR Flash exists on iMX25 PDK board

    ; Check if we are running from NOR flash
;    mov     r0, pc
;    ldr     r1, =IMAGE_BOOT_NORDEV_NOR_PA_START
;    cmp     r0, r1
;    blt     CODEINRAM
;
;    ldr     r1, =IMAGE_BOOT_NORDEV_NOR_PA_END
;    cmp     r0, r1
;    bgt     CODEINRAM
;    
;    ldr     r8, =IMAGE_BOOT_BOOTIMAGE_NOR_PA_START
;    ldr     r1, =IMAGE_BOOT_BOOTIMAGE_RAM_PA_START
;    ldr     r2, =(IMAGE_BOOT_BOOTIMAGE_RAM_SIZE / 16)    ; Bootloader image length (this must be <= the NK
;                                                   ; length in the .bib file).  We are block-copying 
;                                                   ; 16-bytes per iteration.
;                                                   
;    ; Do 4x32-bit block copies from flash->RAM (corrupts r4-r7).
;    ;
;10  ldmia   r8!, {r4-r7}        ; Loads from flash (post increment).
;    stmia   r1!, {r4-r7}        ; Stores to RAM (post increment).
;    subs    r2, r2, #1          ;
;    bne     %B10                ; Done?
;
;
;    ; Now that we've copied ourselves to RAM, jump to the RAM image.  Use the "CodeInRAM" label
;    ; to determine the RAM-based code address to which we should jump.
;    ;
;    ldr     r1, =IMAGE_BOOT_BOOTIMAGE_NOR_PA_START
;    adr     r2, CODEINRAM
;    sub     r2, r2, r1
;    ldr     r0, =IMAGE_BOOT_BOOTIMAGE_RAM_PA_START
;    add     r2, r0, r2
;    mov     pc, r2
;    nop
;    nop
;    nop
;
;CODEINRAM
    ; Now that we're running out of RAM, construct the first-level Section descriptors
    ; to create 1MB mapped regions from the addresses defined in the OEMAddressTable.
    ; This will allow us to enable the MMU and use a virtual address space that matches
    ; the mapping used by the OS image.
    ;
    ; We'll create two different mappings from the addresses specified:
    ;     [8000 0000 --> 9FFF FFFF] = Cacheable, Bufferable
    ;     [A000 0000 --> BFFF FFFF] = NonCacheable, nonBufferable
    ;
    
    ;
    ; Configure wireless external interface module (WEIM)
    ;
    ;Commented out since WEIM not used since no NOR Flash is populated on i.MX25 PDK board.
    ;Looks like iMX35 PDK had NOR Flash on CS0...

;     ldr     r1, =CSP_BASE_REG_PA_WEIM

    ; CS0 control (upper)
    ;   EDC - 3 extra dead cycles (3 << 0)          = 0x00000003
    ;   WWS - 0 extra write wait states (0 << 4)    = 0x00000000
    ;   EW - Posedge DTACK (0 << 7)                 = 0x00000000
    ;   WSC - 12 wait states (12 << 8)              = 0x00000C00
    ;   CNC - 3 CS negation cycles (3 << 14)        = 0x0000C000
    ;   DOL - 0 clock burst latency (0 << 16)       = 0x00000000
    ;   SYNC - Disable sync burst (0 << 20)         = 0x00000000
    ;   PME - Disable page mode (0 << 21)           = 0x00000000
    ;   PSZ - 4 word burst/page (0 << 22)           = 0x00000000
    ;   BCS - 1 BCLK delay (0 << 24)                = 0x00000000
    ;   BCD - /1 burst clock (0 << 28)              = 0x00000000
    ;   WP - No write protect (0 << 30)             = 0x00000000
    ;   SP - All user mode access (0 << 31)         = 0x00000000
    ;                                               ------------
    ;                                                 0x0000CC03
;    ldr     r0, =0x0000CC03
;    str     r0, [r1, #WEIM_CSCR0U_OFFSET]

    ; CS0 control (lower)
    ;   CSEN - Enable chip select (1 << 0)          = 0x00000001
    ;   WRAP - No wrap (0 << 1)                     = 0x00000000
    ;   CRE -  CRE pin 0 (0 << 2)                   = 0x00000000
    ;   PSR - PSRAM mode disabled (0 << 3)          = 0x00000000
    ;   CSN -  2 AHB clocks (2 << 4)                = 0x00000000
    ;   DSZ - 16-bit DATA[15:0] (5 << 8)            = 0x00000500
    ;   EBC - Only write asserts EB (1 << 11)       = 0x00000800
    ;   CSA - 2 half AHB clocks (0 << 12)           = 0x00000000
    ;   EBWN - 3 half AHB clocks (3 << 16)          = 0x00030000
    ;   EBWA - 3 half AHB clocks (3 << 20)          = 0x00300000
    ;   OEN - 2 half AHB clocks (0 << 24)           = 0x00000000
    ;   OEA - 10 half AHB clocks (10 << 28)         = 0xA0000000
    ;                                               ------------
    ;                                                 0xA0330D01
;    ldr     r0, =0xA0330D01
;    str     r0, [r1, #WEIM_CSCR0L_OFFSET]

    ; CS0 control (additional)
    ;   FCE - Data capture by AHB clk (0 << 0)      = 0x00000000
    ;   CNC2 - No CNC increase (0 << 1)             = 0x00000000
    ;   AGE - Disable ack glue logic (0 << 2)       = 0x00000000
    ;   WWU - Forbit wrap on write (0 << 3)         = 0x00000000
    ;   DCT - 2 AHB clocks (0 << 4)                 = 0x00000000
    ;   DWW - wait states same as read (0 << 6)     = 0x00000000
    ;   LBA - 0 half AHB clock (0 << 8)             = 0x00000000
    ;   LBN - 2 half AHB clocks (2 << 10)           = 0x00000800
    ;   LAH - 3 half AHB clocks (2 << 13)           = 0x00000000
    ;   MUM - Non-muxed mode (0 << 14)              = 0x00000000
    ;   RWN - 2 half AHB clocks (2 << 16)           = 0x00020000
    ;   RWA - 2 half AHB clocks (2 << 20)           = 0x00200000
    ;   EBRN - 0 half AHB clocks (0 << 24)          = 0x00000000
    ;   EBRA - 0 half AHB clocks (0 << 28)          = 0x00000000
    ;                                               ------------
    ;                                                 0x00220800
;    ldr     r0, =0x00220800
;    str     r0, [r1, #WEIM_CSCR0A_OFFSET]


BUILDTTB

    add     r11, pc, #g_oalAddressTable-(.+8)    ; Pointer to OEMAddressTable.
    
    ; Set the TTB.
    ;
    ldr     r9, =IMAGE_BOOT_BOOTPT_RAM_PA_START
    ldr     r0, =0xFFFFC000                   ;
    and     r9, r9, r0                        ; Mask off TTB to be on 16K boundary.
    mcr     p15, 0, r9, c2, c0, 0             ; Set the TTB.

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

ACTIVATEMMU
    ; The 1st Level Section Descriptors are setup. Initialize the MMU and turn it on.
    ;
    mov     r1, #1
    mcr     p15, 0, r1, c3, c0, 0   ; Set up access to domain 0.
    mov     r0, #0
    mcr     p15, 0, r0, c8, c7, 0   ; Flush the instruction and data TLBs.
    mcr     p15, 0, r1, c7, c10, 4  ; Drain the write and fill buffers.

    mov     r1, #0x78               ; Bits [6:3] must be written as 1's.
    orr     r1, r1, #0x1            ; Enable MMU.
    orr     r1, r1, #0x1000         ; Enable IC.
    orr     r1, r1, #0x0800         ; Enable BTB.
    orr     r1, r1, #0x4            ; Enable DC.
    ldr     r2, =VirtualStart       ; Get virtual address of 'VirtualStart' label.
    cmp     r2, #0                  ; Make sure no stall on "mov pc,r2" below.

    ; Enable the MMU.
    ;
    mcr     p15, 0, r1, c1, c0, 0   ; MMU ON:  All memory accesses are now virtual.

    ; Jump to the virtual address of the 'VirtualStart' label.
    ;
    mov     pc, r2                  ;
    nop
    nop
    nop

    ; *************************************************************************
    ; *************************************************************************
    ; The MMU and caches are now enabled and we're running in a virtual
    ; address space.
    ;
    
    ALIGN
    
VirtualStart

    ;  Set up a supervisor mode stack.
    ;
    ; NOTE: These values must match the OEMAddressTable and .bib file entries for
    ; the bootloader.
    ;
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
   
    ; r3 now contains the physical launch address.
    ;
    mov     r3, r0

    ; Compute the physical address of the PhysicalStart tag.  We'll jump to this
    ; address once we've turned the MMU and caches off.
    ;
    stmdb   sp!, {r3}
    ldr     r0, =PhysicalStart
    bl      OALVAtoPA
    nop
    ldmia   sp!, {r3}
    
    ; r0 now contains the physical address of 'PhysicalStart'.
    ; r3 now contains the physical launch address.
   
    ; Next, we disable the MMU, and I&D caches.
    ;
    mov     r1, #0x0078
    mcr     p15, 0, r1, c1, c0, 0

    
    ; Jump to 'PhysicalStart'.
    ;
    mov  pc, r0
    nop
    nop
    nop
    nop

PhysicalStart

    ; Flush the I&D TLBs.
    ;
    mcr     p15, 0, r2, c8, c7, 0   ; Flush the I&D TLBs

    ; Jump to the physical launch address.  This should never return...
    ;    
    mov     pc, r3
    nop
    nop
    nop
    nop
    nop
    nop
       
    END

    
