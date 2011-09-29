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
;
; Copyright 2002-2003 Intel Corporation All Rights Reserved.
;**
;** Portions of the source code contained or described herein and all documents
;** related to such source code (Material) are owned by Intel Corporation
;** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
;** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
;** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
;** No other license under any patent, copyright, trade secret or other intellectual
;** property right is granted to or conferred upon you by disclosure or
;** delivery of the Materials, either expressly, by implication, inducement,
;** estoppel or otherwise 
;** Some portion of the Materials may be copyrighted by Microsoft Corporation.
;
;
;*********************************************************************************
;
;  FILENAME:       xlli_LowLev_Init.s
;
;  PURPOSE:        Provides low Level init procedures written specifically for
;                  the Bulverde/Mainstone platform.
;
;
;  LAST MODIFIED:  25-Feb-2005
;******************************************************************************
;
; The functions in this source code are called via a branch with link instruction.
; Unless otherwise specified, no system stack is assumed and no registers are preserved.
;
; NOTES:
;
; The use of these subroutines and/or the order in which they are called is, for the most part,
; arbitrary and is left up to the user. Only a few subroutines must be called in a specific order.
; For example, it makes no sense to go to virtual mode before the MMU is initialized and
; the page table is set up.... and, in order to set up the page table, memory needs to be initialized
; first.
;
        INCLUDE  xlli_Bulverde_defs.inc                 ; Bulverde specific include file
        INCLUDE  xlli_Mainstone_defs.inc                ; Mainstone specific include file
;
        AREA    |text|, CODE, READONLY, ALIGN=5         ; Align =5 required for "ALIGN 32" feature to work.
;
; List of Low Level Init functions in this source code include:
;
        EXPORT xlli_read_SCR              ; Reads the SCR and LCDR virtural registers and places in SRAM
        EXPORT xlli_GPIO_init             ; Get SCR and LCDCR data and Initialize the GPIO ports
        EXPORT xlli_mem_init              ; Memory controller initialization
        EXPORT xlli_mem_restart           ; Restart memory controller
        EXPORT xlli_mem_Tmax              ; Sets maximum memory configuration values
        EXPORT xlli_mem_Topt              ; Sets optimal memory configuration values based on MemClk frequency
        EXPORT xlli_intr_init             ; Interrupt initialization (masks all interrupts)
        EXPORT xlli_freq_change           ; Frequency change sequence
        EXPORT xlli_clks_init             ; Initialize system clocks
        EXPORT xlli_clks_init_sleepReset  ; Initialize system clock after a sleep reset
        EXPORT xlli_ost_init              ; Initalize Operating System Timers
        EXPORT xlli_icache_enable         ; Enable I-Cache, D-Cache, and Branch Target Buffer
        EXPORT xlli_pwrmgr_init           ; Initialize the Power Manager
        EXPORT xlli_setPTB                ; Set the Page Tabe Base address (used for sleep resets)
        EXPORT xlli_initPageTable         ; Initialize Page Table for MMU unit
        EXPORT xlli_MMU_init              ; Initialize the Memory Management Unit
        EXPORT xlli_goVirtual             ; Make the transition from physical address to virtual address
        EXPORT xlli_IMpwr_init            ; Initialize Internal Memory for use
        EXPORT GetSCR                     ; Returns System Configuration Register data to 'C' program caller
        EXPORT xlli_setClocks             ; Reads platform switchs and sets Bulverde frequencies
        EXPORT xlli_getFreq               ; Returns current system clock settings to 'C' program caller
        EXPORT xlli_setBufImpedance       ; Sets SCRAM buffer impedance (C callable)
        EXPORT xlli_setBufImp             ; Sets SCRAM buffer impedance (ROM callable)


;******************************************************************************
;
;
;       *****************
;       *               *
;       * xlli_read_SCR *
;       *               *
;       *****************
;
; This subroutine will reconfigure the GPIO pins in order to read the SCR (system
; Configuration Register) and the LCDCR (LCD Configuration Register) and place this
; information into SRAM for use by other programs.
;
; Once these virtural register have been read, the GPIOs are reconfigured to their
; previous state.
;
; NOTES: Written for the Bulverde Processor on the Mainstone Development Platform.
;
; This subroutine uses and DOES NOT PRESERVE registers r1, r2, r3, r4, r5, r6, r7, r8 and r9
;

xlli_read_SCR   FUNCTION
;
;       Insure the RDH and PH bits on Bulverde must be clear to enable GPIO pins.
;       They are sticky bits so they must be set to clear them.
;
        ldr     r4, =xlli_PMRCREGS_PHYSICAL_BASE
        mov     r2, #(xlli_PSSR_PH | xlli_PSSR_RDH) ; Set the PH and RDH bits to enable all GPIOs
        str     r2, [r4, #xlli_PSSR_offset]         ; Enable all GPIO lines
;
;       Get, and save the present GPIO settings for direction registers 0, 1 and 2
;
        ldr     r4,  =xlli_GPIOREGS_PHYSICAL_BASE  ; Get the GPIO registers base address
        ldr     r5,  [r4, #xlli_GPDR1_offset]      ; Save direction values for GPIOs [63:32]
        ldr     r6,  [r4, #xlli_GPDR2_offset]      ; Save direction values for GPIOs [95:64]
        ldr     r7,  [r4, #xlli_GAFR1_U_offset]    ; Save alt function values for GPIOs [63:48]
        ldr     r8,  [r4, #xlli_GAFR2_L_offset]    ; Save alt function values for GPIOs [79:64]
        ldr     r9,  [r4, #xlli_GAFR2_U_offset]    ; Save alt function values for GPIOs [95:80]

;
;       Clear the bits of those GPIOs we need to read as inputs and write back to the GPIO
;       direction registers. The alternate function bits also need to be cleared. For this
;       particular code sequence "Magic Numbers" are used for the bit clear masks.
;
;       To cut down on the number of registers used we'll do GPDR1 and GPDR2 now and take care of
;       GPDR0 (and it's alternate function bits) later. Because this code is usually run very early
;       in the boot sequence (before SDRAM is brought up) the code assumes there is no stack to
;       preserve the contents of the registers used.
;
        ldr     r2,  =0xFC000000                   ; Register clear mask for GPDR1
        bic     r1,  r5,  r2                       ; Clear the bits we need to read in GPDR1
        str     r1,  [r4, #xlli_GPDR1_offset]      ; Write direction values for GPIOs [63:32]

        ldr     r2,  =0x00C02FFF                   ; Register clear mask for GPDR2
        bic     r1,  r6,  r2                       ; Clear the bits we need to read in GPDR2
        str     r1,  [r4, #xlli_GPDR2_offset]      ; Write direction values for GPIOs [95:64]
;
;       Clear the alternate function bits for the direction bits we need to look at.
;
        ldr     r2,  =0xFFF00000                   ; Register clear mask for GAFR1_U
        bic     r1,  r7,  r2                       ; Clear the alt func for bits we need to read in GAFR1_U
        str     r1,  [r4, #xlli_GAFR1_U_offset]    ; Write alt function values for GPIOs [63:48]

        ldr     r2,  =0x0CFFFFFF                   ; Register clear mask for GAFR2_L
        bic     r1,  r8,  r2                       ; Clear the alt func for bits we need to read in GAFR2_L
        str     r1,  [r4, #xlli_GAFR2_L_offset]    ; Write alt function values for GPIOs [79:64]

        ldr     r2,  =0x0000F000                   ; Register clear mask for GAFR2_U
        bic     r1,  r9,  r2                       ; Clear the alt func for bits we need to read in GAFR2_U
        str     r1,  [r4, #xlli_GAFR2_U_offset]    ; Write alt function values for GPIOs [95:80]

        mov     r1,  #0x200
xlli_1  subs    r1,  r1,  #1                       ; Short delay required to allow the new GPIO...
        bne     xlli_1                             ; ...settings to take effect
;
;       With the GPIO direction registers set, the Platform System Configuration Register is read
;       from GPIO bits 73:58 and are stored in SRAM.
;       This data is placed in the UPPER 16 BITS (31:16) of this register.
;
        ldr     r1,  [r4, #xlli_GPLR1_offset]      ; Get levels for GPIOs [63:32]
        mov     r1,  r1,  LSR #10                  ; Move the data 10 bits to the right
        ldr     r2,  [r4, #xlli_GPLR2_offset]      ; Get levels for GPIOs [95:64]
        mov     r3,  r2                            ; Save a copy for future reference
        mov     r2,  r2,  LSL #22                  ; Move the data 22 bits to the left
        orr     r1,  r1,  r2                       ; This places GPIO data bits [73:58] into bits [31:16]
        ldr     r2,  =0xFFFF0000                   ; Mask word
        and     r1,  r1,  r2                       ; Make sure the bottom 16 bits are clear
;
;       Restore GPDR1, GPDR2, GAFR1_U, GAFR2_L and GAFR2_U to their original values
;
        str     r5,  [r4, #xlli_GPDR1_offset]      ; Restore direction values for GPIOs [63:32]
        str     r6,  [r4, #xlli_GPDR2_offset]      ; Restore direction values for GPIOs [95:64]
        str     r7,  [r4, #xlli_GAFR1_U_offset]    ; Restore alt function values for GPIOs [63:48]
        str     r8,  [r4, #xlli_GAFR2_L_offset]    ; Restore alt function values for GPIOs [79:64]
        str     r9,  [r4, #xlli_GAFR2_U_offset]    ; Restore alt function values for GPIOs [95:80]
;
;       Configure GPDR0, GAFR0_L and GAFR0_U to pick up four status bits we need from this word
;       NOTE: The values in r1 and r4 must be preserved through this section of code.
;
        ldr     r5,  [r4, #xlli_GPDR0_offset]      ; Save direction values for GPIOs [31:0]
        ldr     r6,  [r4, #xlli_GAFR0_L_offset]    ; Save alt function values for GPIOs [15:0]
        ldr     r7,  [r4, #xlli_GAFR0_U_offset]    ; Save alt function values for GPIOs [31:16]

        ldr     r2,  =0xC0084000                   ; Register clear mask for GPDR0
        bic     r8,  r5,  r2                       ; Clear the bits we need to read in GPDR0
        str     r8,  [r4, #xlli_GPDR0_offset]      ; Write direction values for GPIOs [31:0]
;
;       Clear the alternate function bits for the direction bits we need to look at.
;
        ldr     r2,  =0x30000000                   ; Register clear mask for GAFR0_L
        bic     r8,  r6,  r2                       ; Clear the alt func for bits we need to read in GAFR0_L
        str     r8,  [r4, #xlli_GAFR0_L_offset]    ; Write alt function values for GPIOs [15:0]

        ldr     r2,  =0xF00000C0                   ; Register clear mask for GAFR0_U
        bic     r8,  r7,  r2                       ; Clear the alt func for bits we need to read in GAFR0_U
        str     r8,  [r4, #xlli_GAFR0_U_offset]    ; Write alt function values for GPIOs [31:16]

        mov     r2,  #0x200
xlli_2  subs    r2,  r2,  #1                       ; Short delay required to allow the new GPIO...
        bne     xlli_2                             ; ...settings to take effect
;
;       The data that makes up the LCDCR registar (GPIO bits 87, 86, 77, 75, 74, 19, and 14) are
;       stored in bits 6:0 of the PSPR. The xlli code does not use this data - it is only placed
;       into the PSPR to make the data accessable to other users who can then take the data and
;       store it elsewhere or simply ignore it and write over it.
;
;       The SCR data is in the upper 16 bits of r1. Now the data for the LCDCR register needs
;       to be assembled from data saved in r3 in addition to two bits to be read from GPIOs 31:0
;       and this data needs to be ORed into r1 as well.
;
        and     r2,  r3,  #0x2000                  ; isolate bit 77
        mov     r2,  r2,  LSR #9                                   ; Move to bit position #4

        and     r8,  r3,  #0xC00                   ; Extract bits 75:74
        mov     r8,  r8,  LSR #8                   ; Move GPIO bits 75:74 to bit position 3:2
        orr     r2,  r2,  r8                       ; Save these two bits into r2

        and     r8,  r3,  #0xC00000                ; Extract bits 87:86
        mov     r8,  r8,  LSR #17                  ; Move GPIO bits 87:86 to bit position 6:5
        orr     r2,  r2,  r8                       ; Save these two bits into r2

        ldr     r3,  [r4, #xlli_GPLR0_offset]      ; Get levels for GPIOs [31:0]
        mov     r3,  r3,  LSR #14                  ; Allign GPIO bit 14 with bit position 0
        and     r8,  r3,  #0x1                     ; Extract this bit
        orr     r2,  r2,  r8                       ; OR this bit into r2

        mov     r3,  r3,  LSR #4                   ; move GPIO bit 19 into bit location 1
        and     r8,  r3,  #0x2                     ; extract this bit
        orr     r2,  r2,  r8                       ; OR this bit value into r2

        mov     r3,  r3,  LSR #5                   ; move GPIO bits 31:30 into bit locations 8:7
        and     r3,  r3,  #0x180                   ; isolate these bits
        orr     r2,  r2,  r3                       ; OR this bit value into r2
;
;       Restore GPDR0, GAFR0_L and GAFR0_U to their original values
;
        str     r5,  [r4, #xlli_GPDR0_offset]      ; Restore direction values for GPIOs [31:0]
        str     r6,  [r4, #xlli_GAFR0_L_offset]    ; Restore alt function values for GPIOs [15:0]
        str     r7,  [r4, #xlli_GAFR0_U_offset]    ; Restore alt function values for GPIOs [31:16]
;
;       Now we need to read status from GPIOs 103, 104 and 113
;
;       Configure GPDR3, GAFR3_L and GAFR3_U to pick up three status bits we need from this word
;       NOTE: The values in r1 and r4 must be preserved through this section of code.
;
        ldr     r5,  [r4, #xlli_GPDR3_offset]      ; Save direction values for GPIOs [127:96]
        ldr     r6,  [r4, #xlli_GAFR3_L_offset]    ; Save alt function values for GPIOs [111:96]
        ldr     r7,  [r4, #xlli_GAFR3_U_offset]    ; Save alt function values for GPIOs [127:112]

        ldr     r9,  =0x00020180                   ; Register clear mask for GPDR3
        bic     r8,  r5,  r9                       ; Clear the bits we need to read in GPDR3
        str     r8,  [r4, #xlli_GPDR3_offset]      ; Write direction values for GPIOs [127:96]
;
;       Clear the alternate function bits for the direction bits we need to look at.
;
        ldr     r9,  =0x0003C000                   ; Register clear mask for GAFR3_L
        bic     r8,  r6,  r9                       ; Clear the alt func for bits we need to read in GAFR3_L
        str     r8,  [r4, #xlli_GAFR3_L_offset]    ; Write alt function values for GPIOs [111:96]

        ldr     r9,  =0x0000000C                   ; Register clear mask for GAFR3_U
        bic     r8,  r7,  r9                       ; Clear the alt func for bits we need to read in GAFR3_U
        str     r8,  [r4, #xlli_GAFR3_U_offset]    ; Write alt function values for GPIOs [127:112]

        mov     r9,  #0x200
xlli_15 subs    r9,  r9,  #1                       ; Short delay required to allow the new GPIO...
        bne     xlli_15                            ; ...settings to take effect

        ldr     r3,  [r4, #xlli_GPLR3_offset]      ; Get levels for GPIOs [127:96]
;
;       Restore GPDR3, GAFR3_L and GAFR3_U to their original values
;
        str     r5,  [r4, #xlli_GPDR3_offset]      ; Restore direction values for GPIOs [127:96]
        str     r6,  [r4, #xlli_GAFR3_L_offset]    ; Restore alt function values for GPIOs [111:96]
        str     r7,  [r4, #xlli_GAFR3_U_offset]    ; Restore alt function values for GPIOs [127:112]

        mov     r3,  r3,  LSL #2                   ; Move GPIO bits 103:104 to bit locations 11:10
        and     r8,  r3,  #0xC00                   ; extract settings for bits 11:10
        orr     r2,  r2,  r8                       ; Add in to value for system config word

        mov     r3,  r3,  LSR #10                  ; Move GPIO bits 113 to bit location 9
        and     r8,  r3,  #0x400                   ; extract setting for bit 9
        orr     r2,  r2,  r8                       ; Add in to value for system config word
;
;       Read back from APB to ensure our writes have completed before continuing
;
        ldr     r8, [r4, #xlli_GAFR3_U_offset]

;
;       r2 now contains the SCR2 register data in bits 11:0
;       This value gets ORed with the SCR data in bits 31:16 of r1
;
        orr     r1,  r1,  r2                       ; Generate the data word to be stored
;
;       Update SRAM with the above data
;
        ldr     r4,  =xlli_SCR_data                ; Get SRAM address where data is to be stored
        str     r1,  [r4]                          ; Write the contents to SRAM

        mov     pc, lr                             ; Return to calling program

        ENDFUNC

        LTORG

;******************************************************************************
;
;
;       ******************
;       *                *
;       * xlli_GPIO_init * Subroutine
;       *                *
;       ******************
;
; This subroutine sets up the GPIO pins in accordance with the values contained in the platform include file.
;
; NOTES: Written for the Bulverde Processor on the Mainstone Development Platform.
;

xlli_GPIO_init   FUNCTION
;
;
;
;
;       Get GPIO registers base address and configure all GPIO lines.
;
        ldr     r0,  =xlli_GPIOREGS_PHYSICAL_BASE   ; Load the GPIO register block base address

        ldr     r1,  =xlli_GPSR0_value              ; Get the pin set values for GPSR0
        str     r1,  [r0, #xlli_GPSR0_offset]       ; Write the R0 values

        ldr     r2,  =xlli_GPSR1_value              ; Get the pin set values for GPSR1
        str     r2,  [r0, #xlli_GPSR1_offset]       ; Write the R1 values

        ldr     r1,  =xlli_GPSR2_value              ; Get the pin set values for GPSR2
        str     r1,  [r0, #xlli_GPSR2_offset]       ; Write the R2 values

        ldr     r2,  =xlli_GPSR3_value              ; Get the pin set values for GPSR3
        str     r2,  [r0, #xlli_GPSR3_offset]       ; Write the R3 values

        ldr     r1,  =xlli_GPCR0_value              ; Get the pin clear values for GPCR0
        str     r1,  [r0, #xlli_GPCR0_offset]       ; Write the R0 values

        ldr     r2,  =xlli_GPCR1_value              ; Get the pin clear values for GPCR1
        str     r2,  [r0, #xlli_GPCR1_offset]       ; Write the R1 values

        ldr     r1,  =xlli_GPCR2_value              ; Get the pin clear values for GPCR2
        str     r1,  [r0, #xlli_GPCR2_offset]       ; Write the R2 values

        ldr     r2,  =xlli_GPCR3_value              ; Get the pin clear values for GPCR3
        str     r2,  [r0, #xlli_GPCR3_offset]       ; Write the R3 values

        ldr     r1,  =xlli_GPDR0_value              ; Get the pin direction values for GPDR0
        str     r1,  [r0, #xlli_GPDR0_offset]       ; Write the R0 values

        ldr     r2,  =xlli_GPDR1_value              ; Get the pin direction values for GPDR1
        str     r2,  [r0, #xlli_GPDR1_offset]       ; Write the R1 values

        ldr     r1,  =xlli_GPDR2_value              ; Get the pin direction values for GPDR2
        str     r1,  [r0, #xlli_GPDR2_offset]       ; Write the R2 values

        ldr     r2,  =xlli_GPDR3_value              ; Get the pin direction values for GPDR3
        str     r2,  [r0, #xlli_GPDR3_offset]       ; Write the R3 values

        ldr     r1,  =xlli_GAFR0_L_value            ; Get the pin alt function values for GAFR0_L
        str     r1,  [r0, #xlli_GAFR0_L_offset]     ; Write the R0_L values

        ldr     r2,  =xlli_GAFR0_U_value            ; Get the pin alt function values for GAFR0_U
        str     r2,  [r0, #xlli_GAFR0_U_offset]     ; Write the R0_U values

        ldr     r1,  =xlli_GAFR1_L_value            ; Get the pin alt function values for GAFR1_L
        str     r1,  [r0, #xlli_GAFR1_L_offset]     ; Write the R1_L values

        ldr     r2,  =xlli_GAFR1_U_value            ; Get the pin alt function values for GAFR1_U
        str     r2,  [r0, #xlli_GAFR1_U_offset]     ; Write the R1_U values

        ldr     r1,  =xlli_GAFR2_L_value            ; Get the pin alt function values for GAFR2_L
        str     r1,  [r0, #xlli_GAFR2_L_offset]     ; Write the R2_L values

        ldr     r2,  =xlli_GAFR2_U_value            ; Get the pin alt function values for GAFR2_U
        str     r2,  [r0, #xlli_GAFR2_U_offset]     ; Write the R2_U values

        ldr     r1,  =xlli_GAFR3_L_value            ; Get the pin alt function values for GAFR3_L
        str     r1,  [r0, #xlli_GAFR3_L_offset]     ; Write the R3_L values

        ldr     r2,  =xlli_GAFR3_U_value            ; Get the pin alt function values for GAFR3_U
        str     r2,  [r0, #xlli_GAFR3_U_offset]     ; Write the R3_U values
;
;       The RDH and PH bits on Bulverde must be set to enable updated GPIO pins.
;       These are sticky bits.
;
        ldr     r0, =xlli_PMRCREGS_PHYSICAL_BASE
        mov     r2, #(xlli_PSSR_PH | xlli_PSSR_RDH) ; Set the PH and RDH bits to enable all GPIOs
        str     r2, [r0, #xlli_PSSR_offset]         ; Enable all GPIO lines
;
;       Read back from APB to ensure our writes have completed before continuing
;
        ldr     r2, [r0, #xlli_PSSR_offset]

        mov     pc, lr                              ; Return to calling program

        ENDFUNC

        LTORG

;**************************************************************************************************
;
; **************************************************
; **********                              **********
; ********** INITIALIZE MEMORY CONTROLLER **********
; **********                              **********
; **************************************************
;
; The sequence below is based on the recommended memory initializing steps detailed
; in the Bulverde EAS, Volume I (Section 13.17, page 13-72)
;

xlli_mem_init   FUNCTION

;
; ***** STEP 1: *****
;
; Delay 200 uS
;
        ldr     r2,  =xlli_OSTREGS_PHYSICAL_BASE ; Load OS timer base address
        ldr     r3,  [r2, #xlli_OSCR0_offset]    ; Fetch starting value of OSCR0
        add     r3,  r3,  #0x300                 ; Really 0x2E1 is about 200usec, so 0x300 should be plenty
xlli_3  ldr     r1,  [r2, #xlli_OSCR0_offset]    ; Fetch current OSCR0 value
        cmp     r1,  r3                          ; Is the timer past the time out value?
        bmi     xlli_3                           ; No - Loop until it is
;
;  STEP 1 - 1st bullet: Write MSC0, MSC1 and MSC2 (the order is not important)
;  *******************
;
;       The value that is loaded for MSC0 depends on which FLASH memory was used to
;       boot from as determined by the switch position of SW2 (SWAP FLASH) on the
;       Mainstone board. To get the position of this switch, GPIO line #73 must be sensed.
;
        ldr     r4,  =xlli_GPIOREGS_PHYSICAL_BASE   ; Load the GPIO register block base address

        ldr     r2,  [r4, #xlli_GPDR2_offset]       ; Read GPDR2 (direction register) data
        ldr     r5,  [r4, #xlli_GAFR2_L_offset]     ; Read alt function register for GPIO #73
        mov     r1,  r2                             ; Copy direction register data to r1
        mov     r3,  r5                             ; Copy alt function register to r3
        bic     r1,  r1,  #0x200                    ; Clear direction bit for GPIO #73
        bic     r3,  r3,  #0xC0000                  ; Clear alt function bits for GPIO #73
        str     r3,  [r4, #xlli_GAFR2_L_offset]     ; Write alt function register
        str     r1,  [r4, #xlli_GPDR2_offset]       ; Write GPDR2 (direction register) data
;
;       It can take many cycles for the GPIO setting to take effect before the read can be
;       issued. The delay is based on clock cycles rather than elaped time so a simple
;       do-nothing loop will do the trick.
;
        mov     r1,  #0x600                         ; Init counter
xlli_4  subs    r1,  r1,  #1                        ; Decrement counter
        bne     xlli_4                              ; Loop until zero

        ldr     r1,  [r4, #xlli_GPLR2_offset]       ; Read GPLR2 (level register) data
        ands    r1,  r1,  #0x200                    ; Test status of GPIO #73
        ldreq   r1,  =xlli_MSC0_DC_value            ; Get MSC0 setting for daughter card Flash
        ldrne   r1,  =xlli_MSC0_MS_value            ; Get MSC0 setting for Mainstone Flash

        str     r5,  [r4, #xlli_GAFR2_L_offset]     ; Write back original alt function register
        str     r2,  [r4, #xlli_GPDR2_offset]       ; Write back original GPDR2 data
;
;       Read back from APB to ensure our writes have completed before continuing
;       (spanning a bridge here)
;
        ldr     r2, [r4, #xlli_GPDR2_offset]

;
;       Finally - Write the memory control registers
;
        ldr     r4,  =xlli_MEMORY_CONFIG_BASE   ; Get memory controller base address

        str     r1,  [r4, #xlli_MSC0_offset]    ; Write the value out
        ldr     r1,  [r4, #xlli_MSC0_offset]    ; Read back to latch the data

        ldr     r2,  =xlli_MSC1_value           ; Get MSC1 setting
        str     r2,  [r4, #xlli_MSC1_offset]    ; Write the value out
        ldr     r2,  [r4, #xlli_MSC1_offset]    ; Read back to latch the data

        ldr     r1,  =xlli_MSC2_value           ; Get MSC2 setting
        str     r1,  [r4, #xlli_MSC2_offset]    ; Write the value out
        ldr     r1,  [r4, #xlli_MSC2_offset]    ; Read back to latch the data
;
;  STEP 1 - 2nd bullet: Write MECR, MCMEM0, MCMEM1, MCATT0, MCATT1, MCIO0, MCIO1 (order not important)
;  *******************
;
        ldr     r2,  =xlli_MECR_value           ; write MECR
        str     r2,  [r4, #xlli_MECR_offset]

        ldr     r1,  =xlli_MCMEM0_value         ; write MCMEM0
        str     r1,  [r4, #xlli_MCMEM0_offset]

        ldr     r2,  =xlli_MCMEM1_value         ; write MCMEM1
        str     r2,  [r4, #xlli_MCMEM1_offset]

        ldr     r1,  =xlli_MCATT0_value         ; write MCATT0
        str     r1,  [r4, #xlli_MCATT0_offset]

        ldr     r2,  =xlli_MCATT1_value         ; write MCATT1
        str     r2,  [r4, #xlli_MCATT1_offset]

        ldr     r1,  =xlli_MCIO0_value          ; write MCIO0
        str     r1,  [r4, #xlli_MCIO0_offset]

        ldr     r2,  =xlli_MCIO1_value          ; write MCIO1
        str     r2,  [r4, #xlli_MCIO1_offset]
;
;  STEP 1 - 3rd bullet: Write FLYCNFG
;  *******************
;
        ldr     r1,  =xlli_FLYCNFG_value        ; write FLYCNFG
        str     r1,  [r4, #xlli_FLYCNFG_offset]
;

;
;  STEP 1 - 4th bullet: SKIPPED (used only when coming out of sleep)
;  *******************
;
;     (If required, this would be a write to MDCNFG with enable bits deasserted.)

;
;  STEP 1 - 5th bullet: update MDREFR settings
;  *******************
;
        ldr         r2,  [r4, #xlli_MDREFR_offset] ; Get reset state of MDREFR
        mov         r2,  r2, lsr #0xC              ; Shift data 12 bits (0xC) to the right
        mov         r2,  r2, lsl #0xC              ; and shift left 12 bits (Clears DRI field)
                                                   ; because left shifts fill bits with zeros.

        ldr     r1,  =xlli_MDREFR_value            ; Fetch MDREFR value for this platform
        mov         r1,  r1, lsl #0x14             ; use shifts to extract the DRI field using
        mov         r1,  r1, lsr #0x14             ; same method as before but shifting 20 (0x14)
                                                   ; bits left, then right again.

        orr     r2,  r2, r1                    ; insert the DRI field extracted above
        str     r2,  [r4, #xlli_MDREFR_offset] ; write value with valid DRI to MDREFR

        orr     r2,  r2, #xlli_MDREFR_K0RUN    ; Enable K0RUN

        bic     r2,  r2, #xlli_MDREFR_K0DB2    ; Configure K0DB2

        orr     r2,  r2, #xlli_MDREFR_K0DB4    ; Set K0DB4 = MemClk/4

        bic     r2,  r2, #(xlli_MDREFR_K1FREE :OR: xlli_MDREFR_K2FREE)  ; Clear free run clock bits

        orr     r2,  r2, #xlli_MDREFR_K0FREE   ; Set K0FREE (as per Mainstone spec...)

        str     r2,  [r4, #xlli_MDREFR_offset] ; Write back MDREFR

        ;
        ; Preserve MDREFR in r2
        ;

; ***** STEP 2 *****
;
; For systems with Synchronous Flash
;
        ldr     r1,  =xlli_SXCNFG_value
        str     r1,  [r4, #xlli_SXCNFG_offset]

;
; ***** STEP 3 *****
;
; Clear the free run clock bits to enable the use of SDCLK for memory timing
;
        bic     r2, r2, #(xlli_MDREFR_K2FREE :OR: xlli_MDREFR_K1FREE :OR: xlli_MDREFR_K0FREE)

;
; set K1RUN if bank 0 installed
;
        orr     r2, r2, #xlli_MDREFR_K1RUN      ; Enable SDCLK[1]
                orr     r2, r2, #xlli_MDREFR_K1DB2      ; Set K1DB2 to halve Memclk
        bic     r2, r2, #xlli_MDREFR_K2DB2      ; Clear K2DB2
        str     r2, [r4, #xlli_MDREFR_offset]   ; write updated MDREFR
        ldr     r2, [r4, #xlli_MDREFR_offset]   ; read back

        bic     r2, r2, #xlli_MDREFR_SLFRSH     ; Disable self refresh
        str     r2, [r4, #xlli_MDREFR_offset]   ; write updated MDREFR
        ldr     r2, [r4, #xlli_MDREFR_offset]   ; read back

        orr     r2, r2, #xlli_MDREFR_E1PIN      ; Assert E1PIN to enable SDCKE[1]
        str     r2, [r4, #xlli_MDREFR_offset]   ; write updated MDREFR (finished with value in r2)
        ldr     r2, [r4, #xlli_MDREFR_offset]   ; read back

        nop                                     ; Do not remove!
        nop                                     ; Do not remove!
;
; ***** STEP 4 *****
;
; Appropriately configure, but don't enable, each SDRAM partition pair
;
        ldr     r1, =xlli_MDCNFG_value          ; Fetch platform value for MDCNFG

        bic     r1, r1,  #(xlli_MDCNFG_DE0 :OR: xlli_MDCNFG_DE1)   ; Disable all
        bic     r1, r1,  #(xlli_MDCNFG_DE2 :OR: xlli_MDCNFG_DE3)   ; SDRAM banks
;
;       Check for conditional def for 32 vs 16 bit bus width
;
        IF :DEF: xlli_SDRAM_16BIT               ; Set bus width to 16 bits?
        orr     r1, r1,  #xlli_MDCNFG_DWID0     ; Set banks 0/1 for 16 bit width
        ELSE
        bic     r1, r1,  #xlli_MDCNFG_DWID0     ; Set banks 0/1 for 32 bit width
        ENDIF

        orr     r1,     r1,     #xlli_BIT_11    ; This reserved bit should always be set
        orr     r1,     r1,     #xlli_BIT_27    ; This reserved bit should always be set
        str     r1, [r4, #xlli_MDCNFG_offset]   ; Write w/o enabling SDRAM banks

;
; ***** STEP 5 *****  (Delay at least 200 uS)
;
        ldr     r2,  =xlli_OSTREGS_PHYSICAL_BASE ; Load OS timer base address
        ldr     r3,  [r2, #xlli_OSCR0_offset]    ; Fetch starting value of OSCR0
        add     r3,  r3,  #0x300                 ; Really 0x2E1 is about 200usec, so 0x300 should be plenty
xlli_5  ldr     r1,  [r2, #xlli_OSCR0_offset]    ; Fetch current OSCR0 value
        cmp     r1,  r3                          ; Is the timer past the time out value?
        bmi     xlli_5                           ; No - Loop until it is
;
; ***** STEP 6 ***** (Make sure DCACHE is disabled)
;
        mrc     p15, 0, r2, c1, c0, 0           ; load r2 contents of register 1 in CP 15
        bic     r2,  r2,  #xlli_CONTROL_DCACHE  ; Disable D-Cache
        mcr     p15, 0, r2, c1, c0, 0           ; Write back to CP15
;
; ***** STEP 7 *****
;
; Access memory *not yet enabled* for CBR refresh cycles (8)
; - CBR is generated for all banks
;
        ldr     r1, =xlli_SDRAM_PHYSICAL_BASE
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]

        str     r1, [r1]  ;  Fix for erratum #116. Makes up for ineffective 1st mem access.
                          ;  This is being left in for Bulverde for the moment
;
; ***** STEP 8 *****
;
;  Re-enable D-cache if desired (we don't)

;
; ***** STEP 9 *****
;
; Re-enable SDRAM partitions
;
        ldr     r2,  [r4, #xlli_MDCNFG_offset]   ; Fetch the current MDCNFG value

        IF :DEF: SDRAM_SIZE_64_MB               
        orr     r2,  r2,  #xlli_MDCNFG_DE0                            ; Enable SDRAM bank 0
        ELSE
        orr     r2,  r2,  #(xlli_MDCNFG_DE0 :OR: xlli_MDCNFG_DE1)     ; Enable SDRAM bank 0 & 1
        ENDIF
        
        str     r2,  [r4, #xlli_MDCNFG_offset]   ; Write back MDCNFG, enabling the SDRAM bank(s)
;
; ***** STEP 10 *****
;
; Write the MDMRS register to trigger an MRS command to all enabled banks of SDRAM.
;
;
        ldr     r1,  =xlli_MDMRS_value           ; Fetch platform MDMRS value
        str     r1,  [r4, #xlli_MDMRS_offset]    ; Write the MDMRS value back
;
; ***** STEP 11 *****
;
; In systems with SDRAM or Synchronous Flash, optionally enable auto-power-down by setting MDREFR:APD
;
        ldr     r3,  [r4, #xlli_MDREFR_offset]   ; Get MDREFR value
        orr     r3,  r3,  #xlli_MDREFR_APD       ; enable auto power down
        str     r3,  [r4, #xlli_MDREFR_offset]   ; Write value back

        mov     pc,  lr                         ; return to calling routine

        ENDFUNC

        LTORG

;**************************************************************************************************
;
; ***********************************************
; **********                           **********
; ********** RESTART MEMORY CONTROLLER **********
; **********                           **********
; ***********************************************
;
; This command restarts the memory controller and should be called after a frequency change sequence
; or when memory controller settings have been changed (such as xlli_mem_Tmax and xlli_memTopt).
;

xlli_mem_restart   FUNCTION

        ldr     r4,  =xlli_MEMORY_CONFIG_BASE    ; Get memory controller base address
        ldr     r2,  [r4, #xlli_MDREFR_offset]   ; Get MDREFR value
        bic     r3,  r2, #xlli_MDREFR_E0PIN      ; Clear E0PIN to disable SDCKE[0]
        bic     r3,  r3, #xlli_MDREFR_E1PIN      ; Clear E1PIN to disable SDCKE[1]
        b       xlli_5A
;
;       The next line should start on a cache line boundary so we don't accidently hang the system
;
        ALIGN   32
xlli_5A str     r3,  [r4, #xlli_MDREFR_offset]   ; Write value back with E0PIN and E1PIN cleared
        str     r2,  [r4, #xlli_MDREFR_offset]   ; Write value back with E0PIN and E1PIN set
;
; Disable all SDRAM banks
;
        ldr     r1, [r4, #xlli_MDCNFG_offset]    ; Fetch platform value for MDCNFG
        bic     r1, r1,  #(xlli_MDCNFG_DE0 :OR: xlli_MDCNFG_DE1)   ; Disable all
        bic     r1, r1,  #(xlli_MDCNFG_DE2 :OR: xlli_MDCNFG_DE3)   ; SDRAM banks
        str     r1, [r4, #xlli_MDCNFG_offset]   ; Write w/o enabling SDRAM banks
;
; Access memory *not yet enabled* for CBR refresh cycles (8)
; - CBR is generated for all banks
;
        ldr     r1, =xlli_SDRAM_PHYSICAL_BASE
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]
        str     r1, [r1]

        str     r1, [r1]  ;  Fix for erratum #116. Makes up for ineffective 1st mem access.
                          ;  This is being left in for Bulverde for the moment
;
; Re-enable SDRAM partition(s)
;
        ldr     r2,  [r4, #xlli_MDCNFG_offset]   ; Fetch the current MDCNFG value

        IF :DEF: SDRAM_SIZE_64_MB               
        orr     r2,  r2,  #xlli_MDCNFG_DE0                              ; Enable SDRAM bank 0
        ELSE
        orr     r2,  r2,  #(xlli_MDCNFG_DE0 :OR: xlli_MDCNFG_DE1)       ; Enable SDRAM bank 0 & 1
        ENDIF
        
        str     r2,  [r4, #xlli_MDCNFG_offset]   ; Write back MDCNFG, enabling the SDRAM bank(s)
;
; Write the MDMRS register to trigger an MRS command to all enabled banks of SDRAM.
;
;
        ldr     r1,  [r4, #xlli_MDMRS_offset]    ; Fetch platform MDMRS value
        str     r1,  [r4, #xlli_MDMRS_offset]    ; Write the MDMRS value back
;
        ldr     r3,  [r4, #xlli_MDREFR_offset]   ; Get MDREFR value
        str     r3,  [r4, #xlli_MDREFR_offset]   ; Write value back

        mov     pc,  lr                         ; return to calling routine
        ENDFUNC


;**************************************************************************************************
;
; ********************************************************
; **********                                    **********
; ********** MAXIMIZE MEMORY CONTROLLER VALUES  **********
; **********                                    **********
; ********************************************************
;
; NOTE: This function maximizes the timing values in the memory controller so the frequency change
;       sequence can be done safely without the risk of a system hang. Once the frequency change
;       sequence is complete, xlli__mem_Topt may be called to set the optimal timing values.
;
;       This function does not need to be called if the new MemClk frequency is going to be higher
;       than the present MemClk frequency. However, the overhead of this function is minimal so
;       it is suggested this function be called before any change frequency sequence.
;
;       r1, r2, and r3 are used and not preserved.
;
xlli_mem_Tmax   FUNCTION

        ldr     r1,  =xlli_MEMORY_CONFIG_BASE   ; Get memory controller base address
        ldr     r3,  =0x7FF07FF0                ; Maximize bits for RDF, RDN and RRR
;
        ldr     r2,  [r1, #xlli_MSC0_offset]    ; Get MSC0 value
        orr     r2,  r2,  r3                    ; Set all the RDF, RDN and RRR bits
        str     r2,  [r1, #xlli_MSC0_offset]    ; Set the new MSC0 value
        ldr     r2,  [r1, #xlli_MSC0_offset]    ; Read MSC0 value back to lock the values
;
        ldr     r2,  [r1, #xlli_MSC1_offset]    ; Get MSC1 value
        orr     r2,  r2,  r3                    ; Set all the RDF, RDN and RRR bits
        str     r2,  [r1, #xlli_MSC1_offset]    ; Set the new MSC1 value
        ldr     r2,  [r1, #xlli_MSC1_offset]    ; Read MSC1 value back to lock the values
;
        ldr     r2,  [r1, #xlli_MSC2_offset]    ; Get MSC2 value
        orr     r2,  r2,  r3                    ; Set all the RDF, RDN and RRR bits
        str     r2,  [r1, #xlli_MSC2_offset]    ; Set the new MSC2 value
        ldr     r2,  [r1, #xlli_MSC2_offset]    ; Read MSC2 value back to lock the values
;
        ldr     r3,  =0x03000300                ; Set up mask bits for DTC0 and DTC2
        ldr     r2,  [r1, #xlli_MDCNFG_offset]  ; Get present MDCNFG value
        orr     r2,  r2,  r3                    ; Clear the DTC0 and DTC2 bits
        str     r2,  [r1, #xlli_MDCNFG_offset]  ; Write back the MDCNFG value
;
        ldr     r3,  =0xFFF                     ; Set up mask bits for DRI value
        ldr     r2,  [r1, #xlli_MDREFR_offset]  ; Get present MDREFR value
        bic     r2,  r2,  r3                    ; Clear all the DRI bits
        orr     r2,  r2,  #0x13                 ; Set to lowest safe value
        str     r2,  [r1, #xlli_MDREFR_offset]  ; Write back the MDREFR value

        mov     pc,  lr                         ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; ***********************************************************
; **********                                       **********
; ********** SET OPTIMAL MEMORY CONTROLLER VALUES  **********
; **********                                       **********
; ***********************************************************
;
; NOTE: This function sets the TIMING values in the memory controller to the optimum values based on
;       the frequency of the MemClk. This function should be called AFTER the frequency change
;       sequence is complete to get optimal performance out of the memory system.
;
;       If changing the core frequency to a value that increases the MemClk speed, it is strongly
;       suggested that xlli_MSCx_max be called before the frequency change sequence. Once the
;       frequency chage sequence is complete, this routine may be called to reset the optimal
;       values for the memory system and the memory controller restarted.
;
;       r1 through r8 are used and not preserved.
;
xlli_mem_Topt   FUNCTION

        ldr     r1,  =xlli_CLKREGS_PHYSICAL_BASE  ; Get base address of clock registers
        ldr     r2,  [r1, #xlli_CCSR_offset]      ; Get current clock status
        and     r2,  r2,  #0x1F                   ; r2 now contains the CCSR L value

        ldr     r3,  [r1, #xlli_CCCR_offset]      ; Get the current CCCR for the "A" bit
        and     r3,  r3,  #xlli_CCCR_A_Bit_Mask
        mov     r3,  r3,  LSR #25                 ; Move down to bit position 0 to make comparison easy
;
;       Check for the "B" bit from CCCR (CP14,R6)
;
        mrc     p14, 0, r1, c6, c0, 0    ; Get data in CP14, Register 6
        and     r1,  r1,  #0x8           ; Grab just the "B" bit
        mov     r1,  r1,  LSR #2         ; move it to bit position 1
        orr     r9, r3, r1               ; At this point r9 contains the A-bit at pos. 0 & B-bit at pos. 1

        ldr     r1,  =xlli_MEMORY_CONFIG_BASE     ; Load r1 with memory controller base address
;
;       The MemClk speed is determined by the L, A and B values.
;       Unfortunaly, the only practical way to do this is to rip through all the legal
;       values and load a register when there is a match. Illegal values result in no change
;
        ldr     r3,  =0x7FF07FF0                ; Bit mask for RDF, RDN and RRR (MSC0)
        ldr     r4,  [r1, #xlli_MSC0_offset]    ; Get present MSC0 value
        bic     r4,  r4,  r3                    ; Clear the RDF, RDN and RRR bits

        ldr     r5,  =0x03000300                ; DTC0, DTC2 and RESERVED bit mask (MDCNFG)
        ldr     r6,  [r1, #xlli_MDCNFG_offset]  ; Get present MDCNFG value
        bic     r6,  r6,  r5                    ; Clear DTC0 and DTC2
        mov     r5,  #0                         ; Clear r5

        ldr     r7,  =0xC0200FFF                ; DRI and RESERVED bit mask (MDREFR)
        ldr     r8,  [r1, #xlli_MDREFR_offset]  ; Get present MDREFR value
        bic     r8,  r8,  r7                    ; Clear DRI and RESERVED bits
        ldr     r7,  = 0xFFF                    ; Reload r7 with POR DRI value
;
;       r9 =  0x0:  A=0, B=0
;             0x1:  A=1, B=0
;             0x2:  A=0, B=1
;             0x3:  A=1, B=1
;
        cmp     r9, #0x1                         ; A == 1, B == 0 ?
        beq     xlli_A1B0_Table

        cmp     r9, #0x3                         ; A == 1, B == 1 ?
        beq     xlli_A1B1_Table
;
;       Else drop through to A=0, B=x table
;
xlli_A0Bx_Table
        cmp     r2,  #2              ; Is L=2?
        ldreq   r3,  =xlli_MSC0_26   ; Yes - load values
        ldreq   r5,  =xlli_DTC_26
        ldreq   r7,  =xlli_DRI_26
        cmp     r2,  #3              ; Is L=3?
        ldreq   r3,  =xlli_MSC0_39   ; Yes - load values
        ldreq   r5,  =xlli_DTC_39
        ldreq   r7,  =xlli_DRI_39
        cmp     r2,  #4              ; Is L=4?
        ldreq   r3,  =xlli_MSC0_52   ; Yes - load values
        ldreq   r5,  =xlli_DTC_52
        ldreq   r7,  =xlli_DRI_52
        cmp     r2,  #5              ; Is L=5?
        ldreq   r3,  =xlli_MSC0_65   ; Yes - load values
        ldreq   r5,  =xlli_DTC_65
        ldreq   r7,  =xlli_DRI_65
        cmp     r2,  #6              ; Is L=6?
        ldreq   r3,  =xlli_MSC0_78   ; Yes - load values
        ldreq   r5,  =xlli_DTC_78
        ldreq   r7,  =xlli_DRI_78
        cmp     r2,  #7              ; Is L=7?
        ldreq   r3,  =xlli_MSC0_91   ; Yes - load values
        ldreq   r5,  =xlli_DTC_91
        ldreq   r7,  =xlli_DRI_91
        cmp     r2,  #8              ; Is L=8?
        ldreq   r3,  =xlli_MSC0_104  ; Yes - load values
        ldreq   r5,  =xlli_DTC_104
        ldreq   r7,  =xlli_DRI_104
        cmp     r2,  #9              ; Is L=9?
        ldreq   r3,  =xlli_MSC0_117  ; Yes - load values
        ldreq   r5,  =xlli_DTC_117
        ldreq   r7,  =xlli_DRI_117

        cmp     r2,  #10             ; Is L=10?
        ldreq   r3,  =xlli_MSC0_130  ; Yes - load values
        ldreq   r5,  =xlli_DTC_130
        ldreq   r7,  =xlli_DRI_130
        cmp     r2,  #11             ; Is L=11?
        ldreq   r3,  =xlli_MSC0_71   ; Yes - load values
        ldreq   r5,  =xlli_DTC_71
        ldreq   r7,  =xlli_DRI_71
        cmp     r2,  #12             ; Is L=12?
        ldreq   r3,  =xlli_MSC0_78   ; Yes - load values
        ldreq   r5,  =xlli_DTC_78
        ldreq   r7,  =xlli_DRI_78
        cmp     r2,  #13             ; Is L=13?
        ldreq   r3,  =xlli_MSC0_84   ; Yes - load value
        ldreq   r5,  =xlli_DTC_84
        ldreq   r7,  =xlli_DRI_84
        cmp     r2,  #14             ; Is L=14?
        ldreq   r3,  =xlli_MSC0_91   ; Yes - load values
        ldreq   r5,  =xlli_DTC_91
        ldreq   r7,  =xlli_DRI_91
        cmp     r2,  #15             ; Is L=15?
        ldreq   r3,  =xlli_MSC0_97   ; Yes - load values
        ldreq   r5,  =xlli_DTC_97
        ldreq   r7,  =xlli_DRI_97

        cmp     r2,  #16             ; Is L=16?
        ldreq   r3,  =xlli_MSC0_104  ; Yes - load values
        ldreq   r5,  =xlli_DTC_104
        ldreq   r7,  =xlli_DRI_104
        cmp     r2,  #17             ; Is L=17?
        ldreq   r3,  =xlli_MSC0_110  ; Yes - load values
        ldreq   r5,  =xlli_DTC_110
        ldreq   r7,  =xlli_DRI_110
        cmp     r2,  #18             ; Is L=18?
        ldreq   r3,  =xlli_MSC0_117  ; Yes - load values
        ldreq   r5,  =xlli_DTC_117
        ldreq   r7,  =xlli_DRI_117
        cmp     r2,  #19             ; Is L=19?
        ldreq   r3,  =xlli_MSC0_124  ; Yes - load values
        ldreq   r5,  =xlli_DTC_124
        ldreq   r7,  =xlli_DRI_124
        cmp     r2,  #20             ; Is L=20?
        ldreq   r3,  =xlli_MSC0_130  ; Yes - load values
        ldreq   r5,  =xlli_DTC_130
        ldreq   r7,  =xlli_DRI_130

        cmp     r2,  #21             ; Is L=21?
        ldreq   r3,  =xlli_MSC0_68  ; Yes - load values
        ldreq   r5,  =xlli_DTC_68
        ldreq   r7,  =xlli_DRI_68
        cmp     r2,  #22             ; Is L=22?
        ldreq   r3,  =xlli_MSC0_71  ; Yes - load values
        ldreq   r5,  =xlli_DTC_71
        ldreq   r7,  =xlli_DRI_71
        cmp     r2,  #23             ; Is L=23?
        ldreq   r3,  =xlli_MSC0_74  ; Yes - load values
        ldreq   r5,  =xlli_DTC_74
        ldreq   r7,  =xlli_DRI_74
        cmp     r2,  #24             ; Is L=24?
        ldreq   r3,  =xlli_MSC0_78  ; Yes - load values
        ldreq   r5,  =xlli_DTC_78
        ldreq   r7,  =xlli_DRI_78
        cmp     r2,  #25             ; Is L=25?
        ldreq   r3,  =xlli_MSC0_81  ; Yes - load values
        ldreq   r5,  =xlli_DTC_81
        ldreq   r7,  =xlli_DRI_81

        cmp     r2,  #26             ; Is L=26?
        ldreq   r3,  =xlli_MSC0_84  ; Yes - load values
        ldreq   r5,  =xlli_DTC_84
        ldreq   r7,  =xlli_DRI_84
        cmp     r2,  #27             ; Is L=27?
        ldreq   r3,  =xlli_MSC0_87  ; Yes - load values
        ldreq   r5,  =xlli_DTC_87
        ldreq   r7,  =xlli_DRI_87
        cmp     r2,  #28             ; Is L=28?
        ldreq   r3,  =xlli_MSC0_91  ; Yes - load values
        ldreq   r5,  =xlli_DTC_91
        ldreq   r7,  =xlli_DRI_91
        cmp     r2,  #29             ; Is L=29?
        ldreq   r3,  =xlli_MSC0_94   ; Yes - load values
        ldreq   r5,  =xlli_DTC_94
        ldreq   r7,  =xlli_DRI_94
        cmp     r2,  #30             ; Is L=30?
        ldreq   r3,  =xlli_MSC0_97   ; Yes - load values
        ldreq   r5,  =xlli_DTC_97
        ldreq   r7,  =xlli_DRI_97
        cmp     r2,  #31             ; Is L=31?
        ldreq   r3,  =xlli_MSC0_100  ; Yes - load values
        ldreq   r5,  =xlli_DTC_100
        ldreq   r7,  =xlli_DRI_100

        b XLLI_Done_MSC0_Opt_Update

xlli_A1B0_Table

        cmp     r2,  #2              ; Is L=2?
        ldreq   r3,  =xlli_MSC0_13   ; Yes - load values
        ldreq   r5,  =xlli_DTC_13
        ldreq   r7,  =xlli_DRI_13
        cmp     r2,  #3              ; Is L=3?
        ldreq   r3,  =xlli_MSC0_19   ; Yes - load values
        ldreq   r5,  =xlli_DTC_19
        ldreq   r7,  =xlli_DRI_19
        cmp     r2,  #4              ; Is L=4?
        ldreq   r3,  =xlli_MSC0_26   ; Yes - load values
        ldreq   r5,  =xlli_DTC_26
        ldreq   r7,  =xlli_DRI_26
        cmp     r2,  #5              ; Is L=5?
        ldreq   r3,  =xlli_MSC0_32   ; Yes - load values
        ldreq   r5,  =xlli_DTC_32
        ldreq   r7,  =xlli_DRI_32
        cmp     r2,  #6              ; Is L=6?
        ldreq   r3,  =xlli_MSC0_39   ; Yes - load values
        ldreq   r5,  =xlli_DTC_39
        ldreq   r7,  =xlli_DRI_39
        cmp     r2,  #7              ; Is L=7?
        ldreq   r3,  =xlli_MSC0_45   ; Yes - load values
        ldreq   r5,  =xlli_DTC_45
        ldreq   r7,  =xlli_DRI_45

        cmp     r2,  #8              ; Is L=8?
        ldreq   r3,  =xlli_MSC0_52  ; Yes - load values
        ldreq   r5,  =xlli_DTC_52
        ldreq   r7,  =xlli_DRI_52
        cmp     r2,  #9              ; Is L=9?
        ldreq   r3,  =xlli_MSC0_58  ; Yes - load values
        ldreq   r5,  =xlli_DTC_58
        ldreq   r7,  =xlli_DRI_58

        cmp     r2,  #10             ; Is L=10?
        ldreq   r3,  =xlli_MSC0_65  ; Yes - load values
        ldreq   r5,  =xlli_DTC_65
        ldreq   r7,  =xlli_DRI_65
;
;       L11 - L20 ARE THE SAME for A0Bx
;
        cmp     r2,  #11             ; Is L=11?
        ldreq   r3,  =xlli_MSC0_71   ; Yes - load values
        ldreq   r5,  =xlli_DTC_71
        ldreq   r7,  =xlli_DRI_71
        cmp     r2,  #12             ; Is L=12?
        ldreq   r3,  =xlli_MSC0_78   ; Yes - load values
        ldreq   r5,  =xlli_DTC_78
        ldreq   r7,  =xlli_DRI_78
        cmp     r2,  #13             ; Is L=13?
        ldreq   r3,  =xlli_MSC0_84   ; Yes - load values
        ldreq   r5,  =xlli_DTC_84
        ldreq   r7,  =xlli_DRI_84
        cmp     r2,  #14             ; Is L=14?
        ldreq   r3,  =xlli_MSC0_91   ; Yes - load values
        ldreq   r5,  =xlli_DTC_91
        ldreq   r7,  =xlli_DRI_91
        cmp     r2,  #15             ; Is L=15?
        ldreq   r3,  =xlli_MSC0_97   ; Yes - load values
        ldreq   r5,  =xlli_DTC_97
        ldreq   r7,  =xlli_DRI_97
        cmp     r2,  #16             ; Is L=16?
        ldreq   r3,  =xlli_MSC0_104  ; Yes - load values
        ldreq   r5,  =xlli_DTC_104
        ldreq   r7,  =xlli_DRI_104
        cmp     r2,  #17             ; Is L=17?
        ldreq   r3,  =xlli_MSC0_110  ; Yes - load values
        ldreq   r5,  =xlli_DTC_110
        ldreq   r7,  =xlli_DRI_110
        cmp     r2,  #18             ; Is L=18?
        ldreq   r3,  =xlli_MSC0_117  ; Yes - load values
        ldreq   r5,  =xlli_DTC_117
        ldreq   r7,  =xlli_DRI_117
        cmp     r2,  #19             ; Is L=19?
        ldreq   r3,  =xlli_MSC0_124  ; Yes - load values
        ldreq   r5,  =xlli_DTC_124
        ldreq   r7,  =xlli_DRI_124
        cmp     r2,  #20             ; Is L=20?
        ldreq   r3,  =xlli_MSC0_130  ; Yes - load values
        ldreq   r5,  =xlli_DTC_130
        ldreq   r7,  =xlli_DRI_130

        cmp     r2,  #21             ; Is L=21?
        ldreq   r3,  =xlli_MSC0_136  ; Yes - load values
        ldreq   r5,  =xlli_DTC_136
        ldreq   r7,  =xlli_DRI_136
        cmp     r2,  #22             ; Is L=22?
        ldreq   r3,  =xlli_MSC0_143  ; Yes - load values
        ldreq   r5,  =xlli_DTC_143
        ldreq   r7,  =xlli_DRI_143
        cmp     r2,  #23             ; Is L=23?
        ldreq   r3,  =xlli_MSC0_149  ; Yes - load values
        ldreq   r5,  =xlli_DTC_149
        ldreq   r7,  =xlli_DRI_149
        cmp     r2,  #24             ; Is L=24?
        ldreq   r3,  =xlli_MSC0_156  ; Yes - load values
        ldreq   r5,  =xlli_DTC_156
        ldreq   r7,  =xlli_DRI_156
        cmp     r2,  #25             ; Is L=25?
        ldreq   r3,  =xlli_MSC0_162  ; Yes - load values
        ldreq   r5,  =xlli_DTC_162
        ldreq   r7,  =xlli_DRI_162

        cmp     r2,  #26             ; Is L=26?
        ldreq   r3,  =xlli_MSC0_169  ; Yes - load values
        ldreq   r5,  =xlli_DTC_169
        ldreq   r7,  =xlli_DRI_169
        cmp     r2,  #27             ; Is L=27?
        ldreq   r3,  =xlli_MSC0_175  ; Yes - load values
        ldreq   r5,  =xlli_DTC_175
        ldreq   r7,  =xlli_DRI_175
        cmp     r2,  #28             ; Is L=28?
        ldreq   r3,  =xlli_MSC0_182  ; Yes - load values
        ldreq   r5,  =xlli_DTC_182
        ldreq   r7,  =xlli_DRI_182
        cmp     r2,  #29             ; Is L=29?
        ldreq   r3,  =xlli_MSC0_188   ; Yes - load values
        ldreq   r5,  =xlli_DTC_188
        ldreq   r7,  =xlli_DRI_188
        cmp     r2,  #30             ; Is L=30?
        ldreq   r3,  =xlli_MSC0_195   ; Yes - load values
        ldreq   r5,  =xlli_DTC_195
        ldreq   r7,  =xlli_DRI_195
        cmp     r2,  #31             ; Is L=31?
        ldreq   r3,  =xlli_MSC0_201  ; Yes - load values
        ldreq   r5,  =xlli_DTC_201
        ldreq   r7,  =xlli_DRI_201

        b XLLI_Done_MSC0_Opt_Update


xlli_A1B1_Table
        cmp     r2,  #2              ; Is L=2?
        ldreq   r3,  =xlli_MSC0_26   ; Yes - load values
        ldreq   r5,  =xlli_DTC_26
        ldreq   r7,  =xlli_DRI_26
        cmp     r2,  #3              ; Is L=3?
        ldreq   r3,  =xlli_MSC0_39   ; Yes - load values
        ldreq   r5,  =xlli_DTC_39
        ldreq   r7,  =xlli_DRI_39
        cmp     r2,  #4              ; Is L=4?
        ldreq   r3,  =xlli_MSC0_52   ; Yes - load values
        ldreq   r5,  =xlli_DTC_52
        ldreq   r7,  =xlli_DRI_52
        cmp     r2,  #5              ; Is L=5?
        ldreq   r3,  =xlli_MSC0_65   ; Yes - load values
        ldreq   r5,  =xlli_DTC_65
        ldreq   r7,  =xlli_DRI_65
        cmp     r2,  #6              ; Is L=6?
        ldreq   r3,  =xlli_MSC0_78   ; Yes - load values
        ldreq   r5,  =xlli_DTC_78
        ldreq   r7,  =xlli_DRI_78
        cmp     r2,  #7              ; Is L=7?
        ldreq   r3,  =xlli_MSC0_91   ; Yes - load values
        ldreq   r5,  =xlli_DTC_91
        ldreq   r7,  =xlli_DRI_91
        cmp     r2,  #8              ; Is L=8?
        ldreq   r3,  =xlli_MSC0_104  ; Yes - load values
        ldreq   r5,  =xlli_DTC_104
        ldreq   r7,  =xlli_DRI_104
        cmp     r2,  #9              ; Is L=9?
        ldreq   r3,  =xlli_MSC0_117  ; Yes - load values
        ldreq   r5,  =xlli_DTC_117
        ldreq   r7,  =xlli_DRI_117

        cmp     r2,  #10             ; Is L=10?
        ldreq   r3,  =xlli_MSC0_130  ; Yes - load values
        ldreq   r5,  =xlli_DTC_130
        ldreq   r7,  =xlli_DRI_130
        cmp     r2,  #11             ; Is L=11?
        ldreq   r3,  =xlli_MSC0_143   ; Yes - load values
        ldreq   r5,  =xlli_DTC_143
        ldreq   r7,  =xlli_DRI_143
        cmp     r2,  #12             ; Is L=12?
        ldreq   r3,  =xlli_MSC0_156   ; Yes - load values
        ldreq   r5,  =xlli_DTC_156
        ldreq   r7,  =xlli_DRI_156
        cmp     r2,  #13             ; Is L=13?
        ldreq   r3,  =xlli_MSC0_169   ; Yes - load values
        ldreq   r5,  =xlli_DTC_169
        ldreq   r7,  =xlli_DRI_169
        cmp     r2,  #14             ; Is L=14?
        ldreq   r3,  =xlli_MSC0_182   ; Yes - load values
        ldreq   r5,  =xlli_DTC_182
        ldreq   r7,  =xlli_DRI_182
        cmp     r2,  #15             ; Is L=15?
        ldreq   r3,  =xlli_MSC0_195   ; Yes - load values
        ldreq   r5,  =xlli_DTC_195
        ldreq   r7,  =xlli_DRI_195

        cmp     r2,  #16             ; Is L=16?
        ldreq   r3,  =xlli_MSC0_208  ; Yes - load values
        ldreq   r5,  =xlli_DTC_208
        ldreq   r7,  =xlli_DRI_208

XLLI_Done_MSC0_Opt_Update
;
;       Update MSC0
;
        orr     r4,  r4,  r3                    ; Update the RDF, RDN and RRR bits
        str     r4,  [r1, #xlli_MSC0_offset]    ; Set the new MSC0 value
        ldr     r4,  [r1, #xlli_MSC0_offset]    ; Read MSC0 value back to lock the values
;
;       Update DRI bits
;
        orr     r8,  r8,  r7                    ; Update the DRI bits
        str     r8,  [r1, #xlli_MDREFR_offset]  ; Set the new MDREFR value
;
;       if SDClk = MemClk, (K1DB2=0) shift the DTC data to the right by 1 bit
;
        ldr     r4,  =0x01000100                ; DTC mask bits (only need low order DTC bit)
        ands    r8,  r8,   #xlli_MDREFR_K1DB2   ; Test K1DB2 bit (does SDCLK[1] = MemClk freq?)
        movne   r5,  r5, LSR #1                 ; Shift DTC right by 1 bit (divide by 2)
        andne   r5,  r5,  r4                    ; r5 now contains the new DTC value
        orr     r6,  r6,  r5                    ; Update DTC0 and DTC2 bits
        str     r6,  [r1, #xlli_MDCNFG_offset]  ; Write back the MDCNFG value

        mov     pc,  lr                         ; return to calling routine

        ENDFUNC

        LTORG

;**************************************************************************************************
;
; ******************************************************
; **********                                  **********
; ********** INITIALIZE (MASK) ALL INTERRUPTS **********
; **********                                  **********
; ******************************************************
;
; NOTE: On system reset, all interrupts should be cleared by hardware.
;       This enforces disabling of all interrupts to HW boot default conditions.
;
xlli_intr_init   FUNCTION

        ldr     r4,  =xlli_INTERREGS_PHYSICAL_BASE  ; Load controller physical base address
        ldr     r2,  =0x0                           ; zero out a work register
        str     r2,  [r4, #xlli_ICMR_offset]        ; Mask all interrupts (clear mask register)
        str     r2,  [r4, #xlli_ICMR2_offset]       ; Mask all interrupts (clear mask register) 2
        str     r2,  [r4, #xlli_ICLR_offset]        ; Clear the interrupt level register
        str     r2,  [r4, #xlli_ICLR2_offset]       ; Clear the interrupt level register 2
        str     r2,  [r4, #xlli_ICCR_offset]        ; Clear Interrupt Control Register
        str     r2,  [r4, #xlli_ICCR2_offset]       ; Clear Interrupt Control Register 2
;
;       Read back from APB to ensure our writes have completed before continuing
;
        ldr     r2,  [r4, #xlli_ICMR_offset]

        mov     pc,  lr                             ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; **********************************************
; **********                          **********
; ********** INITIALIZE CLOCK MANAGER **********
; **********                          **********
; **********************************************
;
; Disable the peripheral clocks, and set the core clock frequency
;
; NOTE: The Change Frequency Sequence should be called after this function in order
;       for the clock frequencies set in the CCCR register to take effect.
;
;       The code then spins on the oscillator OK bit until the oscilator is stable
;       which can take as long as two seconds.
;

xlli_clks_init   FUNCTION

; Turn Off ALL on-chip peripheral clocks for re-configuration
;
        ldr     r4,  =xlli_CLKREGS_PHYSICAL_BASE; Load clock registers base address
        ldr     r1,  =0x400000                  ; Forces memory clock to stay ON!!
        ldr     r2,  =xlli_CKEN_value           ; Get any other bits required from the include file
        orr     r1,  r1,  r2                    ; OR everything together
        str     r1,  [r4, #xlli_CKEN_offset]    ; ... and write out to the clock enable register
;
; Set Crystal: Memory Freq, Memory:RunMode Freq, RunMode, TurboMode Freq Multipliers,
; set RunMode & TurboMode to default frequency.
;
        ldr     r2,  =xlli_CCCR_value           ; Get CORE_CLK_DEFAULT value
        str     r2,  [r4, #xlli_CCCR_offset]    ; Write to the clock config register
;
; Enable the 32 KHz oscillator and set the 32KHz output enable bits
;
        mov     r1,  #(xlli_OSCC_OON :OR: xlli_OSCC_TOUT_EN)
        str     r1,  [r4, #xlli_OSCC_offset]    ; for RTC and Power Manager
;
; Init Real Time Clock (RTC) registers
;
        ldr     r4,  =xlli_RTCREGS_PHYSICAL_BASE ; Load RTC registers base address
        mov     r2,  #0                          ; Clear a work register
        str     r2,  [r4, #xlli_RTSR_offset]     ; Clear RTC Status register
        str     r2,  [r4, #xlli_RCNR_offset]     ; Clear RTC Counter Register
        str     r2,  [r4, #xlli_RTAR_offset]     ; Clear RTC Alarm Register
        str     r2,  [r4, #xlli_SWCR_offset]     ; Clear Stopwatch Counter Register
        str     r2,  [r4, #xlli_SWAR1_offset]    ; Clear Stopwatch Alarm Register 1
        str     r2,  [r4, #xlli_SWAR2_offset]    ; Clear Stopwatch Alarm Register 2
        str     r2,  [r4, #xlli_PICR_offset]     ; Clear Periodic Counter Register
        str     r2,  [r4, #xlli_PIAR_offset]     ; Clear Interrupt Alarm Register
;       mov     pc,  lr                          ; DISABLED - Return here if A0 silicon
;
; Check the Oscillator OK (OOK) bit in clock register OSCC to insure the timekeeping oscillator
; is enabled and stable before returning to the calling program.
;
        ldr     r4,  =xlli_CLKREGS_PHYSICAL_BASE; Reload clock registers base address
xlli_6
        ldr     r1,  [r4, #xlli_OSCC_offset]    ; Get the status of the OSCC register
        ands    r1,  r1,  #xlli_OSCC_OOK        ; is the oscillator OK bit set?
        beq     xlli_6                          ; Spin in this loop until the bit is set

        mov     pc,  lr                                 ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; ****************************************************************
; **********                                            **********
; ********** INITIALIZE CLOCK MANAGER (FROM SLEEP MODE) **********
; **********                                            **********
; ****************************************************************
;
; Disable the peripheral clocks, and set the core clock frequency
;
;       The code then spins on the oscillator OK bit until the oscilator is stable
;       which can take as long as two seconds.
;

xlli_clks_init_sleepReset   FUNCTION

; Turn Off ALL on-chip peripheral clocks for re-configuration
;
        ldr     r4,  =xlli_CLKREGS_PHYSICAL_BASE; Load clock registers base address
        ldr     r1,  =0x400000                  ; Forces memory clock to stay ON!!
        ldr     r2,  =xlli_CKEN_value           ; Get any other bits required from the include file
        orr     r1,  r1,  r2                    ; OR everything together
        str     r1,  [r4, #xlli_CKEN_offset]    ; ... and write out to the clock enable register
;
; Set Crystal: Memory Freq, Memory:RunMode Freq, RunMode, TurboMode Freq Multipliers,
; set RunMode & TurboMode to default frequency.
;
        ldr     r2,  =xlli_CCCR_value           ; Get CORE_CLK_DEFAULT value
        str     r2,  [r4, #xlli_CCCR_offset]    ; Write to the clock config register
;
;       Read back from APB to ensure our writes have completed before continuing
;
        ldr     r2,  [r4, #xlli_CCCR_offset]

        mov     pc,  lr                                 ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; ***********************************************
; **********                           **********
; ********** FREQUENCY CHANGE SEQUENCE **********
; **********                           **********
; ***********************************************
;
; This subroutine initiates the frequency change sequence and restarts the memory controller
;

xlli_freq_change   FUNCTION

        mrc     p14, 0, r2, c6, c0, 0       ; Get present status (preserve Turbo and Fast Bus bits)
        orr     r2,  r2,  #2                ; Set the F bit
        mcr     p14, 0, r2, c6, c0, 0       ; initiate the frequency change sequence - Wheeeeeeeee!
;
;       If the clock frequency is chaged, the MDREFR Register must be  rewritten, even
;       if it's the same value. This will result in a refresh being performed and the
;       refresh counter being reset to the reset interval. (Section 13.10.3, pg 13-17 of EAS)
;
        ldr     r4,  =xlli_MEMORY_CONFIG_BASE       ; Get memory controller base address
        ldr     r1,  [r4, #xlli_MDREFR_offset]      ; Get the current state of MDREFR
        str     r1,  [r4, #xlli_MDREFR_offset]      ; Re-write this value

        mov     pc,  lr ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; ********************************************************
; **********                                    **********
; ********** INITIALIZE OPERATING SYSTEM TIMERS **********
; **********                                    **********
; ********************************************************
;
; This code segment initializes the OST count registers to zero, clears the
; status bits in OSSR, and zeroes out the match registers.
;
; After this function is called, the OS timers should be initalized to the
; same state as on HW reset.
;
; The interrupt bit for match register 1 is set for use by timing macros.
;

xlli_ost_init   FUNCTION

        ldr    r2,  =xlli_OSTREGS_PHYSICAL_BASE ; get base address for os timer registers
        mov    r3,  #0                          ; clear work register

        str    r3,  [r2, #xlli_OSCR0_offset]    ; zero out OS Timer Count register
        str    r3,  [r2, #xlli_OWER_offset]     ; zero out OS Timer Watchdog Match Enable Register
        str    r3,  [r2, #xlli_OIER_offset]     ; zero out OS Timer Interrupt Enable register
        str    r3,  [r2, #xlli_OSSR_offset]     ; zero out OS Timer Status register

        str    r3,  [r2, #xlli_OMCR4_offset]    ; Zero out Match Control Registers
        str    r3,  [r2, #xlli_OMCR5_offset]
        str    r3,  [r2, #xlli_OMCR6_offset]
        str    r3,  [r2, #xlli_OMCR7_offset]
        str    r3,  [r2, #xlli_OMCR8_offset]
        str    r3,  [r2, #xlli_OMCR9_offset]
        str    r3,  [r2, #xlli_OMCR10_offset]
        str    r3,  [r2, #xlli_OMCR11_offset]

        str    r3,  [r2, #xlli_OSCR0_offset]    ; Zero out count register 0
        str    r3,  [r2, #xlli_OSCR4_offset]    ; Zero out count register 4 - 11
        str    r3,  [r2, #xlli_OSCR5_offset]
        str    r3,  [r2, #xlli_OSCR6_offset]
        str    r3,  [r2, #xlli_OSCR7_offset]
        str    r3,  [r2, #xlli_OSCR8_offset]
        str    r3,  [r2, #xlli_OSCR9_offset]
        str    r3,  [r2, #xlli_OSCR10_offset]
        str    r3,  [r2, #xlli_OSCR11_offset]

        str    r3,  [r2, #xlli_OSMR0_offset]    ; zero-out all 12 match registers
        str    r3,  [r2, #xlli_OSMR1_offset]
        str    r3,  [r2, #xlli_OSMR2_offset]
        str    r3,  [r2, #xlli_OSMR3_offset]
        str    r3,  [r2, #xlli_OSMR4_offset]
        str    r3,  [r2, #xlli_OSMR5_offset]
        str    r3,  [r2, #xlli_OSMR6_offset]
        str    r3,  [r2, #xlli_OSMR7_offset]
        str    r3,  [r2, #xlli_OSMR8_offset]
        str    r3,  [r2, #xlli_OSMR9_offset]
        str    r3,  [r2, #xlli_OSMR10_offset]
        str    r3,  [r2, #xlli_OSMR11_offset]

        ldr    r1,  =xlli_OSSR_ALL              ; Clear the status bits - these are 'sticky' bits,
        str    r1,  [r2, #xlli_OSSR_offset]     ; These bits are cleared by writing 1's to them
;
;       Read back from APB to ensure our writes have completed before continuing
;
        ldr     r1, [r2, #xlli_OSSR_offset]

        mov     pc,  lr                         ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; **********************************************************************
; **********                                                  **********
; ********** ENABLE I-CACHE, D-CACHE AND BRANCH TARGET BUFFER **********
; **********                                                  **********
; **********************************************************************
;
; This code segment enables the Instruction Cache, Data Cache, and the Branch Target Buffer.
;
xlli_icache_enable   FUNCTION

        mrc    p15, 0, r2, c1, c0, 0            ; Get the contents of the ARM control register
        mov    r2,  r2, LSL #18                 ; Upper 18-bits must be written as zero...
        mov    r2,  r2, LSR #18                 ; ....clear them now

        orr    r2,  r2, #xlli_control_icache    ; set the i-cache bit
        orr    r2,  r2, #xlli_control_btb       ; set the btb bit
        orr    r2,  r2, #xlli_control_dcache    ; set the d-cache bit

        ; This code segment writes the contents of r2 into the ARM control register.
        ; Note: you must either format the contents of w1 so that reserved bits
        ;       are written with the proper value, or get the getARMControl values and
        ;       modify the control bits you need.
                ;
        ; make sure this is first instruction in a cache line

        b      xlli_7
        ALIGN  32
xlli_7
        mcr    p15, 0, r2, c1, c0, 0

        ; This code segment guarantees that previous writes to coprocessor 15 have
        ; completed. Depending on what is being modified in cp15 (turning on
        ; the mmu, for example), these instructions may need to be executed
        ; from the icache.

        mrc    p15, 0, r2, c1, c0, 0
        mov    r2,  r2
        sub    pc,  pc, #4

        mov     pc,  lr                         ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; **************************************************
; **********                              **********
; ********** INITIALIZE the POWER MANAGER **********
; **********                              **********
; **************************************************
;
; This code initializes the Power Manager
;
xlli_pwrmgr_init   FUNCTION

        ldr    r2,  =xlli_PMRCREGS_PHYSICAL_BASE ; get base address of power mgr / reset control regs

        mov    r1,  #0                          ; clear a work register
        str    r1,  [r2, #xlli_PWER_offset]     ; Reset Power Manager Wake-up register
        str    r1,  [r2, #xlli_PRER_offset]     ; Clear Power Manager Rising-Edge Detector Enables
        str    r1,  [r2, #xlli_PFER_offset]     ; Clear Power Manager Falling-Edge Detector Enables
        str    r1,  [r2, #xlli_PEDR_offset]     ; Clear Power Manager GPIO edge-Detect Status register
        str    r1,  [r2, #xlli_PGSR0_offset]    ; Clear GPIO sleep state registers (GPIOs 31:0)
        str    r1,  [r2, #xlli_PGSR1_offset]    ; Clear GPIO sleep state registers (GPIOs 63:32)
        str    r1,  [r2, #xlli_PGSR2_offset]    ; Clear GPIO sleep state registers (GPIOs 95:64)
        str    r1,  [r2, #xlli_PGSR3_offset]    ; Clear GPIO sleep state registers (GPIOs 118:96)
        str    r1,  [r2, #xlli_PSTR_offset]     ; Reset Standby Configuration Register
        str    r1,  [r2, #xlli_PVCR_offset]     ; Reset Power Manager Voltage Change Control Register
        str    r1,  [r2, #xlli_PKWR_offset]     ; Clear Keyboard Wake-up Enable Register
        str    r1,  [r2, #xlli_PKSR_offset]     ; Clear Keyboard Edge Detect Status Register
;
;       Initialize the PCFR (Power Manager General Congiguration Manager)
;
        mov    r1,  #xlli_PCFR_OPDE              ; enable 3.68Mhz power-down
;        orr    r1,  r1, #xlli_PCFR_FP            ; enable PCMCIA pin float
;        orr    r1,  r1, #xlli_PCFR_FS            ; enable static memory pin float
                orr    r1,  r1, #xlli_PCFR_SYSEN_EN      ; System power supply enable pin
        str    r1,  [r2, #xlli_PCFR_offset]

;       Init PSLR (Power Manager Sleep Configuration Register) 
        ldr    r1, [r2, #xlli_PSLR_offset]      ; load value already in PSLR
        ldr    r3, =xlli_PSLR_USEDBITS          
        and    r1, r1, r3                       ; 0 out reserved bits
        orr    r1, r1, #xlli_PSLR_SL_PI_RETAIN  ; PI power domain retains state in sleep and deep sleep mode
        orr    r1, r1, #xlli_PSLR_SL_ROD        ; nRESET_OUT is not asserted upon entry into sleep or deep-sleep mode
        str    r1, [r2, #xlli_PSLR_offset]

        mov     pc,  lr                         ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; *****************************************
; **********                     **********
; ********** SET PAGE TABLE BASE **********
; **********                     **********
; *****************************************
;
; Set page table base (used after a sleep reset)
;
xlli_setPTB    FUNCTION

        ldr    r1,  =xlli_p_PageTable ; Get address of Page Table base
        mcr    p15, 0, r1, c2, c0, 0  ; Load Page Table base into CP 15

        mov    pc,  lr                ; return to calling routine

                ENDFUNC

;**************************************************************************************************
;
; ***************************************************************
; **********                                           **********
; ********** INITIALIZE PAGE TABLES FOR MEMORY MAPPING **********
; **********                                           **********
; ***************************************************************
;
; Init the Page Table area of memory by writting zeros out to the table
;

xlli_initPageTable   FUNCTION

        ldr    r1,  =xlli_p_PageTable ; Get address of Page Table base
        mcr    p15, 0, r1, c2, c0, 0 ; Load Page Table base into CP 15
;
;       zero-out the page table memory region
;
        mov    r2,  #xlli_s_PageTable ; get table size
        mov    r3,  #0
xlli_8
        subs   r2,  r2,  #4          ; Increment offset into table
        str    r3,  [r1, r2]         ; Clear table entry
        bne    xlli_8                ; Keep looping until done

        mov    pc,  lr               ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; ***********************************************************
; **********                                       **********
; ********** INITIALIZE THE MEMORY MANAGEMENT UNIT **********
; **********                                       **********
; ***********************************************************
;
;       Initialize the memory management unit
;

xlli_MMU_init      FUNCTION

        ; set DACR
        ldr    r1,  =xlli_DACR
        mcr    p15, 0, r1, c3, c0, 0       ; Set DACR

        ldr    r2,  =xlli_PID
        mov    r2,  r2, LSR #25            ; Clear the lo-order 25 bits
        mov    r2,  r2, LSL #25            ; (may be unnecessary)
        mcr    p15, 0, r2, c13, c0, 0      ; Set PID

        mrc    p15, 0, r1, c1, c0, 1       ; Get the current aux control settings

;       extract the control bits such that reserved bits are 0, this will
;       prep the bits for a subsequent write.

        and    r1,  r1,  #0x33
        orr    r1,  r1,  #xlli_CONTROL_MINIDATA_01 ; orr in the mini-data cache attributes

        mcr    p15, 0, r1, c1, c0, 1       ; Write back the new settings

;       invalidate and enable the BTB

        mcr    p15, 0, r1, c7, c5, 6       ; invalidate the Branch Target Buffer
;
;       enable the BTB
;
        mrc    p15, 0, r1, c1, c0, 0
        mov    r1,  r1, LSL #18            ; Upper 18 bits must be
        mov    r1,  r1, LSR #18            ; written as zeros
        orr    r1,  r1, #xlli_CONTROL_BTB  ; set the BTB bit

        b     xlli_9                       ; 1st instruction on cache line
        ALIGN 32
xlli_9
        mcr    p15, 0, r1, c1, c0, 0       ; write the data back
        mrc    p15, 0, r1, c2, c0, 0       ; Insure that the write completes
        mov    r1,  r1                     ; before continuing
        sub    pc,  pc,  #4

        mov    pc,  lr                     ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; **************************************************************
; **********                                          **********
; ********** FIRE UP THE MMU - SWITCH TO VIRTUAL MODE **********
; **********                                          **********
; **************************************************************
;
;      Make the transistion from physical mode to virtual mode.
;
;      This subroutine assumes thaqt execution is from boot ROM and that
;      the MMU is disabled when this subroutine is called.
;
;      NOTES:
;
;      1. The memory system must already be up and running.
;      2. The page table must be initalized (zeroed out)
;      3. The memory to be accessed must have their page table entries filled in.
;      4. The MMU needs to be initialized first.
;      5. When this subroutine returns to the caller, all addresses are virtual.
;
;      Typically this would be the last low level init subroutine to be run before
;      control is turned over to an operating system.
;

xlli_goVirtual  FUNCTION
;
;       (1) Temporarily overwrite page table entry that maps where we are now.
;
;       This code section effectivly makes the table entry where the program counter
;       (pc) would point now and, the table entry where the new pc value will point,
;       (after going to virtual addressing) result in the code pointing to the same
;       physical address regardless of which pc value is used. This is to
;       cover those cases where the pc is altered after the jump to virtual
;       addresing. If the sections are idenity (flat) mapped, then the following
;       code basicly rewrites a single table entery with a copy of itself. Once the
;       pc has been altered to the virtual address, the original table entry for the
;       address space we are now is is restored to its original value.
;
;       CAUTION: If the page table is mapped to virtual address different from the physical
;       address, this code is likely to result in an exception to 0x10 (Data Abort) unless
;       you modify the code in the section to take this into account.
;
        ldr   r1,  =xlli_p_PageTable       ; Get address of Page Table base

        mov   r4,  pc,  LSR #(20)          ; Current execution location (divide by 1 Mb)
        mov   r4,  r4,  LSL #(2)           ; Insure lower two bits are zero
        add   r4,  r4,  r1                 ; Point to RAM page table entry for this address space
        ldr   r3,  [r4]                    ; Save the existing table entry in r3
;
;       Do not alter r3 or r4 as we will need them later...
;
        ldr   r2,  =(xlli_v_xbBOOTROM :SHR: (18))
        add   r2,  r2,  r1                 ; Point to new (virtual) ROM page table entry
        ldr   r1,  [r2]                    ; Get this (virtural address) table entry....
;
;       Overwrite RAM entry for current (physical address) table entry
;
        str   r1,  [r4]                    ; ....and place it in the current table entry.

;       end-of-part (1)
; -------------------------------------------------
;       (2) setup to enable the MMU
;
;       Generate (in r1) the virtual address the code will jump to once the
;       transition has been made to virtual address mode.
;
        add   r1,  pc, #xlli_12 - (.+8)    ; Load address of xlli_12 into r1
        add   r1,  r1,  #xlli_v_xbBOOTROM  ; add offset to virtual ROM address
;
;       Invalidate both TLBs
;
        mcr p15, 0, r2, c8, c7, 0          ; Invalidates both instruction and data TLBs
;
;       enable mmu
;
        mrc   p15, 0, r2, c1, c0, 0        ; Read current settings in control reg
        mov   r2,  r2, LSL #18             ; Upper 18-bits must be written as zero,
        mov   r2,  r2, LSR #18             ; ... clear them now.
;
;               Set ROM protection, Clear System protection, Set MMU bit
;
        orr   r2, r2, #xlli_control_r      ; set the ROM Protection bit
        bic   r2, r2, #xlli_control_s      ; clear the System Protection bit
        orr   r2, r2, #xlli_control_mmu    ; set the MMU bit
;
;       Write back the new value
;
        b      xlli_11                     ; make sure this is first instruction in a cache line
        ALIGN  32
xlli_11
        mcr    p15, 0, r2, c1, c0, 0       ; Go Virtual!
        mrc    p15, 0, r2, c2, c0, 0       ; Insure that the write completes
        mov    r2,  r2                     ; before continuing
        sub    pc,  pc,  #4
;
;       invalidate both caches, btb
;
        mcr    p15, 0, r2, c7, c7, 0

        mov    pc,  r1                     ; Load virtual address into the pc
        nop                                ; nop instructions to "empty the pipeline"
        nop
        nop
                nop
        nop
        nop
xlli_12
        add   lr,  lr,  #xlli_v_xbBOOTROM  ; Update the link register return address

;       end-of-part (2)
; -------------------------------------------------
;       (3) restore the overwritten page table entry, flush all caches
;       The following instruction assumes the page table is idenity mapped!
;
        str   r3,  [r4]                    ; This restores original page table entry
;
;       Invalidate both TLBs and caches
;
        mcr   p15, 0, r2, c8, c7, 0        ; Invalidates both instruction and data TLBs
        mcr   p15, 0, r2, c7, c7, 0        ; invalidate both caches, BTB

        nop                                ; Make sure the pipe is empty
        nop
        nop
        nop
        nop
        nop
;
;       end-of-part (3)
;       -------------------------------------------------
;       set up cache memory for use
;
        mcr   p15, 0, r1, c7, c6, 0        ; Invalidates the D cache
        mcr   p15, 0, r1, c8, c6, 0        ; Invalidates the data TLB

        mrc    p15, 0, r2, c2, c0, 0       ; Insure that the write completes
        mov    r2,  r2                     ; before continuing
        sub    pc,  pc,  #4
;
;       invalidate the BTB
;
        mcr    p15, 0, r1, c7, c5, 6       ; invalidate the Branch Target Buffer
;
;       Enable ICache, DCache, BTB
;
        mrc   p15, 0, r2, c1, c0, 0        ; Read current settings in control reg
        mov   r2,  r2, LSL #18             ; Upper 18-bits must be written as zero,
        mov   r2,  r2, LSR #18             ; ... clear them now.

        orr   r2, r2, #xlli_control_dcache ; set the DCache bit
        orr   r2, r2, #xlli_control_icache ; set the ICache bit
        orr   r2, r2, #xlli_control_btb
;
;       Write back the new value
;
        b      xlli_13                     ; make sure this is first instruction in a cache line
        ALIGN  32
xlli_13
        mcr    p15, 0, r2, c1, c0, 0
        mrc    p15, 0, r2, c2, c0, 0       ; Insure that the write completes
        mov    r2,  r2                     ; before continuing
        sub    pc,  pc,  #4

        mov    pc,  lr                     ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; ***************************************************************
; **********                                           **********
; ********** INITIALIZE INTERNAL MEMORY POWER REGISTER **********
; **********                                           **********
; ***************************************************************
;
; Power up the internal memory for use (Default on system reset)
;
; Note: This subroutine sets the internal memory to the hardware reset default state
;       which is all memory in run mode with auto wake-up disabled and never going
;       into standby mode.
;

xlli_IMpwr_init      FUNCTION

        ldr    r2,  =xlli_IMEMORY_CONFIG_BASE   ; get base address of IM Power Management control regs
        mov    r1,  #0                          ; Set all memory to RUN mode and disable auto wakeup
        str    r1,  [r2, #xlli_IMPMCR_offset]   ; Write the data to the control register

        mov    pc,  lr                          ; return to calling routine

        ENDFUNC

;**************************************************************************************************
;
; *******************************************************
; **********                                   **********
; ********** GET SYSTEM CONFIGURATION REGISTER **********
; **********                                   **********
; *******************************************************
;
; This is a "C" program callable subroutine that returns the value stored in the
; last SRAM memory location - which presumably still contains the Mainstone
; system configuration data that was put there by the xlli_read_SCR subroutine.
;
; Note: The system stack register must be set up before this subroutine is called.
;

GetSCR  FUNCTION

        stmfd   sp!, {r1, lr}                     ; Save r1 and link register on the stack
        ldr     r1,  =xlli_SCR_data               ; Address where system config data is stored
        ldr     r0,  [r1]                         ; Read the contents of SCR into r0
        ldmfd   sp!, {r1, pc}                     ; Restore r1 and return to caller

        ENDFUNC

;**************************************************************************************************
;
; ************************************************************
; **********                                        **********
; ********** SET SYSTEM CLOCKS FROM SWITCH SETTINGS **********
; **********                                        **********
; ************************************************************
;
; This subroutine sets the Bulverde/Mainstone clocks based on the Mainstone platform switch settings
; NOTE: This subroutine just sets the bits according to switch settings. The user needs to call the
;       change frequency sequence subroutine (xlli_freq_change) for the values to take effect.
;
xlli_setClocks  FUNCTION
;
;       This code segment gets the platform hex switch settings and loads the appropriate N and L values into
;       the CCCR register. Switch register Bits 15:11 and 7 are ignored by this code.
;
        ldr    r1,  =xlli_PLATFORM_REGISTERS             ; Get platform register base address
        ldr    r2,  [r1, #xlli_PLATFORM_SWITCH_offset]   ; Load platform switch settings into r2
        and    r3,  r2, #0x7F                            ; Get both HEX rotary values
        mov    r3,  r3, LSL #2                           ; Multiply by 4 to get word offset
        add    r1,  pc, #xlli_CLK_DATA - (.+8)           ; Load address of CLK_DATA into r1
        add    r1,  r1,  r3                              ; Point to address that contains the CCCR data to be loaded
        ldr    r5,  [r1]                                 ; load CCCR value into r5
        ands   r3,  r2,  #0x80                           ; Is bit 7 set (to set the CCCR A bit)? - This is why we only
                                                         ;  need a table that goes through 7F (they're the same above it)
        orrne  r5,  r5,  #0x2000000                      ; YES - set the A bit
        ldr    r1,  =xlli_CLKREGS_PHYSICAL_BASE          ; Get clocks register base address
        str    r5,  [r1, #xlli_CCCR_offset]              ; Load the new CCCR value
;
;       Read back from APB to ensure our writes have completed before continuing
;
        ldr    r5,  [r1, #xlli_CCCR_offset]

;
;       Check switch bit 9 for SDCLK setting (MemClk/2 or MemClock)
;
        ldr    r1,  =xlli_MEMORY_CONFIG_BASE     ; Clock registers base address
        ldr    r6,  [r1,  #xlli_MDREFR_offset]   ; get present value
        ands   r3,  r2,   #0x200                 ; Test switch bit 9 for "dot" or "no dot" position
        bicne  r6,  r6,   #xlli_MDREFR_K1DB2     ; clear bit
        orreq  r6,  r6,   #xlli_MDREFR_K1DB2     ; set bit so SDCLK = 1/2 MCEMCLK
        str    r6,  [r1,  #xlli_MDREFR_offset]   ; Write data back to register
;
;       Check switch bit 8 for Fast/Normal bus speed.
;       If hex switch setting is 0x5A - 0x7F or 0xDA and greater, the Fast Bus mode bit must
;       be cleared.
;       NOTE: Do not alter the contents of r5 which has the new CCCR value stored in it!
;
        mrc    p14, 0, r4, c6, c0, 0    ; Get data in CP14, Register 6
        ands   r3,  r2,  #0x100         ; Test bit 8 for "dot" or "no dot"
        biceq  r4,  r4,  #0X8           ; Clear Fast Bus speed bit
        beq    xlli_14                  ; B=0, so jump over the part that check if it's save to set B=1
        orrne  r4,  r4,  #0x8           ; Set Fast Bus speed bit

;       At this point, B=1 so we need to compare against 0x5A through 0x7F and then against
;       0xDA through 0xF7.  F8-FA are NECESSARY for A=1, N=3, B=1 is A LEGAL value, so must
;       preserve.  Otherswise, we need to see if we've hit a point where the Fast Bus mode bit must be cleared
;
        and    r3,  r2,  #0x7F          ; Grab just the HEX rotary switch value

;       If we blindly assume A=1 in this compare, we've taken care of most of the problem.  This reduces
;       the amount of compares to 2, as we compare against 0x5A which will take care of the 0xDA case also
        cmp    r3,  #0x5A               ; Compare to a hex switch setting of 0x5A (A=x, B=1)
        ble    xlli_14                  ; Less than or Equal, jump over the next few instructions

        and    r3,  r2,  #0xFF          ; Grab just the HEX rotary switch value AGAIN
        cmp    r3,  #0xF7               ; We've hit the F8-FA, so jump to end
        bgt    xlli_14

;       Now clear the Fast Bus (B-bit) if we've hit this point
        bic    r4,  r4,  #0x8           ; Clear Fast Bus speed bit if greater or equal to 0x5A

xlli_14
;
;       Check switch bit 10 for Turbo or non-Turbo mode.
;
        ands   r3,  r2,  #0x400         ; Test switch bit 10 for "dot" or "no dot" position
        biceq  r4,  r4,  #1             ; Clear Turbo bit
        orrne  r4,  r4,  #1             ; Set Turbo bit

        bic    r4,  r4,  #0x2           ; Clear the F bit (so frequency change doesn't happen here)

;       Now write the value back
        mcr    p14, 0, r4, c6, c0, 0    ; Write data to CP14, Register 6

;       At this point, although we've already set the MDREFR register based off of switch bit 9
;       We need to chack the MDREFR register vs. the desired frequency.  If it's too high, we should
;       halve or even quarter (for flash) if it's too hight to run properly.  This is being done here
;       as it is the intent to call the frequency change sequency after this function.
;       Values that need to halve or quarter MemClk (the rest can be left alone):
;
;           For A=0, B=0:
;               K0DB2=1 (K1DB2 cleared) when L= 5-8 (0x15-0x30), 11-16 (0x3F-0x5A), 21-31 (0x67-0x7F)
;               K0DB4=1 and K1DB2=1 when L= 9 (0x31-0x37), 10 (0x38-0x3E), 17-20 (0x5B-0x7F)
;
;           For A=0, B=1: *NOTE* (L17 and higher are illegal for B=1)
;               K0DB2=1 (K1DB2 cleared) when L= 5-8 (0x15-0x30), 11-16 (0x3F-0x5A)
;               K0DB4=1 and K1DB2=1 when L= 9 (0x31-0x37), 10 (0x38-0x3E)
;
;           For A=1, B=0:
;               K0DB2=1 (K1DB2 cleared) when L= 9-16 (0xB1-0xDA)
;               K0DB4=1 and K1DB2=1 when L= 17-31 (0xDB-0xFF)
;
;           For A=1, B=1: *NOTE* (L17 and higher are illegal for B=1)
;               K0DB2=1 (K1DB2 cleared) when L= 5-8 (0x95-0xB0)
;               K0DB4=1 and K1DB2=1 when L= 9-16 (0xB1-0xDA)
;

        mov     r3, r6  ; R6 from way above still contains the value we wrote to teh MDREFR register.

;       Clear the KxDBx bits, so that way we can set it to what we want

        ldr     r7,  =0x20084000                   ; KxDBx clear mask (leave K1DB2 from above)
        bic     r3,  r3,  r7                       ; Clear the KxDBx bits

;       At this point r5 = CCCR, R4 = B bit (CP14,R6). Pull out the relevant bits

        and    r7,  r5,  #xlli_CCCR_A_Bit_Mask   ; R5 now has just the A bit
        mov    r7,  r7,  LSR #25      ; move it down to the lowest bit (for easy comparison's sake)
        and    r2,  r5,  #0x1F        ; Get the L value
        and    r1,  r4,  #0x8         ; Get the B bit
        mov    r1,  r1,  LSR #3       ; Move it down (that comparison thing...)

        ; Now r0 = A-bit, r1 = B-bit, r2 = L-value

        cmp    r7,  #0x0              ; A == 0?
        bne    xlli_a_equ_1           ; Nope, A=1, jump to that area of code
;
;       A=0 at this point
;
        cmp    r1, #0x0               ; B == 0?
        bne    xlli_a0_b_equ_1        ; Nope, A=0, B=1

        ; A=0, B=0 at this point
        cmp    r2,  #0x5              ; r2 < 5 ?
        bcc    xlli_done_mdrefr_check ; L2-L4 need no MemClk division
        cmp    r2,  #0x8              ; r2 <= 8 ?
        bls    xlli_set_k0db2         ; L5-L8 need MemClk/2
        cmp    r2,  #0xA              ; r2 <= 10?
        bls    xlli_set_k0db4         ; L9-L10 need MemClk/4
        cmp    r2,  #0x10             ; r2 <= 16?
        bls    xlli_set_k0db2         ; L11-L16 need MemClk/2
        cmp    r2,  #0x14             ; r2 <= 20?
        bls    xlli_set_k0db4         ; L17-L20 need MemClk/4
        bhi    xlli_set_k0db2         ; L21-L31 need MemClk/2

xlli_a0_b_equ_1
;
;       A=0, B=1
;
        cmp    r2,  #0x5              ; r2 < 5 ?
        bcc    xlli_done_mdrefr_check ; L2-L4 need no MemClk division
        cmp    r2,  #0x8              ; r2 <= 8 ?
        bls    xlli_set_k0db2         ; L5-L8 need MemClk/2
        cmp    r2,  #0xA              ; r2 <= 10?
        bls    xlli_set_k0db4         ; L9-L10 need MemClk/4
        bhi    xlli_set_k0db2         ; L11-L16 need MemClk/2 - We've already prevented L above 16 when B=1

xlli_a_equ_1
;
;       A=1, B=?
;
        cmp    r1, #0x0               ; B == 0?
        bne    xlli_a1_b_equ_1        ; Nope, A=0, B=1
;
;       A=1, B=0 at this point
;
        cmp    r2,  #0x9              ; r2 < 9 ?
        bcc    xlli_done_mdrefr_check ; L2-L8 need no MemClk division
        cmp    r2,  #0x10              ; r2 <= 16 ?
        bls    xlli_set_k0db2         ; L9-L16 need MemClk/2
        bhi    xlli_set_k0db4         ; L17-L31 need MemClk/4

xlli_a1_b_equ_1
;
;       A=1, B=1
;
        cmp    r2,  #0x5              ; r2 < 5 ?
        bcc    xlli_done_mdrefr_check ; L2-L4 need no MemClk division
        cmp    r2,  #0x8              ; r2 <= 8 ?
        bhi    xlli_set_k0db4         ; L9-L16 need MemClk/4
;
;       Just fall through to k0db2 for L5-8
;
;       NOTE:  Since the expected order is to call xlli_mem_Tmax before calling this function,
;       simply set the KxDBx bits and move on (mem timings already maxed)
;
xlli_set_k0db2
;
;       Set the K0DB2 bit, K1DB2 does not need to be set
;
        mov r2, #0x1
        mov r2, r2, LSL #14  ; Set MDREFR bit 14 (K0DB2)
        orr r3, r3, r2
        b   xlli_done_mdrefr_check ;skip over the MemClk/4 stuff

xlli_set_k0db4
;
; Set the K0DB4 bit, K1DB2 must also be set (frequencies higher than 104MHz)
;
        ldr r2, =0x20020000  ; Set Bits 29 (K0DB4) & 17 (K1DB2)
        orr r3, r3, r2
;
;       Now just fall through to setting the MDREFR back
;
xlli_done_mdrefr_check
;
; Write the MDREFR value back and move on.
;
        ldr     r7,  =xlli_MEMORY_CONFIG_BASE   ; Reload r0 with memory controller base address
        str     r3,  [r7, #xlli_MDREFR_offset]  ; Set the new MDREFR value

        mov     pc,  lr                 ; Return to calling routine
;
; The following table is a lookup table for the values of L and N to be used depending on the hex switch
; setting on the Mainstone development platform. The values listed below are simply the values to be
; loaded directly into the CCCR register.  "xx" = Hex Switch setting, L = Core frequency, N = Turbo multiplier
;
xlli_CLK_DATA
        DCD     0x102     ; "00" L=2, N=1
        DCD     0X182     ; "01" L=2, N=1.5
        DCD     0X202     ; "02" L=2, N=2
        DCD     0X282     ; "03" L=2, N=2.5
        DCD     0X302     ; "04" L=2, N=3
        DCD     0X382     ; "05" L=2, N=3.5
        DCD     0X402     ; "06" L=2, N=4

        DCD     0x103     ; "07" L=3, N=1
        DCD     0X183     ; "08" L=3, N=1.5
        DCD     0X203     ; "09" L=3, N=2
        DCD     0X283     ; "0A" L=3, N=2.5
        DCD     0X303     ; "0B" L=3, N=3
        DCD     0X383     ; "0C" L=3, N=3.5
        DCD     0X403     ; "0D" L=3, N=4

        DCD     0x104     ; "0E" L=4, N=1
        DCD     0X184     ; "0F" L=4, N=1.5
        DCD     0X204     ; "10" L=4, N=2
        DCD     0X284     ; "11" L=4, N=2.5
        DCD     0X304     ; "12" L=4, N=3
        DCD     0X384     ; "13" L=4, N=3.5
        DCD     0X404     ; "14" L=4, N=4

        DCD     0x105     ; "15" L=5, N=1
        DCD     0X185     ; "16" L=5, N=1.5
        DCD     0X205     ; "17" L=5, N=2
        DCD     0X285     ; "18" L=5, N=2.5
        DCD     0X305     ; "19" L=5, N=3
        DCD     0X385     ; "1A" L=5, N=3.5
        DCD     0X405     ; "1B" L=5, N=4

        DCD     0x106     ; "1C" L=6, N=1
        DCD     0X186     ; "1D" L=6, N=1.5
        DCD     0X206     ; "1E" L=6, N=2
        DCD     0X286     ; "1F" L=6, N=2.5
        DCD     0X306     ; "20" L=6, N=3
        DCD     0X386     ; "21" L=6, N=3.5
        DCD     0X406     ; "22" L=6, N=4

        DCD     0x107     ; "23" L=7, N=1
        DCD     0X187     ; "24" L=7, N=1.5
        DCD     0X207     ; "25" L=7, N=2
        DCD     0X287     ; "26" L=7, N=2.5
        DCD     0X307     ; "27" L=7, N=3
        DCD     0X387     ; "28" L=7, N=3.5
        DCD     0X407     ; "29" L=7, N=4

        DCD     0x108     ; "2A" L=8, N=1
        DCD     0X188     ; "2B" L=8, N=1.5
        DCD     0X208     ; "2C" L=8, N=2
        DCD     0X288     ; "2D" L=8, N=2.5
        DCD     0X308     ; "2E" L=8, N=3
        DCD     0X388     ; "2F" L=8, N=3.5
        DCD     0X408     ; "30" L=8, N=4

        DCD     0x109     ; "31" L=9, N=1
        DCD     0X189     ; "32" L=9, N=1.5
        DCD     0X209     ; "33" L=9, N=2
        DCD     0X289     ; "34" L=9, N=2.5
        DCD     0X309     ; "35" L=9, N=3
        DCD     0X389     ; "36" L=9, N=3.5
        DCD     0X409     ; "37" L=9, N=4

        DCD     0x10A     ; "38" L=10, N=1
        DCD     0X18A     ; "39" L=10, N=1.5
        DCD     0X20A     ; "3A" L=10, N=2
        DCD     0X28A     ; "3B" L=10, N=2.5
        DCD     0X30A     ; "3C" L=10, N=3
        DCD     0X38A     ; "3D" L=10, N=3.5
        DCD     0X40A     ; "3E" L=10, N=4

        DCD     0x10B     ; "3F" L=11, N=1
        DCD     0X18B     ; "40" L=11, N=1.5
        DCD     0X20B     ; "41" L=11, N=2
        DCD     0X28B     ; "42" L=11, N=2.5
        DCD     0X30B     ; "43" L=11, N=3
        DCD     0X38B     ; "44" L=11, N=3.5

        DCD     0x10C     ; "45" L=12, N=1
        DCD     0X18C     ; "46" L=12, N=1.5
        DCD     0X20C     ; "47" L=12, N=2
        DCD     0X28C     ; "48" L=12, N=2.5
        DCD     0X30C     ; "49" L=12, N=3

        DCD     0x10D     ; "4A" L=13, N=1
        DCD     0X18D     ; "4B" L=13, N=1.5
        DCD     0X20D     ; "4C" L=13, N=2
        DCD     0X28D     ; "4D" L=13, N=2.5
        DCD     0X30D     ; "4E" L=13, N=3

        DCD     0x10E     ; "4F" L=14, N=1
        DCD     0X18E     ; "50" L=14, N=1.5
        DCD     0X20E     ; "51" L=14, N=2
        DCD     0X28E     ; "52" L=14, N=2.5

        DCD     0x10F     ; "53" L=15, N=1
        DCD     0X18F     ; "54" L=15, N=1.5
        DCD     0X20F     ; "55" L=15, N=2
        DCD     0X28F     ; "56" L=15, N=2.5

        DCD     0x110     ; "57" L=16, N=1
        DCD     0X190     ; "58" L=16, N=1.5
        DCD     0X210     ; "59" L=16, N=2
        DCD     0X290     ; "5A" L=16, N=2.5

        DCD     0x111     ; "5B" L=17, N=1
        DCD     0x191     ; "5C" L=17, N=1.5
        DCD     0x211     ; "5D" L=17, N=2

        DCD     0x112     ; "5E" L=18, N=1
        DCD     0x192     ; "5F" L=18, N=1.5
        DCD     0x212     ; "60" L=18, N=2

        DCD     0x113     ; "61" L=19, N=1
        DCD     0x193     ; "62" L=19, N=1.5
        DCD     0x213     ; "63" L=19, N=2

        DCD     0x114     ; "64" L=20, N=1
        DCD     0x194     ; "65" L=20, N=1.5
        DCD     0x214     ; "66" L=20, N=2

        DCD     0x115     ; "67" L=21, N=1
        DCD     0x195     ; "68" L=21, N=1.5

        DCD     0x116     ; "69" L=22, N=1
        DCD     0x196     ; "6A" L=22, N=1.5

        DCD     0x117     ; "6B" L=23, N=1
        DCD     0x197     ; "6C" L=23, N=1.5

        DCD     0x118     ; "6D" L=24, N=1
        DCD     0x198     ; "6E" L=24, N=1.5

        DCD     0x119     ; "6F" L=25, N=1
        DCD     0x199     ; "70" L=25, N=1.5

        DCD     0x11A     ; "71" L=26, N=1
        DCD     0x19A     ; "72" L=26, N=1.5

        DCD     0x11B     ; "73" L=27, N=1

        DCD     0x11C     ; "74" L=28, N=1

        DCD     0x11D     ; "75" L=29, N=1

        DCD     0x11E     ; "76" L=30, N=1

        DCD     0x11F     ; "77" L=31, N=1

        DCD     0X30E     ; "78" L=14, N=3 for 546 MHz Turbo Mode

        DCD     0X30F     ; "79" L=15, N=3 for 585 MHz Turbo Mode

        DCD     0X310     ; "7A" L=16, N=3 for 624 MHz Turbo Mode

        DCD     0x11F     ; "7B" L=31, N=1 (same values as "77")
        DCD     0x11F     ; "7C" L=31, N=1 (same values as "77")
        DCD     0x11F     ; "7D" L=31, N=1 (same values as "77")
        DCD     0x11F     ; "7E" L=31, N=1 (same values as "77")
        DCD     0x11F     ; "7F" L=31, N=1 (same values as "77")
;
;       For the A bit = 1
;       This table would be exactly the same as the above, only with the 'A' bit set,
;       so don't bother reproducting it.
;
        ENDFUNC
;**************************************************************************************************
;
; **********************************************************
; **********                                      **********
; ********** GET SYSTEM CLOCKS FREQUENCY SETTINGS **********
; **********     ( C Program callable version)    **********
; **********************************************************
;
; This subroutine returns all the data required to decode
; the frequencies of the system busses.
;
; R4 returns the following:
;
;     bits 4:0 = CCSR value of the L bits
;     bits 8:5 = CCSR value of the N bits
;     bit    9 = CLKCFG value for the TURBO bit
;     bit   10 = CLKCFG value for the B (Fast Bus) bit
;     bit   11 = CCSR value for the A bit
;     bit   12 = MDREFR value of K1DB2 bit
;     bit   13 = MDREFR value of K0DB4 bit
;     bit   14 = MDREFR value of K0DB2 bit
;     bit   15 = CLKCFG value of HT bit
;     bit   16 = PPDIS_S bit in the CCSR register
;     bit   17 = CPDIS_S bit in the CCSR register
;

xlli_getFreq  FUNCTION

        stmfd   sp!, {r1, r2, lr}                 ; Save r1, r2 and link register on the stack

        ldr     r1,  =xlli_CLKREGS_PHYSICAL_BASE  ; Address base for clock config registers
        ldr     r0,  [r1, #xlli_CCSR_offset]      ; Read the contents of CCSR into r0
        mov     r1,  r0                           ; and copy into r1
        and     r0,  r0,  #0x1F                   ; Leave just the L bits in r0

        and     r2,  r1,  #0x780                  ; Leave just the N bits in r2
        mov     r2,  r2,  LSR #2                  ; Shift N bits right by 2 bits
        orr     r0,  r0,  r2                      ; and load into r0
        ands    r2,  r1, #xlli_BIT_30             ; Is the PPDIS_S bit set?
        orrne   r0,  r0, #xlli_BIT_16             ; Yes - set bit 16 in R0
        ands    r2,  r1, #xlli_BIT_31             ; Is the CPDIS_S bit set?
        orrne   r0,  r0, #xlli_BIT_17             ; Yes - set bit 17 in R0

        mrc     p14, 0, r2, c6, c0, 0             ; Get data in CP14, Register 6
        ands    r1,  r2,  #xlli_BIT_0             ; Is the turbo bit set?
        orrne   r0,  r0,  #xlli_BIT_9             ; Yes - set bit 9 in r0
        ands    r1,  r2,  #xlli_BIT_3             ; Is the B bit set?
        orrne   r0,  r0,  #xlli_BIT_10            ; Yes - set bit 10 in r0
        ands    r1,  r2,  #xlli_BIT_2             ; is the HT bit set?
        orrne   r0,  r0,  #xlli_BIT_15            ; Yes - set bit 15 in r0

        ldr     r1,  =xlli_CLKREGS_PHYSICAL_BASE  ; Address base for clock config registers
        ldr     r2,  [r1, #xlli_CCCR_offset]      ; Read the contents of CCCR into r2
        ands    r1,  r2,  #0x2000000              ; Is the A bit set?
        orrne   r0,  r0,  #xlli_BIT_11            ; Yes - set bit 11 in r0

        ldr     r1,  =xlli_MEMORY_CONFIG_BASE     ; Get base address of memory registers
        ldr     r2,  [r1, #xlli_MDREFR_offset]    ; Get contents of MDREFR register
        ands    r1,  r2,  #xlli_MDREFR_K1DB2      ; Is the K1DB2 bit set?
        orrne   r0,  r0,  #xlli_BIT_12            ; Yes - set bit 12 in r0
        ands    r1,  r2,  #xlli_MDREFR_K0DB4      ; Is the K0DB4 bit set?
        orrne   r0,  r0,  #xlli_BIT_13            ; Yes - set bit 13 in r0
        ands    r1,  r2,  #xlli_MDREFR_K0DB2      ; Is the K0DB2 bit set?
        orrne   r0,  r0,  #xlli_BIT_14            ; Yes - set bit 14 in r0

        ldmfd   sp!, {r1, r2, pc}                 ; Restore r1, r2 and return to caller

        ENDFUNC

;**************************************************************************************************
;
; ******************************************************
; **********                                  **********
; ********** SET SDRAM MEMORY BUFFER STRENGTH **********
; **********   ( C Program callable version)  **********
; ******************************************************
;
; This subroutine is a "C" callable function that adjusts the buffer drive impedance for SDRAM
;
; REQUIRED PARAMETER:
;
;       r0 = buffer strength (Range of 0x0 to 0xF)
;
;       Default value is 0x5 on system boot. A higher value results in a lower
;       buffer impedance. This subroutine only changes buffer impedance that
;       affect SDRAM. See Developer's Manual (Memory Controller) for more details.
;

xlli_setBufImpedance  FUNCTION

        stmfd   sp!, {r1, r2, lr}                 ; Save r1, r2 and link register on the stack
        ldr     r1,  =xlli_MEMORY_CONFIG_BASE     ; Get memory controller base address
;
;       Duplicate the value in lowest nibble so it is duplicated in all 8 nibbels
;
        and     r0,  r0,  #0xF                    ; Isolate bits 3:0 of parameter
        mov     r2,  r0,  LSL #4                  ; Shift 4 bits to the left
        orr     r0,  r0,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r0,  r0,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r0,  r0,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r0,  r0,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r0,  r0,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r0,  r0,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r0,  r0,  r2                      ; Copy in next significant nibble
;
;       The 8 nibbles in r0 are now all the same value
;
        str     r0,  [r1, #xlli_BSCNTR3_offset]   ; Set buffer strength for address lines
        str     r0,  [r1, #xlli_BSCNTR0_offset]   ; Set buffer strength for SDCLK
        ldr     r2,  [r1, #xlli_BSCNTR1_offset]   ; Get buffer strength for BSCNTR1
        and     r2,  r2,  #0xF0                   ; Preserve bits 7:4 in r2
        bic     r0,  r0,  #0xF0                   ; Clear bits 7:4 in r0
        orr     r2,  r0,  r2                      ; OR with the value from BSCNTR1
        str     r2,  [r1, #xlli_BSCNTR1_offset]   ; Write buffer strength to BSCNTR1
        str     r0,  [r1, #xlli_BSCNTR2_offset]   ; Write buffer strength to BSCNTR2        
;
;       Read back from APB to ensure our writes have completed before continuing
;
        ldr     r0,  [r1, #xlli_BSCNTR2_offset]

        ldmfd   sp!, {r1, r2, pc}                 ; Restore r1, r2 and return to caller

        ENDFUNC
;**************************************************************************************************
;
; ******************************************************
; **********                                  **********
; ********** SET SDRAM MEMORY BUFFER STRENGTH **********
; **********       (ROM based version)        **********
; ******************************************************
;
; This subroutine adjusts the buffer drive impedance for SDRAM
;
; REQUIRED PARAMETER:
;
;       r3 = buffer strength (Range of 0x0 to 0xF)
;
;       Default value is 0x5 on system boot. A higher value results in a lower
;       buffer impedance. This subroutine only changes buffer impedance that
;       affect SDRAM. See Developer's Manual (Memory Controller) for more details.
;

xlli_setBufImp  FUNCTION

        ldr     r1,  =xlli_MEMORY_CONFIG_BASE     ; Get memory controller base address
;
;       Duplicate the value in lowest nibble so it is duplicated in all 8 nibbels
;
        and     r3,  r3,  #0xF                    ; Isolate bits 3:0 of parameter
        mov     r2,  r3                           ; Copy into r2
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r2,  r3,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r2,  r3,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r2,  r3,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r2,  r3,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r2,  r3,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r2,  r3,  r2                      ; Copy in next significant nibble
        mov     r2,  r2,  LSL #4                  ; Shift 4 bits to the left
        orr     r2,  r3,  r2                      ; Copy in next significant nibble
;
;       The 8 nibbles in r2 are now all the same value
;
        str     r2,  [r1, #xlli_BSCNTR3_offset]   ; Set buffer strength for address lines
        str     r2,  [r1, #xlli_BSCNTR0_offset]   ; Set buffer strength for SDCLK
        ldr     r3,  [r1, #xlli_BSCNTR1_offset]   ; Get buffer strength for BSCNTR1
        and     r3,  r3,  #0xF0                   ; Preserve bits 7:4 in r3
        bic     r2,  r2,  #0xF0                   ; Clear bits 7:4 in r2
        orr     r3,  r3,  r2                      ; OR with the value from BSCNTR1
        str     r3,  [r1, #xlli_BSCNTR1_offset]   ; Write buffer strength to BSCNTR1
        str     r2,  [r1, #xlli_BSCNTR2_offset]   ; Write buffer strength to BSCNTR2 
;
;       Read back from APB to ensure our writes have completed before continuing
;
        ldr     r0, [r1, #xlli_BSCNTR2_offset]

        mov     pc,  lr                           ; Return to calling routine

        ENDFUNC

        END



