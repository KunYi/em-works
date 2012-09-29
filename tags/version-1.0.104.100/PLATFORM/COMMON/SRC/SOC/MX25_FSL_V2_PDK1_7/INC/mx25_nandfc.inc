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
;; Header:  mx25_nandfc.inc
;;
;; Provides definitions for NANDFC module on MX25.
;;
;;------------------------------------------------------------------------------

;;------------------------------------------------------------------------------
;; GENERAL MODULE CONSTANTS
;;------------------------------------------------------------------------------


;;------------------------------------------------------------------------------
;; REGISTER LAYOUT
;;------------------------------------------------------------------------------


;;------------------------------------------------------------------------------
;; REGISTER OFFSETS
;;------------------------------------------------------------------------------
NANDFC_MAIN_BUFF0_OFFSET            EQU     (0x0)
NANDFC_MAIN_BUFF1_OFFSET            EQU     (0x200)
NANDFC_MAIN_BUFF2_OFFSET            EQU     (0x400)
NANDFC_MAIN_BUFF3_OFFSET            EQU     (0x600)
NANDFC_MAIN_BUFF4_OFFSET            EQU     (0x800)
NANDFC_MAIN_BUFF5_OFFSET            EQU     (0xA00)
NANDFC_MAIN_BUFF6_OFFSET            EQU     (0xC00)
NANDFC_MAIN_BUFF7_OFFSET            EQU     (0xE00)

NANDFC_SPARE_BUFF0_OFFSET           EQU     (0x1000)
NANDFC_SPARE_BUFF1_OFFSET           EQU     (0x1040)
NANDFC_SPARE_BUFF2_OFFSET           EQU     (0x1080)
NANDFC_SPARE_BUFF3_OFFSET           EQU     (0x10C0)
NANDFC_SPARE_BUFF4_OFFSET           EQU     (0x1100)
NANDFC_SPARE_BUFF5_OFFSET           EQU     (0x1140)
NANDFC_SPARE_BUFF6_OFFSET           EQU     (0x1180)
NANDFC_SPARE_BUFF7_OFFSET           EQU     (0x11C0)

NANDFC_RAM_BUFFER_ADDRESS_OFFSET    EQU     (0x1E04)
NANDFC_NAND_FLASH_ADDR_OFFSET       EQU     (0x1E06)
NANDFC_NAND_FLASH_CMD_OFFSET        EQU     (0x1E08)
NANDFC_NFC_CONFIGURATION_OFFSET     EQU     (0x1E0A)
NANDFC_ECC_STATUS_RESULT1_OFFSET    EQU     (0x1E0C)
NANDFC_ECC_STATUS_RESULT2_OFFSET    EQU     (0x1E0E)
NANDFC_SPAS_OFFSET                  EQU     (0x1E10)
NANDFC_NF_WR_PROT_OFFSET            EQU     (0x1E12)
NANDFC_NAND_FLASH_WR_PR_ST_OFFSET   EQU     (0x1E18)
NANDFC_NAND_FLASH_CONFIG1_OFFSET    EQU     (0x1E1A)
NANDFC_NAND_FLASH_CONFIG2_OFFSET    EQU     (0x1E1C)

NANDFC_UNLOCK_START_BLK_ADD0_OFFSET EQU     (0x1E20)
NANDFC_UNLOCK_END_BLK_ADD0_OFFSET   EQU     (0x1E22)
NANDFC_UNLOCK_START_BLK_ADD1_OFFSET EQU     (0x1E24)
NANDFC_UNLOCK_END_BLK_ADD1_OFFSET   EQU     (0x1E26)
NANDFC_UNLOCK_START_BLK_ADD2_OFFSET EQU     (0x1E28)
NANDFC_UNLOCK_END_BLK_ADD2_OFFSET   EQU     (0x1E2A)
NANDFC_UNLOCK_START_BLK_ADD3_OFFSET EQU     (0x1E2C)
NANDFC_UNLOCK_END_BLK_ADD3_OFFSET   EQU     (0x1E2E)


;;------------------------------------------------------------------------------
;; REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
;;------------------------------------------------------------------------------



;;------------------------------------------------------------------------------
;; REGISTER BIT FIELD WIDTHS
;;------------------------------------------------------------------------------




;;------------------------------------------------------------------------------
;; REGISTER BIT WRITE VALUES
;;------------------------------------------------------------------------------

NANDFC_CONFIG2_FCMD                 EQU     (1 << 0)
NANDFC_CONFIG2_FADD                 EQU     (1 << 1)
NANDFC_CONFIG2_FDI                  EQU     (1 << 2)
NANDFC_CONFIG2_FDO_PAGE             EQU     (1 << 3)
NANDFC_CONFIG2_FDO_ID               EQU     (1 << 4)
NANDFC_CONFIG2_FDO_STATUS           EQU     (1 << 5)
NANDFC_CONFIG2_INT                  EQU     (1 << 15)


;;------------------------------------------------------------------------------
;; HELPER MACROS
;;------------------------------------------------------------------------------


    END
