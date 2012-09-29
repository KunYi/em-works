//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: MX25_nandfc.h
//
//  Provides definitions for the NAND Flash Controller (NANDFC) module that
//  is available on MX25 Freescale SOCs.
//
//------------------------------------------------------------------------------
#ifndef __MX25_NANDFC_H
#define __MX25_NANDFC_H

#if    __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define NANDFC_MAIN_BUFFER_SIZE     (512)
#define NANDFC_SPARE_BUFFER_SIZE    (64)
#define NANDFC_NUM_BUFFERS          (8)


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef    struct
{
    UINT16 MAIN[NANDFC_NUM_BUFFERS][NANDFC_MAIN_BUFFER_SIZE / sizeof(UINT16)];
    UINT16 SPARE[NANDFC_NUM_BUFFERS][NANDFC_SPARE_BUFFER_SIZE / sizeof(UINT16)];
    UINT16 _reserved0[(0x1E00-0x1200)/sizeof(UINT16)];
    UINT16 _reserved1;
    UINT16 _reserved2;
    UINT16 RAM_BUFFER_ADDRESS;
    UINT16 NAND_FLASH_ADDR;
    UINT16 NAND_FLASH_CMD;
    UINT16 NFC_CONFIGURATION;
    UINT16 ECC_STATUS_RESULT1;  
    UINT16 ECC_STATUS_RESULT2;  
    UINT16 SPAS;                
    UINT16 NF_WR_PROT;
    UINT16 _reserved3;
    UINT16 _reserved4;
    UINT16 NAND_FLASH_WR_PR_ST;
    UINT16 NAND_FLASH_CONFIG1;
    UINT16 NAND_FLASH_CONFIG2;
    UINT16 _reserved5;
    UINT16 UNLOCK_START_BLK_ADD0;
    UINT16 UNLOCK_END_BLK_ADD0;
    UINT16 UNLOCK_START_BLK_ADD1;
    UINT16 UNLOCK_END_BLK_ADD1;
    UINT16 UNLOCK_START_BLK_ADD2;
    UINT16 UNLOCK_END_BLK_ADD2;
    UINT16 UNLOCK_START_BLK_ADD3;
    UINT16 UNLOCK_END_BLK_ADD3;
} CSP_NANDFC_REGS, *PCSP_NANDFC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
// NANDFC Buffers
#define NANDFC_MAIN_BUFF0_OFFSET            (0x0)
#define NANDFC_MAIN_BUFF1_OFFSET            (0x200)
#define NANDFC_MAIN_BUFF2_OFFSET            (0x400)
#define NANDFC_MAIN_BUFF3_OFFSET            (0x600)
#define NANDFC_MAIN_BUFF4_OFFSET            (0x800)
#define NANDFC_MAIN_BUFF5_OFFSET            (0xA00)
#define NANDFC_MAIN_BUFF6_OFFSET            (0xC00)
#define NANDFC_MAIN_BUFF7_OFFSET            (0xE00)
#define NANDFC_SPARE_BUFF0_OFFSET           (0x1000)
#define NANDFC_SPARE_BUFF1_OFFSET           (0x1040)
#define NANDFC_SPARE_BUFF2_OFFSET           (0x1080)
#define NANDFC_SPARE_BUFF3_OFFSET           (0x10C0)
#define NANDFC_SPARE_BUFF4_OFFSET           (0x1100)
#define NANDFC_SPARE_BUFF5_OFFSET           (0x1140)
#define NANDFC_SPARE_BUFF6_OFFSET           (0x1180)
#define NANDFC_SPARE_BUFF7_OFFSET           (0x11C0)

// NANDFC Registers
#define NANDFC_RAM_BUFFER_ADDRESS_OFFSET    (0x1E04)
#define NANDFC_NAND_FLASH_ADDR_OFFSET       (0x1E06)
#define NANDFC_NAND_FLASH_CMD_OFFSET        (0x1E08)
#define NANDFC_NFC_CONFIGURATION_OFFSET     (0x1E0A)
#define NANDFC_ECC_STATUS_RESULT1_OFFSET    (0x1E0C)
#define NANDFC_ECC_STATUS_RESULT2_OFFSET    (0x1E0E)
#define NANDFC_SPAS_OFFSET                  (0x1E10)
#define NANDFC_WR_PROT_OFFSET               (0x1E12)
#define NANDFC_FLASH_WR_PR_ST_OFFSET        (0x1E18)
#define NANDFC_NAND_FLASH_CONFIG1_OFFSET    (0x1E1A)
#define NANDFC_NAND_FLASH_CONFIG2_OFFSET    (0x1E1C)
#define NANDFC_UNLOCK_START_BLK_ADD0_OFFSET (0x1E20)
#define NANDFC_UNLOCK_END_BLK_ADD0_OFFSET   (0x1E22)
#define NANDFC_UNLOCK_START_BLK_ADD1_OFFSET (0x1E24)
#define NANDFC_UNLOCK_END_BLK_ADD1_OFFSET   (0x1E26)
#define NANDFC_UNLOCK_START_BLK_ADD2_OFFSET (0x1E28)
#define NANDFC_UNLOCK_END_BLK_ADD2_OFFSET   (0x1E2A)
#define NANDFC_UNLOCK_START_BLK_ADD3_OFFSET (0x1E2C)
#define NANDFC_UNLOCK_END_BLK_ADD3_OFFSET   (0x1E2E)




//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define NANDFC_RAM_BUFFER_ADDRESS_RBA_LSH          0
#define NANDFC_RAM_BUFFER_ADDRESS_CS_LSH           4

#define NANDFC_NFC_CONFIGURATION_BLS_LSH           0

#define NANDFC_ECC_STATUS_RESULT1_NOSER1_LSH       0
#define NANDFC_ECC_STATUS_RESULT1_NOSER2_LSH       4
#define NANDFC_ECC_STATUS_RESULT1_NOSER3_LSH       8
#define NANDFC_ECC_STATUS_RESULT1_NOSER4_LSH       12

#define NANDFC_ECC_STATUS_RESULT2_NOSER5_LSH       0
#define NANDFC_ECC_STATUS_RESULT2_NOSER6_LSH       4
#define NANDFC_ECC_STATUS_RESULT2_NOSER7_LSH       8
#define NANDFC_ECC_STATUS_RESULT2_NOSER8_LSH       12

#define NANDFC_SPAS_SPAS_LSH                       0

#define NANDFC_NF_WR_PROT_WPC_LSH                  0

#define NANDFC_NAND_FLASH_WR_PR_ST_LTS0_LSH          0
#define NANDFC_NAND_FLASH_WR_PR_ST_LS0_LSH           1
#define NANDFC_NAND_FLASH_WR_PR_ST_US0_LSH           2
#define NANDFC_NAND_FLASH_WR_PR_ST_LTS1_LSH          3
#define NANDFC_NAND_FLASH_WR_PR_ST_LS1_LSH           4
#define NANDFC_NAND_FLASH_WR_PR_ST_US1_LSH           5
#define NANDFC_NAND_FLASH_WR_PR_ST_LTS2_LSH          6
#define NANDFC_NAND_FLASH_WR_PR_ST_LS2_LSH           7
#define NANDFC_NAND_FLASH_WR_PR_ST_US2_LSH           8
#define NANDFC_NAND_FLASH_WR_PR_ST_LTS3_LSH          9
#define NANDFC_NAND_FLASH_WR_PR_ST_LS3_LSH           10
#define NANDFC_NAND_FLASH_WR_PR_ST_US3_LSH           11

#define NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_LSH      0
#define NANDFC_NAND_FLASH_CONFIG1_DMA_MODE_LSH      1
#define NANDFC_NAND_FLASH_CONFIG1_SP_EN_LSH         2
#define NANDFC_NAND_FLASH_CONFIG1_ECC_EN_LSH        3
#define NANDFC_NAND_FLASH_CONFIG1_INT_MSK_LSH       4
#define NANDFC_NAND_FLASH_CONFIG1_NF_BIG_LSH        5
#define NANDFC_NAND_FLASH_CONFIG1_NFC_RST_LSH       6
#define NANDFC_NAND_FLASH_CONFIG1_NF_CE_LSH         7
#define NANDFC_NAND_FLASH_CONFIG1_SYM_LSH           8
#define NANDFC_NAND_FLASH_CONFIG1_PPB_LSH           9
#define NANDFC_NAND_FLASH_CONFIG1_FP_INT_LSH        11

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
#define NANDFC_RAM_BUFFER_ADDRESS_RBA_WID          3
#define NANDFC_RAM_BUFFER_ADDRESS_CS_WID           2

#define NANDFC_NFC_CONFIGURATION_BLS_WID            2

#define NANDFC_ECC_STATUS_RESULT1_NOSER1_WID         4
#define NANDFC_ECC_STATUS_RESULT1_NOSER2_WID         4
#define NANDFC_ECC_STATUS_RESULT1_NOSER3_WID         4
#define NANDFC_ECC_STATUS_RESULT1_NOSER4_WID         4

#define NANDFC_ECC_STATUS_RESULT2_NOSER5_WID         4
#define NANDFC_ECC_STATUS_RESULT2_NOSER6_WID         4
#define NANDFC_ECC_STATUS_RESULT2_NOSER7_WID         4
#define NANDFC_ECC_STATUS_RESULT2_NOSER8_WID         4

#define NANDFC_SPAS_SPAS_WID                           8

#define NANDFC_NF_WR_PROT_WPC_WID                   3

#define NANDFC_NAND_FLASH_WR_PR_ST_LTS0_WID          1
#define NANDFC_NAND_FLASH_WR_PR_ST_LS0_WID           1
#define NANDFC_NAND_FLASH_WR_PR_ST_US0_WID           1
#define NANDFC_NAND_FLASH_WR_PR_ST_LTS1_WID          1
#define NANDFC_NAND_FLASH_WR_PR_ST_LS1_WID           1
#define NANDFC_NAND_FLASH_WR_PR_ST_US1_WID           1
#define NANDFC_NAND_FLASH_WR_PR_ST_LTS2_WID          1
#define NANDFC_NAND_FLASH_WR_PR_ST_LS2_WID           1
#define NANDFC_NAND_FLASH_WR_PR_ST_US2_WID           1
#define NANDFC_NAND_FLASH_WR_PR_ST_LTS3_WID          1
#define NANDFC_NAND_FLASH_WR_PR_ST_LS3_WID           1
#define NANDFC_NAND_FLASH_WR_PR_ST_US3_WID           1

#define NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_WID      1
#define NANDFC_NAND_FLASH_CONFIG1_DMA_MODE_WID      1
#define NANDFC_NAND_FLASH_CONFIG1_SP_EN_WID         1
#define NANDFC_NAND_FLASH_CONFIG1_ECC_EN_WID        1
#define NANDFC_NAND_FLASH_CONFIG1_INT_MSK_WID       1
#define NANDFC_NAND_FLASH_CONFIG1_NF_BIG_WID        1
#define NANDFC_NAND_FLASH_CONFIG1_NFC_RST_WID       1
#define NANDFC_NAND_FLASH_CONFIG1_NF_CE_WID         1
#define NANDFC_NAND_FLASH_CONFIG1_SYM_WID           1
#define NANDFC_NAND_FLASH_CONFIG1_PPB_WID           2
#define NANDFC_NAND_FLASH_CONFIG1_FP_INT_WID        1

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
#define NANDFC_NFC_CONFIGURATION_BLS_LOCKED               1
#define NANDFC_NFC_CONFIGURATION_BLS_UNLOCKED             2

// Common for all eight sections
#define NANDFC_ECC_STATUS_RESULT_NOSER_NO_ERR             0
#define NANDFC_ECC_STATUS_RESULT_NOSER_1SB_ERR            1
#define NANDFC_ECC_STATUS_RESULT_NOSER_2SB_ERR            2
#define NANDFC_ECC_STATUS_RESULT_NOSER_3SB_ERR            3
#define NANDFC_ECC_STATUS_RESULT_NOSER_4SB_ERR            4

// eight sections
#define NANDFC_ECC_STATUS_RESULT_NOSER1_4SB_ERR         0x0004
#define NANDFC_ECC_STATUS_RESULT_NOSER2_4SB_ERR         0x0040
#define NANDFC_ECC_STATUS_RESULT_NOSER3_4SB_ERR         0x0400
#define NANDFC_ECC_STATUS_RESULT_NOSER4_4SB_ERR         0x4000
#define NANDFC_ECC_STATUS_RESULT_NOSER5_4SB_ERR         0x0004
#define NANDFC_ECC_STATUS_RESULT_NOSER6_4SB_ERR         0x0040
#define NANDFC_ECC_STATUS_RESULT_NOSER7_4SB_ERR         0x0400
#define NANDFC_ECC_STATUS_RESULT_NOSER8_4SB_ERR         0x4000

#define NANDFC_ECC_STATUS_RESULT_NOSER1_8SB_ERR         0x0008
#define NANDFC_ECC_STATUS_RESULT_NOSER2_8SB_ERR         0x0080
#define NANDFC_ECC_STATUS_RESULT_NOSER3_8SB_ERR         0x0800
#define NANDFC_ECC_STATUS_RESULT_NOSER4_8SB_ERR         0x8000
#define NANDFC_ECC_STATUS_RESULT_NOSER5_8SB_ERR         0x0008
#define NANDFC_ECC_STATUS_RESULT_NOSER6_8SB_ERR         0x0080
#define NANDFC_ECC_STATUS_RESULT_NOSER7_8SB_ERR         0x0800
#define NANDFC_ECC_STATUS_RESULT_NOSER8_8SB_ERR         0x8000

#define NANDFC_NF_WR_PROT_WPC_UNLOCK                      4
#define NANDFC_NF_WR_PROT_WPC_LOCK_ALL                    2
#define NANDFC_NF_WR_PROT_WPC_LOCK_TIGHT                  1
 

#define NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_8BIT         0
#define NANDFC_NAND_FLASH_CONFIG1_ECC_MODE_4BIT         1

#define NANDFC_NAND_FLASH_CONFIG1_DMA_MODE_SECTION      0
#define NANDFC_NAND_FLASH_CONFIG1_DMA_MODE_PAGE         1

#define NANDFC_NAND_FLASH_CONFIG1_SP_EN_MAIN_SPARE        0     
#define NANDFC_NAND_FLASH_CONFIG1_SP_EN_SPARE_ONLY        1

#define NANDFC_NAND_FLASH_CONFIG1_ECC_EN_BYPASS           0
#define NANDFC_NAND_FLASH_CONFIG1_ECC_EN_ENABLE           1

#define NANDFC_NAND_FLASH_CONFIG1_INT_MSK_UNMASK          0
#define NANDFC_NAND_FLASH_CONFIG1_INT_MSK_MASK            1

#define NANDFC_NAND_FLASH_CONFIG1_NF_BIG_LITTLE           0
#define NANDFC_NAND_FLASH_CONFIG1_NF_BIG_BIG              1

#define NANDFC_NAND_FLASH_CONFIG1_NFC_RST_NORESET         0
#define NANDFC_NAND_FLASH_CONFIG1_NFC_RST_RESET           1

#define NANDFC_NAND_FLASH_CONFIG1_NF_CE_UNFORCE           0
#define NANDFC_NAND_FLASH_CONFIG1_NF_CE_FORCE             1

#define NANDFC_NAND_FLASH_CONFIG1_SYM_SYMMETRIC     1
#define NANDFC_NAND_FLASH_CONFIG1_SYM_ASYMMETRIC    0

#define NANDFC_NAND_FLASH_CONFIG1_PPB_32PAGES_PER_BLOCK    0
#define NANDFC_NAND_FLASH_CONFIG1_PPB_64PAGES_PER_BLOCK    1
#define NANDFC_NAND_FLASH_CONFIG1_PPB_128PAGES_PER_BLOCK   2
#define NANDFC_NAND_FLASH_CONFIG1_PPB_256PAGES_PER_BLOCK   3

#define NANDFC_NAND_FLASH_CONFIG1_FP_INT_FULL_PAGE_ENABLE   1


#ifdef __cplusplus
}
#endif

#endif // __MX25_NANDFC_H

