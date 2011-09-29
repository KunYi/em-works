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
;++
;
; Module Name:
;
;    startup.s
;
; Abstract:
;
;    This module implements Bulverde initialization code.  It's responsible
;    for setting up the Bulverde core.  Board-level initialization is done
;    in OEM-specific code.
;
; Environment:
;
; Revision History:
;
; Notes:
;
;  Register Useage:  r10 is used to hold the contents of the RCSR throughout 
;                    this module.  The rest of the registers are fair game.
;
;--
;
;-------------------------------------------------------------------------------
; Copyright 2000-2003 Intel Corporation All Rights Reserved.
;
; Portions of the source code contained or described herein and all documents
; related to such source code (Material) are owned by Intel Corporation
; or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
; Title to the Material remains with Intel Corporation or its suppliers and licensors. 
; Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
; No other license under any patent, copyright, trade secret or other intellectual
; property right is granted to or conferred upon you by disclosure or
; delivery of the Materials, either expressly, by implication, inducement,
; estoppel or otherwise 
; Some portion of the Materials may be copyrighted by Microsoft Corporation.
;
;-------------------------------------------------------------------------------
;

    ; Disable listing
    OPT    2

    INCLUDE kxarm.h
    INCLUDE bulverde.inc
    INCLUDE bulverde_macros.inc
    INCLUDE xlli_bulverde_defs.inc
    INCLUDE xllp_pm_sleepcontext.inc
   
    ; Re-enable listing 
    OPT 1
  
    ; PQOAL BSP imports 
    IMPORT OALStartUp 
    IMPORT OALXScaleSetFrequencies
   
    ; XLLI/XLLP imports
    IMPORT xlli_mem_init
    IMPORT xlli_intr_init
    IMPORT xlli_mem_Tmax
    IMPORT xlli_mem_Topt
    IMPORT xlli_mem_restart
    IMPORT xlli_ost_init
    IMPORT xlli_pwrmgr_init
    IMPORT xlli_IMpwr_init
    IMPORT XllpPmValidateResumeFromSleep
    IMPORT XllpPmGoToContextRestoration
    IMPORT xlli_GPIO_init
    IMPORT xlli_setClocks
    IMPORT xlli_freq_change
   
 
    STARTUPTEXT


;*******************************************************************************
;
;   StartUp() is the entry point on Reset (all forms of Reset)
;
;   Desription:  StartUp is the first routine executed when powering on
;       the system.  It is also executed first after all forms of XScale
;       resets.  The code is shared between the bootloader and the CE kernel.
;
;       Regardless of the build type, we disable the MMU and caches
;       immediately and flush them.
;
;*******************************************************************************

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Note 1:  Normally, we'd have our exception vectors here, but the linker 
;   currently insert a 4K "jump page" at the start of the image.  Therefore, 
;   we do NOT have control of the vector code.  They insert a branch to 
;   StartUp at physical address 0.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  Note 2: - The MMU assumed to be inactive at this time so physical addresses
;   should be used.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    LEAF_ENTRY StartUp

    ; Perform pre-initialization (enter supervisor mode, disable MMU and caches,
    ; and determine the reason for the reset.
    ;
    bl      PreInit

    ; r10 now contains the contents of the power manager registers RCSR in the 
    ; lower half and PSSR in the upper half.  If we're in this routine because
    ; of a hardware/power-on reset, then we need to continue in this routine and
    ; initialize all hardware.  Otherwise, we'll assume the hardware's already
    ; been initialized and we can skip.
    ;

    ;jump directly to OALstartup if this was not a reset
    tst     r10, #(RCSR_HARD_RESET :OR: RCSR_WDOG_RESET :OR: RCSR_SLEEP_RESET :OR: RCSR_GPIO_RESET)
    beq     OALStartUp
   
    ; If we're here because of a GPIO reset, skip the memory controller
    ; initialization because all memory registers (except for configuration
    ; registers are maintained across the reboot).
    ;
    tst     r10, #RCSR_GPIO_RESET
    bne     Continue_StartUp

    ; Initialize the Bulverde memory controller.
    ;
    bl      xlli_mem_init

    ; If we're here because of a hardware reset then skip sleep reset check
    tst     r10, #RCSR_HARD_RESET
    bne     Continue_StartUp

    ; We may be here because of a sleep reset.  Try to resume from the sleep
    ; state.  At this point, it may be a watchdog reset or a sleep/software reset.
    ; 
    ldr     r0, =xlli_PMRCREGS_PHYSICAL_BASE   ; read the PSPR register
    ldr     r0, [r0, #xlli_PSPR_offset]        ; 
    mov     r1, r10                            ; packed RCSR+PSSR required in r1
    bl      XllpPmValidateResumeFromSleep      ; r0-r6 are lost
    cmp     r0, #0                             ; zero return: OK to restore
    bne     Failed_Sleep_Resume                ; treat as a full-init reset
       
    ; At this point, the reset could have been caused by a resume from sleep
    ; or by a software reset.  For now, assume it was a sleep-based reset.
    ;
    ; TODO: handling a software reset requires use of a flag - where to put it?
    ;
    b       Sleep_Reset_Not_SoftReset
    ;
Failed_Sleep_Resume

    ; Make sure sleep-mediated reset flag is cleared (won't be set if we're here 
    ; because of a POR, hardware, or watchdog reset.
    ;
    ldr     r1, =xlli_RCSR_SMR
    bic     r10, r10, r1
    
Continue_StartUp

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Common startup code.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; Initialize the interrupt controller.
    ; 
    bl      xlli_intr_init

    ; Enable core clocks (don't touch the RTC).
    ;
    bl      EnableClks

    ; Set maximum memory controller values, then reset the controller for them
    ; to take effect.  If we're here because of a GPIO reset, these are the
    ; configuration values that didn't persist across the reboot.
    ;
    bl      xlli_mem_Tmax
    bl      xlli_mem_restart

    ; Use OEM-selected clock frequencies this is a function call into OEM code
    ; which may need to abstract board-specific GPIO switch reads, etc. to 
    ; select clock frequencies.
    ;
    bl      OALXScaleSetFrequencies
    
    ; Set optimal memory controller values (based on the memory clock frequency
    ; chosen above, then reset the controller for them to take effect.
    ;
    bl      xlli_mem_Topt
    bl      xlli_mem_restart

    ; Initialize the OS timers.
    ;
    bl      xlli_ost_init

    ; Initialize the power manager.
    ;
    bl      xlli_pwrmgr_init

    ; Power-up internal memory.
    ;
    bl      xlli_IMpwr_init

    ; Jump to OAL startup code.
    ;
    b       OALStartUp

Sleep_Reset_Not_SoftReset

    ;Initialize the clocks with the following XLLI sequence
    bl      xlli_mem_Tmax
    bl      xlli_mem_restart

    bl      xlli_GPIO_init

    bl      xlli_setClocks ; will poke CCCR so must do Frequency Change
    bl      xlli_freq_change

    bl      xlli_mem_Topt
    bl      xlli_mem_restart

    ldr     r0,  =xlli_PMRCREGS_PHYSICAL_BASE   ; Get the Power Manager base address
    ldr     r0,  [r0, #xlli_PSPR_offset]        ; Phys. addr of save data
    mov     r1,  r10                            ; Packed RCSR+PSSR
    b       XllpPmGoToContextRestoration        ; Never returns.

    ; if it returns jump to Failed Reset
    b       Failed_Sleep_Resume
    
    ENTRY_END
;-------------------------------------------------------------------------------

    LTORG                           ; insert a literal pool here.
    
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;
; PreInit: perform CPU pre-initialization tasks (enter supervisor mode, disable
;          MMUs and caches, determine reset reason, etc.
;
; Inputs: r10 contains an optional flag - TBD
; 
; On return: r10 has RCSR in lower half and PSSR in upper half
;
; Register used: r0-r2, r10, r12
;
;-------------------------------------------------------------------------------
;
    ALIGN
PreInit

    ; Put the CPU in Supervisor mode (SVC) and disable IRQ and FIQ interrupts.
    ;
    ldr     r0, =(Mode_SVC :OR: NoIntsMask)
    msr     cpsr_c, r0

    ; Disable the MMU, caches, and write-buffer and flush.
    ;
    ldr     r0, =0x2043             ; enable access to all coprocessors
    mcr     p15, 0, r0, c15, c1, 0  ;
    CPWAIT  r0                      ;

    ldr     r0, =0x00000078         ; get a zero to turn things off (must write bits[6:3] as 1s)
    mcr     p15, 0, r0, c1, c0, 0   ; turn off MMU, I&D caches, and write buffer 
    CPWAIT  r0                      ;

    ;ldr     r0, =0x00000000         ; get a zero to turn things off
    ;mcr     p15, 0, r0, c8, c7, 0   ; flush (invalidate) I/D TLBs
    ;mcr     p15, 0, r0, c7, c7, 0   ; flush (invalidate) I/D caches
    ;mcr     p15, 0, r0, c7, c10, 4  ; drain the write buffer
    ;nop                             ;
    ;nop                             ;
    ;nop                             ;

    mvn     r0, #0                  ; grant manager access to all domains
    mcr     p15, 0, r0, c3, c0, 0   ;

    ; Read the reset cause bits in RCSR.
    ;
    ldr     r0,  =BULVERDE_BASE_REG_PA_PWR
    ldr     r10, [r0, #RCSR_OFFSET]

    ; Extract the reset cause bits.
    ;
    mov     r2,  #RCSR_ALL          ; Mask RCSR
    and     r10,  r10,  r2          ; r10 now holds the conditioned Reset Reason

    ; Clear the reset cause bits (they're sticky).
    ;
    str     r2,  [r0, #RCSR_OFFSET]

    ; Read and store PSSR in the upper half of r10.
    ;
    ldr     r12, [r0, #PSSR_OFFSET]
    mov     r2,   #PSSR_VALID_MASK  ; mask PSSR (all in lower byte)
    and     r12,  r12,  r2          ; r12 now holds the conditioned PSSR
    mov     r12,  r12,  lsl #16     ; move to upper half of register
    orr     r10,  r10,  r12         ; r10 now has RCSR in lower half and PSSR in upper

    ; Enable data aborts for VDD and BATT faults (alternative is to use interrupt)
    ;
    mov     r1, #(PMCR_BIDAE:OR:PMCR_VIDAE)     ; Enable imprecise data aborts on VDD and BATT faults.
    str     r1, [r0, #PMCR_OFFSET]

  IF Interworking :LOR: Thumbing
    bx  lr
  ELSE
    mov  pc, lr          ; return to caller.
  ENDIF

    
;-------------------------------------------------------------------------------

    LTORG                           ; insert a literal pool here.
    
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;
; EnableClks: Enable the Bulverde core clocks/timers.
;
; Inputs: None.
; 
; On return: N/A.
;
; Register used: r1, r2
;
;-------------------------------------------------------------------------------
;
    ALIGN
EnableClks
    ; Enable only the clocks that are necessary for startup (memory controller
    ; and OS timer clocks).
    ;
    ldr     r1, =BULVERDE_BASE_REG_PA_CLKMGR
    ldr     r2, =CKEN_DEFAULT
    str     r2, [r1, #CKEN_OFFSET]

  IF Interworking :LOR: Thumbing
    bx  lr
  ELSE
    mov  pc, lr          ; return to caller.
  ENDIF

    
;-------------------------------------------------------------------------------

    LTORG                           ; insert a literal pool here.

    END

