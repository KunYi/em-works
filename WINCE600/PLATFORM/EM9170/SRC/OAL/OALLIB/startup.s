;******************************************************************************
;*
;* Copyright (c) Microsoft Corporation.  All rights reserved.
;*
;* Use of this source code is subject to the terms of the Microsoft end-user
;* license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
;* If you did not accept the terms of the EULA, you are not authorized to use
;* this source code. For a copy of the EULA, please see the LICENSE.RTF on your
;* install media.
;*
;******************************************************************************
;*
;* Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
;* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;*
;******************************************************************************
;*
;* FILE:    startup.s
;*
;* PURPOSE: Before control is transferred to the kernel, the boot loader
;*          calls this StartUp code to put the CPU into an initialized state.
;*
;******************************************************************************
    IF :LNOT: :DEF: BOOTLOADER
        GBLL        BOOTLOADER
BOOTLOADER      SETL    {FALSE}
    ENDIF
    
    OPT 2                                       ; disable listing
    INCLUDE kxarm.h
    OPT 1                                       ; reenable listing
    
    INCLUDE mx25_base_mem.inc
    INCLUDE mx25_base_regs.inc
    INCLUDE mx25_crm.inc
    
    IF :LNOT: BOOTLOADER
        IMPORT  KernelStart
    ENDIF

;
; ARM constants
;
ARM_CPSR_PRECISE            EQU     (1 << 8)
ARM_CPSR_IRQDISABLE         EQU     (1 << 7)
ARM_CPSR_FIQDISABLE         EQU     (1 << 6)
ARM_CPSR_MODE_SVC           EQU     0x13
ARM_CTRL_ICACHE             EQU     (1 << 12)
ARM_CTRL_DCACHE             EQU     (1 << 2)
ARM_CTRL_MMU                EQU     (1 << 0)
ARM_CTRL_VECTORS            EQU     (1 << 13)
ARM_CACR_FULL               EQU     0x3

;
; MAX Constants
;
MAX_MPR0_OFFSET             EQU     0x0000
MAX_MPR1_OFFSET             EQU     0x0100
MAX_MPR2_OFFSET             EQU     0x0200
MAX_MPR3_OFFSET             EQU     0x0300
MAX_MPR4_OFFSET             EQU     0x0400

; CRM Lock flags
MPLL_LOCK_FLAG              EQU     0x00008000
UPLL_LOCK_FLAG              EQU     0x00008000


;******************************************************************************
;*
;* FUNCTION:    StartUp
;*
;* DESCRIPTION: System bootstrap function
;*
;* PARAMETERS:  None
;*
;* RETURNS:     None
;*
;******************************************************************************

    STARTUPTEXT
    LEAF_ENTRY StartUp
    
    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Put the processor in supervisor mode
    ; Disable the interrupt request (IRQ) and fast interrupt request (FIQ) 
    ; inputs
    ;--------------------------------------------------------------------------
    mrs     r0, cpsr                            ; r0 = CPSR
    mov     r0, #ARM_CPSR_MODE_SVC              ; enter supervisor mode
    orr     r0, r0, #ARM_CPSR_IRQDISABLE        ; disable normal IRQ
    orr     r0, r0, #ARM_CPSR_FIQDISABLE        ; disable fast IRQ
    msr     cpsr_c, r0                          ; update CPSR control bits
   
    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Disable memory management unit (MMU) and both the instruction and data 
    ; caches
    ;--------------------------------------------------------------------------
    mrc     p15, 0, r0, c1, c0, 0               ; r0 = system control reg
    bic     r0, r0, #ARM_CTRL_ICACHE            ; disable ICache
    bic     r0, r0, #ARM_CTRL_DCACHE            ; disable DCache
    bic     r0, r0, #ARM_CTRL_MMU               ; disable MMU
    bic     r0, r0, #ARM_CTRL_VECTORS           ; set vector base to 0x00000000
    mcr     p15, 0, r0, c1, c0, 0               ; update system control reg

    ; ENABLE INSTRUCTION CACHE
    ; The clock configuration from code in RAM is complicated.
    ; So activate the instrcution cache (using CoProc15) permit to execute clock
    ; configuration from cache (so no RAM access).
    mrc     p15, 0, r1, c1, c0, 0   ; r0 = system control reg
    orr     r1, r1, #0x1000         ; Enable Instruction Cache (IC)
    mcr     p15, 0, r1, c1, c0, 0   ; update system control reg.

    ; AIPS Init
    ; Configure AIPS controller

    mov r1,#0
    ldr r2,=0x77777777
    
    ldr r0,=0x43f00000
    str r1,[r0,#0x40]       ;setmem  0x43f00040 0x00000000  32
    str r1,[r0,#0x44]       ;setmem  0x43f00044 0x00000000  32
    str r1,[r0,#0x48]       ;setmem  0x43f00048 0x00000000  32
    str r1,[r0,#0x4C]       ;setmem  0x43f0004C 0x00000000  32
    str r1,[r0,#0x50]       ;setmem  0x43f00050 0x00000000  32  
    str r2,[r0,#0x0]        ;setmem  0x43f00000 0x77777777  32
    str r2,[r0,#0x4]        ;setmem  0x43f00004 0x77777777  32  
    ldr r0,=0x53f00000      
    str r1,[r0,#0x40]       ;setmem  0x53f00040 0x00000000  32
    str r1,[r0,#0x44]       ;setmem  0x53f00044 0x00000000  32
    str r1,[r0,#0x48]       ;setmem  0x53f00048 0x00000000  32
    str r1,[r0,#0x4C]       ;setmem  0x53f0004C 0x00000000  32  
    str r1,[r0,#0x50]       ;setmem  0x53f00050 0x00000000  32
    str r2,[r0,#0x0]        ;setmem  0x53f00000 0x77777777  32  
    str r2,[r0,#0x4]        ;setmem  0x53f00004 0x77777777  32

    ;
    ; configure AHB crossbar switch (MAX) registers
    ;
    ;
    ; M0 - ICache
    ; M1 - DCache
    ; M2 - USB OTG
    ; M3 - RTIC
    ; M4 - SDMA/eSDHC2
    ldr     r1, =CSP_BASE_REG_PA_MAX

    ; Master Priority (0 = highest priority)
    ;   M4 > M2 > M3 > M0 > M1
    ;   SDMA/eSDHC2 > USB OTG > RTIC > ICache > DCache
    ldr r0, =(0x00002143)

    ; Master priority configured the same for all slaves
    str     r0, [r1, #MAX_MPR0_OFFSET]
    str     r0, [r1, #MAX_MPR1_OFFSET]
    str     r0, [r1, #MAX_MPR2_OFFSET]
    str     r0, [r1, #MAX_MPR3_OFFSET]
    str     r0, [r1, #MAX_MPR4_OFFSET]

    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Determine the reason you are in the startup code, such as cold reset,
    ; watchdog reset, GPIO reset, and sleep reset.
    ;--------------------------------------------------------------------------


    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Configure the GPIO lines per the requirements of the board. GPIO lines
    ; must be enabled for on-board features like LED.
    ;--------------------------------------------------------------------------


    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Configure the memory controller, set refresh frequency, and enable
    ; clocks.  Program data width and memory timing values and power up the
    ; banks.
    ;--------------------------------------------------------------------------
    ; Configure CRM (Clock Register Module)
    ;----------------------------------------    
    ldr     r1, =CSP_BASE_REG_PA_CRM

    ; Configure MCU PLL
    ;
    ;   MCUPLL  = (CKIH) * 2 * (MFI + MFN/(MFD+1)) / (PDF+1)
    ;           = 24 MHz * 2 * (11 + 1/(11+1)) / (0+1)
    ;           = 532 MHz
    ;
    ;   MFN = 1 = (1 << 0)                          = 0x00000001
    ;   MFI = 11 = (11 << 10)                       = 0x00002C00
    ;   MFD = 11 = (11 << 16)                       = 0x000B0000
    ;   PD = 0 = (0 << 26)                          = 0x00000000
    ;   BRMO = first order = (1 << 31)              = 0x80000000
    ;                                               ------------
    ;                                                 0x800B2C01
    ldr     r0, =0x800B2C01
    str     r0, [r1, #CRM_MPCTL_OFFSET]

    ; Configure USB PLL
    ;
    ;   USBPLL  = (CKIH) * 2 * (MFI + MFN/(MFD+1)) / (PDF+1)
    ;           = 24 * 2 * (5 + 0/(0+1)) / (0+1)
    ;           = 240 MHz
    ;
    ;   MFN = 0 = (0 << 0)                          = 0x00000000
    ;   MFI = 5 = (5 << 10)                         = 0x00001400
    ;   MFD = 0 = (0 << 16)                         = 0x00000000
    ;   PD = 0 = (0 << 26)                          = 0x00000000
    ;   BRMO = first order = (1 << 31)              = 0x80000000
    ;                                               ------------
    ;                                                 0x80001400
    ldr     r0, =0x80001400
    str     r0, [r1, #CRM_UPCTL_OFFSET]
    
    ; Configure Clock Control Register
    ;   Configure ARM and AHB Clock
    ;
    ;   ARM Clock 399Mhz = MCUPLL * DIVGEN / ARM_DIVIDER
    ;                    = 532Mhz * 0.75   / 1
    ;                    = 399 Mhz
    ;       ARM_DIVIDER (0 << 30)   = 0x00000000
    ;       ARM_SRC     (1 << 14)   = 0x00004000
    ;
    ;   AHB Clock 133Mhz = ARMCLK / AHB_DIVIDER
    ;                    = 399Mhz / 3
    ;                    = 133 Mhz
    ;       AHB_DIVIDER (2 << 28)   = 0x20000000
    ;
    ;   USB Clock 60Mhz =  UPLL  / USB_DIVIDER
    ;                   = 240Mhz / 4
    ;                   = 60Mhz
    ;       USB_DIVIDER (3 << 16)   = 0x00030000
    ;
    ;   USB PLL Enable (0 << 23)    = 0x00000000
    ;   MPLL and UPLL reset (3<<26) = 0x0C000000
    ;   LP CTL (0<<24)              = 0x00000000
    ;                               -------------
    ;                                 0x2C034000
    ldr     r0, =0x2C034000

    ldr     r4, =10000

    ALIGN 32
    str     r0, [r1, #CRM_CCTL_OFFSET]
    
; Wait PLL statibilisation
waitClock
    sub r4, r4, #1
    cmp r4, #0
    bne waitClock

    ; Now wait the MPLL is reset, so wait on the MPLL lock bit
WaitForMPLL
    ldr     r0, [r1, #CRM_MPCTL_OFFSET]
    tst     r0, #MPLL_LOCK_FLAG
    beq     WaitForMPLL

    ; Now wait the UPLL is reset, so wait on the UPLL lock bit
WaitForUPLL
    ldr     r0, [r1, #CRM_UPCTL_OFFSET]
    tst     r0, #UPLL_LOCK_FLAG
    beq     WaitForUPLL

    ;   Misc Control Register already correctly configured
    ;       Default value is correct 0x4300 0000
    ;       PER CLK MUX = HCLK Always >> So PER Clock input already 133Mhz
    ;       USB CLK MUX = UPLL Output
    ;       CLK OUTPUT  = Disable


    ; Now set the peripheral clock divider default value
    ; All peripheral clock divider are set to divide by 2.
    
    ; For PER clock from 0 to 3
    ;PCDR0               = 0x01010101
    ldr     r0, =0x01010101
    str       r0, [r1, #CRM_PCDR0_OFFSET]
    
    ; For PER clock from 4 to 7
    ;PCDR1               = 0x01010101
    ldr     r0, =0x01010101
    str       r0, [r1, #CRM_PCDR1_OFFSET]
    
    ; For PER clock from 8 to 11
    ; NFC Clock 33.25MHz
    ;PCDR2               = 0x01010103
    ldr     r0, =0x01010103
    str       r0, [r1, #CRM_PCDR2_OFFSET]
    
    ; For PER clock from 12 to 15
    ;PCDR3               = 0x01010101
    ldr     r0, =0x01010101
    str       r0, [r1, #CRM_PCDR3_OFFSET]



    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Flush or invalidate the instruction and data caches and the translation
    ; look-aside buffer (TLB) and empty the write buffers
    ;--------------------------------------------------------------------------


    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Initialize the real-time clock count register to 0 and enable the
    ; real-time clock. 
    ;--------------------------------------------------------------------------


    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Set up the power management/monitoring registers. Set conditions during
    ; sleep modes. 
    ;--------------------------------------------------------------------------


    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Turn on all board-level clocks and on chip peripheral clocks.
    ;--------------------------------------------------------------------------


    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Get the physical base address of the OEMAddressTable and store in r0.
    ;--------------------------------------------------------------------------
    ; Get the physical base address of the OEMAddressTable and store in r0.
    add     r0, pc, #g_oalAddressTable - (. + 8)
    
    ;--------------------------------------------------------------------------
    ; MS RECOMMENDATION:
    ; Jump to KernelStart to boot WindowsCE or BootloaderMain for bootloader
    ;--------------------------------------------------------------------------
    ; Jump to KernelStart to boot WindowsCE or BootloaderMain for bootloader
    b       KernelStart

spin
    b       spin

    ; Include memory configuration file with g_oalAddressTable

    INCLUDE oemaddrtab_cfg.inc

    ENTRY_END StartUp

    END
