//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_sdma.h
//
//  Provides definitions for the SDMA (Smart DMA) host interface 
//  that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_SDMA_H
#define __COMMON_SDMA_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define SDMA_CONTEXT_WITH_SCRATCH       1

#define SDMA_NUM_CHANNELS               32
#define SDMA_ROM_SIZE                   (4*1024)
#define SDMA_RAM_SIZE                   (8*1024)
#define SDMA_MAX_RAM_CODE_SIZE          (SDMA_RAM_SIZE-(0x800*2))

// Instruction memory map
#define SDMA_PM_ROM_START               0x0000
#define SDMA_PM_RAM_START               0x1000
#define SDMA_PM_CONTEXT_START           0x1000
#define SDMA_PM_CODE_START              0x1800

// Data memory map
#define SDMA_DM_ROM_START               0x0000
#define SDMA_DM_RAM_START               0x0800
#define SDMA_DM_CONTEXT_START           0x0800
#define SDMA_DM_CODE_START              0x0C00

// Chan 0 commands
#define SDMA_CMD_C0_SET_DM              0x01
#define SDMA_CMD_C0_SET_PM              0x04
#define SDMA_CMD_C0_GET_DM              0x02
#define SDMA_CMD_C0_GET_PM              0x08
#define SDMA_CMD_C0_SETCTX              0x07
#define SDMA_CMD_C0_GETCTX              0x03

// Support fixed buffer descriptor storage by limiting the number
// of buffer desciptors supported per channel.  The OEM can
// specify if static storage (i.e. internal RAM) or dynamic 
// allocation is used by changing the implementation for 
// BSPSdmaAllocChain
#define SDMA_STATIC_BUF_DESC            64

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct 
{
    UINT32 MC0PTR;
    UINT32 INTR;
    UINT32 STOP_STAT;
    UINT32 HSTART;
    UINT32 EVTOVR;
    UINT32 DSPOVR;
    UINT32 HOSTOVR;
    UINT32 EVTPEND;
    UINT32 DSPENBL;
    UINT32 RESET;
    UINT32 EVTERR;
    UINT32 INTRMASK;
    UINT32 PSW;
    UINT32 EVTERRDBG;
    UINT32 CONFIG;
    UINT32 reserved1;
    UINT32 ONCE_ENB;
    UINT32 ONCE_DATA;
    UINT32 ONCE_INSTR;
    UINT32 ONCE_STAT;
    UINT32 ONCE_CMD;
    UINT32 EVT_MIRROR;
    UINT32 ILLINSTADDR;
    UINT32 CHN0ADDR;
    UINT32 reserved2[4];
    UINT32 XTRIG_CONF1;
    UINT32 XTRIG_CONF2;
    UINT32 reserved3[2];
    UINT32 CHNENBL[SDMA_NUM_CHANNELS];
    UINT32 CHNPRI[SDMA_NUM_CHANNELS];
} CSP_SDMA_REGS, *PCSP_SDMA_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define SDMA_MC0PTR_OFFSET              0x0000
#define SDMA_INTR_OFFSET                0x0004
#define SDMA_STOP_STAT_OFFSET           0x0008
#define SDMA_HSTART_OFFSET              0x000C
#define SDMA_EVTOVR_OFFSET              0x0010
#define SDMA_DSPOVR_OFFSET              0x0014
#define SDMA_HOSTOVR_OFFSET             0x0018
#define SDMA_EVTPEND_OFFSET             0x001C
#define SDMA_DSPENBL_OFFSET             0x0020
#define SDMA_RESET_OFFSET               0x0024
#define SDMA_EVTERR_OFFSET              0x0028
#define SDMA_INTRMASK_OFFSET            0x002C
#define SDMA_PSW_OFFSET                 0x0030
#define SDMA_EVTERRDBG_OFFSET           0x0034
#define SDMA_CONFIG_OFFSET              0x0038
#define SDMA_ONCE_ENB_OFFSET            0x003C
#define SDMA_ONCE_DATA_OFFSET           0x0040
#define SDMA_ONCE_INSTR_OFFSET          0x0044
#define SDMA_ONCE_STAT_OFFSET           0x0048
#define SDMA_ONCE_CMD_OFFSET            0x004C
#define SDMA_EVT_MIRROR_OFFSET          0x0050
#define SDMA_ILLINSTADDR_OFFSET         0x0054
#define SDMA_CHN0ADDR_OFFSET            0x0058
#define SDMA_XTRIG_CONF1_OFFSET         0x0070
#define SDMA_XTRIG_CONF2_OFFSET         0x0074
#define SDMA_CHNENBL_OFFSET             0x0080
#define SDMA_CHNPRI_OFFSET              0x0100


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define SDMA_RESET_RESET_LSH            0
#define SDMA_RESET_RESCHED_LSH          1

#define SDMA_PSW_CCR_LSH                0
#define SDMA_PSW_CCP_LSH                5
#define SDMA_PSW_NCR_LSH                8
#define SDMA_PSW_NCP_LSH                13

#define SDMA_CONFIG_CSM_LSH             0
#define SDMA_CONFIG_ACR_LSH             4
#define SDMA_CONFIG_RTDOBS_LSH          11
#define SDMA_CONFIG_DSPDMA_LSH          12

#define SDMA_ONCE_ENB_ENB_LSH           0

#define SDMA_ONCE_INSTR_INSTR_LSH       0

#define SDMA_ONCE_STAT_ECDR_LSH         0
#define SDMA_ONCE_STAT_MST_LSH          7
#define SDMA_ONCE_STAT_SWB_LSH          8
#define SDMA_ONCE_STAT_ODR_LSH          9
#define SDMA_ONCE_STAT_EDR_LSH          10
#define SDMA_ONCE_STAT_RCV_LSH          11
#define SDMA_ONCE_STAT_PST_LSH          12

#define SDMA_ONCE_CMD_CMD_LSH           0

#define SDMA_CHN0ADDR_CHN0ADDR_LSH      0
#define SDMA_CHN0ADDR_SMSZ_LSH          14

#define SDMA_XTRIG_CONF1_NUM0_LSH       0
#define SDMA_XTRIG_CONF1_CNF0_LSH       6
#define SDMA_XTRIG_CONF1_NUM1_LSH       8
#define SDMA_XTRIG_CONF1_CNF1_LSH       14
#define SDMA_XTRIG_CONF1_NUM2_LSH       16
#define SDMA_XTRIG_CONF1_CNF2_LSH       22
#define SDMA_XTRIG_CONF1_NUM3_LSH       24
#define SDMA_XTRIG_CONF1_CNF3_LSH       30

#define SDMA_XTRIG_CONF2_NUM4_LSH       0
#define SDMA_XTRIG_CONF2_CNF4_LSH       6
#define SDMA_XTRIG_CONF2_NUM5_LSH       8
#define SDMA_XTRIG_CONF2_CNF5_LSH       14
#define SDMA_XTRIG_CONF2_NUM6_LSH       16
#define SDMA_XTRIG_CONF2_CNF6_LSH       22
#define SDMA_XTRIG_CONF2_NUM7_LSH       24
#define SDMA_XTRIG_CONF2_CNF7_LSH       30

#define SDMA_CHNPRI_CHNPRI_LSH          0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define SDMA_RESET_RESET_WID            1
#define SDMA_RESET_RESCHED_WID          1

#define SDMA_PSW_CCR_WID                5
#define SDMA_PSW_CCP_WID                3
#define SDMA_PSW_NCR_WID                5
#define SDMA_PSW_NCP_WID                3

#define SDMA_CONFIG_CSM_WID             2
#define SDMA_CONFIG_ACR_WID             1
#define SDMA_CONFIG_RTDOBS_WID          1
#define SDMA_CONFIG_DSPDMA_WID          1

#define SDMA_ONCE_ENB_ENB_WID           1

#define SDMA_ONCE_INSTR_INSTR_WID       16

#define SDMA_ONCE_STAT_ECDR_WID         3
#define SDMA_ONCE_STAT_MST_WID          1
#define SDMA_ONCE_STAT_SWB_WID          1
#define SDMA_ONCE_STAT_ODR_WID          1
#define SDMA_ONCE_STAT_EDR_WID          1
#define SDMA_ONCE_STAT_RCV_WID          1
#define SDMA_ONCE_STAT_PST_WID          4

#define SDMA_ONCE_CMD_CMD_WID           4

#define SDMA_CHN0ADDR_CHN0ADDR_WID      14
#define SDMA_CHN0ADDR_SMSZ_WID          1

#define SDMA_XTRIG_CONF1_NUM0_WID       5
#define SDMA_XTRIG_CONF1_CNF0_WID       1
#define SDMA_XTRIG_CONF1_NUM1_WID       5
#define SDMA_XTRIG_CONF1_CNF1_WID       1
#define SDMA_XTRIG_CONF1_NUM2_WID       5
#define SDMA_XTRIG_CONF1_CNF2_WID       1
#define SDMA_XTRIG_CONF1_NUM3_WID       5
#define SDMA_XTRIG_CONF1_CNF2_WID       1

#define SDMA_XTRIG_CONF2_NUM4_WID       5
#define SDMA_XTRIG_CONF2_CNF4_WID       1
#define SDMA_XTRIG_CONF2_NUM5_WID       5
#define SDMA_XTRIG_CONF2_CNF5_WID       1
#define SDMA_XTRIG_CONF2_NUM6_WID       5
#define SDMA_XTRIG_CONF2_CNF6_WID       1
#define SDMA_XTRIG_CONF2_NUM7_WID       5
#define SDMA_XTRIG_CONF2_CNF7_WID       1

#define SDMA_CHNPRI_CHNPRI_WID          3

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// RESET
#define SDMA_RESET_RESET_INACTIVE       0
#define SDMA_RESET_RESET_ACTIVE         1

#define SDMA_RESET_RESCHED_INACTIVE     0
#define SDMA_RESET_RESCHED_FORCE        1

// PSW
#define SDMA_PSW_CCR_NOPEND             0

#define SDMA_PSW_CCP_NORUN              0

// CONFIG
#define SDMA_CONFIG_CSM_STATIC          0
#define SDMA_CONFIG_CSM_DYNLOWPWR       1
#define SDMA_CONFIG_CSM_DYNNOLOOP       2
#define SDMA_CONFIG_CSM_DYNAMIC         3

#define SDMA_CONFIG_ACR_AHB2X           0
#define SDMA_CONFIG_ACR_AHB1X           1

#define SDMA_CONFIG_RTDOBS_DISABLE      0
#define SDMA_CONFIG_RTDOBS_ENABLE       1

#define SDMA_CONFIG_RTDOBS_NOTUSED      0
#define SDMA_CONFIG_RTDOBS_USED         1

// ONCE_ENB
#define SDMA_ONCE_ENB_ENB_JTAGACCESS    0
#define SDMA_ONCE_ENB_ENB_HOSTACCESS    1

// ONCE_STAT
#define SDMA_ONCE_STAT_ECDR_ADDRA       0x1
#define SDMA_ONCE_STAT_ECDR_ADDRB       0x2
#define SDMA_ONCE_STAT_ECDR_DATA        0x4

#define SDMA_ONCE_STAT_MST_JTAG         0
#define SDMA_ONCE_STAT_MST_HOST         1

#define SDMA_ONCE_STAT_SWB_INACTIVE     0
#define SDMA_ONCE_STAT_SWB_ACTIVE       1

#define SDMA_ONCE_STAT_ODR_INACTIVE     0
#define SDMA_ONCE_STAT_ODR_ACTIVE       1

#define SDMA_ONCE_STAT_EDR_INACTIVE     0
#define SDMA_ONCE_STAT_EDR_ACTIVE       1

#define SDMA_ONCE_STAT_RCV_NOCMDPEND    0
#define SDMA_ONCE_STAT_RCV_CMDPEND      1

#define SDMA_ONCE_STAT_PST_PROGRAM      0
#define SDMA_ONCE_STAT_PST_DATA         1
#define SDMA_ONCE_STAT_PST_FLOW         2
#define SDMA_ONCE_STAT_PST_LOOPFLOW     3
#define SDMA_ONCE_STAT_PST_DEBUG        4
#define SDMA_ONCE_STAT_PST_FUNCUNIT     5
#define SDMA_ONCE_STAT_PST_SLEEP        6
#define SDMA_ONCE_STAT_PST_CONTEXTSAVE  7
#define SDMA_ONCE_STAT_PST_PROGSLP      8
#define SDMA_ONCE_STAT_PST_DATASLP      9
#define SDMA_ONCE_STAT_PST_FLOWSLP      10
#define SDMA_ONCE_STAT_PST_LOOPFLOWSLP  11
#define SDMA_ONCE_STAT_PST_DEBUGSLP     12
#define SDMA_ONCE_STAT_PST_FUNCUNITSLP  13
#define SDMA_ONCE_STAT_PST_RESETSLP     14
#define SDMA_ONCE_STAT_PST_CONTEXTLOAD  15

// ONCE_CMD
#define SDMA_ONCE_CMD_CMD_RSTATUS       0
#define SDMA_ONCE_CMD_CMD_DMOV          1
#define SDMA_ONCE_CMD_CMD_EXECONCE      2
#define SDMA_ONCE_CMD_CMD_RUNCORE       3
#define SDMA_ONCE_CMD_CMD_EXECCORE      4
#define SDMA_ONCE_CMD_CMD_DEBUGRQST     5
#define SDMA_ONCE_CMD_CMD_RBUFFER       6

// ONCE_CHN0ADDR
#define SDMA_CHN0ADDR_SMSZ_24WORDCNTX   0
#define SDMA_CHN0ADDR_SMSZ_32WORDCNTX   1

// XTRIG_CONF
#define SDMA_XTRIG_CONF_CNF_CHANNEL     0
#define SDMA_XTRIG_CONF_CNF_DMAREQ      1

// CHNPRI
#define SDMA_CHNPRI_CHNPRI_DISABLE      0
#define SDMA_CHNPRI_CHNPRI_LOWEST       1
#define SDMA_CHNPRI_CHNPRI_HIGHEST      7

// Bit field positions (left shift)
#define SDMA_MODE_COUNT_LSH         0
#define SDMA_MODE_DONE_LSH          16
#define SDMA_MODE_WRAP_LSH          17
#define SDMA_MODE_CONT_LSH          18
#define SDMA_MODE_INTR_LSH          19
#define SDMA_MODE_ERROR_LSH         20
#define SDMA_MODE_EXT_LSH           23
#define SDMA_MODE_COMMAND_LSH       24
#define SDMA_MODE_DATAWIDTH_LSH     24


// Bit field widths
#define SDMA_MODE_COUNT_WID         16
#define SDMA_MODE_DONE_WID          1
#define SDMA_MODE_WRAP_WID          1
#define SDMA_MODE_CONT_WID          1
#define SDMA_MODE_INTR_WID          1
#define SDMA_MODE_ERROR_WID         1
#define SDMA_MODE_EXT_WID           1
#define SDMA_MODE_COMMAND_WID       8
#define SDMA_MODE_DATAWIDTH_WID     2


// Bit field values
#define SDMA_MODE_DONE_READY        0
#define SDMA_MODE_DONE_NOTREADY     1

#define SDMA_MODE_WRAP_NOWRAP       0
#define SDMA_MODE_WRAP_WRAP2FIRST   1

#define SDMA_MODE_CONT_END          0
#define SDMA_MODE_CONT_NEXT         1

#define SDMA_MODE_INTR_NOSEND       0
#define SDMA_MODE_INTR_SEND         1

#define SDMA_MODE_ERROR_NOTFOUND    0
#define SDMA_MODE_ERROR_FOUND       1

#define SDMA_MODE_EXT_UNUSED        0
#define SDMA_MODE_EXT_USED          1

#define SDMA_MODE_DATAWIDTH_32      0
#define SDMA_MODE_DATAWIDTH_8       1
#define SDMA_MODE_DATAWIDTH_16      2
#define SDMA_MODE_DATAWIDTH_24      3


//-----------------------------------------------------------------------------
// SDMA SOFTWARE INTERFACE DEFINITIONS
//-----------------------------------------------------------------------------
typedef struct 
{
    volatile UINT32 mode;
    volatile UINT32 memAddrPA;
} SDMA_BUF_DESC, *PSDMA_BUF_DESC;

typedef struct 
{
    volatile UINT32 mode;
    volatile UINT32 srcAddrPA;
    volatile UINT32 destAddrPA;
} SDMA_BUF_DESC_EXT, *PSDMA_BUF_DESC_EXT;

typedef struct
{
    volatile UINT32 curBufDescPA;
    volatile UINT32 baseBufDescPA;
    volatile UINT32 chanDesc;
    volatile UINT32 chanStatus;    
} SDMA_CHAN_CTRL_BLOCK, *PSDMA_CHAN_CTRL_BLOCK;

typedef struct
{
    PVOID       pBaseBufDescUA;
    UINT32      numBufDesc;
    UINT32      dmaMask;
    UINT32      perAddr;
    UINT32      scriptAddr;
    BOOL        bExtended;
    BOOL        bStaticBufDesc;
    BOOL        bBufIntMem;
} SDMA_CHAN_DESC, *PSDMA_CHAN_DESC;

typedef struct  
{
    volatile UINT32 PC_RPC;
    volatile UINT32 SPC_EPC;
    volatile UINT32 GR[8];
    volatile UINT32 MDA;
    volatile UINT32 MSA;
    volatile UINT32 MS;
    volatile UINT32 MD;
    volatile UINT32 PDA;
    volatile UINT32 PSA;
    volatile UINT32 PS;
    volatile UINT32 PD;
    volatile UINT32 CA;
    volatile UINT32 CS;
    volatile UINT32 DDA;
    volatile UINT32 DSA;
    volatile UINT32 DS;
    volatile UINT32 DD;
#ifdef SDMA_CONTEXT_WITH_SCRATCH
    volatile UINT32 scratch[8];
#endif
} SDMA_CHANNEL_CONTEXT, *PSDMA_CHANNEL_CONTEXT;

typedef struct
{
    SDMA_CHAN_CTRL_BLOCK chanCtrlBlk[SDMA_NUM_CHANNELS];
    SDMA_CHAN_DESC chanDesc[SDMA_NUM_CHANNELS];
    SDMA_CHANNEL_CONTEXT chanCtxt;
    SDMA_BUF_DESC_EXT chan0BufDesc;
    SDMA_BUF_DESC_EXT chanBufDesc[SDMA_NUM_CHANNELS-1][SDMA_STATIC_BUF_DESC];
} SDMA_HOST_SHARED_REGION, *PSDMA_HOST_SHARED_REGION;

#ifdef __cplusplus
}
#endif

#endif // __COMMON_SDMA_H
