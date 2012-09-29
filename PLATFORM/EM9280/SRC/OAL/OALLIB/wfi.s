;------------------------------------------------------------------------------
;
;   Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;  Module: wfi.s
;
;  This module implements the OAL assembly-level support for entering the ARM
;  wait-for-interrupt low-power mode.
;
;------------------------------------------------------------------------------
    INCLUDE kxarm.h

;
; ARM constants
;
ARM_CTRL_ICACHE             EQU     (1 << 12)
ARM_CTRL_DCACHE             EQU     (1 << 2)

    TEXTAREA

;------------------------------------------------------------------------------
;
;  Function: OALCPUEnterWFI
;
;  This function provides the instruction sequence for requesting the ARM CPU 
;  to enter the WFI (wait-for-interrupt).  This routine will be called by
;  the OALCPUIdle and OEMPowerOff.  
;
;  Parameters:
;      None.
;
;  Returns:
;      None.
;
;------------------------------------------------------------------------------
    LEAF_ENTRY OALCPUEnterWFI

    mov     r0, #0
    mrc     p15, 0, r0, c1, c0, 0               ; Read system control reg
    bic     r0, r0, #ARM_CTRL_ICACHE            ; Disable I-cache
    bic     r0, r0, #ARM_CTRL_DCACHE            ; Disable D-cache
    mcr     p15, 0, r0, c1, c0, 0               ; Update system control reg

    mov     r0,#0
    mcr     p15, 0, r0, c7, c0, 4               ; Enter WFI

    nop
    nop
    nop
    nop

    mrc     p15, 0, r0, c1, c0, 0               ; Read system control reg
    orr     r0, r0, #ARM_CTRL_ICACHE            ; Enable I-cache
    orr     r0, r0, #ARM_CTRL_DCACHE            ; Enable D-cache
    mcr     p15, 0, r0, c1, c0, 0               ; Update system control reg
    mov     pc, lr

    ENTRY_END  
    
    END
