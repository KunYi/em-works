;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;------------------------------------------------------------------------------
;
;   Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;  File:  oemaddrtab_cfg.inc
;
;  This file is used to define g_oalAddressTable. This table is passed to
;  KernelStart to establish physical to virtual memory mapping. This table
;  is used also in IOMEM OAL module to map between physical and virtual
;  memory addresses via OALPAtoVA/OALVAtoPA functions.
;
;------------------------------------------------------------------------------

; Export Definition

        EXPORT  g_oalAddressTable[DATA]

;------------------------------------------------------------------------------
;
; TABLE FORMAT
;       cached address, physical address, size
;------------------------------------------------------------------------------

g_oalAddressTable

    DCD 0x80000000, CSP_BASE_MEM_PA_CSD0, 64						; RAM image mapping      
    ;DCD 0x91300000, CSP_BASE_REG_PA_NANDFC, 36					; NANDMA + NANDSA + NANDFC
    ;; we keep a gap of 36 MB because of the logical addressing of the NAND binaries    
    ;DCD 0x93700000, CSP_BASE_MEM_PA_IRAM,  64						; Internal RAM 
    ;; we keep a gap of 64MB because of the logical addressing of the SD/MMC binaries    
    ;DCD 0x97700000, CSP_BASE_REG_PA_AIPS1, 1
    ;DCD 0x97800000, CSP_BASE_REG_PA_AIPS2, 1
    ;DCD 0x97900000, CSP_BASE_REG_PA_AVIC,  1
    ;DCD 0x97a00000, CSP_BASE_REG_PA_PERIPH_REGS1,  1			; some peripheral regs (such as FEC)
    ;DCD 0x97b00000, CSP_BASE_MEM_PA_SPI, 1							; SPI FLASH
    ;DCD 0x97c00000, CSP_BASE_MEM_PA_CSD1, 4             
    ;; we keep a gap of 4MB because of the logical addressing of the SPI and I2C binaries     
    ;
    ; CS&ZHL MAY-19-2011: change the gap of NANDFC to 40MB to consistent with image_cfg.h
    ;   
    ;DCD 0x91300000, CSP_BASE_REG_PA_NANDFC,				40        ; => 0xBB000000
    ;DCD 0x93B00000, CSP_BASE_MEM_PA_IRAM,					64        ; => 0x78000000, actual size = 128KB
    ;DCD 0x97B00000, CSP_BASE_REG_PA_AIPS1,					1			; => 0x43F00000
    ;DCD 0x97C00000, CSP_BASE_REG_PA_AIPS2,					1			; => 0x53F00000
    ;DCD 0x97D00000, CSP_BASE_REG_PA_AVIC,					1			; => 0x68000000
    ;DCD 0x97E00000, CSP_BASE_REG_PA_PERIPH_REGS1,	1			; => 0x50000000, including FEC, UART3-5 etc.
    ;DCD 0x97F00000, CSP_BASE_MEM_PA_SPI,						1          ; SPI FLASH
    ;DCD 0x98000000, CSP_BASE_MEM_PA_CSD1,					4			; we keep a gap of 4MB because of the logical addressing of the SPI and I2C binaries     
    ;DCD 0x98400000, CSP_BASE_MEM_PA_CS5,					1          ; => 0xB6000000, CS&ZHL JUN-11-2011: region for CPLD and ISA in EM9170
    ;DCD 0x98500000, CSP_BASE_MEM_PA_BUSCTL,				1          ; => 0xB8000000, CS&ZHL JUN-11-2011: region for WEIM etc.
;
;CS&ZHL AUG-12-2011: NAND -> 42MB
;
    DCD 0x91300000, CSP_BASE_REG_PA_NANDFC,				42        ; => 0xBB000000
    DCD 0x93D00000, CSP_BASE_MEM_PA_IRAM,					64        ; => 0x78000000, actual size = 128KB
    DCD 0x97D00000, CSP_BASE_REG_PA_AIPS1,					1			; => 0x43F00000
    DCD 0x97E00000, CSP_BASE_REG_PA_AIPS2,					1			; => 0x53F00000
    DCD 0x97F00000, CSP_BASE_REG_PA_AVIC,					1			; => 0x68000000
    DCD 0x98000000, CSP_BASE_REG_PA_PERIPH_REGS1,	1			; => 0x50000000, including FEC, UART3-5 etc.
    DCD 0x98100000, CSP_BASE_MEM_PA_SPI,						1          ; SPI FLASH
    DCD 0x98200000, CSP_BASE_MEM_PA_CSD1,					4			; we keep a gap of 4MB because of the logical addressing of the SPI and I2C binaries     
    DCD 0x98600000, CSP_BASE_MEM_PA_CS5,					1          ; => 0xB6000000, CS&ZHL JUN-11-2011: region for CPLD and ISA in EM9170
    DCD 0x98700000, CSP_BASE_MEM_PA_BUSCTL,				1          ; => 0xB8000000, CS&ZHL JUN-11-2011: region for WEIM etc.
    DCD 0x00000000, 0x00000000, 0												; Terminate table

;------------------------------------------------------------------------------

        END
