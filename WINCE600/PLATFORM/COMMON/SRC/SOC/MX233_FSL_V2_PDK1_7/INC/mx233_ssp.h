//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: mx233_ssp.h
//
//  Provides definitions for the SSP (SYNCHRONOUS SERIAL PORTS)
//  module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __MX233_SSP_H
#define __MX233_SSP_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 CTRL0[4];        // 0x000 
    UINT32 CMD0[4];         // 0x010 
    UINT32 CMD1[4];         // 0x020 
    UINT32 COMPREF[4];      // 0x030 
    UINT32 COMPMASK[4];     // 0x040 
    UINT32 TIMING[4];       // 0x050 
    UINT32 CTRL1[4];        // 0x060 
    UINT32 DATA[4];         // 0x070 
    UINT32 SDRESP0[4];      // 0x080 
    UINT32 SDRESP1[4];      // 0x090 
    UINT32 SDRESP2[4];      // 0x0A0 
    UINT32 SDRESP3[4];      // 0x0B0 
    UINT32 STATUS[4];       // 0x0C0 
    UINT32 DEBUGREG[4];     // 0x100 
    UINT32 VERSION[4];      // 0x110
} CSP_SSP_REGS, *PCSP_SSP_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define SSP_CTRL0_OFFSET           0x0000 
#define SSP_CMD0_OFFSET            0x0010 
#define SSP_CMD1_OFFSET            0x0020 
#define SSP_COMPREF_OFFSET         0x0030 
#define SSP_COMPMASK_OFFSET        0x0040 
#define SSP_TIMING_OFFSET          0x0050 
#define SSP_CTRL1_OFFSET           0x0060 
#define SSP_DATA_OFFSET            0x0070 
#define SSP_SDRESP0_OFFSET         0x0080 
#define SSP_SDRESP1_OFFSET         0x0090 
#define SSP_SDRESP2_OFFSET         0x00A0 
#define SSP_SDRESP3_OFFSE          0x00B0 
#define SSP_STATUS_OFFSET          0x00C0 
#define SSP_DEBUG_OFFSET           0x0100 
#define SSP_VERSION_OFFSET         0x0110


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define SSP_CTRL0_XFER_COUNT_LSH                0
#define SSP_CTRL0_ENABLE_LSH                    16
#define SSP_CTRL0_GET_RESP_LSH                  17
#define SSP_CTRL0_CHECK_RESP_LSH                18
#define SSP_CTRL0_LONG_RESP_LSH                 19
#define SSP_CTRL0_WAIT_FOR_CMD_LSH              20
#define SSP_CTRL0_WAIT_FOR_IRQ_LSH              21
#define SSP_CTRL0_BUS_WIDTH_LSH                 22
#define SSP_CTRL0_DATA_XFER_LSH                 24
#define SSP_CTRL0_READ_LSH                      25
#define SSP_CTRL0_IGNORE_CRC_LSH                26
#define SSP_CTRL0_LOCK_CS_LSH                   27
#define SSP_CTRL0_SDIO_IRQ_CHECK_LSH            28
#define SSP_CTRL0_RUN_LSH                       29
#define SSP_CTRL0_CLKGATE_LSH                   30
#define SSP_CTRL0_SFTRST_LSH                    31

#define SSP_CMD0_BLOCK_COUNT_LSH                0
#define SSP_CMD0_BLOCK_SIZE_LSH                 16
#define SSP_CMD0_APPEND_8CYC_LSH                20
#define SSP_CMD0_CONT_CLKING_EN_LSH             21
#define SSP_CMD0_SLOW_CLKING_EN_LSH             22

#define SSP_TIMING_CLOCK_RATE_LSH               0
#define SSP_TIMING_CLOCK_DIVIDE_LSH             8
#define SSP_TIMING_TIMEOUT_LSH                  16

#define SSP_CTRL1_SSP_MODE_LSH                  0
#define SSP_CTRL1_WORD_LENGTH_LSH               4
#define SSP_CTRL1_SLAVE_MODE_LSH                8
#define SSP_CTRL1_POLARITY_LSH                  9
#define SSP_CTRL1_PHASE_LSH                     10
#define SSP_CTRL1_SLAVE_OUT_DISABLE_LSH         11
#define SSP_CTRL1_CEATA_CCS_ERR_EN_LSH          12
#define SSP_CTRL1_DMA_ENABLE_LSH                13
#define SSP_CTRL1_FIFO_OVERRUN_IRQ_EN_LSH       14
#define SSP_CTRL1_FIFO_OVERRUN_IRQ_LSH          15
#define SSP_CTRL1_RECV_TIMEOUT_IRQ_EN_LSH       16
#define SSP_CTRL1_RECV_TIMEOUT_IRQ_LSH          17
#define SSP_CTRL1_CEATA_CCS_ERR_IRQ_EN_LSH      18
#define SSP_CTRL1_CEATA_CCS_ERR_IRQ_LSH         19
#define SSP_CTRL1_FIFO_UNDERRUN_EN_LSH          20
#define SSP_CTRL1_FIFO_UNDERRUN_IRQ_LSH         11
#define SSP_CTRL1_DATA_CRC_IRQ_EN_LSH           22
#define SSP_CTRL1_DATA_CRC_IRQ_LSH              23
#define SSP_CTRL1_DATA_TIMEOUT_IRQ_EN_LSH       24
#define SSP_CTRL1_DATA_TIMEOUT_IRQ_LSH          25
#define SSP_CTRL1_RESP_TIMEOUT_IRQ_EN_LSH       26
#define SSP_CTRL1_RESP_TIMEOUT_IRQ_LSH          27
#define SSP_CTRL1_RESP_ERR_IRQ_EN_LSH           28
#define SSP_CTRL1_RESP_ERR_IRQ_LSH              29
#define SSP_CTRL1_SDIO_IRQ_EN_LSH               30
#define SSP_CTRL1_SDIO_IRQ_LSH                  31

#define SSP_STATUS_BUSY_LSH                     0
#define SSP_STATUS_DATA_BUSY_LSH                2
#define SSP_STATUS_CMD_BUSY_LSH                 3 
#define SSP_STATUS_FIFO_UNDRFLW_LSH             4
#define SSP_STATUS_FIFO_EMPTY_LSH               5
#define SSP_STATUS_FIFO_FULL_LSH                8
#define SSP_STATUS_FIFO_OVRFLW_LSH              9
#define SSP_STATUS_CEATA_CCS_ERR_LSH            10
#define SSP_STATUS_RECV_TIMEOUT_STAT_LSH        11
#define SSP_STATUS_TIMEOUT_LSH                  12
#define SSP_STATUS_DATA_CRC_ERR_LSH             13
#define SSP_STATUS_RESP_TIMEOUT_LSH             14
#define SSP_STATUS_RESP_ERR_LSH                 15
#define SSP_STATUS_RESP_CRC_ERR_LSH             16
#define SSP_STATUS_SDIO_IRQ_LSH                 17
#define SSP_STATUS_DMAEND_LSH                   18
#define SSP_STATUS_DMAREQ_LSH                   19
#define SSP_STATUS_DMATERM_LSH                  20
#define SSP_STATUS_DMASENSE_LSH                 21
#define SSP_STATUS_CARD_DETECT_LSH              28
#define SSP_STATUS_SD_PRESENT_LSH               29
#define SSP_STATUS_MS_PRESENT_LSH               30
#define SSP_STATUS_PRESENT_LSH                  31  


#define SSP_DEBUG_SSP_RXD_LSH                   0
#define SSP_DEBUG_SSP_RESP_LSH                  8 
#define SSP_DEBUG_SSP_CMD_LSH                   9
#define SSP_DEBUG_CMD_SM_LSH                    10
#define SSP_DEBUG_MMC_SM_LSH                    12
#define SSP_DEBUG_DMA_SM_LSH                    16
#define SSP_DEBUG_CMD_OE_LSH                    19
#define SSP_DEBUG_MSTK_SM_LSH                   20
#define SSP_DEBUG_DAT_SM_LSH                    24
#define SSP_DEBUG_DATA_STALL_LSH                27
#define SSP_DEBUG_DATACRC_ERR_LSH               28

#define SSP_VERSION_STEP_LSH                    0
#define SSP_VERSION_MINOR_LSH                   16 
#define SSP_VERSION_MAJOR_LSH                   24


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define SSP_CTRL0_XFER_COUNT_WID                16
#define SSP_CTRL0_ENABLE_WID                    1
#define SSP_CTRL0_GET_RESP_WID                  1
#define SSP_CTRL0_CHECK_RESP_WID                1
#define SSP_CTRL0_LONG_RESP_WID                 1
#define SSP_CTRL0_WAIT_FOR_CMD_WID              1
#define SSP_CTRL0_WAIT_FOR_IRQ_WID              1
#define SSP_CTRL0_BUS_WIDTH_WID                 2
#define SSP_CTRL0_DATA_XFER_WID                 1
#define SSP_CTRL0_READ_WID                      1
#define SSP_CTRL0_IGNORE_CRC_WID                1
#define SSP_CTRL0_LOCK_CS_WID                   1
#define SSP_CTRL0_SDIO_IRQ_CHECK_WID            1
#define SSP_CTRL0_RUN_WID                       1
#define SSP_CTRL0_CLKGATE_WID                   1
#define SSP_CTRL0_SFTRST_WID                    1

#define SSP_CMD0_CMD_WID                        8
#define SSP_CMD0_BLOCK_COUNT_WID                8
#define SSP_CMD0_BLOCK_SIZE_WID                 4
#define SSP_CMD0_APPEND_8CYC_WID                1
#define SSP_CMD0_CONT_CLKING_EN_WID             1
#define SSP_CMD0_SLOW_CLKING_EN_WID             1

#define SSP_TIMING_CLOCK_RATE_WID               8
#define SSP_TIMING_CLOCK_DIVIDE_WID             8
#define SSP_TIMING_TIMEOUT_WID                  16

#define SSP_CTRL1_SSP_MODE_WID                  4
#define SSP_CTRL1_WORD_LENGTH_WID               4
#define SSP_CTRL1_SLAVE_MODE_WID                1
#define SSP_CTRL1_POLARITY_WID                  1
#define SSP_CTRL1_PHASE_WID                     1
#define SSP_CTRL1_SLAVE_OUT_DISABLE_WID         1
#define SSP_CTRL1_CEATA_CCS_ERR_EN_WID          1
#define SSP_CTRL1_DMA_ENABLE_WID                1
#define SSP_CTRL1_FIFO_OVERRUN_IRQ_EN_WID       1
#define SSP_CTRL1_FIFO_OVERRUN_IRQ_WID          1
#define SSP_CTRL1_RECV_TIMEOUT_IRQ_EN_WID       1
#define SSP_CTRL1_RECV_TIMEOUT_IRQ_WID          1
#define SSP_CTRL1_CEATA_CCS_ERR_IRQ_EN_WID      1
#define SSP_CTRL1_CEATA_CCS_ERR_IRQ_WID         1
#define SSP_CTRL1_FIFO_UNDERRUN_EN_WID          1
#define SSP_CTRL1_FIFO_UNDERRUN_IRQ_WID         1
#define SSP_CTRL1_DATA_CRC_IRQ_EN_WID           1
#define SSP_CTRL1_DATA_CRC_IRQ_WID              1
#define SSP_CTRL1_DATA_TIMEOUT_IRQ_EN_WID       1
#define SSP_CTRL1_DATA_TIMEOUT_IRQ_WID          1
#define SSP_CTRL1_RESP_TIMEOUT_IRQ_EN_WID       1
#define SSP_CTRL1_RESP_TIMEOUT_IRQ_WID          1
#define SSP_CTRL1_RESP_ERR_IRQ_EN_WID           1
#define SSP_CTRL1_RESP_ERR_IRQ_WID              1
#define SSP_CTRL1_SDIO_IRQ_EN_WID               1
#define SSP_CTRL1_SDIO_IRQ_WID                  1

#define SSP_STATUS_BUSY_WID                     1
#define SSP_STATUS_DATA_BUSY_WID                1
#define SSP_STATUS_CMD_BUSY_WID                 1 
#define SSP_STATUS_FIFO_UNDRFLW_WID             1
#define SSP_STATUS_FIFO_EMPTY_WID               1
#define SSP_STATUS_FIFO_FULL_WID                1
#define SSP_STATUS_FIFO_OVRFLW_WID              1
#define SSP_STATUS_CEATA_CCS_ERR_WID            1
#define SSP_STATUS_RECV_TIMEOUT_STAT_WID        1
#define SSP_STATUS_TIMEOUT_WID                  1
#define SSP_STATUS_DATA_CRC_ERR_WID             1
#define SSP_STATUS_RESP_TIMEOUT_WID             1
#define SSP_STATUS_RESP_ERR_WID                 1
#define SSP_STATUS_RESP_CRC_ERR_WID             1
#define SSP_STATUS_SDIO_IRQ_WID                 1
#define SSP_STATUS_DMAEND_WID                   1
#define SSP_STATUS_DMAREQ_WID                   1
#define SSP_STATUS_DMATERM_WID                  1
#define SSP_STATUS_DMASENSE_WID                 1
#define SSP_STATUS_CARD_DETECT_WID              1
#define SSP_STATUS_SD_PRESENT_WID               1
#define SSP_STATUS_MS_PRESENT_WID               1
#define SSP_STATUS_PRESENT_WID                  1  


#define SSP_DEBUG_SSP_RXD_WID                   8
#define SSP_DEBUG_SSP_RESP_WID                  1 
#define SSP_DEBUG_SSP_CMD_WID                   1
#define SSP_DEBUG_CMD_SM_WID                    1
#define SSP_DEBUG_MMC_SM_WID                    4
#define SSP_DEBUG_DMA_SM_WID                    3
#define SSP_DEBUG_CMD_OE_WID                    1
#define SSP_DEBUG_MSTK_SM_WID                   4
#define SSP_DEBUG_DAT_SM_WID                    3
#define SSP_DEBUG_DATA_STALL_WID                1
#define SSP_DEBUG_DATACRC_ERR_WID               4

#define SSP_VERSION_STEP_WID                    16
#define SSP_VERSION_MINOR_WID                   8 
#define SSP_VERSION_MAJOR_WID                   8

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define BV_SSP_CMD0_CMD__MMC_GO_IDLE_STATE         0x00
#define BV_SSP_CMD0_CMD__MMC_SEND_OP_COND          0x01
#define BV_SSP_CMD0_CMD__MMC_ALL_SEND_CID          0x02
#define BV_SSP_CMD0_CMD__MMC_SET_RELATIVE_ADDR     0x03
#define BV_SSP_CMD0_CMD__MMC_SET_DSR               0x04
#define BV_SSP_CMD0_CMD__MMC_RESERVED_5            0x05
#define BV_SSP_CMD0_CMD__MMC_SWITCH                0x06
#define BV_SSP_CMD0_CMD__MMC_SELECT_DESELECT_CARD  0x07
#define BV_SSP_CMD0_CMD__MMC_SEND_EXT_CSD          0x08
#define BV_SSP_CMD0_CMD__MMC_SEND_CSD              0x09
#define BV_SSP_CMD0_CMD__MMC_SEND_CID              0x0A
#define BV_SSP_CMD0_CMD__MMC_READ_DAT_UNTIL_STOP   0x0B
#define BV_SSP_CMD0_CMD__MMC_STOP_TRANSMISSION     0x0C
#define BV_SSP_CMD0_CMD__MMC_SEND_STATUS           0x0D
#define BV_SSP_CMD0_CMD__MMC_BUSTEST_R             0x0E
#define BV_SSP_CMD0_CMD__MMC_GO_INACTIVE_STATE     0x0F
#define BV_SSP_CMD0_CMD__MMC_SET_BLOCKLEN          0x10
#define BV_SSP_CMD0_CMD__MMC_READ_SINGLE_BLOCK     0x11
#define BV_SSP_CMD0_CMD__MMC_READ_MULTIPLE_BLOCK   0x12
#define BV_SSP_CMD0_CMD__MMC_BUSTEST_W             0x13
#define BV_SSP_CMD0_CMD__MMC_WRITE_DAT_UNTIL_STOP  0x14
#define BV_SSP_CMD0_CMD__MMC_SET_BLOCK_COUNT       0x17
#define BV_SSP_CMD0_CMD__MMC_WRITE_BLOCK           0x18
#define BV_SSP_CMD0_CMD__MMC_WRITE_MULTIPLE_BLOCK  0x19
#define BV_SSP_CMD0_CMD__MMC_PROGRAM_CID           0x1A
#define BV_SSP_CMD0_CMD__MMC_PROGRAM_CSD           0x1B
#define BV_SSP_CMD0_CMD__MMC_SET_WRITE_PROT        0x1C
#define BV_SSP_CMD0_CMD__MMC_CLR_WRITE_PROT        0x1D
#define BV_SSP_CMD0_CMD__MMC_SEND_WRITE_PROT       0x1E
#define BV_SSP_CMD0_CMD__MMC_ERASE_GROUP_START     0x23
#define BV_SSP_CMD0_CMD__MMC_ERASE_GROUP_END       0x24
#define BV_SSP_CMD0_CMD__MMC_ERASE                 0x26
#define BV_SSP_CMD0_CMD__MMC_FAST_IO               0x27
#define BV_SSP_CMD0_CMD__MMC_GO_IRQ_STATE          0x28
#define BV_SSP_CMD0_CMD__MMC_LOCK_UNLOCK           0x2A
#define BV_SSP_CMD0_CMD__MMC_APP_CMD               0x37
#define BV_SSP_CMD0_CMD__MMC_GEN_CMD               0x38
#define BV_SSP_CMD0_CMD__SD_GO_IDLE_STATE          0x00
#define BV_SSP_CMD0_CMD__SD_ALL_SEND_CID           0x02
#define BV_SSP_CMD0_CMD__SD_SEND_RELATIVE_ADDR     0x03
#define BV_SSP_CMD0_CMD__SD_SET_DSR                0x04
#define BV_SSP_CMD0_CMD__SD_IO_SEND_OP_COND        0x05
#define BV_SSP_CMD0_CMD__SD_SELECT_DESELECT_CARD   0x07
#define BV_SSP_CMD0_CMD__SD_SEND_CSD               0x09
#define BV_SSP_CMD0_CMD__SD_SEND_CID               0x0A
#define BV_SSP_CMD0_CMD__SD_STOP_TRANSMISSION      0x0C
#define BV_SSP_CMD0_CMD__SD_SEND_STATUS            0x0D
#define BV_SSP_CMD0_CMD__SD_GO_INACTIVE_STATE      0x0F
#define BV_SSP_CMD0_CMD__SD_SET_BLOCKLEN           0x10
#define BV_SSP_CMD0_CMD__SD_READ_SINGLE_BLOCK      0x11
#define BV_SSP_CMD0_CMD__SD_READ_MULTIPLE_BLOCK    0x12
#define BV_SSP_CMD0_CMD__SD_WRITE_BLOCK            0x18
#define BV_SSP_CMD0_CMD__SD_WRITE_MULTIPLE_BLOCK   0x19
#define BV_SSP_CMD0_CMD__SD_PROGRAM_CSD            0x1B
#define BV_SSP_CMD0_CMD__SD_SET_WRITE_PROT         0x1C
#define BV_SSP_CMD0_CMD__SD_CLR_WRITE_PROT         0x1D
#define BV_SSP_CMD0_CMD__SD_SEND_WRITE_PROT        0x1E
#define BV_SSP_CMD0_CMD__SD_ERASE_WR_BLK_START     0x20
#define BV_SSP_CMD0_CMD__SD_ERASE_WR_BLK_END       0x21
#define BV_SSP_CMD0_CMD__SD_ERASE_GROUP_START      0x23
#define BV_SSP_CMD0_CMD__SD_ERASE_GROUP_END        0x24
#define BV_SSP_CMD0_CMD__SD_ERASE                  0x26
#define BV_SSP_CMD0_CMD__SD_LOCK_UNLOCK            0x2A
#define BV_SSP_CMD0_CMD__SD_IO_RW_DIRECT           0x34
#define BV_SSP_CMD0_CMD__SD_IO_RW_EXTENDED         0x35
#define BV_SSP_CMD0_CMD__SD_APP_CMD                0x37
#define BV_SSP_CMD0_CMD__SD_GEN_CMD                0x38

// CTRL1
#define BV_SSP_CTRL1_WORD_LENGTH__RESERVED0         0x0
#define BV_SSP_CTRL1_WORD_LENGTH__RESERVED1         0x1
#define BV_SSP_CTRL1_WORD_LENGTH__RESERVED2         0x2
#define BV_SSP_CTRL1_WORD_LENGTH__FOUR_BITS         0x3
#define BV_SSP_CTRL1_WORD_LENGTH__EIGHT_BITS        0x7
#define BV_SSP_CTRL1_WORD_LENGTH__SIXTEEN_BITS      0xF

// SSP_MODE
#define BV_SSP_CTRL1_SSP_MODE__SPI                  0x0
#define BV_SSP_CTRL1_SSP_MODE__SSI                  0x1
#define BV_SSP_CTRL1_SSP_MODE__SD_MMC               0x3
#define BV_SSP_CTRL1_SSP_MODE__MS                   0x4
#define BV_SSP_CTRL1_SSP_MODE__CE_ATA               0x7

//DAT_SM_MODE
#define BV_SSP_DEBUG_DAT_SM__DSM_IDLE               0x0
#define BV_SSP_DEBUG_DAT_SM__DSM_WORD               0x2
#define BV_SSP_DEBUG_DAT_SM__DSM_CRC1               0x3
#define BV_SSP_DEBUG_DAT_SM__DSM_CRC2               0x4
#define BV_SSP_DEBUG_DAT_SM__DSM_END                0x5

//DMA_SM_MODE
#define BV_SSP_DEBUG_DMA_SM__DMA_IDLE               0x0
#define BV_SSP_DEBUG_DMA_SM__DMA_DMAREQ             0x1
#define BV_SSP_DEBUG_DMA_SM__DMA_DMAACK             0x2
#define BV_SSP_DEBUG_DMA_SM__DMA_STALL              0x3
#define BV_SSP_DEBUG_DMA_SM__DMA_BUSY               0x4
#define BV_SSP_DEBUG_DMA_SM__DMA_DONE               0x5
#define BV_SSP_DEBUG_DMA_SM__DMA_COUNT              0x6

//MSTK_SM_MODE
#define BV_SSP_DEBUG_MSTK_SM__MSTK_IDLE             0x0
#define BV_SSP_DEBUG_MSTK_SM__MSTK_CKON             0x1
#define BV_SSP_DEBUG_MSTK_SM__MSTK_BS1              0x2
#define BV_SSP_DEBUG_MSTK_SM__MSTK_TPC              0x3
#define BV_SSP_DEBUG_MSTK_SM__MSTK_BS2              0x4
#define BV_SSP_DEBUG_MSTK_SM__MSTK_HDSHK            0x5
#define BV_SSP_DEBUG_MSTK_SM__MSTK_BS3              0x6
#define BV_SSP_DEBUG_MSTK_SM__MSTK_RW               0x7
#define BV_SSP_DEBUG_MSTK_SM__MSTK_CRC1             0x8
#define BV_SSP_DEBUG_MSTK_SM__MSTK_CRC2             0x9
#define BV_SSP_DEBUG_MSTK_SM__MSTK_BS0              0xA
#define BV_SSP_DEBUG_MSTK_SM__MSTK_END1             0xB
#define BV_SSP_DEBUG_MSTK_SM__MSTK_END2W            0xC
#define BV_SSP_DEBUG_MSTK_SM__MSTK_END2R            0xD
#define BV_SSP_DEBUG_MSTK_SM__MSTK_DONE             0xE

//MMC_SM_MODE
#define BV_SSP_DEBUG_MMC_SM__MMC_IDLE               0x0
#define BV_SSP_DEBUG_MMC_SM__MMC_CMD                0x1
#define BV_SSP_DEBUG_MMC_SM__MMC_TRC                0x2
#define BV_SSP_DEBUG_MMC_SM__MMC_RESP               0x3
#define BV_SSP_DEBUG_MMC_SM__MMC_RPRX               0x4
#define BV_SSP_DEBUG_MMC_SM__MMC_TX                 0x5
#define BV_SSP_DEBUG_MMC_SM__MMC_CTOK               0x6
#define BV_SSP_DEBUG_MMC_SM__MMC_RX                 0x7
#define BV_SSP_DEBUG_MMC_SM__MMC_CCS                0x8
#define BV_SSP_DEBUG_MMC_SM__MMC_PUP                0x9
#define BV_SSP_DEBUG_MMC_SM__MMC_WAIT               0xA

//CMD_SM__MODE
#define BV_SSP_DEBUG_CMD_SM__CSM_IDLE               0x0
#define BV_SSP_DEBUG_CMD_SM__CSM_INDEX              0x1
#define BV_SSP_DEBUG_CMD_SM__CSM_ARG                0x2
#define BV_SSP_DEBUG_CMD_SM__CSM_CRC                0x3


//------------------------------------------------------------------------------
// REGISTER BIT MASK VALUES
//------------------------------------------------------------------------------
#define BM_SSP_CTRL0_SFTRST                     0x80000000
#define BM_SSP_CTRL0_CLKGATE                    0x40000000
#define BM_SSP_CTRL0_RUN                        0x20000000
#define BM_SSP_CTRL0_SDIO_IRQ_CHECK             0x10000000
#define BM_SSP_CTRL0_LOCK_CS                    0x08000000
#define BM_SSP_CTRL0_IGNORE_CRC                 0x04000000
#define BM_SSP_CTRL0_READ                       0x02000000
#define BM_SSP_CTRL0_DATA_XFER                  0x01000000
#define BM_SSP_CTRL0_BUS_WIDTH                  0x00C00000
#define BM_SSP_CTRL0_WAIT_FOR_IRQ               0x00200000
#define BM_SSP_CTRL0_WAIT_FOR_CMD               0x00100000
#define BM_SSP_CTRL0_LONG_RESP                  0x00080000
#define BM_SSP_CTRL0_CHECK_RESP                 0x00040000
#define BM_SSP_CTRL0_GET_RESP                   0x00020000
#define BM_SSP_CTRL0_ENABLE                     0x00010000
#define BM_SSP_CTRL0_XFER_COUNT                 0x0000FFFF

#define BM_SSP_CMD0_SLOW_CLKING_EN              0x00400000
#define BM_SSP_CMD0_CONT_CLKING_EN              0x00200000
#define BM_SSP_CMD0_APPEND_8CYC                 0x00100000
#define BM_SSP_CMD0_BLOCK_SIZE                  0x000F0000
#define BM_SSP_CMD0_BLOCK_COUNT                 0x0000FF00
#define BM_SSP_CMD0_CMD                         0x000000FF

#define BM_SSP_CMD1_CMD_ARG                     0xFFFFFFFF

#define BM_SSP_COMPREF_REFERENCE                0xFFFFFFFF

#define BM_SSP_COMPMASK_MASK                    0xFFFFFFFF

#define BM_SSP_TIMING_TIMEOUT                   0xFFFF0000
#define BM_SSP_TIMING_CLOCK_DIVIDE              0x0000FF00
#define BM_SSP_TIMING_CLOCK_RATE                0x000000FF

#define BM_SSP_CTRL1_SDIO_IRQ                   0x80000000
#define BM_SSP_CTRL1_SDIO_IRQ_EN                0x40000000
#define BM_SSP_CTRL1_RESP_ERR_IRQ               0x20000000
#define BM_SSP_CTRL1_RESP_ERR_IRQ_EN            0x10000000
#define BM_SSP_CTRL1_RESP_TIMEOUT_IRQ           0x08000000
#define BM_SSP_CTRL1_RESP_TIMEOUT_IRQ_EN        0x04000000
#define BM_SSP_CTRL1_DATA_TIMEOUT_IRQ           0x02000000
#define BM_SSP_CTRL1_DATA_TIMEOUT_IRQ_EN        0x01000000
#define BM_SSP_CTRL1_DATA_CRC_IRQ               0x00800000
#define BM_SSP_CTRL1_DATA_CRC_IRQ_EN            0x00400000
#define BM_SSP_CTRL1_FIFO_UNDERRUN_IRQ          0x00200000
#define BM_SSP_CTRL1_FIFO_UNDERRUN_EN           0x00100000
#define BM_SSP_CTRL1_CEATA_CCS_ERR_IRQ          0x00080000
#define BM_SSP_CTRL1_CEATA_CCS_ERR_IRQ_EN       0x00040000
#define BM_SSP_CTRL1_RECV_TIMEOUT_IRQ           0x00020000
#define BM_SSP_CTRL1_RECV_TIMEOUT_IRQ_EN        0x00010000
#define BM_SSP_CTRL1_FIFO_OVERRUN_IRQ           0x00008000
#define BM_SSP_CTRL1_FIFO_OVERRUN_IRQ_EN        0x00004000
#define BM_SSP_CTRL1_DMA_ENABLE                 0x00002000
#define BM_SSP_CTRL1_CEATA_CCS_ERR_EN           0x00001000
#define BM_SSP_CTRL1_SLAVE_OUT_DISABLE          0x00000800
#define BM_SSP_CTRL1_PHASE                      0x00000400
#define BM_SSP_CTRL1_POLARITY                   0x00000200
#define BM_SSP_CTRL1_SLAVE_MODE                 0x00000100
#define BM_SSP_CTRL1_WORD_LENGTH                0x000000F0
#define BM_SSP_CTRL1_SSP_MODE                   0x0000000F

#define BM_SSP_DATA_DATA                        0xFFFFFFFF

#define BM_SSP_SDRESP0_RESP0                    0xFFFFFFFF

#define BM_SSP_SDRESP1_RESP1                    0xFFFFFFFF

#define BM_SSP_SDRESP2_RESP2                    0xFFFFFFFF
#define BM_SSP_SDRESP3_RESP3                    0xFFFFFFFF

#define BM_SSP_STATUS_PRESENT                   0x80000000
#define BM_SSP_STATUS_MS_PRESENT                0x40000000
#define BM_SSP_STATUS_SD_PRESENT                0x20000000
#define BM_SSP_STATUS_CARD_DETECT               0x10000000
#define BM_SSP_STATUS_DMASENSE                  0x00200000
#define BM_SSP_STATUS_DMATERM                   0x00100000
#define BM_SSP_STATUS_DMAREQ                    0x00080000
#define BM_SSP_STATUS_DMAEND                    0x00040000
#define BM_SSP_STATUS_SDIO_IRQ                  0x00020000
#define BM_SSP_STATUS_RESP_CRC_ERR              0x00010000
#define BM_SSP_STATUS_RESP_ERR                  0x00008000
#define BM_SSP_STATUS_RESP_TIMEOUT              0x00004000
#define BM_SSP_STATUS_DATA_CRC_ERR              0x00002000
#define BM_SSP_STATUS_TIMEOUT                   0x00001000
#define BM_SSP_STATUS_RECV_TIMEOUT_STAT         0x00000800
#define BM_SSP_STATUS_CEATA_CCS_ERR             0x00000400
#define BM_SSP_STATUS_FIFO_OVRFLW               0x00000200
#define BM_SSP_STATUS_FIFO_FULL                 0x00000100
#define BM_SSP_STATUS_FIFO_EMPTY                0x00000020
#define BM_SSP_STATUS_FIFO_UNDRFLW              0x00000010
#define BM_SSP_STATUS_CMD_BUSY                  0x00000008
#define BM_SSP_STATUS_DATA_BUSY                 0x00000004
#define BM_SSP_STATUS_BUSY                      0x00000001

#define BM_SSP_DEBUG_DATACRC_ERR                0xF0000000
#define BM_SSP_DEBUG_DATA_STALL                 0x08000000
#define BM_SSP_DEBUG_DAT_SM                     0x07000000
#define BM_SSP_DEBUG_MSTK_SM                    0x00F00000
#define BM_SSP_DEBUG_CMD_OE                     0x00080000
#define BM_SSP_DEBUG_DMA_SM                     0x00070000
#define BM_SSP_DEBUG_MMC_SM                     0x0000F000
#define BM_SSP_DEBUG_CMD_SM                     0x00000C00
#define BM_SSP_DEBUG_SSP_CMD                    0x00000200
#define BM_SSP_DEBUG_SSP_RESP                   0x00000100
#define BM_SSP_DEBUG_SSP_RXD                    0x000000FF

#define BM_SSP_VERSION_MAJOR                    0xFF000000
#define BM_SSP_VERSION_MINOR                    0x00FF0000
#define BM_SSP_VERSION_STEP                     0x0000FFFF



#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // _MX233_SSP_H

////////////////////////////////////////////////////////////////////////////////
