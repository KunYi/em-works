;;
;; Copyright (c) Microsoft Corporation.  All rights reserved.
;;
;;
;; Use of this source code is subject to the terms of the Microsoft end-user
;; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
;; If you did not accept the terms of the EULA, you are not authorized to use
;; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
;; install media.
;;
;;------------------------------------------------------------------------------
;;
;; Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
;; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;;
;;------------------------------------------------------------------------------
;;
;; Header:  mx25_crm.inc
;;
;;  This header file defines the Physical Addresses (PA) of
;;  the base registers for the System on Chip (SoC) components.
;;
;; Provides definitions for PLL, Clock & Reset controller module based on MX25.
;;
;;------------------------------------------------------------------------------


;;------------------------------------------------------------------------------
;; GENERAL MODULE CONSTANTS
;;------------------------------------------------------------------------------
CRM_CGR_CG_MASK         EQU         0x1
CRM_PCDR_DIV_MASK       EQU         0x3F

;;------------------------------------------------------------------------------
;; REGISTER LAYOUT
;;------------------------------------------------------------------------------


;;------------------------------------------------------------------------------
;; REGISTER OFFSETS
;;------------------------------------------------------------------------------
CRM_MPCTL_OFFSET        EQU         0x00    ;;MCU PLL Control Register
CRM_UPCTL_OFFSET        EQU         0x04    ;;USB PLL Control Register
CRM_CCTL_OFFSET         EQU         0x08    ;;Clock Control Register
CRM_CGCR0_OFFSET        EQU         0x0C    ;;Clock Gating Control Register 0
CRM_CGCR1_OFFSET        EQU         0x10    ;;Clock Gating Control Register 1
CRM_CGCR2_OFFSET        EQU         0x14    ;;Clock Gating Control Register 2
CRM_PCDR0_OFFSET        EQU         0x18    ;;Per Clock Divider Register 0
CRM_PCDR1_OFFSET        EQU         0x1C    ;;Per Clock Divider Register 1
CRM_PCDR2_OFFSET        EQU         0x20    ;;Per Clock Divider Register 2
CRM_PCDR3_OFFSET        EQU         0x24    ;;Per Clock Divider Register 3
CRM_RCSR_OFFSET         EQU         0x28    ;;CRM Status Register
CRM_CRDR_OFFSET         EQU         0x2C    ;;CRM Debug Register
CRM_DCVR0_OFFSET        EQU         0x30    ;;DPTC Comparator Value Registers
CRM_DCVR1_OFFSET        EQU         0x34    ;;DPTC Comparator Value Registers
CRM_DCVR2_OFFSET        EQU         0x38    ;;DPTC Comparator Value Registers
CRM_DCVR3_OFFSET        EQU         0x3C    ;;DPTC Comparator Value Registers
CRM_LTR0_OFFSET         EQU         0x40    ;;Load Tracking Register 0
CRM_LTR1_OFFSET         EQU         0x44    ;;Load Tracking Register 1
CRM_LTR2_OFFSET         EQU         0x48    ;;Load Tracking Register 2
CRM_LTR3_OFFSET         EQU         0x4C    ;;Load Tracking Register 3
CRM_LTBR0_OFFSET        EQU         0x50    ;;Load Tracking Buffer Register 0
CRM_LTBR1_OFFSET        EQU         0x54    ;;Load Tracking Buffer Register 1
CRM_PMCR0_OFFSET        EQU         0x58    ;;Power Management Control Register 0
CRM_PMCR1_OFFSET        EQU         0x5C    ;;Power Management Control Register 1
CRM_PMCR2_OFFSET        EQU         0x60    ;;Power Management Control Register 2
CRM_MCR_OFFSET          EQU         0x64    ;;Misc Control Register
CRM_LPIMR0_OFFSET       EQU         0x68    ;;Low Power Interrupt Mask Registers
CRM_LPIMR1_OFFSET       EQU         0x6C    ;;Low Power Interrupt Mask Registers


;;------------------------------------------------------------------------------
;; REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
;;------------------------------------------------------------------------------




;;------------------------------------------------------------------------------
;; REGISTER BIT FIELD WIDTHS
;;------------------------------------------------------------------------------




;;------------------------------------------------------------------------------
;; REGISTER BIT WRITE VALUES
;;------------------------------------------------------------------------------



;;------------------------------------------------------------------------------
;; HELPER MACROS
;;------------------------------------------------------------------------------


    END
    
