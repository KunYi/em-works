;------------------------------------------------------------------------------
;
;   Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
    INCLUDE armmacros.s

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
    mov     r0,#0
    mcr     p15, 0, r0, c7, c0, 4               ; Enter WFI

    RETURN

    END
