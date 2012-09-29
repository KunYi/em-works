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
;* Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

;
; ARM constants
;
ARM_CPSR_PRECISE            EQU     (1 << 8)
ARM_CPSR_IRQDISABLE         EQU     (1 << 7)
ARM_CPSR_FIQDISABLE         EQU     (1 << 6)
ARM_CPSR_MODE_SVC           EQU     0x13

ARM_CTRL_MMU                EQU     (1 << 0)
ARM_CTRL_DCACHE             EQU     (1 << 2)
ARM_CTRL_FLOW               EQU     (1 << 11)
ARM_CTRL_ICACHE             EQU     (1 << 12)
ARM_CTRL_VECTORS            EQU     (1 << 13)
ARM_CTRL_TRE                EQU     (1 << 28)

ARM_CACR_FULL               EQU     0x3

ARM_AUXCR_L2EN              EQU     (1 << 1)

; VFP uses coproc 10 for single-precision instructions
ARM_VFP_SP_COP              EQU     10
ARM_VFP_SP_ACCESS           EQU     (ARM_CACR_FULL << (ARM_VFP_SP_COP*2))

; VFP uses coproc 11 for double-precision instructions
ARM_VFP_DP_COP              EQU     11
ARM_VFP_DP_ACCESS           EQU     (ARM_CACR_FULL << (ARM_VFP_DP_COP*2))

; Configure coprocessor access control
ARM_CACR_CONFIG             EQU     (ARM_VFP_SP_ACCESS | ARM_VFP_DP_ACCESS)

    IF :LNOT: :DEF: BOOTLOADER
        GBLL        BOOTLOADER
BOOTLOADER      SETL    {FALSE}
    ENDIF
    
    OPT 2                                       ; disable listing
    INCLUDE kxarm.h
    OPT 1                                       ; reenable listing

    IF :LNOT: BOOTLOADER
        IMPORT  KernelStart
        ;IMPORT  OALInitCpuHclkClock
        ;IMPORT  OALInitIOClock        
    ENDIF
    
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
    mrs     r0, cpsr                        ; Get current mode bits.
    bic     r0, r0, #0x1F                   ; Clear mode bits.
    orr     r0, r0, #0xD3                   ; Disable IRQs/FIQs, SVC mode
    msr     cpsr_c, r0                      ; Enter supervisor mode
    
    ;---------------------------------------------------------------
    ; Initialize cache & CP15 control register
    ;---------------------------------------------------------------
    ; Even if in most cases caches are disabled at this moment
    ; first clean both data & instruction cache, then disable
    ; I-cache. We also clear system and rom protection bits
    ; as they are enabled after reset/ROM code.
    ; Clean & invalidate D cache
    
10  mrc     p15, 0, r15, c7, c14, 3
    bne     %b10

    ;Flush TLB and chches
    mov     r0, #0
    mcr     p15, 0, r0, c8, c5, 0   ; flush instruction TLB    
    
    mov     r0, #0
    mcr     p15, 0, r0, c8, c6, 0   ; flush data TLB   
    
    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 0   ; Flush instruction cache 

    ; Set CP15 control bits register
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #(1 :SHL: 12)           ; I-cache
    bic     r0, r0, #(1 :SHL: 9)            ; ROM protection
    bic     r0, r0, #(1 :SHL: 8)            ; system protection
    bic     r0, r0, #(1 :SHL: 3)            ; write buffer
    bic     r0, r0, #(1 :SHL: 2)            ; D-cache
    orr     r0, r0, #(1 :SHL: 1)            ; alignment fault
    bic     r0, r0, #(1 :SHL: 0)            ; MMU Off
    mcr     p15, 0, r0, c1, c0, 0
    
   ; IF :LNOT: :DEF: BOOTLOADER        
   ; bl  OALInitCpuHclkClock
   ; bl  OALInitIOClock
   ; ENDIF        
    ;---------------------------------------------------------------
    ; Jump to WinCE KernelStart
    ;---------------------------------------------------------------
    ; Compute the OEMAddressTable's physical address and 
    ; load it into r0. KernelStart expects r0 to contain
    ; the physical address of this table. The MMU isn't 
    ; turned on until well into KernelStart.  
    ;
    
    add     r0, pc, #g_oalAddressTable - (. + 8)
    b       KernelStart
    nop
    nop
    nop
    nop
    nop
    nop
    
STALL
    b       STALL               ; Spin forever.
    
    ;
    ; Include memory configuration file with g_oalAddressTable
    ;
    INCLUDE oemaddrtab_cfg.inc

;-------------------------------------------------------------------------------

    END 

