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
;------------------------------------------------------------------------------
;
;  File:  omap2420_intc.inc

OMAP2420_INTC_L1_REGS_PA        EQU     0xFFFECB00
OMAP2420_INTC_L2A_REGS_PA       EQU     0xFFFE0000 ;+KHwBaseInthLevel2Section1
OMAP2420_INTC_L2B_REGS_PA       EQU     0xFFFE0100
OMAP2420_INTC_L2C_REGS_PA       EQU     0xFFFE0200
OMAP2420_INTC_L2D_REGS_PA       EQU     0xFFFE0300

OMAP2420_INTC_ITR_REGS_OA		EQU     0x00
OMAP2420_INTC_MIR_REGS_OA		EQU     0x04
OMAP2420_INTC_SIR_IRQ_REGS_OA	EQU     0x10
OMAP2420_INTC_SIR_FIQ_REGS_OA	EQU     0x14
OMAP2420_INTC_CNTL_REGS_OA		EQU     0x18
OMAP2420_INTC_ILR0_REGS_OA		EQU     0x1C
OMAP2420_INTC_SIR_REGS_OA		EQU     0x9C
OMAP2420_INTC_ENH_CNTL_REGS_OA	EQU     0xA0
OMAP2420_INTC_GMR_REGS_OA		EQU     0xA0
OMAP2420_INTC_STATUS_REGS_OA	EQU     0xA0
OMAP2420_INTC_OCP_CFG_REGS_OA	EQU     0xA4
OMAP2420_INTC_INTH_REV_REGS_OA	EQU     0xA8

;------------------------------------------------------------------------------

                                        END
