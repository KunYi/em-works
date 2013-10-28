//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: MX31_nandfc.h
//
//  Provides definitions for the NAND Flash Controller (NANDFC) module that
//  is available on MX31 Freescale SOCs.
//
//------------------------------------------------------------------------------
#ifndef __MX31_NANDFC_H
#define __MX31_NANDFC_H

#if    __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define MX31_NANDFC_MAIN_BUFFER_SIZE     (512)
#define MX31_NANDFC_SPARE_BUFFER_SIZE    (16)
#define MX31_NANDFC_NUM_BUFFERS          (4)

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef    struct
{
    UINT16 MAIN[NANDFC_NUM_BUFFERS][NANDFC_MAIN_BUFFER_SIZE / sizeof(UINT16)];
    UINT16 SPARE[NANDFC_NUM_BUFFERS][NANDFC_SPARE_BUFFER_SIZE / sizeof(UINT16)];
    UINT16 _reserved0[(0xE00-0x840)/sizeof(UINT16)];
    UINT16 _reserved1;
    UINT16 _reserved2;
    UINT16 MX31_RAM_BUFF_ADDRESS;
    UINT16 MX31_NAND_FLASH_ADD;
    UINT16 MX31_NAND_FLASH_CMD;
    UINT16 MX31_NFC_CONFIGURATION;
    UINT16 MX31_ECC_STATUS_RESULT;
    UINT16 _reserved3;
    UINT16 _reserved4;
    UINT16 MX31_NF_WR_PROT;
    UINT16 MX31_UNLOCK_START_BLK_ADD;
    UINT16 MX31_UNLOCK_END_BLK_ADD;
    UINT16 MX31_NAND_FLASH_WR_PR_ST;
    UINT16 MX31_NAND_FLASH_CONFIG1;
    UINT16 MX31_NAND_FLASH_CONFIG2;
} MX31_CSP_NANDFC_REGS, *PMX31_CSP_NANDFC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
// NANDFC Buffers
#define MX31_NANDFC_MAIN_BUFF0_OFFSET            (0x0)
#define MX31_NANDFC_MAIN_BUFF1_OFFSET            (0x200)
#define MX31_NANDFC_MAIN_BUFF2_OFFSET            (0x400)
#define MX31_NANDFC_MAIN_BUFF3_OFFSET            (0x600)
#define MX31_NANDFC_SPARE_BUFF0_OFFSET           (0x800)
#define MX31_NANDFC_SPARE_BUFF1_OFFSET           (0x810)
#define MX31_NANDFC_SPARE_BUFF2_OFFSET           (0x820)
#define MX31_NANDFC_SPARE_BUFF3_OFFSET           (0x830)

// NANDFC Registers
#define MX31_NANDFC_RAM_BUFF_ADDRESS_OFFSET      (0xE04)
#define MX31_NANDFC_NAND_FLASH_ADD_OFFSET        (0xE06)
#define MX31_NANDFC_NAND_FLASH_CMD_OFFSET        (0xE08)
#define MX31_NANDFC_NFC_CONFIGURATION_OFFSET     (0xE0A)
#define MX31_NANDFC_ECC_STATUS_RESULT_OFFSET     (0xE0C)
#define MX31_NANDFC_NF_WR_PROT_OFFSET            (0xE12)
#define MX31_NANDFC_UNLOCK_START_BLK_ADD_OFFSET  (0xE14)
#define MX31_NANDFC_UNLOCK_END_BLK_ADD_OFFSET    (0xE16)
#define MX31_NANDFC_FLASH_WR_PR_ST_OFFSET        (0xE18)
#define MX31_NANDFC_NAND_FLASH_CONFIG1_OFFSET    (0xE1A)
#define MX31_NANDFC_NAND_FLASH_CONFIG2_OFFSET    (0xE1C)


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define MX31_NANDFC_NFC_CONFIGURATION_BLS_LSH            0

#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER1_LSH         0
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER2_LSH         4
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER3_LSH         8
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER4_LSH         12

#define MX31_NANDFC_NF_WR_PROT_WPC_LSH                   0

#define MX31_NANDFC_NAND_FLASH_WR_PR_ST_LTS_LSH          0
#define MX31_NANDFC_NAND_FLASH_WR_PR_ST_LS_LSH           1
#define MX31_NANDFC_NAND_FLASH_WR_PR_ST_US_LSH           2

#define MX31_NANDFC_NAND_FLASH_CONFIG1_SP_EN_LSH         2
#define MX31_NANDFC_NAND_FLASH_CONFIG1_ECC_EN_LSH        3
#define MX31_NANDFC_NAND_FLASH_CONFIG1_INT_MSK_LSH       4
#define MX31_NANDFC_NAND_FLASH_CONFIG1_NF_BIG_LSH        5
#define MX31_NANDFC_NAND_FLASH_CONFIG1_NFC_RST_LSH       6
#define MX31_NANDFC_NAND_FLASH_CONFIG1_NF_CE_LSH         7
#define MX31_NANDFC_NAND_FLASH_CONFIG1_ONE_CYCLE_LSH     8
#define MX31_NANDFC_NAND_FLASH_CONFIG1_MLC_LSH           9

#define MX31_NANDFC_NAND_FLASH_CONFIG2_FCMD_LSH          0
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FADD_LSH          1
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDI_LSH           2
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDO_LSH           3
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDO_PAGE_LSH      3
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDO_ID_LSH        4
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDO_STATUS_LSH    5
#define MX31_NANDFC_NAND_FLASH_CONFIG2_INT_LSH           15

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define MX31_NANDFC_NFC_CONFIGURATION_BLS_WID            2

#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER1_WID         4
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER2_WID         4
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER3_WID         4
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER4_WID         4

#define MX31_NANDFC_NF_WR_PROT_WPC_WID                   3

#define MX31_NANDFC_NAND_FLASH_WR_PR_ST_LTS_WID          1
#define MX31_NANDFC_NAND_FLASH_WR_PR_ST_LS_WID           1
#define MX31_NANDFC_NAND_FLASH_WR_PR_ST_US_WID           1

#define MX31_NANDFC_NAND_FLASH_CONFIG1_SP_EN_WID         1
#define MX31_NANDFC_NAND_FLASH_CONFIG1_ECC_EN_WID        1
#define MX31_NANDFC_NAND_FLASH_CONFIG1_INT_MSK_WID       1
#define MX31_NANDFC_NAND_FLASH_CONFIG1_NF_BIG_WID        1
#define MX31_NANDFC_NAND_FLASH_CONFIG1_NFC_RST_WID       1
#define MX31_NANDFC_NAND_FLASH_CONFIG1_NF_CE_WID         1
#define MX31_NANDFC_NAND_FLASH_CONFIG1_ONE_CYCLE_WID     1
#define MX31_NANDFC_NAND_FLASH_CONFIG1_MLC_WID           1

#define MX31_NANDFC_NAND_FLASH_CONFIG2_FCMD_WID          1
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FADD_WID          1
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDI_WID           1
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDO_WID           3
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDO_PAGE_WID      1
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDO_ID_WID        1
#define MX31_NANDFC_NAND_FLASH_CONFIG2_FDO_STATUS_WID    1
#define MX31_NANDFC_NAND_FLASH_CONFIG2_INT_WID           1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define MX31_NANDFC_NFC_CONFIGURATION_BLS_LOCKED               1
#define MX31_NANDFC_NFC_CONFIGURATION_BLS_UNLOCKED             2

#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER_NO_ERR             0
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER_1SB_ERR            1
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER_2SB_ERR            2
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER_3SB_ERR            3
#define MX31_NANDFC_ECC_STATUS_RESULT_NOSER_4SB_ERR            4

#define MX31_NANDFC_NF_WR_PROT_WPC_UNLOCK                      4
#define MX31_NANDFC_NF_WR_PROT_WPC_LOCK_ALL                    2
#define MX31_NANDFC_NF_WR_PROT_WPC_LOCK_TIGHT                  1

#define MX31_NANDFC_NAND_FLASH_CONFIG1_SP_EN_MAIN_SPARE        0     
#define MX31_NANDFC_NAND_FLASH_CONFIG1_SP_EN_SPARE_ONLY        1

#define MX31_NANDFC_NAND_FLASH_CONFIG1_ECC_EN_BYPASS           0
#define MX31_NANDFC_NAND_FLASH_CONFIG1_ECC_EN_ENABLE           1

#define MX31_NANDFC_NAND_FLASH_CONFIG1_INT_MSK_UNMASK          0
#define MX31_NANDFC_NAND_FLASH_CONFIG1_INT_MSK_MASK            1

#define MX31_NANDFC_NAND_FLASH_CONFIG1_NF_BIG_LITTLE           0
#define MX31_NANDFC_NAND_FLASH_CONFIG1_NF_BIG_BIG              1

#define MX31_NANDFC_NAND_FLASH_CONFIG1_NFC_RST_NORESET         0
#define MX31_NANDFC_NAND_FLASH_CONFIG1_NFC_RST_RESET           1

#define MX31_NANDFC_NAND_FLASH_CONFIG1_NF_CE_UNFORCE           0
#define MX31_NANDFC_NAND_FLASH_CONFIG1_NF_CE_FORCE             1

#define MX31_NANDFC_NAND_FLASH_CONFIG1_ONE_CYCLE_SYMMETRIC     1
#define MX31_NANDFC_NAND_FLASH_CONFIG1_ONE_CYCLE_ASYMMETRIC    0

#define MX31_NANDFC_NAND_FLASH_CONFIG1_MLC_MLC                 1
#define MX31_NANDFC_NAND_FLASH_CONFIG1_MLC_SLC                 0


#ifdef __cplusplus
}
#endif

#endif // __MX31_NANDFC_H

