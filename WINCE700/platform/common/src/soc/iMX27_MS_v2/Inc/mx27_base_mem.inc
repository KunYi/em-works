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
;------------------------------------------------------------------------------
;
; Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
; Header: mx27_base_mem.h
;
; This header file defines the Physical Addresses (PA) of
; the SoC chip selects and external memory assignments.
;
; Provides memory map definitions for MX27.
;
;------------------------------------------------------------------------------
;
; INFORMATION
;
; The physical addresses for SoC chip selects are fixed, hence they are 
; defined  in the CPU's common directory. The virtual addresses of the SoC 
; chip selects are defined by the OEM and are configured in the 
; platform's configuration directory by the file: 
; .../PLATFORM/<NAME>/SRC/CONFIG/CPU_BASE_REG_CFG.H.
;
;-----------------------------------------------------------------------------
;
; NAMING CONVENTIONS
;
; CPU_BASE_MEM_ is the standard prefix for CPU chip selects.
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
;      CPU_BASE_MEM_<ADDRTYPE>_<SUBSYSTEM>
;
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------
; MX27 INTERNAL MEMORY MAP
;-----------------------------------------------------------------------------
CSP_BASE_MEM_PA_AIPI1           EQU     (0x10000000)
CSP_BASE_MEM_PA_AIPI2           EQU     (0x10020000)
CSP_BASE_MEM_PA_VRAM            EQU     (0xFFFF4C00)

;-----------------------------------------------------------------------------
; MX27 EXTERNAL MEMORY MAP
;-----------------------------------------------------------------------------
CSP_BASE_MEM_PA_CSD0            EQU     (0xA0000000)
CSP_BASE_MEM_PA_CSD1            EQU     (0xB0000000)

CSP_BASE_MEM_PA_CS0             EQU     (0xC0000000)
CSP_BASE_MEM_PA_CS1             EQU     (0xC8000000)
CSP_BASE_MEM_PA_CS2             EQU     (0xD0000000)
CSP_BASE_MEM_PA_CS3             EQU     (0xD2000000)
CSP_BASE_MEM_PA_CS4             EQU     (0xD4000000)
CSP_BASE_MEM_PA_CS5             EQU     (0xD6000000)
CSP_BASE_MEM_PA_PCMCIA_CF       EQU     (0xDC000000)

    END
