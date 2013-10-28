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
//  Header: mxarm11_nandfc.h
//
//  Provides definitions for the NAND Flash Controller (NANDFC) module that
//  is available on ARM11-based Freescale SOCs.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_NANDFC_H
#define __MXARM11_NANDFC_H

#if    __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define NANDFC_MAIN_BUFFER_SIZE     (512)
#define NANDFC_SPARE_BUFFER_SIZE    (16)
#define NANDFC_NUM_BUFFERS          (4)

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef    struct
{
    UINT16 MAIN[NANDFC_NUM_BUFFERS][NANDFC_MAIN_BUFFER_SIZE / sizeof(UINT16)];
    UINT16 SPARE[NANDFC_NUM_BUFFERS][NANDFC_SPARE_BUFFER_SIZE / sizeof(UINT16)];
    UINT16 _reserved0[(0xE00-0x840)/sizeof(UINT16)];
    UINT16 NFC_BUFSIZE;
    UINT16 _reserved1;
    UINT16 RAM_BUFF_ADDRESS;
    UINT16 NAND_FLASH_ADD;
    UINT16 NAND_FLASH_CMD;
    UINT16 NFC_CONFIGURATION;
    UINT16 ECC_STATUS_RESULT;
    UINT16 ECC_RSLT_MAIN_AREA;
    UINT16 ECC_RSLT_SPARE_AREA;
    UINT16 NF_WR_PROT;
    UINT16 UNLOCK_START_BLK_ADD;
    UINT16 UNLOCK_END_BLK_ADD;
    UINT16 NAND_FLASH_WR_PR_ST;
    UINT16 NAND_FLASH_CONFIG1;
    UINT16 NAND_FLASH_CONFIG2;
} CSP_NANDFC_REGS, *PCSP_NANDFC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
// NANDFC Buffers
#define NANDFC_MAIN_BUFF0_OFFSET            (0x0)
#define NANDFC_MAIN_BUFF1_OFFSET            (0x200)
#define NANDFC_MAIN_BUFF2_OFFSET            (0x400)
#define NANDFC_MAIN_BUFF3_OFFSET            (0x600)
#define NANDFC_SPARE_BUFF0_OFFSET           (0x800)
#define NANDFC_SPARE_BUFF1_OFFSET           (0x810)
#define NANDFC_SPARE_BUFF2_OFFSET           (0x820)
#define NANDFC_SPARE_BUFF3_OFFSET           (0x830)

// NANDFC Registers
#define NANDFC_NFC_BUFSIZE_OFFSET           (0xE00)
#define NANDFC_RAM_BUFF_ADDRESS_OFFSET      (0xE04)
#define NANDFC_NAND_FLASH_ADD_OFFSET        (0xE06)
#define NANDFC_NAND_FLASH_CMD_OFFSET        (0xE08)
#define NANDFC_NFC_CONFIGURATION_OFFSET     (0xE0A)
#define NANDFC_ECC_STATUS_RESULT_OFFSET     (0xE0C)
#define NANDFC_ECC_RSLT_MAIN_AREA_OFFSET    (0xE0E)
#define NANDFC_ECC_RSLT_SPARE_AREA_OFFSET   (0xE10)
#define NANDFC_NF_WR_PROT_OFFSET            (0xE12)
#define NANDFC_UNLOCK_START_BLK_ADD_OFFSET  (0xE14)
#define NANDFC_UNLOCK_END_BLK_ADD_OFFSET    (0xE16)
#define NANDFC_FLASH_WR_PR_ST_OFFSET        (0xE18)
#define NANDFC_NAND_FLASH_CONFIG1_OFFSET    (0xE1A)
#define NANDFC_NAND_FLASH_CONFIG2_OFFSET    (0xE1C)


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define NANDFC_NFC_CONFIGURATION_BLS_LSH            0

#define NANDFC_ECC_STATUS_RESULT_ERM_LSH            0
#define NANDFC_ECC_STATUS_RESULT_ERS_LSH            2

#define NANDFC_ECC_RSLT_MAIN_AREA_X8_RESULT2_LSH    0
#define NANDFC_ECC_RSLT_MAIN_AREA_X8_RESULT1_LSH    3
#define NANDFC_ECC_RSLT_MAIN_AREA_X16_RESULT2_LSH   0
#define NANDFC_ECC_RSLT_MAIN_AREA_X16_RESULT1_LSH   4

#define NANDFC_ECC_RSLT_SPARE_AREA_X8_RESULT3_LSH   0
#define NANDFC_ECC_RSLT_SPARE_AREA_X8_RESULT4_LSH   3
#define NANDFC_ECC_RSLT_SPARE_AREA_X16_RESULT3_LSH  0
#define NANDFC_ECC_RSLT_SPARE_AREA_X16_RESULT4_LSH  4

#define NANDFC_NF_WR_PROT_WPC_LSH                   0

#define NANDFC_NAND_FLASH_WR_PR_ST_LTS_LSH          0
#define NANDFC_NAND_FLASH_WR_PR_ST_LS_LSH           1
#define NANDFC_NAND_FLASH_WR_PR_ST_US_LSH           2

#define NANDFC_NAND_FLASH_CONFIG1_SP_EN_LSH         2
#define NANDFC_NAND_FLASH_CONFIG1_ECC_EN_LSH        3
#define NANDFC_NAND_FLASH_CONFIG1_INT_MSK_LSH       4
#define NANDFC_NAND_FLASH_CONFIG1_NF_BIG_LSH        5
#define NANDFC_NAND_FLASH_CONFIG1_NFC_RST_LSH       6
#define NANDFC_NAND_FLASH_CONFIG1_NF_CE_LSH         7

#define NANDFC_NAND_FLASH_CONFIG2_FCMD_LSH          0
#define NANDFC_NAND_FLASH_CONFIG2_FADD_LSH          1
#define NANDFC_NAND_FLASH_CONFIG2_FDI_LSH           2
#define NANDFC_NAND_FLASH_CONFIG2_FDO_LSH           3
#define NANDFC_NAND_FLASH_CONFIG2_FDO_PAGE_LSH      3
#define NANDFC_NAND_FLASH_CONFIG2_FDO_ID_LSH        4
#define NANDFC_NAND_FLASH_CONFIG2_FDO_STATUS_LSH    5
#define NANDFC_NAND_FLASH_CONFIG2_INT_LSH           15

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define NANDFC_NFC_CONFIGURATION_BLS_WID            2

#define NANDFC_ECC_STATUS_RESULT_ERM_WID            2
#define NANDFC_ECC_STATUS_RESULT_ERS_WID            2

#define NANDFC_ECC_RSLT_MAIN_AREA_X8_RESULT2_WID    3
#define NANDFC_ECC_RSLT_MAIN_AREA_X8_RESULT1_WID    9
#define NANDFC_ECC_RSLT_MAIN_AREA_X16_RESULT2_WID   4
#define NANDFC_ECC_RSLT_MAIN_AREA_X16_RESULT1_WID   8

#define NANDFC_ECC_RSLT_SPARE_AREA_X8_RESULT3_WID   3
#define NANDFC_ECC_RSLT_SPARE_AREA_X8_RESULT4_WID   2
#define NANDFC_ECC_RSLT_SPARE_AREA_X16_RESULT3_WID  4
#define NANDFC_ECC_RSLT_SPARE_AREA_X16_RESULT4_WID  1

#define NANDFC_NF_WR_PROT_WPC_WID                   3

#define NANDFC_NAND_FLASH_WR_PR_ST_LTS_WID          1
#define NANDFC_NAND_FLASH_WR_PR_ST_LS_WID           1
#define NANDFC_NAND_FLASH_WR_PR_ST_US_WID           1

#define NANDFC_NAND_FLASH_CONFIG1_SP_EN_WID         1
#define NANDFC_NAND_FLASH_CONFIG1_ECC_EN_WID        1
#define NANDFC_NAND_FLASH_CONFIG1_INT_MSK_WID       1
#define NANDFC_NAND_FLASH_CONFIG1_NF_BIG_WID        1
#define NANDFC_NAND_FLASH_CONFIG1_NFC_RST_WID       1
#define NANDFC_NAND_FLASH_CONFIG1_NF_CE_WID         1

#define NANDFC_NAND_FLASH_CONFIG2_FCMD_WID          1
#define NANDFC_NAND_FLASH_CONFIG2_FADD_WID          1
#define NANDFC_NAND_FLASH_CONFIG2_FDI_WID           1
#define NANDFC_NAND_FLASH_CONFIG2_FDO_WID           3
#define NANDFC_NAND_FLASH_CONFIG2_FDO_PAGE_WID      1
#define NANDFC_NAND_FLASH_CONFIG2_FDO_ID_WID        1
#define NANDFC_NAND_FLASH_CONFIG2_FDO_STATUS_WID    1
#define NANDFC_NAND_FLASH_CONFIG2_INT_WID           1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define NANDFC_NFC_BUFSIZE_BUFSIZE_1K_BYTES         0
#define NANDFC_NFC_BUFSIZE_BUFSIZE_2K_BYTES         1

#define NANDFC_NFC_CONFIGURATION_BLS_LOCKED         1
#define NANDFC_NFC_CONFIGURATION_BLS_UNLOCKED       2

#define NANDFC_ECC_STATUS_RESULT_ERM_NO_ERR         0
#define NANDFC_ECC_STATUS_RESULT_ERM_1BIT_ERR       1
#define NANDFC_ECC_STATUS_RESULT_ERM_2BIT_ERR       2

#define NANDFC_ECC_STATUS_RESULT_ERS_NO_ERR         0
#define NANDFC_ECC_STATUS_RESULT_ERS_1BIT_ERR       1
#define NANDFC_ECC_STATUS_RESULT_ERS_2BIT_ERR       2

#define NANDFC_NF_WR_PROT_WPC_UNLOCK                4
#define NANDFC_NF_WR_PROT_WPC_LOCK_ALL              2
#define NANDFC_NF_WR_PROT_WPC_LOCK_TIGHT            1

#define NANDFC_NAND_FLASH_CONFIG1_SP_EN_MAIN_SPARE  0     
#define NANDFC_NAND_FLASH_CONFIG1_SP_EN_SPARE_ONLY  1

#define NANDFC_NAND_FLASH_CONFIG1_ECC_EN_BYPASS     0
#define NANDFC_NAND_FLASH_CONFIG1_ECC_EN_ENABLE     1

#define NANDFC_NAND_FLASH_CONFIG1_INT_MSK_UNMASK    0
#define NANDFC_NAND_FLASH_CONFIG1_INT_MSK_MASK      1

#define NANDFC_NAND_FLASH_CONFIG1_NF_BIG_LITTLE     0
#define NANDFC_NAND_FLASH_CONFIG1_NF_BIG_BIG        1

#define NANDFC_NAND_FLASH_CONFIG1_NFC_RST_NORESET   0
#define NANDFC_NAND_FLASH_CONFIG1_NFC_RST_RESET     1

#define NANDFC_NAND_FLASH_CONFIG1_NF_CE_UNFORCE     0
#define NANDFC_NAND_FLASH_CONFIG1_NF_CE_FORCE       1


#ifdef __cplusplus
}
#endif

#endif // __MXARM11_NANDFC_H

