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
;;  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
;;  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;;  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;;
;;------------------------------------------------------------------------------
;;
;;  Header: mx51_base_reg.h
;;
;;  This header file defines the Physical Addresses (PA) of
;;  the base registers for the System on Chip (SoC) components.
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
;; PERIPHERAL REGISTER MEMORY MAP
;;-----------------------------------------------------------------------------

;; On-chip AHB peripherals
CSP_BASE_REG_PA_GPU             EQU     (0x30000000)
CSP_BASE_REG_PA_IPU             EQU     (0x40000000)
CSP_BASE_REG_PA_TZIC            EQU     (0x8FFFC000)
CSP_BASE_REG_PA_GPU2D           EQU     (0xD0000000)
CSP_BASE_REG_PA_TZIC_TO2        EQU     (0xE0000000)


;; AIPS #1 SPBA peripherals
CSP_BASE_REG_PA_ESDHC1          EQU     (0x70004000)
CSP_BASE_REG_PA_ESDHC2          EQU     (0x70008000)
CSP_BASE_REG_PA_UART3           EQU     (0x7000C000)
CSP_BASE_REG_PA_ECSPI1          EQU     (0x70010000)
CSP_BASE_REG_PA_SSI2            EQU     (0x70014000)
CSP_BASE_REG_PA_SDMA_INTERNAL   EQU     (0x7001C000)
CSP_BASE_REG_PA_ESDHC3          EQU     (0x70020000)
CSP_BASE_REG_PA_ESDHC4          EQU     (0x70024000)
CSP_BASE_REG_PA_SPDIF           EQU     (0x70028000)
CSP_BASE_REG_PA_PATA_UDMA       EQU     (0x70030000)
CSP_BASE_REG_PA_SLIMBUS         EQU     (0x70034000)
CSP_BASE_REG_PA_HSI2C           EQU     (0x70038000)
CSP_BASE_REG_PA_SPBA            EQU     (0x7003C000)

;; AIPS #1 on-platform peripherals
CSP_BASE_REG_PA_AIPS1           EQU     (0x73F00000)

;; AIPS #1 off-platform peripherals
CSP_BASE_REG_PA_USB             EQU     (0x73F80000)
CSP_BASE_REG_PA_GPIO1           EQU     (0x73F84000)
CSP_BASE_REG_PA_GPIO2           EQU     (0x73F88000)
CSP_BASE_REG_PA_GPIO3           EQU     (0x73F8C000)
CSP_BASE_REG_PA_GPIO4           EQU     (0x73F90000)
CSP_BASE_REG_PA_KPP             EQU     (0x73F94000)
CSP_BASE_REG_PA_WDOG1           EQU     (0x73F98000)
CSP_BASE_REG_PA_WDOG2           EQU     (0x73F9C000)
CSP_BASE_REG_PA_GPT             EQU     (0x73FA0000)
CSP_BASE_REG_PA_SRTC            EQU     (0x73FA4000)
CSP_BASE_REG_PA_IOMUXC          EQU     (0x73FA8000)
CSP_BASE_REG_PA_EPIT1           EQU     (0x73FAC000)
CSP_BASE_REG_PA_EPIT2           EQU     (0x73FB0000)
CSP_BASE_REG_PA_PWM1            EQU     (0x73FB4000)
CSP_BASE_REG_PA_PWM2            EQU     (0x73FB8000)
CSP_BASE_REG_PA_UART1           EQU     (0x73FBC000)
CSP_BASE_REG_PA_UART2           EQU     (0x73FC0000)
CSP_BASE_REG_PA_USB_PL301       EQU     (0x73FC4000)
CSP_BASE_REG_PA_SRC             EQU     (0x73FD0000)
CSP_BASE_REG_PA_CCM             EQU     (0x73FD4000)
CSP_BASE_REG_PA_GPC             EQU     (0x73FD8000)


;; AIPS #2 on-platform peripherals
CSP_BASE_REG_PA_AIPS2           EQU     (0x83F00000)

;; AIPS #2 off-platform peripherals
CSP_BASE_REG_PA_DPLL1           EQU     (0x83F80000)
CSP_BASE_REG_PA_DPLL2           EQU     (0x83F84000)
CSP_BASE_REG_PA_DPLL3           EQU     (0x83F88000)
CSP_BASE_REG_PA_AHBMAX          EQU     (0x83F94000)
CSP_BASE_REG_PA_IIM             EQU     (0x83F98000)
CSP_BASE_REG_PA_CSU             EQU     (0x83F9C000)
CSP_BASE_REG_PA_ELBOW           EQU     (0x83FA0000)
CSP_BASE_REG_PA_OWIRE           EQU     (0x83FA4000)
CSP_BASE_REG_PA_FIRI            EQU     (0x83FA8000)
CSP_BASE_REG_PA_ECSPI2          EQU     (0x83FAC000)
CSP_BASE_REG_PA_SDMA            EQU     (0x83FB0000)
CSP_BASE_REG_PA_SCC             EQU     (0x83FB4000)
CSP_BASE_REG_PA_ROMCP           EQU     (0x83FB8000)
CSP_BASE_REG_PA_RTIC            EQU     (0x83FBC000)
CSP_BASE_REG_PA_CSPI            EQU     (0x83FC0000)
CSP_BASE_REG_PA_I2C2            EQU     (0x83FC4000)
CSP_BASE_REG_PA_I2C1            EQU     (0x83FC8000)
CSP_BASE_REG_PA_SSI1            EQU     (0x83FCC000)
CSP_BASE_REG_PA_AUDMUX          EQU     (0x83FD0000)
CSP_BASE_REG_PA_M4IF            EQU     (0x83FD8000)
CSP_BASE_REG_PA_ESDCTL          EQU     (0x83FD9000)
CSP_BASE_REG_PA_WEIM            EQU     (0x83FDA000)
CSP_BASE_REG_PA_NFC_AXI     EQU     (0xCFFF0000)
CSP_BASE_REG_PA_NFC_IP          EQU     (0x83FDB000)
CSP_BASE_REG_PA_EMI             EQU     (0x83FDBF00)
CSP_BASE_REG_PA_MIPI            EQU     (0x83FDC000)
CSP_BASE_REG_PA_PATA_PIO        EQU     (0x83FE0000)
CSP_BASE_REG_PA_SIM             EQU     (0x83FE4000)
CSP_BASE_REG_PA_SSI3            EQU     (0x83FE8000)
CSP_BASE_REG_PA_FEC             EQU     (0x83FEC000)
CSP_BASE_REG_PA_TVE             EQU     (0x83FF0000)
CSP_BASE_REG_PA_VPU             EQU     (0x83FF4000)
CSP_BASE_REG_PA_SAHARA          EQU     (0x83FF8000)

    END
