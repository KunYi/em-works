//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: common_sim.h
//
//  Provides definitions for SIM/SMARTCARD Controller Interface
//  module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_SIM_H__
#define __COMMON_SIM_H__

#if __cplusplus
extern "C" {
#endif

#include "common_types.h"

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    REG32 PORT1_CNTL;               // 0x00: Port1 control register
    REG32 SETUP;                         // 0x04: Setup register
    REG32 PORT1_DETECT;          // 0x08: Port1 detect register
    REG32 PORT1_XMT_BUF;         // 0x0C: Port1 transmit buffer register
    REG32 PORT1_RCV_BUF;         // 0x10: Port1 receiver buffer register
    REG32 PORT0_CNTL;                // 0x14: Port0 control register
    REG32 CNTL;                              // 0x18: SIM control register
    REG32 CLK_PRESCALER;              // 0x1C: Clock Prescaler register
    //REG32 CLOCK_SELECT;              // 0x1C: Clock select register
    REG32 RCV_THRESHOLD;         // 0x20: Receiver threshold register
    REG32 ENABLE;                        // 0x24: Enable register
    REG32 XMT_STATUS;                // 0x28: Transmit status register
    REG32 RCV_STATUS;                // 0x2C: Receiver status register
    REG32 INT_MASK;                      // 0x30: Interrupt mask register
    REG32 PORT0_XMT_BUF;          // 0x34: Port0 transmit buffer register
    REG32 PORT0_RCV_BUF;         // 0x38: Port0 receiver buffer register
    REG32 PORT0_DETECT;              // 0x3C: Port0 detect register
    REG32 DATA_FORMAT;               // 0x40: Data format register
    REG32 XMT_THRESHOLD;         // 0x44: Transmit threshold register
    REG32 GUARD_CNTL;                // 0x48: Transmit guard control register
    REG32 OD_CONFIG;                 // 0x4C: Open drain configuration register
    REG32 RESET_CNTL;                // 0x50: Reset control register
    REG32 CHAR_WAIT;                 // 0x54: Character wait time register
    REG32 GPCNT;                         // 0x58: General purpose counter register
    REG32 DIVISOR;                       // 0x5C: Divisor register
    REG32 BWT;                               // 0x60: Block wait time register
    REG32 BGT;                               // 0x64: Block guard time register 
    REG32 BWT_H;                         // 0x68: Block wait time register HIGH
    REG32 XMT_FIFO_STAT;                //0x6C: Transmit FIFO status register
    REG32 RCV_FIFO_CNT;                 //0x70: Receive FIFO counter register 
    REG32 RCV_FIFO_WPRT;                //0x74: Receive FIFO write pointer register
    REG32 RCV_FIFO_RPTR;                //0x78: Receive FIFO read pointer register
    } CSP_SIM_REG, *PCSP_SIM_REG;


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define SIM_PORT1_CNTL_SAPD_LSH             0
#define SIM_PORT1_CNTL_SVEN_LSH             1
#define SIM_PORT1_CNTL_STEN_LSH             2
#define SIM_PORT1_CNTL_SRST_LSH             3
#define SIM_PORT1_CNTL_SCEN_LSH             4
#define SIM_PORT1_CNTL_SCSP_LSH             5
#define SIM_PORT1_CNTL_3VOLT_LSH            6
#define SIM_PORT1_CNTL_SFPD_LSH             7

#define SIM_SETUP_AMODE_LSH                 0
#define SIM_SETUP_SPS_LSH                   1

#define SIM_PORT1_DETECT_SDIM_LSH           0
#define SIM_PORT1_DETECT_SDI_LSH            1
#define SIM_PORT1_DETECT_SPDP_LSH           2
#define SIM_PORT1_DETECT_SPDS_LSH           3

#define SIM_PORT1_XMT_BUF_XMT_BUF_LSH       0

#define SIM_PORT1_RCV_BUF_RCV_BUF_LSH       0
#define SIM_PORT1_RCV_BUF_PE_LSH            8
#define SIM_PORT1_RCV_BUF_FE_LSH            9
#define SIM_PORT1_RCV_BUF_CWT_LSH           10

#define SIM_PORT0_CNTL_SAPD_LSH             0
#define SIM_PORT0_CNTL_SVEN_LSH             1
#define SIM_PORT0_CNTL_STEN_LSH             2
#define SIM_PORT0_CNTL_SRST_LSH             3
#define SIM_PORT0_CNTL_SCEN_LSH             4
#define SIM_PORT0_CNTL_SCSP_LSH             5
#define SIM_PORT0_CNTL_3VOLT_LSH            6
#define SIM_PORT0_CNTL_SFPD_LSH             7

#define SIM_CNTL_ICM_LSH                    1
#define SIM_CNTL_ANACK_LSH                  2
#define SIM_CNTL_ONACK_LSH                  3
#define SIM_CNTL_SAMPLE12_LSH               4
#define SIM_CNTL_BAUD_SEL_LSH               6
#define SIM_CNTL_GPCNT_CLK_SEL_LSH          9
#define SIM_CNTL_CWTEN_LSH                  11
#define SIM_CNTL_LRCEN_LSH                  12
#define SIM_CNTL_CRCEN_LSH                  13
#define SIM_CNTL_XMT_CRC_LRC_LSH            14
#define SIM_CNTL_BWTEN_LSH                  15

#define SIM_CLK_PRESCALER_LSH               0

#define SIM_RCV_THRESHOLD_RDT_LSH           0
#define SIM_RCV_THRESHOLD_RTH_LSH           9

#define SIM_ENABLE_RCVEN_LSH                0
#define SIM_ENABLE_XMTEN_LSH                1

#define SIM_XMT_STATUS_XTE_LSH              0
#define SIM_XMT_STATUS_TFE_LSH              3
#define SIM_XMT_STATUS_ETC_LSH              4
#define SIM_XMT_STATUS_TC_LSH               5
#define SIM_XMT_STATUS_TFO_LSH              6
#define SIM_XMT_STATUS_TDTF_LSH             7
#define SIM_XMT_STATUS_GPCNT_LSH            8

#define SIM_RCV_STATUS_OEF_LSH              0
#define SIM_RCV_STATUS_RFE_LSH              1
#define SIM_RCV_STATUS_RFD_LSH              4
#define SIM_RCV_STATUS_RDRF_LSH             5
#define SIM_RCV_STATUS_LRCOK_LSH            6
#define SIM_RCV_STATUS_CRCOK_LSH            7
#define SIM_RCV_STATUS_CWT_LSH              8
#define SIM_RCV_STATUS_RTE_LSH              9
#define SIM_RCV_STATUS_BWT_LSH              10
#define SIM_RCV_STATUS_BGT_LSH              11


#define SIM_INT_MASK_RIM_LSH                0
#define SIM_INT_MASK_TCIM_LSH               1
#define SIM_INT_MASK_OIM_LSH                2
#define SIM_INT_MASK_ETCIM_LSH              3
#define SIM_INT_MASK_TFEIM_LSH              4
#define SIM_INT_MASK_XTM_LSH                5
#define SIM_INT_MASK_TFOM_LSH               6
#define SIM_INT_MASK_TDTFM_LSH              7
#define SIM_INT_MASK_GPCNTM_LSH             8
#define SIM_INT_MASK_CWTM_LSH               9
#define SIM_INT_MASK_RTM_LSH                10
#define SIM_INT_MASK_BWTM_LSH               11
#define SIM_INT_MASK_BGTM_LSH               12
#define SIM_INT_MASK_RFEM_LSH               13

#define SIM_PORT0_XMT_BUF_XMT_BUF_LSH       0

#define SIM_PORT0_RCV_BUF_RCV_BUF_LSH       0
#define SIM_PORT0_RCV_BUF_PE_LSH            8
#define SIM_PORT0_RCV_BUF_FE_LSH            9
#define SIM_PORT0_RCV_BUF_CWT_LSH           10

#define SIM_PORT0_DETECT_SDIM_LSH           0
#define SIM_PORT0_DETECT_SDI_LSH            1
#define SIM_PORT0_DETECT_SPDP_LSH           2
#define SIM_PORT0_DETECT_SPDS_LSH           3

#define SIM_DATA_FORMAT_IC_LSH              0

#define SIM_XMT_THRESHOLD_TDT_LSH           0
#define SIM_XMT_THRESHOLD_XTH_LSH           4

#define SIM_GUARD_CNTL_GETU_LSH             0
#define SIM_GUARD_CNTL_RCVR11_LSH           8

#define SIM_OD_CONFIG_OD_P0_LSH             0
#define SIM_OD_CONFIG_OD_P1_LSH             1

#define SIM_RESET_CNTL_FLUSH_RCV_LSH        0
#define SIM_RESET_CNTL_FLUSH_XMT_LSH        1
#define SIM_RESET_CNTL_SOFT_RESET_LSH       2
#define SIM_RESET_CNTL_KILL_CLK_LSH         3
#define SIM_RESET_CNTL_DOZE_LSH             4
#define SIM_RESET_CNTL_STOP_LSH             5
#define SIM_RESET_CNTL_DEBUG_LSH            6

#define SIM_CHAR_WAIT_CWT_LSH               0

#define SIM_GPCNT_LSH                       0

#define SIM_DIVISOR_LSH                     0

#define SIM_BWT_LSH                         0

#define SIM_BGT_LSH                         0

#define SIM_BWT_H_LSH                       0

#define SIM_XMT_FIFO_STAT_RPTR_LSH          0
#define SIM_XMT_FIFO_STAT_WPTR_LSH          4
#define SIM_XMT_FIFO_STAT_CNT_LSH           8

#define SIM_RCV_FIFO_CNT_LSH                0

#define SIM_RCV_FIFO_WPTR_LSH               0

#define SIM_RCV_FIFO_RPTR_LSH               0    

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define SIM_PORT1_CNTL_SAPD_WID             1
#define SIM_PORT1_CNTL_SVEN_WID             1
#define SIM_PORT1_CNTL_STEN_WID             1
#define SIM_PORT1_CNTL_SRST_WID             1
#define SIM_PORT1_CNTL_SCEN_WID             1
#define SIM_PORT1_CNTL_SCSP_WID             1
#define SIM_PORT1_CNTL_3VOLT_WID            1
#define SIM_PORT1_CNTL_SFPD_WID             1

#define SIM_SETUP_AMODE_WID                 1
#define SIM_SETUP_SPS_WID                   1

#define SIM_PORT1_DETECT_SDIM_WID           1
#define SIM_PORT1_DETECT_SDI_WID            1
#define SIM_PORT1_DETECT_SPDP_WID           1
#define SIM_PORT1_DETECT_SPDS_WID           1

#define SIM_PORT1_XMT_BUF_XMT_BUF_WID       8

#define SIM_PORT1_RCV_BUF_RCV_BUF_WID       8
#define SIM_PORT1_RCV_BUF_PE_WID            1
#define SIM_PORT1_RCV_BUF_FE_WID            1
#define SIM_PORT1_RCV_BUF_CWT_WID           1

#define SIM_PORT0_CNTL_SAPD_WID             1
#define SIM_PORT0_CNTL_SVEN_WID             1
#define SIM_PORT0_CNTL_STEN_WID             1
#define SIM_PORT0_CNTL_SRST_WID             1
#define SIM_PORT0_CNTL_SCEN_WID             1
#define SIM_PORT0_CNTL_SCSP_WID             1
#define SIM_PORT0_CNTL_3VOLT_WID            1
#define SIM_PORT0_CNTL_SFPD_WID             1

#define SIM_CNTL_ICM_WID                    1
#define SIM_CNTL_ANACK_WID                  1
#define SIM_CNTL_ONACK_WID                  1
#define SIM_CNTL_SAMPLE12_WID               1
#define SIM_CNTL_BAUD_SEL_WID               3
#define SIM_CNTL_GPCNT_CLK_SEL_WID          2
#define SIM_CNTL_CWTEN_WID                  1
#define SIM_CNTL_LRCEN_WID                  1
#define SIM_CNTL_CRCEN_WID                  1
#define SIM_CNTL_XMT_CRC_LRC_WID            1
#define SIM_CNTL_BWTEN_WID                  1

#define SIM_CLK_PRESCALER_WID               7

#define SIM_RCV_THRESHOLD_RDT_WID           9
#define SIM_RCV_THRESHOLD_RTH_WID           4

#define SIM_ENABLE_RCVEN_WID                1
#define SIM_ENABLE_XMTEN_WID                1

#define SIM_XMT_STATUS_XTE_WID              1
#define SIM_XMT_STATUS_TFE_WID              1
#define SIM_XMT_STATUS_ETC_WID              1
#define SIM_XMT_STATUS_TC_WID               1
#define SIM_XMT_STATUS_TFO_WID              1
#define SIM_XMT_STATUS_TDTF_WID             1
#define SIM_XMT_STATUS_GPCNT_WID            1

#define SIM_RCV_STATUS_OEF_WID              1
#define SIM_RCV_STATUS_RPE_WID              1
#define SIM_RCV_STATUS_RFD_WID              1
#define SIM_RCV_STATUS_RDRF_WID             1
#define SIM_RCV_STATUS_LRCOK_WID            1
#define SIM_RCV_STATUS_CRCOK_WID            1
#define SIM_RCV_STATUS_CWT_WID              1
#define SIM_RCV_STATUS_RTE_WID              1
#define SIM_RCV_STATUS_BWT_WID              1
#define SIM_RCV_STATUS_BGT_WID              1

#define SIM_INT_MASK_RIM_WID                1
#define SIM_INT_MASK_TCIM_WID               1
#define SIM_INT_MASK_OIM_WID                1
#define SIM_INT_MASK_ETCIM_WID              1
#define SIM_INT_MASK_TFEIM_WID              1
#define SIM_INT_MASK_XTM_WID                1
#define SIM_INT_MASK_TFOM_WID               1
#define SIM_INT_MASK_TDTFM_WID              1
#define SIM_INT_MASK_GPCNTM_WID             1
#define SIM_INT_MASK_CWTM_WID               1
#define SIM_INT_MASK_RTM_WID                1
#define SIM_INT_MASK_BWTM_WID               1
#define SIM_INT_MASK_BGTM_WID               1
#define SIM_INT_MASK_RFEM_WID               1

#define SIM_PORT0_XMT_BUF_XMT_BUF_WID       8

#define SIM_PORT0_RCV_BUF_RCV_BUF_WID       8
#define SIM_PORT0_RCV_BUF_PE_WID            1
#define SIM_PORT0_RCV_BUF_FE_WID            1
#define SIM_PORT0_RCV_BUF_CWT_WID           1

#define SIM_PORT0_DETECT_SDIM_WID           1
#define SIM_PORT0_DETECT_SDI_WID            1
#define SIM_PORT0_DETECT_SPDP_WID           1
#define SIM_PORT0_DETECT_SPDS_WID           1

#define SIM_DATA_FORMAT_IC_WID              1

#define SIM_XMT_THRESHOLD_TDT_WID           4
#define SIM_XMT_THRESHOLD_XTH_WID           4

#define SIM_GUARD_CNTL_GETU_WID             8
#define SIM_GUARD_CNTL_RCVR11_WID           1

#define SIM_OD_CONFIG_OD_P0_WID             1
#define SIM_OD_CONFIG_OD_P1_WID             1

#define SIM_RESET_CNTL_FLUSH_RCV_WID        1
#define SIM_RESET_CNTL_FLUSH_XMT_WID        1
#define SIM_RESET_CNTL_SOFT_RESET_WID       1
#define SIM_RESET_CNTL_KILL_CLK_WID         1
#define SIM_RESET_CNTL_DOZE_WID             1
#define SIM_RESET_CNTL_STOP_WID             1
#define SIM_RESET_CNTL_DEBUG_WID            1

#define SIM_CHAR_WAIT_CWT_WID               16

#define SIM_GPCNT_WID                       16

#define SIM_DIVISOR_WID                     8

#define SIM_BWT_WID                         16

#define SIM_BGT_WID                         16

#define SIM_BWT_H_WID                       16

#define SIM_XMT_FIFO_STAT_RPTR_WID          4
#define SIM_XMT_FIFO_STAT_WPTR_WID          4
#define SIM_XMT_FIFO_STAT_CNT_WID           4

#define SIM_RCV_FIFO_CNT_WID                8

#define SIM_RCV_FIFO_WPTR_WID               8

#define SIM_RCV_FIFO_RPTR_WID               8   

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//PORT1_CNTL
#define SIM_PORT1_CNTL_SAPD_ENABLE          1
#define SIM_PORT1_CNTL_SAPD_DISABLE         0

#define SIM_PORT1_CNTL_SVEN_ENABLE          1
#define SIM_PORT1_CNTL_SVEN_DISABLE         0

#define SIM_PORT1_CNTL_STEN_ENABLE          1
#define SIM_PORT1_CNTL_STEN_DISABLE         0

#define SIM_PORT1_CNTL_SRST_ENABLE          1
#define SIM_PORT1_CNTL_SRST_DISABLE         0

#define SIM_PORT1_CNTL_SCEN_ENABLE          1
#define SIM_PORT1_CNTL_SCEN_DISABLE         0

#define SIM_PORT1_CNTL_SCSP_ENABLE          1
#define SIM_PORT1_CNTL_SCSP_DISABLE         0

#define SIM_PORT1_CNTL_3VOLT_ENABLE         1
#define SIM_PORT1_CNTL_3VOLT_DISABLE        0

#define SIM_PORT1_CNTL_SFPD_ENABLE          1
#define SIM_PORT1_CNTL_SFPD_DISABLE         0

//SETUP
#define SIM_SETUP_AMODE_ENABLE              1
#define SIM_SETUP_AMODE_DISABLE             0

#define SIM_SETUP_SPS_PORT1                 1
#define SIM_SETUP_SPS_PORT0                 0

//PORT1_DETECT
#define SIM_PORT1_DETECT_SDIM_MASK          1
#define SIM_PORT1_DETECT_SDIM_ENABLE        0

#define SIM_PORT1_DETECT_SDI_YES            1
#define SIM_PORT1_DETECT_SDI_NO             0

#define SIM_PORT1_DETECT_SPDS_RE            1
#define SIM_PORT1_DETECT_SPDS_FE            0

//PORT0_CNTL
#define SIM_PORT0_CNTL_SAPD_ENABLE          1
#define SIM_PORT0_CNTL_SAPD_DISABLE         0

#define SIM_PORT0_CNTL_SVEN_ENABLE          1
#define SIM_PORT0_CNTL_SVEN_DISABLE         0

#define SIM_PORT0_CNTL_STEN_ENABLE          1
#define SIM_PORT0_CNTL_STEN_DISABLE         0

#define SIM_PORT0_CNTL_SRST_ENABLE          1
#define SIM_PORT0_CNTL_SRST_DISABLE         0

#define SIM_PORT0_CNTL_SCEN_ENABLE          1
#define SIM_PORT0_CNTL_SCEN_DISABLE         0

#define SIM_PORT0_CNTL_SCSP_ENABLE          1
#define SIM_PORT0_CNTL_SCSP_DISABLE         0

#define SIM_PORT0_CNTL_3VOLT_ENABLE         1
#define SIM_PORT0_CNTL_3VOLT_DISABLE        0

#define SIM_PORT0_CNTL_SFPD_ENABLE          1
#define SIM_PORT0_CNTL_SFPD_DISABLE         0

//CNTL
#define SIM_CNTL_ICM_ENABLE                 1
#define SIM_CNTL_ICM_DISABLE                0

#define SIM_CNTL_ANACK_ENABLE               1
#define SIM_CNTL_ANACK_DISABLE              0

#define SIM_CNTL_ONACK_ENABLE               1
#define SIM_CNTL_ONACK_DISABLE              0

#define SIM_CNTL_SAMPLE12_ENABLE            1
#define SIM_CNTL_SAMPLE12_DISABLE           0

#define SIM_CNTL_GPCNT_CLK_SEL_XMT          3
#define SIM_CNTL_GPCNT_CLK_SEL_RCV          2
#define SIM_CNTL_GPCNT_CLK_SEL_CARD         1
#define SIM_CNTL_GPCNT_CLK_SEL_DISABLE      0

#define SIM_CNTL_CWTEN_ENABLE               1
#define SIM_CNTL_CWTEN_DISABLE              0

#define SIM_CNTL_LRCEN_ENABLE               1
#define SIM_CNTL_LRCEN_DISABLE              0

#define SIM_CNTL_CRCEN_ENABLE               1
#define SIM_CNTL_CRCEN_DISABLE              0

#define SIM_CNTL_XMT_CRC_LRC_ENABLE         1
#define SIM_CNTL_XMT_CRC_LRC_DISABLE        0

#define SIM_CNTL_BWTEN_ENABLE               1
#define SIM_CNTL_BWTEN_DISABLE              0

//ENABLE
#define SIM_ENABLE_RCVEN_ENABLE             1
#define SIM_ENABLE_RCVEN_DISABLE            0

#define SIM_ENABLE_XMTEN_ENABLE             1
#define SIM_ENABLE_XMTEN_DISABLE            0

//XMT_STATUS
#define SIM_XMT_STATUS_XTE_CLR              1
#define SIM_XMT_STATUS_TFE_CLR              1
#define SIM_XMT_STATUS_ETC_CLR              1
#define SIM_XMT_STATUS_TC_CLR               1
#define SIM_XMT_STATUS_TFO_CLR              1
#define SIM_XMT_STATUS_TDTF_CLR             1
#define SIM_XMT_STATUS_GPCNT_CLR            1

//RCV_STATUS
#define SIM_RCV_STATUS_OEF_CLR              1
#define SIM_RCV_STATUS_RFE_CLR              1
#define SIM_RCV_STATUS_RFD_CLR              1
#define SIM_RCV_STATUS_RDRF_CLR             1
#define SIM_RCV_STATUS_LRCOK_CLR            1
#define SIM_RCV_STATUS_CRCOK_CLR            1
#define SIM_RCV_STATUS_CWT_CLR              1
#define SIM_RCV_STATUS_RTE_CLR              1
#define SIM_RCV_STATUS_BWT_CLR              1
#define SIM_RCV_STATUS_BGT_CLR              1

//INT_MASK
#define SIM_INT_MASK_RIM_MASK               1
#define SIM_INT_MASK_RIM_ENABLE             0

#define SIM_INT_MASK_TCIM_MASK              1
#define SIM_INT_MASK_TCIM_ENABLE            0

#define SIM_INT_MASK_OIM_MASK               1
#define SIM_INT_MASK_OIM_ENABLE             0

#define SIM_INT_MASK_ETCIM_MASK             1
#define SIM_INT_MASK_ETCIM_ENABLE           0

#define SIM_INT_MASK_TFEIM_MASK             1
#define SIM_INT_MASK_TFEIM_ENABLE           0

#define SIM_INT_MASK_XTM_MASK               1
#define SIM_INT_MASK_XTM_ENABLE             0

#define SIM_INT_MASK_TFOM_MASK              1
#define SIM_INT_MASK_TFOM_ENABLE            0

#define SIM_INT_MASK_TDTFM_MASK             1
#define SIM_INT_MASK_TDTFM_ENABLE           0

#define SIM_INT_MASK_GPCNTM_MASK            1
#define SIM_INT_MASK_GPCNTM_ENABLE          0

#define SIM_INT_MASK_CWTM_MASK              1
#define SIM_INT_MASK_CWTM_ENABLE            0

#define SIM_INT_MASK_RTM_MASK               1
#define SIM_INT_MASK_RTM_ENABLE             0

#define SIM_INT_MASK_BWTM_MASK              1
#define SIM_INT_MASK_BWTM_ENABLE            0

#define SIM_INT_MASK_BGTM_MASK              1
#define SIM_INT_MASK_BGTM_ENABLE            0

#define SIM_INT_MASK_RFEM_MASK              1
#define SIM_INT_MASK_RFEM_ENABLE            0

//PORT0_DETECT
#define SIM_PORT0_DETECT_SDIM_MASK          1
#define SIM_PORT0_DETECT_SDIM_ENABLE        0

#define SIM_PORT0_DETECT_SDI_YES            1
#define SIM_PORT0_DETECT_SDI_NO             0

#define SIM_PORT0_DETECT_SPDS_RE            1
#define SIM_PORT0_DETECT_SPDS_FE            0

//DATA_FORMAT
#define SIM_DATA_FORMAT_IC_ENABLE           1
#define SIM_DATA_FORMAT_IC_DISABLE          0

//GUARD_CNTL
#define SIM_GUARD_CNTL_RCVR11_ENABLE        1
#define SIM_GUARD_CNTL_RCVR11_DISABLE       0

//OD_CONFIG
#define SIM_OD_CONFIG_OD_P0_OD              1
#define SIM_OD_CONFIG_OD_P0_PP              0

#define SIM_OD_CONFIG_OD_P1_OD              1
#define SIM_OD_CONFIG_OD_P1_PP              0

//RESET_CNTL
#define SIM_RESET_CNTL_FLUSH_RCV_ENABLE     1
#define SIM_RESET_CNTL_FLUSH_RCV_DISABLE    0

#define SIM_RESET_CNTL_FLUSH_XMT_ENABLE     1
#define SIM_RESET_CNTL_FLUSH_XMT_DISABLE    0

#define SIM_RESET_CNTL_SOFTRST_ENABLE       1
#define SIM_RESET_CNTL_SOFTRST_DISABLE      0

#define SIM_RESET_CNTL_KILLCLK_ENABLE       1
#define SIM_RESET_CNTL_KILLCLK_DISABLE      0

#define SIM_RESET_CNTL_DOZE_ENABLE          1
#define SIM_RESET_CNTL_DOZE_DISABLE         0

#define SIM_RESET_CNTL_STOP_ENABLE          1
#define SIM_RESET_CNTL_STOP_DISABLE         0

#define SIM_RESET_CNTL_DBUG_ENABLE          1
#define SIM_RESET_CNTL_DBUG_DISABLE         0
#ifdef __cplusplus
}
#endif

#endif // __COMMON_SIM_H__
