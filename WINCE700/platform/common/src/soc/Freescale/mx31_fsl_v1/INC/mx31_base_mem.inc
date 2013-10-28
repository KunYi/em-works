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
;;  Header: MX31_base_mem.h
;;
;;  This header file defines the Physical Addresses (PA) of
;;  the SoC chip selects and external memory assignments.
;;
;;  Provides memory map definitions specific to the MX31 SoC.
;;
;;------------------------------------------------------------------------------


;;-----------------------------------------------------------------------------
;;  INFORMATION
;;
;;  The physical addresses for SoC chip selects are fixed, hence they are 
;;  defined  in the CPU's common directory. The virtual addresses of the SoC 
;;  chip selects are defined by the OEM and are configured in the 
;;  platform's configuration directory by the file: 
;;  .../PLATFORM/<NAME>/SRC/CONFIG/CPU_BASE_REG_CFG.H.
;;
;;-----------------------------------------------------------------------------


;;-----------------------------------------------------------------------------
;;  NAMING CONVENTIONS
;;
;;  CPU_BASE_MEM_ is the standard prefix for CPU chip selects.
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
;;      CPU_BASE_MEM_<ADDRTYPE>_<SUBSYSTEM>
;;
;;-----------------------------------------------------------------------------


;;-----------------------------------------------------------------------------
;; MX31-SPECIFIC INTERNAL MEMORY MAP
;;-----------------------------------------------------------------------------
CSP_BASE_MEM_PA_SECURE_IROM     EQU     (0x00000000)
CSP_BASE_MEM_PA_IROM            EQU     (0x00404000)
CSP_BASE_MEM_PA_IRAM            EQU     (0x1FFFC000)


;;-----------------------------------------------------------------------------
;; MX31-SPECIFIC EXTERNAL MEMORY MAP
;;-----------------------------------------------------------------------------
CSP_BASE_MEM_PA_CS5             EQU     (0xB6000000)
CSP_BASE_MEM_PA_PCMCIA_CF       EQU     (0xBC000000)
CSP_BASE_MEM_PA_GACC            EQU     (0xC0000000)

    END
