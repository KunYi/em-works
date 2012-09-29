//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//  Header: mx25_fec.h
//
//  Provides definitions for the Fast Ethernet Controller (FEC) module that
//  is available on ARM9-based Freescale SOCs.
//
//------------------------------------------------------------------------------
#ifndef __MX25_FEC_H
#define __MX25_FEC_H

#if    __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define FEC_EIR_CLEARALL_MASK   0xfff80000

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef    struct
{
    REG32   _PAD;
    REG32   EIR;
    REG32   EIMR;
    REG32   _PAD1;
    REG32   RDAR;
    REG32   TDAR;
    REG32   _PAD2[3];
    REG32   ECR;
    REG32   _PAD3[6];
    REG32   MMFR;
    REG32   MSCR;
    REG32   _PAD4[7];
    REG32   MIBC;
    REG32   _PAD5[7];
    REG32   RCR;
    REG32   _PAD6[15];
    REG32   TCR;
    REG32   _PAD7[7];
    REG32   PALR;
    REG32   PAUR;
    REG32   OPD;
    REG32   _PAD8[10];
    REG32   IAUR;
    REG32   IALR;
    REG32   GAUR;
    REG32   GALR;
    REG32   _PAD9[7];
    REG32   TFWR;
    REG32   _PAD10;
    REG32   FRBR;
    REG32   FRSR;
    REG32   _PAD11[11];
    REG32   ERDSR;
    REG32   ETDSR;
    //offset 0x188
    REG32   EMRBR;
    REG32   _PAD12[93];
    //offset 0x300
    REG32   MIIGSK_CFGR;
    REG32   _PAD13;
    //offset 0x308
    REG32   MIIGSK_ENR;
} CSP_FEC_REGS, *PCSP_FEC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------

#define FEC_EIR_OFFSET                      (0x004)
#define FEC_EIMR_OFFSET                     (0x008)
#define FEC_RDAR_OFFSET                     (0x010)
#define FEC_TDAR_OFFSET                     (0x014)
#define FEC_ECR_OFFSET                      (0x024)
#define FEC_MMFR_OFFSET                     (0x040)
#define FEC_MSCR_OFFSET                     (0x044)
#define FEC_MIBC_OFFSET                     (0x064)
#define FEC_RCR_OFFSET                      (0x084)
#define FEC_TCR_OFFSET                      (0x0C4)
#define FEC_PALR_OFFSET                     (0x0E4)
#define FEC_PAUR_OFFSET                     (0x0E8)
#define FEC_OPD_OFFSET                      (0x0EC)
#define FEC_IAUR_OFFSET                     (0x118)
#define FEC_IALR_OFFSET                     (0x11C)
#define FEC_GAUR_OFFSET                     (0x120)
#define FEC_GALR_OFFSET                     (0x124)
#define FEC_TFWR_OFFSET                     (0x144)
#define FEC_FRBR_OFFSET                     (0x14C)
#define FEC_FRSR_OFFSET                     (0x150)
#define FEC_ERDSR_OFFSET                    (0x180)
#define FEC_ETDSR_OFFSET                    (0x184)
#define FEC_EMRBR_OFFSET                    (0x188)



//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

// EIR
#define FEC_EIR_UN_LSH                      19
#define FEC_EIR_RL_LSH                      20
#define FEC_EIR_LC_LSH                      21
#define FEC_EIR_EBERR_LSH                   22
#define FEC_EIR_MII_LSH                     23
#define FEC_EIR_RXB_LSH                     24
#define FEC_EIR_RXF_LSH                     25
#define FEC_EIR_TXB_LSH                     26
#define FEC_EIR_TXF_LSH                     27
#define FEC_EIR_GBA_LSH                     28
#define FEC_EIR_BABT_LSH                    29
#define FEC_EIR_BABR_LSH                    30
#define FEC_EIR_HBERR_LSH                   31

// EIMR
#define FEC_EIMR_UN_LSH                     19
#define FEC_EIMR_RL_LSH                     20
#define FEC_EIMR_LC_LSH                     21
#define FEC_EIMR_EBERR_LSH                  22
#define FEC_EIMR_MII_LSH                    23
#define FEC_EIMR_RXB_LSH                    24
#define FEC_EIMR_RXF_LSH                    25
#define FEC_EIMR_TXB_LSH                    26
#define FEC_EIMR_TXF_LSH                    27
#define FEC_EIMR_GBA_LSH                    28
#define FEC_EIMR_BABT_LSH                   29
#define FEC_EIMR_BABR_LSH                   30
#define FEC_EIMR_HBERR_LSH                  31

// RDAR
#define FEC_RDAR_ACTIVE_LSH                 24

// TDAR
#define FEC_TDAR_ACTIVE_LSH                 24

// ECR
#define FEC_ECR_RESET_LSH                   0
#define FEC_ECR_ETHEREN_LSH                 1

// MMFR
#define FEC_MMFR_DATA_LSH                   0
#define FEC_MMFR_TA_LSH                     16
#define FEC_MMFR_RA_LSH                     18
#define FEC_MMFR_PA_LSH                     23
#define FEC_MMFR_OP_LSH                     28
#define FEC_MMFR_ST_LSH                     30

// MSCR
#define FEC_MSCR_MIISPEED_LSH               1
#define FEC_MSCR_PREAMBLE_LSH               7

// MIBC
#define FEC_MIBC_MBIDLE_LSH                 30
#define FEC_MIBC_MBEN_LSH                   31

// RCR
#define FEC_RCR_LOOP_LSH                    0
#define FEC_RCR_DRT_LSH                     1
#define FEC_RCR_MIIMODE_LSH                 2
#define FEC_RCR_PROM_LSH                    3
#define FEC_RCR_BCREJ_LSH                   4
#define FEC_RCR_FCE_LSH                     5
#define FEC_RCR_MAXFL_LSH                   16

// TCR
#define FEC_TCR_GTS_LSH                     0
#define FEC_TCR_HBC_LSH                     1
#define FEC_TCR_FDEN_LSH                    2
#define FEC_TCR_TFCPAUSE_LSH                3
#define FEC_TCR_RFCPAUSE_LSH                4

// PALR
#define FEC_PALR_PADDR1_LSH                 0

// PAUR
#define FEC_PAUR_TYPE_LSH                   0
#define FEC_PAUR_PADDR2_LSH                 16

// OPD
#define FEC_OPD_PAUSEDUR_LSH                0
#define FEC_OPD_OPCODE_LSH                  16  

// IAUR
#define FEC_IAUR_IADDR1_LSH                 0   

// IALR
#define FEC_IALR_IADDR2_LSH                 0

// GAUR         
#define FEC_GAUR_GADDR1_LSH                 0   

// GALR
#define FEC_GALR_GADDR2_LSH                 0

// TFWR
#define FEC_TFWR_XWMRK_LSH                  0

// FRBR
#define FEC_FRBR_RBOUND_LSH                 2   

// FRSR
#define FEC_FRSR_RFSTART_LSH                2

// ERDSR
#define FEC_ERDSR_RDESSTART_LSH             0

// ETDSR
#define FEC_ETDSR_XDESSTART_LSH             0

// EMRBR
#define FEC_EMRBR_RBUFSIZE_LSH              0

// MIIGSK_CFGR
#define FEC_MIIGSK_CFGR_FRCONT_LSH          6
#define FEC_MIIGSK_CFGR_LBMODE_LSH          4
#define FEC_MIIGSK_CFGR_EMODE_LSH           3
#define FEC_MIIGSK_CFGR_IF_MODE_LSH         0

// MIIGSK_ENR
#define FEC_MIIGSK_ENR_READY_LSH            2
#define FEC_MIIGSK_ENR_EN_LSH               1      


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

// EIR
#define FEC_EIR_UN_WID                      1
#define FEC_EIR_RL_WID                      1
#define FEC_EIR_LC_WID                      1
#define FEC_EIR_EBERR_WID                   1
#define FEC_EIR_MII_WID                     1
#define FEC_EIR_RXB_WID                     1
#define FEC_EIR_RXF_WID                     1
#define FEC_EIR_TXB_WID                     1
#define FEC_EIR_TXF_WID                     1
#define FEC_EIR_GBA_WID                     1
#define FEC_EIR_BABT_WID                    1
#define FEC_EIR_BABR_WID                    1
#define FEC_EIR_HBERR_WID                   1

// EIMR
#define FEC_EIMR_UN_WID                     1
#define FEC_EIMR_RL_WID                     1
#define FEC_EIMR_LC_WID                     1
#define FEC_EIMR_EBERR_WID                  1
#define FEC_EIMR_MII_WID                    1
#define FEC_EIMR_RXB_WID                    1
#define FEC_EIMR_RXF_WID                    1
#define FEC_EIMR_TXB_WID                    1
#define FEC_EIMR_TXF_WID                    1
#define FEC_EIMR_GBA_WID                    1
#define FEC_EIMR_BABT_WID                   1
#define FEC_EIMR_BABR_WID                   1
#define FEC_EIMR_HBERR_WID                  1

// RDAR
#define FEC_RDAR_ACTIVE_WID                 1

// TDAR
#define FEC_TDAR_ACTIVE_WID                 1

// ECR
#define FEC_ECR_RESET_WID                   1
#define FEC_ECR_ETHEREN_WID                 1

// MMFR
#define FEC_MMFR_DATA_WID                   16
#define FEC_MMFR_TA_WID                     2
#define FEC_MMFR_RA_WID                     5
#define FEC_MMFR_PA_WID                     5
#define FEC_MMFR_OP_WID                     2
#define FEC_MMFR_ST_WID                     2

// MSCR
#define FEC_MSCR_MIISPEED_WID               6
#define FEC_MSCR_PREAMBLE_WID               1

// MIBC
#define FEC_MIBC_MBIDLE_WID                 1
#define FEC_MIBC_MBEN_WID                   1

// RCR
#define FEC_RCR_LOOP_WID                    1
#define FEC_RCR_DRT_WID                     1
#define FEC_RCR_MIIMODE_WID                 1
#define FEC_RCR_PROM_WID                    1
#define FEC_RCR_BCREJ_WID                   1
#define FEC_RCR_FCE_WID                     1
#define FEC_RCR_MAXFL_WID                   11

// TCR
#define FEC_TCR_GTS_WID                     1
#define FEC_TCR_HBC_WID                     1
#define FEC_TCR_FDEN_WID                    1
#define FEC_TCR_TFCPAUSE_WID                1
#define FEC_TCR_RFCPAUSE_WID                1

// PALR
#define FEC_PALR_PADDR1_WID                 32

// PAUR
#define FEC_PAUR_TYPE_WID                   16
#define FEC_PAUR_PADDR2_WID                 16

// OPD
#define FEC_OPD_PAUSEDUR_WID                16
#define FEC_OPD_OPCODE_WID                  16

// IAUR
#define FEC_IAUR_IADDR1_WID                 32

// IALR
#define FEC_IALR_IADDR2_WID                 32  

// GAUR         
#define FEC_GAUR_GADDR1_WID                 32  

// GALR
#define FEC_GALR_GADDR2_WID                 32  

// TFWR
#define FEC_TFWR_XWMRK_WID                  2

// FRBR
#define FEC_FRBR_RBOUND_WID                 8

// FRSR
#define FEC_FRSR_RFSTART_WID                8

// ERDSR
#define FEC_ERDSR_RDESSTART_WID             32

// ETDSR
#define FEC_ETDSR_XDESSTART_WID             32

// EMRBR
#define FEC_EMRBR_RBUFSIZE_WID              11

// MIIGSK_CFGR
#define FEC_MIIGSK_CFGR_FRCONT_WID          1
#define FEC_MIIGSK_CFGR_LBMODE_WID          1
#define FEC_MIIGSK_CFGR_EMODE_WID           1
#define FEC_MIIGSK_CFGR_IF_MODE_WID         2

// MIIGSK_ENR
#define FEC_MIIGSK_ENR_READY_WID            1
#define FEC_MIIGSK_ENR_EN_WID               1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// EIMR
#define FEC_EIMR_UN_UNMASK                  1
#define FEC_EIMR_UN_MASK                    0

#define FEC_EIMR_RL_UNMASK                  1
#define FEC_EIMR_RL_MASK                    0

#define FEC_EIMR_LC_UNMASK                  1
#define FEC_EIMR_LC_MASK                    0

#define FEC_EIR_EBERR_UNMASK                1
#define FEC_EIR_EBERR_MASK                  0

#define FEC_EIMR_MII_UNMASK                 1
#define FEC_EIMR_MII_MASK                   0

#define FEC_EIMR_RXB_UNMASK                 1
#define FEC_EIMR_RXB_MASK                   0

#define FEC_EIMR_RXF_UNMASK                 1
#define FEC_EIMR_RXF_MASK                   0

#define FEC_EIMR_TXB_UNMASK                 1
#define FEC_EIMR_TXB_MASK                   0

#define FEC_EIMR_TXF_UNMASK                 1
#define FEC_EIMR_TXF_MASK                   0

#define FEC_EIMR_GBA_UNMASK                 1
#define FEC_EIMR_GBA_MASK                   0

#define FEC_EIMR_BABT_UNMASK                1
#define FEC_EIMR_BABT_MASK                  0

#define FEC_EIMR_BABR_UNMASK                1
#define FEC_EIMR_BABR_MASK                  0

#define FEC_EIMR_HBERR_UNMASK               1
#define FEC_EIMR_HBERR_MASK                 0

// RDAR
#define FEC_RDAR_ACTIVE_ACTIVE              1
#define FEC_RDAR_ACTIVE_NONACTIVE           0

// TDAR
#define FEC_TDAR_ACTIVE_ACTIVE              1
#define FEC_TDAR_ACTIVE_NONACTIVE           0

// ECR
#define FEC_ECR_RESET_RESET                 1
#define FEC_ECR_RESET_NONRESET              0

#define FEC_ECR_ETHEREN_ENABLE              1
#define FEC_ECR_ETHEREN_DISABLE             0

// MMFR
#define FEC_MMFR_TA_VALUE                   2

#define FEC_MMFR_OP_WRITE                   1
#define FEC_MMFR_OP_READ                    2

#define FEC_MMFR_ST_VALUE                   1

// MSCR
#define FEC_MSCR_PREAMBLE_DISABLE           1
#define FEC_MSCR_PREAMBLE_ENABLE            0

// MIBC
#define FEC_MIBC_MBIDLE_IDLE                1
#define FEC_MIBC_MBIDLE_NONIDLE             0

#define FEC_MIBC_MBEN_DISABLE               1
#define FEC_MIBC_MBEN_ENABLE                0

// RCR
#define FEC_RCR_LOOP_ENABLE                 1
#define FEC_RCR_LOOP_DISABLE                0

#define FEC_RCR_DRT_ENABLE                  1
#define FEC_RCR_DRT_DISABLE                 0

#define FEC_RCR_MIIMODE_ENABLE              1
#define FEC_RCR_MIIMODE_DISABLE             0

#define FEC_RCR_PROM_ENABLE                 1
#define FEC_RCR_PROM_DISABLE                0

#define FEC_RCR_BCREJ_ENABLE                1
#define FEC_RCR_BCREJ_DISABLE               0

#define FEC_RCR_FCE_ENABLE                  1
#define FEC_RCR_FCE_DISABLE                 0

// TCR
#define FEC_TCR_GTS_ENABLE                  1
#define FEC_TCR_GTS_DISABLE                 0

#define FEC_TCR_HBC_ENABLE                  1
#define FEC_TCR_HBC_DISABLE                 0

#define FEC_TCR_FDEN_ENABLE                 1
#define FEC_TCR_FDEN_DISABLE                0

#define FEC_TCR_TFCPAUSE_ENABLE             1
#define FEC_TCR_TFCPAUSE_DISABLE            0

#define FEC_TCR_RFCPAUSE_ENABLE             1
#define FEC_TCR_RFCPAUSE_DISABLE            0

// PAUR
#define FEC_PAUR_TYPE_VALUE                 0x8808

// OPD
#define FEC_OPD_OPCODE_VALUE                1

// TFWR
#define FEC_TFWR_XWMRK_64BYTES              0
#define FEC_TFWR_XWMRK_128BYTES             2
#define FEC_TFWR_XWMRK_192BYTES             3

// MIIGSK_CFGR
#define FEC_MIIGSK_CFGR_FRCONT_100          0
#define FEC_MIIGSK_CFGR_FRCONT_10           1
#define FEC_MIIGSK_CFGR_LBMODE_OFF          0
#define FEC_MIIGSK_CFGR_LBMODE_ON           1
#define FEC_MIIGSK_CFGR_EMODE_OFF           0
#define FEC_MIIGSK_CFGR_EMODE_ON            1
#define FEC_MIIGSK_CFGR_IF_MODE_RMII        1
#define FEC_MIIGSK_CFGR_IF_MODE_MII         0

// MIIGSK_ENR
#define FEC_MIIGSK_ENR_READY                1
#define FEC_MIIGSK_ENR_NOT_READY            0
#define FEC_MIIGSK_ENR_EN_ENABLED           1
#define FEC_MIIGSK_ENR_EN_DISABLED          0

#ifdef __cplusplus
}
#endif

#endif // __MX25_FEC_H

