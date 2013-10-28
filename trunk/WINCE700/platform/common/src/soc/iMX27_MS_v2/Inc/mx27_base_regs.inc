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
;-----------------------------------------------------------------------------
;
; Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
; Header: mx27_base_reg.inc
;
; This header file defines the Physical Addresses (PA) of 
; the base registers for the System on Chip (SoC) components.
;
; Provides memory map definitions for MX27 peripherals.
;
;------------------------------------------------------------------------------
;
; INFORMATION
;
; The physical addresses for SoC registers are fixed, hence they are defined
; in the CPU's common directory. The virtual addresses of the SoC registers
; are defined by the OEM and are configured in the platform's configuration 
; directory by the file: .../PLATFORM/<NAME>/SRC/CONFIG/CPU_BASE_REG_CFG.H.
;
;-----------------------------------------------------------------------------
;
; NAMING CONVENTIONS
;
; CPU_BASE_REG_ is the standard prefix for CPU base registers.
;
; Memory ranges are accessed using physical, uncached, or cached addresses,
; depending on the system state. The following abbreviations are used for
; each addressing type:
;
;      PA - physical address
;      UA - uncached virtual address
;      CA - cached virtual address
;
; The naming convention for CPU base registers is:
;
;      CPU_BASE_REG_<ADDRTYPE>_<SUBSYSTEM>
;
;-----------------------------------------------------------------------------


;-----------------------------------------------------------------------------
; MX27 REGISTER MEMORY MAP
;-----------------------------------------------------------------------------
;
; AIPI1 periperhals
;
CSP_BASE_REG_PA_AIPI1           EQU     (0x10000000)
CSP_BASE_REG_PA_DMAC            EQU     (0x10001000)
CSP_BASE_REG_PA_WDOG            EQU     (0x10002000)
CSP_BASE_REG_PA_GPT1            EQU     (0x10003000)
CSP_BASE_REG_PA_GPT2            EQU     (0x10004000)
CSP_BASE_REG_PA_GPT3            EQU     (0x10005000)
CSP_BASE_REG_PA_PWM             EQU     (0x10006000)
CSP_BASE_REG_PA_RTC             EQU     (0x10007000)
CSP_BASE_REG_PA_KPP             EQU     (0x10008000)
CSP_BASE_REG_PA_OWIRE           EQU     (0x10009000)
CSP_BASE_REG_PA_UART1           EQU     (0x1000A000)
CSP_BASE_REG_PA_UART2           EQU     (0x1000B000)
CSP_BASE_REG_PA_UART3           EQU     (0x1000C000)
CSP_BASE_REG_PA_UART4           EQU     (0x1000D000)
CSP_BASE_REG_PA_CSPI1           EQU     (0x1000E000)
CSP_BASE_REG_PA_CSPI2           EQU     (0x1000F000)
CSP_BASE_REG_PA_SSI1            EQU     (0x10010000)
CSP_BASE_REG_PA_SSI2            EQU     (0x10011000)
CSP_BASE_REG_PA_I2C1            EQU     (0x10012000)
CSP_BASE_REG_PA_SDHC1           EQU     (0x10013000)
CSP_BASE_REG_PA_SDHC2           EQU     (0x10014000)
CSP_BASE_REG_PA_GPIO            EQU     (0x10015000)
CSP_BASE_REG_PA_AUDMUX          EQU     (0x10016000)
CSP_BASE_REG_PA_CSPI3           EQU     (0x10017000)
CSP_BASE_REG_PA_MSHC            EQU     (0x10018000)
CSP_BASE_REG_PA_GPT4            EQU     (0x10019000)
CSP_BASE_REG_PA_GPT5            EQU     (0x1001A000)
CSP_BASE_REG_PA_UART5           EQU     (0x1001B000)
CSP_BASE_REG_PA_UART6           EQU     (0x1001C000)
CSP_BASE_REG_PA_I2C2            EQU     (0x1001D000)
CSP_BASE_REG_PA_SDHC3           EQU     (0x1001E000)
CSP_BASE_REG_PA_GPT6            EQU     (0x1001F000)

;
; AIPI2 Peripherals
;
CSP_BASE_REG_PA_AIPI2           EQU     (0x10020000)
CSP_BASE_REG_PA_LCDC            EQU     (0x10021000)
CSP_BASE_REG_PA_SLCDC           EQU     (0x10022000)
CSP_BASE_REG_PA_VCM             EQU     (0x10023000)
CSP_BASE_REG_PA_USB             EQU     (0x10024000)
CSP_BASE_REG_PA_SAHARA          EQU     (0x10025000)
CSP_BASE_REG_PA_EMMA            EQU     (0x10026000)
CSP_BASE_REG_PA_CRM             EQU     (0x10027000)
CSP_BASE_REG_PA_SYSCTRL         EQU     (0x10027800)    ; Share slot 7 with CRM
CSP_BASE_REG_PA_IIM             EQU     (0x10028000)
; Reserved (Slot 9)
CSP_BASE_REG_PA_RTIC            EQU     (0x1002A000)
CSP_BASE_REG_PA_FEC             EQU     (0x1002B000)
CSP_BASE_REG_PA_SCC             EQU     (0x1002C000)    ; Two slots
; Reserved (Slots 14 - 26)
CSP_BASE_REG_PA_ETBREG          EQU     (0x1003B000)
CSP_BASE_REG_PA_ETBRAM          EQU     (0x1003C000)    ; Two slots
CSP_BASE_REG_PA_JAM             EQU     (0x1003E000)
CSP_BASE_REG_PA_MAX             EQU     (0x1003F000)

;
; Non-AIPI Peripherals
;
CSP_BASE_REG_PA_AITC            EQU     (0x10040000)
CSP_BASE_REG_PA_ROMPATCH        EQU     (0x10041000)
CSP_BASE_REG_PA_CSI             EQU     (0x80000000)
CSP_BASE_REG_PA_ATA             EQU     (0x80001000)
CSP_BASE_REG_PA_NANDFC          EQU     (0xD8000000)
CSP_BASE_REG_PA_ESDRAMC         EQU     (0xD8001000)
CSP_BASE_REG_PA_WEIM            EQU     (0xD8002000)
CSP_BASE_REG_PA_M3IF            EQU     (0xD8003000)
CSP_BASE_REG_PA_PCMCIA          EQU     (0xD8004000)

    END
    
