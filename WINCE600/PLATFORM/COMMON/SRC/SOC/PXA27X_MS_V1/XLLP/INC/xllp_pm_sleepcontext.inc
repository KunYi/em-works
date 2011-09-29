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
;******************************************************************************
;;
;;  COPYRIGHT (C) 2002 Intel Corporation.
;;
;;  This software as well as the software described in it is furnished under 
;;  license and may only be used or copied in accordance with the terms of the 
;;  license. The information in this file is furnished for informational use 
;;  only, is subject to change without notice, and should not be construed as 
;;  a commitment by Intel Corporation. Intel Corporation assumes no 
;;  responsibility or liability for any errors or inaccuracies that may appear 
;;  in this document or any software that may be provided in association with 
;;  this document. 
;;  Except as permitted by such license, no part of this document may be 
;;  reproduced, stored in a retrieval system, or transmitted in any form or by 
;;  any means without the express written consent of Intel Corporation. 
;;
;;  FILENAME:       xllp_Pm_SleepContext.inc
;;
;; LAST MODIFIED:   5/28/02
;;
;******************************************************************************
;
;
; Include file for Bulverde Processor based platforms
;  NOTE: This file must be kept in sync with xllp_Pm_SleepContext.h
;
    IF  :DEF: xllp_Pm_SleepContext_inc
    ELSE ; IF  :DEF: xllp_Pm_SleepContext_inc
xllp_Pm_SleepContext_inc    EQU 1


; For sanity check in checksum
XLLI_MAX_SLEEP_DATA_COUNT  EQU  0x200

XLLP_STORE_WMX_REGS_BYTES   EQU 156

WORD_SIZE                   EQU (4)

; Template that defines the ASM-addressable part of the 
; XLLP_PM_ENTER_SLEEP_PARAMS_S structure used for the sleep parameter.
; Note that the ASM-addressable section begins at the start of the structure.

; /////////////////////////////////////////////////////////////////////////
; //
; // Definition of parameter structure needed by XllpPmEnterSleep()
; //
; // Note: keep this in sync with XLLP_PM_ENTER_SLEEP_PARAMS_S defined in xllp_pm_sleepcontext.h
; 
; typedef struct  XLLP_PM_ENTER_SLEEP_PARAMS_S
SleepParams_Data_Start              EQU  0 
SleepParams_SleepDataAreaPA         EQU (SleepParams_Data_Start)
SleepParams_SleepDataAreaVA         EQU (SleepParams_SleepDataAreaPA       + WORD_SIZE)
SleepParams_PWRMODE                 EQU (SleepParams_SleepDataAreaVA        + WORD_SIZE)
SleepParams_EndOfInterestingArea    EQU (SleepParams_PWRMODE                 +WORD_SIZE)

; End of template for ASM use.  C-level usage extends beyond this.
; Don't base any other symbols or addressing on "SleepParams_" symbols

;=================================================================
;=================================================================


; Template that defines the ASM-addressable part of the 
; XLLP_PM_SLEEP_SAVE_DATA_S structure used for the sleep saved data area.
; Note that the ASM-addressable section begins at the start of the structure.
; Note: keep this in sync with XLLP_PM_SLEEP_SAVE_DATA_S defined in xllp_pm_sleepcontext.h

;typedef struct  XLLP_PM_SLEEP_SAVE_DATA_S
SleepState_Data_Start           EQU 0 
SleepState_CHKSUM               EQU ( SleepState_Data_Start )
SleepState_WordCount            EQU ( SleepState_CHKSUM          + WORD_SIZE )
SleepState_AwakeAddr            EQU ( SleepState_WordCount       + WORD_SIZE )
SleepState_SleepDataAreaVA      EQU ( SleepState_AwakeAddr       + WORD_SIZE )
SleepState_SleepParamVA         EQU ( SleepState_SleepDataAreaVA + WORD_SIZE )

; // ARM* registers and most coprocessors must be processed locally
; //  using assembly language.
; // ARM* unique registers
; 
SleepState_ENTRY_CPSR           EQU ( SleepState_SleepParamVA  + WORD_SIZE )
SleepState_ENTRY_SP             EQU ( SleepState_ENTRY_CPSR  + WORD_SIZE )
SleepState_ENTRY_SPSR           EQU ( SleepState_ENTRY_SP    + WORD_SIZE )
SleepState_SYS_SP               EQU ( SleepState_ENTRY_SPSR  + WORD_SIZE )
SleepState_FIQ_SP               EQU ( SleepState_SYS_SP      + WORD_SIZE )
SleepState_ABT_SP               EQU ( SleepState_FIQ_SP      + WORD_SIZE )
SleepState_IRQ_SP               EQU ( SleepState_ABT_SP      + WORD_SIZE )
SleepState_UND_SP               EQU ( SleepState_IRQ_SP      + WORD_SIZE )
SleepState_SVC_SP               EQU ( SleepState_UND_SP      + WORD_SIZE )

; // Saved Coprocessor regs other than CP0+1 (includes MMU values)
SleepState_Cp15_ACR_MMU         EQU ( SleepState_SVC_SP         + WORD_SIZE )
SleepState_Cp15_AUXCR_MMU       EQU ( SleepState_Cp15_ACR_MMU   + WORD_SIZE )
SleepState_Cp15_TTBR_MMU        EQU ( SleepState_Cp15_AUXCR_MMU + WORD_SIZE )
SleepState_Cp15_DACR_MMU        EQU ( SleepState_Cp15_TTBR_MMU  + WORD_SIZE )
SleepState_Cp15_PID_MMU         EQU ( SleepState_Cp15_DACR_MMU  + WORD_SIZE )
SleepState_Cp15_CPAR            EQU (SleepState_Cp15_PID_MMU    + WORD_SIZE )
SleepState_EndOfInterestingArea EQU ( SleepState_Cp15_CPAR)

; End of template for ASM use.  C-level usage extends beyond this.
; Don't base any other symbols or addressing on "SleepState_" symbols



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  Some important constants that aren't defined elsewhere
;;
;;
;; Intel® XScale Core Developer’s Manual December, 2000
;;

; AND masks: all zero bits are read-as-0, write-as-0.

XLLP_VLD_MSK_ALL             EQU    (0xFFFFFFFF)

    ; Bits 6:3 must be ones when written, but they are also read as ones -
    ;  so they are treated as simply valid for save/restore purposes.
XLLP_CP15_ACR_MMU_VLD_MSK    EQU    (0x00003BFF) ;CP15, CRn1, opcode_2 = 0
    ; P-bit (bit 1) not implemented for this processor
XLLP_CP15_AUXCR_MMU_VLD_MSK  EQU    (0x00000031)  ;CP15, CRn1, opcode_2 = 1
XLLP_CP15_TTBR_MMU_VLD_MSK   EQU    (0xFFFFC000)  ;CP15, CRn2
XLLP_CP15_DACR_MMU_VLD_MSK   EQU    XLLP_VLD_MSK_ALL
XLLP_CP15_PID_MMU_VLD_MSK    EQU    (0xFE000000) ; p15, 0, Rd, c13, c0, 0
XLLP_CP15_CPAR_VLD_MSK       EQU    (0x00003FFF) ; Cp15, CR15; CRm1, opcode_2=0


        ENDIF ; ELSE ; IF  :DEF: xllp_Pm_SleepContext_inc

    END