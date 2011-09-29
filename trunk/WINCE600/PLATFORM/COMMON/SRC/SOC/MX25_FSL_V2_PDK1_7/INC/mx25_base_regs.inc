;;------------------------------------------------------------------------------
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
;;  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
;;  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;;  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;;
;;------------------------------------------------------------------------------
;;
;;  Header: mx25_base_regs.inc
;;
;;  This header file defines the Physical Addresses (PA) of
;;  the base registers for the System on Chip (SoC) components.
;;
;;  Provides memory map definitions that are specific to the MX25 
;;  architecture.
;;
;;-----------------------------------------------------------------------------

;;-----------------------------------------------------------------------------
;;  INFORMATION
;;
;;  The physical addresses for SoC registers are fixed, hence they are defined
;;  in the CPU's common directory. The virtual addresses of the SoC registers
;;  are defined by the OEM and are configured in the platform's configuration
;;  directory by the file: .../PLATFORM/<NAME>/SRC/INC/CPU_BASE_REG_CFG.H.
;;
;;-----------------------------------------------------------------------------

;;-----------------------------------------------------------------------------
;;  NAMING CONVENTIONS
;;
;;  CPU_BASE_REG_ is the standard prefix for CPU base registers.
;;
;;  Memory ranges are accessed using physical, uncached, or cached addresses,
;;  depending on the system state. The following abbreviations are used for
;;  each addressing type:
;;
;;      PA - physical address
;;      UA - uncached virtual address
;;      CA - cached virtual address
;;
;;  The naming convention for CPU base registers is:
;;
;;      CPU_BASE_REG_<ADDRTYPE>_<SUBSYSTEM>
;;
;;-----------------------------------------------------------------------------


CSP_BASE_REG_PA_CRM         EQU     (0x53F80000)

CSP_BASE_REG_PA_AVIC        EQU     (0x68000000)

CSP_BASE_REG_PA_ESDCTL			EQU		(0xB8001000)
CSP_BASE_REG_PA_WEIMCTL      EQU		(0xB8002000)			; CS&ZHL JUN-11-2011: setup CS5 for CPLD in EM9170

CSP_BASE_REG_PA_NANDFC      EQU     (0xBB000000)

CSP_BASE_REG_PA_CSPI1       EQU     (0x43FA4000)
CSP_BASE_REG_PA_CSPI2       EQU     (0x50010000)
CSP_BASE_REG_PA_CSPI3       EQU     (0x50004000)

CSP_BASE_REG_PA_ESDHC1      EQU     (0x53FB4000)
CSP_BASE_REG_PA_ESDHC2      EQU     (0x53FB8000)

CSP_BASE_REG_PA_IOMUXC      EQU     (0x43FAC000)
CSP_BASE_REG_PA_MAX         EQU     (0x43F04000)


;;-----------------------------------------------------------------------------
;; MX25-SPECIFIC PERIPHERAL REGISTER MEMORY MAP
;;-----------------------------------------------------------------------------

    END
    
