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
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
;

        INCLUDE kxarm.h

        TEXTAREA
        

;-------------------------------------------------------------------------------
;
; void SaveVfpCtrlRegs(LPDWORD lpExtra, int nMaxRegs)
;
; Saves the FPINST and FPINST2 VFP system registers (depending on the FPEXC bits)
;
        LEAF_ENTRY SaveVfpCtrlRegs

        FMRX    r12, FPEXC

        TST     r12, #(1 << 31)         ; check the FPEXC.EX bit
        BEQ     skipFPINST
        MRC     p10, 7, r2, c9, c0, 0   ; FMRX r2, FPINST

skipFPINST
        TST     r12, #(1 << 28)         ; check the FPEXC.FP2V bit
        BEQ     skipFPINST2
        MRC     p10, 7, r3, c10, c0, 0  ; FMRX r3, FPINST2

skipFPINST2
        STMIA   r0, {r2, r3}
        bx      lr
        
        ENTRY_END SaveVfpCtrlRegs
        

;-------------------------------------------------------------------------------
;
; void RestoreVfpCtrlRegs(LPDWORD lpExtra, int nMaxRegs)
;
; Restores both FPINST and FPINST2 VFP system registers
;
        LEAF_ENTRY RestoreVfpCtrlRegs

        ; This functions is not currently used, so DebugBreak here
        ;
        DCD     0xE6000010

        LDMIA   r0, {r2, r3}
        
        MCR     p10, 7, r2, c9, c0, 0   ; FMXR FPINST, r2
        MCR     p10, 7, r3, c10, c0, 0  ; FMXR FPINST2, r3
        
        bx      lr
        
        ENTRY_END RestoreVfpCtrlRegs


;-------------------------------------------------------------------------------
;
; DWORD VfpReadFpsid(void)
;
; Returns the value of the FPSID register
;
        LEAF_ENTRY VfpReadFpsid

        fmrx    r0, fpsid
        bx      lr
        
        ENTRY_END VfpReadFpsid


;-------------------------------------------------------------------------------
;
; DWORD VfpReadMVFR(int index)
;
; Returns the value of the MVFRx registers (VFPv3 and later)
;
        LEAF_ENTRY VfpReadMVFR

        cmp     r0, #0
        DCD     0x0EF70A10  ; VMRSEQ r0, MVFR0
        DCD     0x1EF60A10  ; VMRSNE r0, MVFR1
        bx      lr
        
        ENTRY_END VfpReadMVFR
        

;-------------------------------------------------------------------------------
;
; void VfpEnableCoproc(void)
;
; Enables the VFP/Neon coprocessor bits in p15's CACR register
;
        LEAF_ENTRY VfpEnableCoproc

        ; set CACR[23-20]
        mrc     p15, 0, r0, c1, c0, 2
        orr     r0, r0, #(0xF :SHL: 20)
        mcr     p15, 0, r0, c1, c0, 2
        
        bx      lr
        
        ENTRY_END VfpEnableCoproc
        

;------------------------------------------------------------------------------

        END



