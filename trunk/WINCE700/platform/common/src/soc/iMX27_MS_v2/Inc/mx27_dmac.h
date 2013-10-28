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
// Copyright (C) 2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// Header: mx27_dmac.h
//
// Provides definitions for DMAC module based on i.MX27.
//
//------------------------------------------------------------------------------

#ifndef __MX27_DMAC_H
#define __MX27_DMAC_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define DMAC_NUM_CHANNELS   16

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    REG32 SAR;        // 00: Channel Source Address Register
    REG32 DAR;        // 04: Channel Destination Address Register
    REG32 CNTR;       // 08: Channel Count Register
    REG32 CCR;        // 0C: Channel Control Register
    REG32 RSSR;       // 10: Channel Request Source Select Register
    REG32 BLR;        // 14: Channel Burst Length Register
    REG32 RTOR_BUCR;  // 18: Channel Request Time-Out Register (REN in CCR == 1)
                      // 18: Channel Bus Utilization Control Registers (REN in CCR == 0)
    REG32 CCNR;       // 1C: Channel Read-only Counter Register
    REG32 _PAD[8];
} CSP_DMAC_CHANNEL_REGS, *PCSP_DMAC_CHANNEL_REGS;

typedef struct {
    REG32 DCR;          // 000: DMA Control Register
    REG32 DISR;         // 004: DMA Interrupt Status Register
    REG32 DIMR;         // 008: DMA Interrupt Mask Register
    REG32 DBTOSR;       // 00C: DMA Burst Time-Out Status Register
    REG32 DRTOSR;       // 010: DMA Request Time-Out Status Register
    REG32 DSESR;        // 014: DMA Transfer Error Status Register
    REG32 DBOSR;        // 018: DMA Buffer Overflow Status Register
    REG32 DBTOCR;       // 01C: DMA Burst Time-Out Control Register
    REG32 _PAD[8];
    REG32 WSRA;         // 040: W-Size Register A
    REG32 XSRA;         // 044: X-Size Register A
    REG32 YSRA;         // 048: Y-Size Register A
    REG32 WSRB;         // 04C: W-Size Register B
    REG32 XSRB;         // 050: X-Size Register B
    REG32 YSRB;         // 054: Y-Size Register B
    REG32 _PAD1[10];
    CSP_DMAC_CHANNEL_REGS CHANNEL[DMAC_NUM_CHANNELS];
} CSP_DMAC_REGS, *PCSP_DMAC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define DMAC_DCR_OFFSET                 0x0000
#define DMAC_DISR_OFFSET                0x0004
#define DMAC_DIMR_OFFSET                0x0008
#define DMAC_DBTOSR_OFFSET              0x000C
#define DMAC_DRTOSR_OFFSET              0x0010
#define DMAC_DSESR_OFFSET               0x0014
#define DMAC_DBOSR_OFFSET               0x0018
#define DMAC_DBTOCR_OFFSET              0x001C

#define DMAC_WSRA_OFFSET                0x0040
#define DMAC_XSRA_OFFSET                0x0044
#define DMAC_YSRA_OFFSET                0x0048
#define DMAC_WSRB_OFFSET                0x004C
#define DMAC_XSRB_OFFSET                0x0050
#define DMAC_YSRB_OFFSET                0x0054

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
// DCR
#define DMAC_DCR_DEN_LSH                0
#define DMAC_DCR_DRST_LSH               1
#define DMAC_DCR_DAM_LSH                2

// DBTOCR
#define DMAC_DBTOCR_CNT_LSH             0
#define DMAC_DBTOCR_EN_LSH              15

// WSRx
#define DMAC_WSR_WS_LSH                 0

// XSRx
#define DMAC_XSR_XS_LSH                 0

// YSRx
#define DMAC_YSR_YS_LSH                 0

// Channel registers
// SAR
#define DMAC_SAR_SA_LSH                 0

// DAR
#define DMAC_DAR_DA_LSH                 0

// CNTR
#define DMAC_CNTR_CNT_LSH               0

// CCR
#define DMAC_CCR_CEN_LSH                0
#define DMAC_CCR_FRC_LSH                1
#define DMAC_CCR_RPT_LSH                2
#define DMAC_CCR_REN_LSH                3
#define DMAC_CCR_SSIZ_LSH               4
#define DMAC_CCR_DSIZ_LSH               6
#define DMAC_CCR_MSEL_LSH               8
#define DMAC_CCR_MDIR_LSH               9
#define DMAC_CCR_SMOD_LSH               10
#define DMAC_CCR_DMOD_LSH               12
#define DMAC_CCR_ACRPT_LSH              14

// RSSR
#define DMAC_RSSR_RSS_LSH               0

// BLR
#define DMAC_BLR_BL_LSH                 0

// RTOR
#define DMAC_RTOR_CNT_LSH               0
#define DMAC_RTOR_PSC_LSH               13
#define DMAC_RTOR_CLK_LSH               14
#define DMAC_RTOR_EN_LSH                15

// BUCR
#define DMAC_BUCR_BU_CNT_LSH            0

// CCNR
#define DMAC_CCNR_CCNR_LSH              0
//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// DCR
#define DMAC_DCR_DEN_WID                1
#define DMAC_DCR_DRST_WID               1
#define DMAC_DCR_DAM_WID                1

// DBTOCR
#define DMAC_DBTOCR_CNT_WID             15
#define DMAC_DBTOCR_EN_WID              1

// WSRx
#define DMAC_WSR_WS_WID                 16

// XSRx
#define DMAC_XSR_XS_WID                 16

// YSRx
#define DMAC_YSR_YS_WID                 16

// Channel registers
// SAR
#define DMAC_SAR_SA_WID                 32

// DAR
#define DMAC_DAR_DA_WID                 32

// CNTR
#define DMAC_CNTR_CNT_WID               24

// CCR
#define DMAC_CCR_CEN_WID                1
#define DMAC_CCR_FRC_WID                1
#define DMAC_CCR_RPT_WID                1
#define DMAC_CCR_REN_WID                1
#define DMAC_CCR_SSIZ_WID               2
#define DMAC_CCR_DSIZ_WID               2
#define DMAC_CCR_MSEL_WID               1
#define DMAC_CCR_MDIR_WID               1
#define DMAC_CCR_SMOD_WID               2
#define DMAC_CCR_DMOD_WID               2
#define DMAC_CCR_ACRPT_WID              1

// RSSR
#define DMAC_RSSR_RSS_WID               6

// BLR
#define DMAC_BLR_BL_WID                 6

// RTOR
#define DMAC_RTOR_CNT_WID               13
#define DMAC_RTOR_PSC_WID               1
#define DMAC_RTOR_CLK_WID               1
#define DMAC_RTOR_EN_WID                1

// BUCR
#define DMAC_BUCR_BU_CNT_WID            16

// CCNR
#define DMAC_CCNR_CCNR_WID              24

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// DCR
#define DMAC_DCR_DEN_DISABLE            0
#define DMAC_DCR_DEN_ENABLE             1
#define DMAC_DCR_DRST_NOEFFECT          0
#define DMAC_DCR_DRST_RESET             1
#define DMAC_DCR_DAM_PRIVILEGED         0
#define DMAC_DCR_DAM_USER               1

// DBTOCR
#define DMAC_DBTOCR_EN_DISABLE          0
#define DMAC_DBTOCR_EN_ENABLE           1

// Channel registers
// CCR
#define DMAC_CCR_CEN_DISABLE            0
#define DMAC_CCR_CEN_ENABLE             1
#define DMAC_CCR_FRC_NOEFFECT           0
#define DMAC_CCR_FRC_FORCE_DMA_CYCLE    1
#define DMAC_CCR_RPT_DISABLE            0
#define DMAC_CCR_RPT_ENABLE             1
#define DMAC_CCR_REN_DISABLE            0
#define DMAC_CCR_REN_ENABLE             1
#define DMAC_CCR_SSIZ_32BIT             0 
#define DMAC_CCR_SSIZ_8BIT              1
#define DMAC_CCR_SSIZ_16BIT             2
#define DMAC_CCR_DSIZ_32BIT             0 
#define DMAC_CCR_DSIZ_8BIT              1
#define DMAC_CCR_DSIZ_16BIT             2
#define DMAC_CCR_MSEL_2DMEM_SETA        0
#define DMAC_CCR_MSEL_2DMEM_SETB        1
#define DMAC_CCR_MDIR_INCREMENT         0
#define DMAC_CCR_MDIR_DECREMENT         1
#define DMAC_CCR_SMOD_LINEAR            0
#define DMAC_CCR_SMOD_2DMEM             1
#define DMAC_CCR_SMOD_FIFO              2
#define DMAC_CCR_DMOD_LINEAR            0
#define DMAC_CCR_DMOD_2DMEM             1
#define DMAC_CCR_DMOD_FIFO              2
#define DMAC_CCR_ACRPT_DISABLE          0
#define DMAC_CCR_ACRPT_AUTOCLEAR_RPT    1

// BLR
#define DMAC_BLR_BL_64_BYTES            0

// RTOR
#define DMAC_RTOR_PSC_DIV1              0
#define DMAC_RTOR_PSC_DIV256            1
#define DMAC_RTOR_CLK_HCLK              0
#define DMAC_RTOR_CLK_32KHZ_REF         1
#define DMAC_RTOR_EN_DISABLE            0
#define DMAC_RTOR_EN_ENABLE             1

#ifdef __cplusplus
}
#endif

#endif // __MX27_DMAC_H
