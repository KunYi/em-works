//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: NFC.h
//
//  Provides definitions for the NAND Flash Controller (NFC) module that
//  is available on MX51 Freescale SOCs.
//
//------------------------------------------------------------------------------
#ifndef __MX51_NFC_H
#define __MX51_NFC_H

#if    __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define NFC_MAIN_BUFFER_SIZE     (512)
#define NFC_SPARE_BUFFER_SIZE    (64)
#define NFC_NUM_BUFFERS          (8)
#define MAX_NUM_OF_DEV          (8)


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------

// Definition for AXI registers
typedef    struct
{
    UINT32 MAIN[NFC_NUM_BUFFERS][NFC_MAIN_BUFFER_SIZE / sizeof(UINT32)];
    UINT32 SPARE[NFC_NUM_BUFFERS][NFC_SPARE_BUFFER_SIZE / sizeof(UINT32)];
    UINT32 _reserved0[(0xE00-0x200)/sizeof(UINT32)];
    UINT32 NAND_CMD;
    UINT32 NAND_ADD_LOW[MAX_NUM_OF_DEV];
    UINT32 NAND_ADD_HIGH[MAX_NUM_OF_DEV/2];
    UINT32 NFC_CONFIGURATION1;
    UINT32 ECC_STATUS_RESULT;    
    UINT32 ECC_STATUS_SUM;
    UINT32 LAUNCH_NFC;
} CSP_NFC_AXI_REGS, *PCSP_NFC_AXI_REGS;

// Definition for IP registers
typedef    struct
{
    UINT32 NF_WR_PROT;
    UINT32 UNLOCK_BLK_ADD[MAX_NUM_OF_DEV];
    UINT32 NFC_CONFIGURATION2;
    UINT32 NFC_CONFIGURATION3;    
    UINT32 NFC_IPC;
    UINT32 AXI_ERR_ADD;
    UINT32 NFC_DELAY_LINE;
} CSP_NFC_IP_REGS, *PCSP_NFC_IP_REGS;

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

// NAND_CMD register
#define NFC_NAND_CMD_COMMAND0_LSH             0
#define NFC_NAND_CMD_COMMAND1_LSH             8               
#define NFC_NAND_CMD_COMMAND2_LSH             16
#define NFC_NAND_CMD_COMMAND3_LSH             24

//NAND_ADD_LOW register
#define NFC_NAND_ADD_LOW_COLUMN_LSH             0  
#define NFC_NAND_ADD_LOW_ROW_LSH                16

//NAND_ADD_LOW register
#define NFC_NAND_ADD_HIGH_EVEN_GROUP_LSH            0  
#define NFC_NAND_ADD_HIGH_ODD_GROUP_LSH             16

// NFC_CONFIGURATION1 register
#define NFC_NFC_CONFIGURATION1_SP_EN_LSH            0
#define NFC_NFC_CONFIGURATION1_NF_CE_LSH            1
#define NFC_NFC_CONFIGURATION1_NFC_RST_LSH          2
#define NFC_NFC_CONFIGURATION1_RBA_LSH              4
#define NFC_NFC_CONFIGURATION1_NUM_OF_ITE_LSH       8
#define NFC_NFC_CONFIGURATION1_CS_LSH               12
#define NFC_NFC_CONFIGURATION1_NF_STATUS_LSH        16

// ECC_STATUS_RESULT register
#define NFC_ECC_STATUS_RESULT_NOSER1_LSH         0
#define NFC_ECC_STATUS_RESULT_NOSER2_LSH         4
#define NFC_ECC_STATUS_RESULT_NOSER3_LSH         8
#define NFC_ECC_STATUS_RESULT_NOSER4_LSH         12
#define NFC_ECC_STATUS_RESULT_NOSER5_LSH         16
#define NFC_ECC_STATUS_RESULT_NOSER6_LSH         20
#define NFC_ECC_STATUS_RESULT_NOSER7_LSH         24
#define NFC_ECC_STATUS_RESULT_NOSER8_LSH         28

// STATUS_SUM register
#define NFC_STATUS_SUM_NF_STATUS_SUM_LSH            0
#define NFC_STATUS_SUM_ECC_SUM_LSH                  8

// LAUNCH_NFC register
#define NFC_LAUNCH_NFC_FCMD_LSH                     0
#define NFC_LAUNCH_NFC_FADD_LSH                     1
#define NFC_LAUNCH_NFC_FDI_LSH                      2
#define NFC_LAUNCH_NFC_FDO_PAGE_LSH                 3
#define NFC_LAUNCH_NFC_FDO_ID_LSH                   4
#define NFC_LAUNCH_NFC_FDO_STATUS_LSH               5
#define NFC_LAUNCH_NFC_AUTO_PROG_LSH                6
#define NFC_LAUNCH_NFC_AUTO_READ_LSH                7
#define NFC_LAUNCH_NFC_AUTO_ERASE_LSH               9
#define NFC_LAUNCH_NFC_AUTO_COPYBACK0_LSH           10
#define NFC_LAUNCH_NFC_AUTO_COPYBACK1_LSH           11
#define NFC_LAUNCH_NFC_AUTO_STATUS_LSH              12

// NF_WR_PROT register
#define NFC_NF_WR_PROT_WPC_LSH                   0
#define NFC_NF_WR_PROT_CS2L_LSH                  3
#define NFC_NF_WR_PROT_BLS_LSH                   6
#define NFC_NF_WR_PROT_LTS0_LSH                  8
#define NFC_NF_WR_PROT_LS0_LSH                   9
#define NFC_NF_WR_PROT_US0_LSH                   10
#define NFC_NF_WR_PROT_LTS1_LSH                  11
#define NFC_NF_WR_PROT_LS1_LSH                   12
#define NFC_NF_WR_PROT_US1_LSH                   13
#define NFC_NF_WR_PROT_LTS2_LSH                  14
#define NFC_NF_WR_PROT_LS2_LSH                   15
#define NFC_NF_WR_PROT_US2_LSH                   16
#define NFC_NF_WR_PROT_LTS3_LSH                  17
#define NFC_NF_WR_PROT_LS3_LSH                   18
#define NFC_NF_WR_PROT_US3_LSH                   19
#define NFC_NF_WR_PROT_LTS4_LSH                  20
#define NFC_NF_WR_PROT_LS4_LSH                   21
#define NFC_NF_WR_PROT_US4_LSH                   22
#define NFC_NF_WR_PROT_LTS5_LSH                  23
#define NFC_NF_WR_PROT_LS5_LSH                   24
#define NFC_NF_WR_PROT_US5_LSH                   25
#define NFC_NF_WR_PROT_LTS6_LSH                  26
#define NFC_NF_WR_PROT_LS6_LSH                   27
#define NFC_NF_WR_PROT_US6_LSH                   28
#define NFC_NF_WR_PROT_LTS7_LSH                  29
#define NFC_NF_WR_PROT_LS7_LSH                   30
#define NFC_NF_WR_PROT_US7_LSH                   31

// UNLOCK_BLK_ADD register
#define NFC_UNLOCK_BLK_ADD_USBA_LSH                 0
#define NFC_UNLOCK_BLK_ADD_UEBA_LSH                 16

// NFC_CONFIGURATION2
#define NFC_NFC_CONFIGURATION2_PS_LSH                   0
#define NFC_NFC_CONFIGURATION2_SYM_LSH                  2
#define NFC_NFC_CONFIGURATION2_ECC_EN_LSH               3
#define NFC_NFC_CONFIGURATION2_NUM_CMD_PHASES_LSH       4
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES0_LSH      5
#define NFC_NFC_CONFIGURATION2_ECC_MODE_LSH             6
#define NFC_NFC_CONFIGURATION2_PPB_LSH                  7
#define NFC_NFC_CONFIGURATION2_EDC_LSH                  9
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES1_LSH      12
#define NFC_NFC_CONFIGURATION2_AUTO_PROG_DONE_MSK_LSH   14
#define NFC_NFC_CONFIGURATION2_INT_MSK_LSH              15
#define NFC_NFC_CONFIGURATION2_SPAS_LSH                 16
#define NFC_NFC_CONFIGURATION2_ST_CMD_LSH               24

//NFC_CONFIGURATION3
#define NFC_NFC_CONFIGURATION3_ADD_OP_LSH               0
#define NFC_NFC_CONFIGURATION3_TOO_LSH                  2
#define NFC_NFC_CONFIGURATION3_FW_LSH                   3
#define NFC_NFC_CONFIGURATION3_SB2R_LSH                 4
#define NFC_NFC_CONFIGURATION3_NF_BIG_LSH               7
#define NFC_NFC_CONFIGURATION3_SBB_LSH                  8
#define NFC_NFC_CONFIGURATION3_DMA_MODE_LSH             11
#define NFC_NFC_CONFIGURATION3_NUM_OF_DEVICES_LSH       12
#define NFC_NFC_CONFIGURATION3_RBB_MODE_LSH             15       
#define NFC_NFC_CONFIGURATION3_FMP_LSH                  16
#define NFC_NFC_CONFIGURATION3_NO_SDMA_LSH              20

// NFC_IPC
#define NFC_NFC_IPC_CREQ_LSH                        0
#define NFC_NFC_IPC_CACK_LSH                        1
#define NFC_NFC_IPC_DMA_STATUS_LSH                  26
#define NFC_NFC_IPC_RB_B_LSH                        28
#define NFC_NFC_IPC_LPS_LSH                         29
#define NFC_NFC_IPC_AUTO_PROG_DONE_LSH              30
#define NFC_NFC_IPC_INT_LSH                         31

//AXI_ERR_ADD
#define NFC_AXI_ERR_ADD_LSH                         0


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

// NAND_CMD register
#define NFC_NAND_CMD_COMMAND0_WID             8
#define NFC_NAND_CMD_COMMAND1_WID             8               
#define NFC_NAND_CMD_COMMAND2_WID             8
#define NFC_NAND_CMD_COMMAND3_WID             8

//NAND_ADD_LOW register
#define NFC_NAND_ADD_LOW_COLUMN_WID             16  
#define NFC_NAND_ADD_LOW_ROW_WID                16

//NAND_ADD_HIGH register
#define NFC_NAND_ADD_HIGH_EVEN_GROUP_WID        16  
#define NFC_NAND_ADD_HIGH_ODD_GROUP_WID         16

// NFC_CONFIGURATION1 register
#define NFC_NFC_CONFIGURATION1_SP_EN_WID         1
#define NFC_NFC_CONFIGURATION1_NF_CE_WID         1
#define NFC_NFC_CONFIGURATION1_NFC_RST_WID       1
#define NFC_NFC_CONFIGURATION1_RBA_WID           3
#define NFC_NFC_CONFIGURATION1_NUM_OF_ITE_WID     4
#define NFC_NFC_CONFIGURATION1_CS_WID            3
#define NFC_NFC_CONFIGURATION1_NF_STATUS_WID      8

// ECC_STATUS_RESULT register
#define NFC_ECC_STATUS_RESULT_NOSER1_WID         4
#define NFC_ECC_STATUS_RESULT_NOSER2_WID         4
#define NFC_ECC_STATUS_RESULT_NOSER3_WID         4
#define NFC_ECC_STATUS_RESULT_NOSER4_WID         4
#define NFC_ECC_STATUS_RESULT_NOSER5_WID         4
#define NFC_ECC_STATUS_RESULT_NOSER6_WID         4
#define NFC_ECC_STATUS_RESULT_NOSER7_WID         4
#define NFC_ECC_STATUS_RESULT_NOSER8_WID         4

// STATUS_SUM register
// Each NAND flash has 1 bit stored in the segment to indicate the value of pass/fail status bit
#define NFC_STATUS_SUM_NF_STATUS_SUM_WID            8
// Each NAND flash has 1 bit stored in the segment to indicate the value of ECC result for last page read.
#define NFC_STATUS_SUM_ECC_SUM_WID                  8

// LAUNCH_NFC register
#define NFC_LAUNCH_NFC_FCMD_WID                     1
#define NFC_LAUNCH_NFC_FADD_WID                     1
#define NFC_LAUNCH_NFC_FDI_WID                      1
#define NFC_LAUNCH_NFC_FDO_PAGE_WID                 1
#define NFC_LAUNCH_NFC_FDO_ID_WID                   1
#define NFC_LAUNCH_NFC_FDO_STATUS_WID               1
#define NFC_LAUNCH_NFC_AUTO_PROG_WID                1
#define NFC_LAUNCH_NFC_AUTO_READ_WID                1
#define NFC_LAUNCH_NFC_AUTO_ERASE_WID               1
#define NFC_LAUNCH_NFC_AUTO_COPYBACK0_WID           1
#define NFC_LAUNCH_NFC_AUTO_COPYBACK1_WID           1
#define NFC_LAUNCH_NFC_AUTO_STATUS_WID              1

// NF_WR_PROT register
#define NFC_NF_WR_PROT_WPC_WID                      3
#define NFC_NF_WR_PROT_CS2L_WID                     3
#define NFC_NF_WR_PROT_BLS_WID                      2
#define NFC_NF_WR_PROT_LTS0_WID                  1
#define NFC_NF_WR_PROT_LS0_WID                   1
#define NFC_NF_WR_PROT_US0_WID                   1
#define NFC_NF_WR_PROT_LTS1_WID                  1
#define NFC_NF_WR_PROT_LS1_WID                   1
#define NFC_NF_WR_PROT_US1_WID                   1
#define NFC_NF_WR_PROT_LTS2_WID                  1
#define NFC_NF_WR_PROT_LS2_WID                   1
#define NFC_NF_WR_PROT_US2_WID                   1
#define NFC_NF_WR_PROT_LTS3_WID                  1
#define NFC_NF_WR_PROT_LS3_WID                   1
#define NFC_NF_WR_PROT_US3_WID                   1
#define NFC_NF_WR_PROT_LTS4_WID                  1
#define NFC_NF_WR_PROT_LS4_WID                   1
#define NFC_NF_WR_PROT_US4_WID                   1
#define NFC_NF_WR_PROT_LTS5_WID                  1
#define NFC_NF_WR_PROT_LS5_WID                   1
#define NFC_NF_WR_PROT_US5_WID                   1
#define NFC_NF_WR_PROT_LTS6_WID                  1
#define NFC_NF_WR_PROT_LS6_WID                   1
#define NFC_NF_WR_PROT_US6_WID                   1
#define NFC_NF_WR_PROT_LTS7_WID                  1
#define NFC_NF_WR_PROT_LS7_WID                   1
#define NFC_NF_WR_PROT_US7_WID                   1

// UNLOCK_BLK_ADD0 register
#define NFC_UNLOCK_BLK_ADD_USBA_WID                 16
#define NFC_UNLOCK_BLK_ADD_UEBA_WID                 16

// NFC_CONFIGURATION2
#define NFC_NFC_CONFIGURATION2_PS_WID               2
#define NFC_NFC_CONFIGURATION2_SYM_WID              1
#define NFC_NFC_CONFIGURATION2_ECC_EN_WID           1
#define NFC_NFC_CONFIGURATION2_NUM_CMD_PHASES_WID   1
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES0_WID  1
#define NFC_NFC_CONFIGURATION2_ECC_MODE_WID        1
#define NFC_NFC_CONFIGURATION2_PPB_WID              2
#define NFC_NFC_CONFIGURATION2_EDC_WID              3
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES1_WID  2
#define NFC_NFC_CONFIGURATION2_AUTO_PROG_DONE_MSK_WID   1
#define NFC_NFC_CONFIGURATION2_INT_MSK_WID          1
#define NFC_NFC_CONFIGURATION2_SPAS_WID             8
#define NFC_NFC_CONFIGURATION2_ST_CMD_WID           84

//NFC_CONFIGURATION3
#define NFC_NFC_CONFIGURATION3_ADD_OP_WID           2
#define NFC_NFC_CONFIGURATION3_TOO_WID              1
#define NFC_NFC_CONFIGURATION3_FW_WID               1
#define NFC_NFC_CONFIGURATION3_SB2R_WID             3
#define NFC_NFC_CONFIGURATION3_NF_BIG_WID           1
#define NFC_NFC_CONFIGURATION3_SBB_WID              3
#define NFC_NFC_CONFIGURATION3_DMA_MODE_WID         1
#define NFC_NFC_CONFIGURATION3_NUM_OF_DEVICES_WID   3
#define NFC_NFC_CONFIGURATION3_RBB_MODE_WID         1
#define NFC_NFC_CONFIGURATION3_FMP_WID              4
#define NFC_NFC_CONFIGURATION3_NO_SDMA_WID          1

// NFC_IPC
#define NFC_NFC_IPC_CREQ_WID                        1
#define NFC_NFC_IPC_CACK_WID                        1
#define NFC_NFC_IPC_DMA_STATUS_WID                  2
#define NFC_NFC_IPC_RB_B_WID                        1
#define NFC_NFC_IPC_LPS_WID                         1
#define NFC_NFC_IPC_INT_WID                         1
#define NFC_NFC_IPC_AUTO_PROG_DONE_WID              1
#define NFC_NFC_IPC_INT_WID                         1

//AXI_ERR_ADD
#define NFC_AXI_ERR_ADD_WID                         32


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// NFC_CONFIGURATION1 register
#define NFC_NFC_CONFIGURATION1_SP_EN_MAIN_SPARE         0
#define NFC_NFC_CONFIGURATION1_SP_EN_SPARE_ONLY         1

#define NFC_NFC_CONFIGURATION1_NF_CE_UNFORCE            0
#define NFC_NFC_CONFIGURATION1_NF_CE_FORCE              1

#define NFC_NFC_CONFIGURATION1_NFC_RST_NORESET          0
#define NFC_NFC_CONFIGURATION1_NFC_RST_REST             1

#define NFC_NFC_CONFIGURATION1_RBA_FIRST_RAM            0
#define NFC_NFC_CONFIGURATION1_RBA_SECOND_RAM           1
#define NFC_NFC_CONFIGURATION1_RBA_THIRD_RAM            2
#define NFC_NFC_CONFIGURATION1_RBA_FORTH_RAM            3
#define NFC_NFC_CONFIGURATION1_RBA_FIFTH_RAM            4
#define NFC_NFC_CONFIGURATION1_RBA_SIXTH_RAM            5
#define NFC_NFC_CONFIGURATION1_RBA_SEVENTH_RAM          6
#define NFC_NFC_CONFIGURATION1_RBA_EIGHTH_RAM           7

#define NFC_NFC_CONFIGURATION1_CS_ACTIVE_CS0            0

// Launch NFC register
#define NFC_LAUNCH_NFC_AUTO_PROG_ENABLE                 1
#define NFC_LAUNCH_NFC_AUTO_PROG_DISABLE                0

#define NFC_LAUNCH_NFC_AUTO_READ_ENABLE                 1
#define NFC_LAUNCH_NFC_AUTO_READ_DISABLE                0

#define NFC_LAUNCH_NFC_AUTO_ERASE_ENABLE                1
#define NFC_LAUNCH_NFC_AUTO_ERASE_DISABLE               0

// for a single device operation which is specified by addr_op 
#define NFC_LAUNCH_NFC_AUTO_COPYBACK0_ENABLE            1
#define NFC_LAUNCH_NFC_AUTO_COPYBACK0_DISABLE           0

// for all devices operation 
#define NFC_LAUNCH_NFC_AUTO_COPYBACK1_ENABLE            1
#define NFC_LAUNCH_NFC_AUTO_COPYBACK1_DISABLE           0

//for a single device specified by active_cs
#define NFC_LAUNCH_NFC_AUTO_STATUS_ENABLE               1
#define NFC_LAUNCH_NFC_AUTO_STATUS_DISABLE              0

// ECC_STATUS_RESULT register
#define NFC_ECC_STATUS_RESULT_NOSER_NO_ERR             0
#define NFC_ECC_STATUS_RESULT_NOSER_1SB_ERR            1
#define NFC_ECC_STATUS_RESULT_NOSER_2SB_ERR            2
#define NFC_ECC_STATUS_RESULT_NOSER_3SB_ERR            3
#define NFC_ECC_STATUS_RESULT_NOSER_4SB_ERR            4
#define NFC_ECC_STATUS_RESULT_NOSER_5SB_ERR            5
#define NFC_ECC_STATUS_RESULT_NOSER_6SB_ERR            6
#define NFC_ECC_STATUS_RESULT_NOSER_7SB_ERR            7
#define NFC_ECC_STATUS_RESULT_NOSER_8SB_ERR            8

// NF_WR_PROT register
#define NFC_NF_WR_PROT_WPC_UNLOCK                      4
#define NFC_NF_WR_PROT_WPC_LOCK_ALL                    2
#define NFC_NF_WR_PROT_WPC_LOCK_TIGHT                  1

#define NFC_NF_WR_PROT_BLS_LOCKED                      1
#define NFC_NF_WR_PROT_BLS_UNLOCKED                    2

#define NFC_NF_WR_PROT_CS2L_CS0LOCK_STATUS             0                 

// NFC_CONFIGURATION2
#define NFC_NFC_CONFIGURATION2_PS_512B                  0
#define NFC_NFC_CONFIGURATION2_PS_2KB                   1
#define NFC_NFC_CONFIGURATION2_PS_4KB                   2
#define NFC_NFC_CONFIGURATION2_PS_8KB                   3

#define NFC_NFC_CONFIGURATION2_SYM_SYMMETRIC           1
#define NFC_NFC_CONFIGURATION2_SYM_ASYMMETRIC          0

#define NFC_NFC_CONFIGURATION2_ECC_EN_BYPASS           0
#define NFC_NFC_CONFIGURATION2_ECC_EN_ENABLE           1

//Used for page read
//How many commands needed for read
#define NFC_NFC_CONFIGURATION2_NUM_CMD_PHASES_2       1
#define NFC_NFC_CONFIGURATION2_NUM_CMD_PHASES_1       0

//Used for erase operation: 
//1: 2 cycle less than what specified by num_of_adr_phase1
//0: 1 cycle less than what specified by num_of_adr_phase1
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES0_2        1
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES0_1        0

#define NFC_NFC_CONFIGURATION2_ECC_MODE_8BIT           0
#define NFC_NFC_CONFIGURATION2_ECC_MODE_4BIT           1

#define NFC_NFC_CONFIGURATION2_PPB_32_PAGES            0
#define NFC_NFC_CONFIGURATION2_PPB_64_PAGES            1
#define NFC_NFC_CONFIGURATION2_PPB_128_PAGES           2
#define NFC_NFC_CONFIGURATION2_PPB_256_PAGES           3

//Used for read/program operation: 
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES1_3        0
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES1_4        1
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES1_5        2
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES1_6        4

#define NFC_NFC_CONFIGURATION2_AUTO_PROG_DONE_MSK_MASK      1
#define NFC_NFC_CONFIGURATION2_AUTO_PROG_DONE_MSK_UNMASK    0

#define NFC_NFC_CONFIGURATION2_INT_MSK_UNMASK          0
#define NFC_NFC_CONFIGURATION2_INT_MSK_MASK            1

// NFC_CONFIGURATION3
//No CE extraction, single address group used
#define NFC_NFC_CONFIGURATION3_ADD_OP_ADDR_GROUP0      0
//CE extraction, single address group used
#define NFC_NFC_CONFIGURATION3_ADD_OP_ADDR_GROUP0_CS    1
//No CE extraction, all address groups used
#define NFC_NFC_CONFIGURATION3_ADD_OP_ADDR_GROUP        2
//CE extraction, all address groups used
#define NFC_NFC_CONFIGURATION3_ADD_OP_ADDR_GROUP_CS     3

#define NFC_NFC_CONFIGURATION3_TOO_ONE_DEVICE           0
#define NFC_NFC_CONFIGURATION3_TOO_TWO_DEVICES          1

#define NFC_NFC_CONFIGURATION3_FW_8BIT                  1
#define NFC_NFC_CONFIGURATION3_FW_16BIT                 0

#define NFC_CONFIGURATION3_NF_BIG_LITTLE                0
#define NFC_CONFIGURATION3_NF_BIG_BIG                   1

//2INT: dma_rd_req and dma_wr_req
#define NFC_NFC_CONFIGURATION3_DMA_MODE_2INT            0
//1INT: dma_rd_req | dma_wr_req
#define NFC_NFC_CONFIGURATION3_DMA_MODE_1INT            1

#define NFC_NFC_CONFIGURATION3_RBB_MODE_CMD             0
#define NFC_NFC_CONFIGURATION3_RBB_MODE_SIGNAL          1

#define NFC_NFC_CONFIGURATION3_FMP_NONE                 0
#define NFC_NFC_CONFIGURATION3_FMP_64                   1
#define NFC_NFC_CONFIGURATION3_FMP_128                  2
#define NFC_NFC_CONFIGURATION3_FMP_256                  3
#define NFC_NFC_CONFIGURATION3_FMP_512                  4
#define NFC_NFC_CONFIGURATION3_FMP_1K                   5
#define NFC_NFC_CONFIGURATION3_FMP_2K                   6
#define NFC_NFC_CONFIGURATION3_FMP_4K                   7

#define NFC_NFC_CONFIGURATION3_NO_SDMA_ENABLE           1
#define NFC_NFC_CONFIGURATION3_NO_SDMA_DISABLE          0

// NFC_IPC
#define NFC_NFC_IPC_CREQ_NO_REQUEST                     0
#define NFC_NFC_IPC_CREQ_REQUEST                        1

#define NFC_NFC_IPC_CACK_FORBIDDEN                      0
#define NFC_NFC_IPC_CACK_PERMITTED                      1

#define NFC_NFC_IPC_DMA_STATUS_NO_REQ                   0
#define NFC_NFC_IPC_DMA_STATUS_WR_REQ                   1
#define NFC_NFC_IPC_DMA_STATUS_RD_REQ                   2
#define NFC_NFC_IPC_DMA_STATUS_WR_RD_REQ                3

#define NFC_NFC_IPC_RB_B_BUSY                           0
#define NFC_NFC_IPC_RB_B_READY                          1

#define NFC_NFC_IPC_LPS_NORMAL_MODE                0
#define NFC_NFC_IPC_LPS_LOWPOWER_MODE              1

#define NFC_NFC_IPC_AUTO_PROG_DONE_ONGOING          0
#define NFC_NFC_IPC_AUTO_PROG_DONE_FINISHED         1

#define NFC_NFC_IPC_INT_ONGOING                     0
#define NFC_NFC_IPC_INT_FINISHED                    1

#define NFC_ECC_BIT_UNCORRECTABLE_ERROR             0xF
#define NFC_ECC_SECTION_BITS                        4

#ifdef __cplusplus
}
#endif

#endif // __MX51_NFC_H

