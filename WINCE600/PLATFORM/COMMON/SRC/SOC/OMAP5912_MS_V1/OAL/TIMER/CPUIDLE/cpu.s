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
; Portions Copyright (c) Texas Instruments.  All rights reserved. 
;
;------------------------------------------------------------------------------

        INCLUDE kxarm.h
        INCLUDE omap5912_emif.inc

        EXPORT OALCPUIdle
        IMPORT g_pOALEMIFRegs
        
        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALCPUIdle
;
;
;  This routine is called by the kernel when there are no threads ready to
;  run. The CPU should be put into a reduced power mode and halted. It is
;  important to be able to resume execution quickly upon receiving an interrupt.
;
;
 
        ALIGN   1024                    ; Align on 1KB boundary to avoid
                                        ; ARM926EJ-S WFI bug

        LEAF_ENTRY OALCPUIdle

        mov     r0, #0
        mcr     p15, 0, r0, c7, c10, 4

        ; Prefetch sleep command         
        add     r0, pc, #(SLEEP - . + 8)
        mcr     p15, 0, r0, c7, c13, 1
        
        ; Set SDRAM to self-refresh mode
        ldr     r0, =g_pOALEMIFRegs
        ldr     r0, [r0]
        ldr     r1, [r0, #OMAP5912_EMIF_EMIFF_CONFIG_REGS_OA]
        orr     r1, r1, #(1 :SHL: 0)
        str     r1, [r0, #OMAP5912_EMIF_EMIFF_CONFIG_REGS_OA]

        ; Write Sleep Command
SLEEP   mcr     p15, 0, r0, c7, c0, 4

        nop
        nop
        nop
        nop
        
        mov pc, lr
 
        ENTRY_END

        END

;-------------------------------------------------------------------------------
