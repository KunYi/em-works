;------------------------------------------------------------------------------
;
;   Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
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
    LEAF_ENTRY EnableIrqInterrupt

    MRS     r2,CPSR            ;Save Current Program Status Register.
    bic   r1,r2,#0x80        ;Enable IRQ if set.
    MSR     CPSR_c,r1

    ENTRY_END  

    LEAF_ENTRY DisableIrqInterrupt

    MRS     r2,CPSR            ;Save Current Program Status Register.
    orr      r1,r2,#0x80        ;Disable IRQ if clear
    MSR     CPSR_c,r1

    ENTRY_END

   
    END
