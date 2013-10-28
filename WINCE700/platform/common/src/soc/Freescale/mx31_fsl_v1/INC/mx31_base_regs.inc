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
;;------------------------------------------------------------------------------
;;
;;  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
;;  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;;  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;;
;;------------------------------------------------------------------------------
;;
;;  Header: MX31_base_reg.h
;;
;;  This header file defines the Physical Addresses (PA) of
;;  the base registers for the System on Chip (SoC) components.
;;
;;  Provides memory map definitions that are specific to the MX31
;;  architecture.
;;
;;-----------------------------------------------------------------------------

 
;;-----------------------------------------------------------------------------
;;  INFORMATION
;;
;;  The physical addresses for SoC registers are fixed, hence they are defined
;;  in the CPU's common directory. The virtual addresses of the SoC registers
;;  are defined by the OEM and are configured in the platform's configuration
;;  directory by the file: .../PLATFORM/<NAME>/SRC/CONFIG/CPU_BASE_REG_CFG.H.
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


;;-----------------------------------------------------------------------------
;; MX31-SPECIFIC PERIPHERAL REGISTER MEMORY MAP
;;-----------------------------------------------------------------------------

;; AIPS A periperhals
CSP_BASE_REG_PA_I2C3            EQU     (0x43F84000)
CSP_BASE_REG_PA_USBOTG          EQU     (0x43F88000)
CSP_BASE_REG_PA_ATA_CTRL        EQU     (0x43F8C000)
CSP_BASE_REG_PA_I2C2            EQU     (0x43F98000)
CSP_BASE_REG_PA_IOMUXC          EQU     (0x43FAC000)
CSP_BASE_REG_PA_UART4           EQU     (0x43FB0000)
CSP_BASE_REG_PA_UART5           EQU     (0x43FB4000)
CSP_BASE_REG_PA_ECT_IPBUS1      EQU     (0x43FB8000)
CSP_BASE_REG_PA_ECT_IPBUS2      EQU     (0x43FBC000)

;; AIPS B Peripherals
CSP_BASE_REG_PA_SIM             EQU     (0x50018000)
CSP_BASE_REG_PA_ATA_DMA         EQU     (0x50020000)
CSP_BASE_REG_PA_MSHC1           EQU     (0x50024000)
CSP_BASE_REG_PA_MSHC2           EQU     (0x50028000)
CSP_BASE_REG_PA_CCM             EQU     (0x53F80000)
CSP_BASE_REG_PA_CSPI3           EQU     (0x53F84000)
CSP_BASE_REG_PA_GPIO3           EQU     (0x53FA4000)
CSP_BASE_REG_PA_MPEG4_ENCODER   EQU     (0x53FC8000)
CSP_BASE_REG_PA_GPIO2           EQU     (0x53FD0000)
CSP_BASE_REG_PA_RTIC            EQU     (0x53FEC000)

;; Non-AIPS Peripherals
CSP_BASE_REG_PA_PCMCIA          EQU     (0xB8004000)

    END
