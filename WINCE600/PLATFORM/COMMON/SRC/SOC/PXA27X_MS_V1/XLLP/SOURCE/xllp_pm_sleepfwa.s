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
;******************************************************************************
;
;  COPYRIGHT (C) 2002 Intel Corporation.
;
;  This software as well as the software described in it is furnished under 
;  license and may only be used or copied in accordance with the terms of the 
;  license. The information in this file is furnished for informational use 
;  only, is subject to change without notice, and should not be construed as 
;  a commitment by Intel Corporation. Intel Corporation assumes no 
;  responsibility or liability for any errors or inaccuracies that may appear 
;  in this document or any software that may be provided in association with 
;  this document.
; 
;  Except as permitted by such license, no part of this document may be 
;  reproduced, stored in a retrieval system, or transmitted in any form or by 
;  any means without the express written consent of Intel Corporation. 
;
;  FILENAME:       xllp_Pm_SleepFWA.s
;
;  PURPOSE:        Provides assembly level code for startup code support of
;                   context restoration after sleep
;                  for the Bulverde/Mainstone processor/platform.
;                  
;
;  LAST MODIFIED:  8/27/02
;******************************************************************************
;
; The functions in this source code are called via a branch with link instruction.
;
; NOTES:
;
; These functions assume that the MMU is enabled, except for any which receive all addresses
; as parameters.
;
        INCLUDE  xlli_Bulverde_defs.inc                 ; Bulverde specific include file
        INCLUDE  xlli_Mainstone_defs.inc                ; Mainstone specific include file
        INCLUDE  xllp_Pm_SleepContext.inc               ; Definitions for this file
    IF :DEF: XLLP_RESUME_DELAY
        INCLUDE  mainstone.mac                          ; Definitions for this file
    ENDIF  ; :DEF: XLLP_RESUME_DELAY
;
        AREA    |.text|, CODE, READONLY, ALIGN=5        ; Align =5 required for "ALIGN 32" to work
;
; Functions made public by this file:
;
        EXPORT XllpPmChecksumSleepDataPh
        EXPORT XllpPmValidateResumeFromSleep
        EXPORT XllpPmGoToContextRestoration

;**************************************************************************************************
;
;   XllpPmValidateResumeFromSleep  FUNCTION
; 
; Parameters:
;   R0: value of PSPR (Power Manager Scratch Pad Register) at reset.
;       - Assumed to be physical address of saved data area
;   R1: contains a condensed copy of the RCSR and PSSR. 
;       (RCSR (bits 15:0 in 15:0 of R1) + PSSR (bits 15:0 in 31:16 of R1)
;
; Return value: in R0.
;   0x00: Success.  Sleep and valid checksum in data area
;   0x01: Not a sleep reset.
;   0x02: Other reset in addition to sleep reset.  (This may eventually become a success case.)
;   0x03: Bad checksum for saved data area.
; 
; Assumes no stack, MMU disabled.
;       Already initialized:
;           GPIOs, memory controller
;
; Uses (destroys) R0,r1,r2,r3,r4,r5,r6
;

XllpPmValidateResumeFromSleep  FUNCTION

    tst r1, #xlli_RCSR_SMR
    moveq r0, #01 ; Not sleep, return with code.
    beq   XllpPmValidateResumeFromSleep_Lab1    ; lr not corrupted.
    ; Sleep detected.  Validate checksum.

    ; Save parameters for future use if successful
    mov r4, r0      ; PSPR, containing phys addr of save area
    mov r5, r1      ; condensed copy of the RCSR and PSSR.
    mov r6, lr

    add r0, r0, #SleepState_WordCount ; WordCount is first checksummable location
    ldr r1, [r0]                     ; Get actual wordcount to checksum
                                     ;  r0..r3 lost in checksum routine
    ldr r2, =XLLI_MAX_SLEEP_DATA_COUNT ; Sanity check
    cmp r1, r2
    movgt r0, #03 ; bad data size, so declare bad checksum.
	bgt	  XllpPmValidateResumeFromSleep_Lab1    ; lr not corrupted

    bl XllpPmChecksumSleepDataPh     ; Actual checksum in R0
    ldr r2, [r4, #SleepState_CHKSUM] ; Pre-sleep checksum

    mov     r1, r0
    cmp     r2, r0 
    mov     r0, #0                     ; Assume success
    movne   r0, #03                    ; Bad checksum code.

    mov     r1, r5      ; Restore value.  Will be needed later

    ; Need to restore link register
    mov lr, r6

XllpPmValidateResumeFromSleep_Lab1
  ; lr ready for return

    IF :DEF: Interworking
        IF Interworking :LOR: Thumbing
         bx  lr
        ELSE
         mov  pc, lr          ; return
        ENDIF ; ELSE of IF Interworking :LOR: Thumbing
    ELSE ; IF :DEF: Interworking
         mov  pc, lr          ; return
    ENDIF ; ELSE OF IF :DEF: Interworking

    ENDFUNC ; 

;; End of XllpPmValidateResumeFromSleep()
;**************************************************************************************************


;**************************************************************************************************
;
; XllpPmGoToContextRestoration  FUNCTION
;
; Restores pre-sleep MMU configuration and jumps to stored address of context 
;   restoration function
;
; Inputs: 
;   R0:  phys addr of sleep save area
;   R1:  condensed copy of the RCSR and PSSR as needed by 
;         context restoration code
;
;  Does not return, treat as void Fn ()
;
; Assumes: MMU disabled, validated data save area for MMU values
;

XllpPmGoToContextRestoration  FUNCTION

;    mov  r2, #20  ; all interesting data in phase 1.
;    bl XllpPmDumpMemLEDs
    ;    r0: contains start address
    ;    r2: contains count
    ;    uses: r2,r3,r4,r5,r6

    ldr r11, [r0, #SleepState_Cp15_DACR_MMU]   ; load the MMU domain access info
    ldr r9,  [r0, #SleepState_Cp15_TTBR_MMU]      ; load the MMU TTB info 
    ldr r8,  [r0, #SleepState_Cp15_ACR_MMU]      ; load the MMU control info 
    ldr r7,  [r0, #SleepState_Cp15_AUXCR_MMU ]  ; 
    ldr r6,  [r0, #SleepState_Cp15_PID_MMU ] ; 
    ldr r5,  [r0, #SleepState_AwakeAddr ]   ; load the LR address

    IF :DEF: XLLP_RESUME_DELAY
        ldr         r3, =FPGA_REGS_BASE_PHYSICAL         
        ldr         r4, =0x88801111
        setHexLED   r3, r4
        ldr     r4, =0x10;
XllpPmGoToContextRestoration_Lab1
        subs    r4, r4, #1
        bne     XllpPmGoToContextRestoration_Lab1
    ENDIF ; :DEF: XLLP_RESUME_DELAY

    mcr p15, 0, r11, c3, c0, 0      ; setup access to domain 0
    mcr p15, 0, r9,  c2, c0, 0      ; TTB address
    mcr p15, 0, r2,  c8, c7, 0      ; Invalidate I+D TLBs
    ldr r0, [r0, #SleepState_SleepDataAreaVA]

    b   XllpPmGoToContextRestoration_Lab2          ; Make sure everything is in the cache
    ALIGN 32
XllpPmGoToContextRestoration_Lab2
    mcr p15, 0, r8,  c1, c0, 0      ; restore MMU control
    mcr p15, 0, r7,  c1, c1, 0      ; restore MMU Aux control
    mcr p15, 0, r6, c13, c0, 0      ; restore PID

    mov  pc, r5                     ;  & jump to new virtual address (back up Power management stack)
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    ENDFUNC 

;; End of XllpPmGoToContextRestoration()
;**************************************************************************************************


;**************************************************************************************************
;
; XllpPmChecksumSleepDataPh
;
; Does not assume a stack or MMU enable/disable state.
; 
; Inputs: 
;       R0: Base address of area to checksum
;       R1: Number of 4-byte words in area
;       Assumes return address in link register
;
; Returns:
;       R0: Checksum
;
; Uses: R0, R1, R2, R3
;
; Note: must be identical with XllpPmChecksumSleepDataVi
;


XllpPmChecksumSleepDataPh  FUNCTION

    ; do Checksum on the buffer

    ldr     r2, =(0x5A72)                  ; Pick a non-zero seed.
XllpPmChecksumSleepDataPh_Lab1
    ldr     r3, [r0], #4                   ; get value and increment pointer
    add     r2, r2,   r3
    mov     r2, r2,   ROR #31              ; Rotate left by one bit position
    subs    r1, r1,   #1                   ; Count down for entire buffer size
    bne     XllpPmChecksumSleepDataPh_Lab1
    mov     r0, r2

    IF :DEF: Interworking
        IF Interworking :LOR: Thumbing
         bx  lr
        ELSE
         mov  pc, lr          ; return
        ENDIF ; ELSE of IF Interworking :LOR: Thumbing
    ELSE ; IF :DEF: Interworking
         mov  pc, lr          ; return
    ENDIF ; ELSE OF IF :DEF: Interworking

    ENDFUNC

;; End of XllpPmChecksumSleepDataPh ()
;**************************************************************************************************

    IF 0=1

;**************************************************************************************************

XllpPmDumpMemLEDs   FUNCTION
;    r0: contains start address
;    r2: contains count
;    uses: r2,r3,r4,r5,r6

; Assume slower loops because of flash execution.
    mov r4, r0

    ldr      r3, =FPGA_REGS_BASE_PHYSICAL         

XllpPmDumpMemLEDs_LabOuter

    ldr r5, [r4]
    setHexLED   r3, r5
    ldr     r6, =0x10000000;
XllpPmDumpMemLEDs_LabInner

    subs    r6, r6, #1
    bne     XllpPmDumpMemLEDs_LabInner

;; cjw.  Add separator in case of duplicate data.

    ldr r5, =(0x77777777)
    setHexLED   r3, r5
    ldr     r6, =0x100000;
XllpPmDumpMemLEDs_LabInner2

    subs    r6, r6, #1
    bne     XllpPmDumpMemLEDs_LabInner2


    add     r4, r4, #4
    subs    r2, r2, #1
    bne     XllpPmDumpMemLEDs_LabOuter

    IF :DEF: Interworking
        IF Interworking :LOR: Thumbing
         bx  lr
        ELSE
         mov  pc, lr          ; return
        ENDIF ; ELSE of IF Interworking :LOR: Thumbing
    ELSE ; IF :DEF: Interworking
         mov  pc, lr          ; return
    ENDIF ; ELSE OF IF :DEF: Interworking

    ENDFUNC

;; end of XllpPmDumpMemLEDs
;**************************************************************************************************

    ENDIF ; 0

    END
  
