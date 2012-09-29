//------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_ecspi.h
//
//  Provides definitions for eCSPI (Configurable Serial Peripheral Interface)
//  module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_ECSPI_H
#define __COMMON_ECSPI_H

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
    UINT32 RXDATA;
    UINT32 TXDATA;
    UINT32 CONTROLREG;
    UINT32 CONFIGREG;
    UINT32 INTREG;
    UINT32 DMAREG;
    UINT32 STATREG;
    UINT32 PERIODREG;
    UINT32 TESTREG;
    UINT32 MSG0REG;
    UINT32 MSG1REG;
    UINT32 MSG2REG;
    UINT32 MSG3REG;
    UINT32 MSG4REG;
    UINT32 MSG5REG;
    UINT32 MSG6REG;
    UINT32 MSG7REG;
    UINT32 MSG8REG;
    UINT32 MSG9REG;
    UINT32 MSG10REG;
    UINT32 MSG11REG;
    UINT32 MSG12REG;
    UINT32 MSG13REG;
    UINT32 MSG14REG;
    UINT32 MSG15REG;
} CSP_CSPI_REG, *PCSP_CSPI_REG;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define CSPI_RXDATA_OFFSET          0x0000
#define CSPI_TXDATA_OFFSET          0x0004
#define CSPI_CONTROLREG_OFFSET      0x0008
#define CSPI_CONFIGREG_OFFSET       0x000C
#define CSPI_INTREG_OFFSET          0x0010
#define CSPI_DMAREG_OFFSET          0x0014
#define CSPI_STATREG_OFFSET         0x0018
#define CSPI_PERIODREG_OFFSET       0x001C
#define CSPI_TESTREG_OFFSET         0x0020
#define CSPI_MSG0REG_OFFSET         0x0024
#define CSPI_MSG1REG_OFFSET         0x0028
#define CSPI_MSG2REG_OFFSET         0x002C
#define CSPI_MSG3REG_OFFSET         0x0030
#define CSPI_MSG4REG_OFFSET         0x0034
#define CSPI_MSG5REG_OFFSET         0x0038
#define CSPI_MSG6REG_OFFSET         0x003C
#define CSPI_MSG7REG_OFFSET         0x0040
#define CSPI_MSG8REG_OFFSET         0x0044
#define CSPI_MSG9REG_OFFSET         0x0048
#define CSPI_MSG10REG_OFFSET        0x004C
#define CSPI_MSG11REG_OFFSET        0x0050
#define CSPI_MSG12REG_OFFSET        0x0054
#define CSPI_MSG13REG_OFFSET        0x0058
#define CSPI_MSG14REG_OFFSET        0x005C
#define CSPI_MSG15REG_OFFSET        0x0060


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define CSPI_CONTROLREG_EN_LSH            0
#define CSPI_CONTROLREG_HW_LSH            1
#define CSPI_CONTROLREG_XCH_LSH           2
#define CSPI_CONTROLREG_SMC_LSH           3
#define CSPI_CONTROLREG_CHANNELMODE_LSH   4
#define CSPI_CONTROLREG_CHANNELMODE0_LSH  4
#define CSPI_CONTROLREG_CHANNELMODE1_LSH  5
#define CSPI_CONTROLREG_CHANNELMODE2_LSH  6
#define CSPI_CONTROLREG_CHANNELMODE3_LSH  7
#define CSPI_CONTROLREG_POSTDIVIDER_LSH   8
#define CSPI_CONTROLREG_PREDIVIDER_LSH    12
#define CSPI_CONTROLREG_DRCTL_LSH         16
#define CSPI_CONTROLREG_CHANNELSELECT_LSH 18
#define CSPI_CONTROLREG_BURSTLENGTH_LSH   20

#define CSPI_CONFIGREG_SCLKPHA_LSH        0
#define CSPI_CONFIGREG_SCLKPHA0_LSH       0
#define CSPI_CONFIGREG_SCLKPHA1_LSH       1
#define CSPI_CONFIGREG_SCLKPHA2_LSH       2
#define CSPI_CONFIGREG_SCLKPHA3_LSH       3
#define CSPI_CONFIGREG_SCLKPOL_LSH        4
#define CSPI_CONFIGREG_SCLKPOL0_LSH       4
#define CSPI_CONFIGREG_SCLKPOL1_LSH       5
#define CSPI_CONFIGREG_SCLKPOL2_LSH       6
#define CSPI_CONFIGREG_SCLKPOL3_LSH       7
#define CSPI_CONFIGREG_SSBCTRL_LSH        8
#define CSPI_CONFIGREG_SSBCTRL0_LSH       8
#define CSPI_CONFIGREG_SSBCTRL1_LSH       9
#define CSPI_CONFIGREG_SSBCTRL2_LSH       10
#define CSPI_CONFIGREG_SSBCTRL3_LSH       11
#define CSPI_CONFIGREG_SSBPOL_LSH         12
#define CSPI_CONFIGREG_SSBPOL0_LSH        12
#define CSPI_CONFIGREG_SSBPOL1_LSH        13
#define CSPI_CONFIGREG_SSBPOL2_LSH        14
#define CSPI_CONFIGREG_SSBPOL3_LSH        15
#define CSPI_CONFIGREG_DATACTL_LSH        16
#define CSPI_CONFIGREG_DATACTL0_LSH       16
#define CSPI_CONFIGREG_DATACTL1_LSH       17
#define CSPI_CONFIGREG_DATACTL2_LSH       18
#define CSPI_CONFIGREG_DATACTL3_LSH       19
#define CSPI_CONFIGREG_SCLKCTL_LSH        20
#define CSPI_CONFIGREG_SCLKCTL0_LSH       20
#define CSPI_CONFIGREG_SCLKCTL1_LSH       21
#define CSPI_CONFIGREG_SCLKCTL2_LSH       22
#define CSPI_CONFIGREG_SCLKCTL3_LSH       23
#define CSPI_CONFIGREG_HTLENGTH_LSH       24

#define CSPI_INTREG_TEEN_LSH              0
#define CSPI_INTREG_TDREN_LSH             1
#define CSPI_INTREG_TFEN_LSH              2
#define CSPI_INTREG_RREN_LSH              3
#define CSPI_INTREG_RDFEN_LSH             4
#define CSPI_INTREG_RFEN_LSH              5
#define CSPI_INTREG_ROEN_LSH              6
#define CSPI_INTREG_TCEN_LSH              7

#define CSPI_DMAREG_TXWATER_LSH     0
#define CSPI_DMAREG_TXDMAEN_LSH     7
#define CSPI_DMAREG_RXWATER_LSH     16
#define CSPI_DMAREG_RXDMAEN_LSH     23
#define CSPI_DMAREG_RXDMALEN_LSH    24
#define CSPI_DMAREG_RXTDEN_LSH      31

#define CSPI_STATREG_TE_LSH               0
#define CSPI_STATREG_TDR_LSH              1
#define CSPI_STATREG_TF_LSH               2
#define CSPI_STATREG_RR_LSH               3
#define CSPI_STATREG_RDF_LSH              4
#define CSPI_STATREG_RF_LSH               5
#define CSPI_STATREG_RO_LSH               6
#define CSPI_STATREG_TC_LSH               7

#define CSPI_PERIODREG_SAMPLEPERIOD_LSH   0
#define CSPI_PERIODREG_CSRC_LSH           15
#define CSPI_PERIODREG_CSDCTRL_LSH        16

#define CSPI_TESTREG_TXCNT_LSH            0
#define CSPI_TESTREG_RXCNT_LSH            8
#define CSPI_TESTREG_SMSTATUS_LSH         16
#define CSPI_TESTREG_CL_LSH               28
#define CSPI_TESTREG_LBC_LSH              31


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define CSPI_CONTROLREG_EN_WID            1
#define CSPI_CONTROLREG_HW_WID            1
#define CSPI_CONTROLREG_XCH_WID           1
#define CSPI_CONTROLREG_SMC_WID           1
#define CSPI_CONTROLREG_CHANNELMODE0_WID  1
#define CSPI_CONTROLREG_CHANNELMODE1_WID  1
#define CSPI_CONTROLREG_CHANNELMODE2_WID  1
#define CSPI_CONTROLREG_CHANNELMODE3_WID  1
#define CSPI_CONTROLREG_POSTDIVIDER_WID   4
#define CSPI_CONTROLREG_PREDIVIDER_WID    4
#define CSPI_CONTROLREG_DRCTL_WID         2
#define CSPI_CONTROLREG_CHANNELSELECT_WID 2
#define CSPI_CONTROLREG_BURSTLENGTH_WID   12

#define CSPI_CONFIGREG_SCLKPHA_WID        4
#define CSPI_CONFIGREG_SCLKPHA0_WID       1
#define CSPI_CONFIGREG_SCLKPHA1_WID       1
#define CSPI_CONFIGREG_SCLKPHA2_WID       1
#define CSPI_CONFIGREG_SCLKPHA3_WID       1
#define CSPI_CONFIGREG_SCLKPOL_WID        4
#define CSPI_CONFIGREG_SCLKPOL0_WID       1
#define CSPI_CONFIGREG_SCLKPOL1_WID       1
#define CSPI_CONFIGREG_SCLKPOL2_WID       1
#define CSPI_CONFIGREG_SCLKPOL3_WID       1
#define CSPI_CONFIGREG_SSBCTRL_WID        4
#define CSPI_CONFIGREG_SSBCTRL0_WID       1
#define CSPI_CONFIGREG_SSBCTRL1_WID       1
#define CSPI_CONFIGREG_SSBCTRL2_WID       1
#define CSPI_CONFIGREG_SSBCTRL3_WID       1
#define CSPI_CONFIGREG_SSBPOL_WID         4
#define CSPI_CONFIGREG_SSBPOL0_WID        1
#define CSPI_CONFIGREG_SSBPOL1_WID        1
#define CSPI_CONFIGREG_SSBPOL2_WID        1
#define CSPI_CONFIGREG_SSBPOL3_WID        1
#define CSPI_CONFIGREG_DATACTL_WID        4
#define CSPI_CONFIGREG_DATACTL0_WID       1
#define CSPI_CONFIGREG_DATACTL1_WID       1
#define CSPI_CONFIGREG_DATACTL2_WID       1
#define CSPI_CONFIGREG_DATACTL3_WID       1
#define CSPI_CONFIGREG_SCLKCTL_WID        4
#define CSPI_CONFIGREG_SCLKCTL0_WID       1
#define CSPI_CONFIGREG_SCLKCTL1_WID       1
#define CSPI_CONFIGREG_SCLKCTL2_WID       1
#define CSPI_CONFIGREG_SCLKCTL3_WID       1
#define CSPI_CONFIGREG_HTLENGTH_WID       5

#define CSPI_INTREG_TEEN_WID              1
#define CSPI_INTREG_TDREN_WID             1
#define CSPI_INTREG_TFEN_WID              1
#define CSPI_INTREG_RREN_WID              1
#define CSPI_INTREG_RDFEN_WID             1
#define CSPI_INTREG_RFEN_WID              1
#define CSPI_INTREG_ROEN_WID              1
#define CSPI_INTREG_TCEN_WID              1

#define CSPI_DMAREG_TXWATERMARK_WID       6
#define CSPI_DMAREG_TXDEN_WID             1
#define CSPI_DMAREG_RXWATERMARK_WID       6
#define CSPI_DMAREG_RXDEN_WID             1
#define CSPI_DMAREG_RXDMALENGTH_WID       6
#define CSPI_DMAREG_RXTDEN_WID            1

#define CSPI_STATREG_TE_WID               1
#define CSPI_STATREG_TDR_WID              1
#define CSPI_STATREG_TF_WID               1
#define CSPI_STATREG_RR_WID               1
#define CSPI_STATREG_RDF_WID              1
#define CSPI_STATREG_RF_WID               1
#define CSPI_STATREG_RO_WID               1
#define CSPI_STATREG_TC_WID               1

#define CSPI_PERIODREG_SAMPLEPERIOD_WID   15
#define CSPI_PERIODREG_CSRC_WID           1
#define CSPI_PERIODREG_CSDCTRL_WID        6

#define CSPI_TESTREG_TXCNT_WID            7
#define CSPI_TESTREG_RXCNT_WID            7
#define CSPI_TESTREG_SMSTATUS_WID         4
#define CSPI_TESTREG_CL_WID               2
#define CSPI_TESTREG_LBC_WID              1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// CONTROLREG

#define CSPI_CONTROLREG_CHANNELSELECT_0     0x0
#define CSPI_CONTROLREG_CHANNELSELECT_1     0x1
#define CSPI_CONTROLREG_CHANNELSELECT_2     0x2
#define CSPI_CONTROLREG_CHANNELSELECT_3     0x3

#define CSPI_CONTROLREG_DRCTL_DONTCARE      0x0
#define CSPI_CONTROLREG_DRCTL_FALLING_EDGE  0x1
#define CSPI_CONTROLREG_DRCTL_ACTIVE_LOW    0x2
#define CSPI_CONTROLREG_DRCTL_RSV           0x3

#define CSPI_CONTROLREG_POSTDIVIDER_DIV1     0x0
#define CSPI_CONTROLREG_POSTDIVIDER_DIV2     0x1
#define CSPI_CONTROLREG_POSTDIVIDER_DIV4     0x2
#define CSPI_CONTROLREG_POSTDIVIDER_DIV8     0x3
#define CSPI_CONTROLREG_POSTDIVIDER_DIV16    0x4
#define CSPI_CONTROLREG_POSTDIVIDER_DIV32    0x5
#define CSPI_CONTROLREG_POSTDIVIDER_DIV64    0x6
#define CSPI_CONTROLREG_POSTDIVIDER_DIV128   0x7
#define CSPI_CONTROLREG_POSTDIVIDER_DIV256   0x8
#define CSPI_CONTROLREG_POSTDIVIDER_DIV512   0x9
#define CSPI_CONTROLREG_POSTDIVIDER_DIV1024  0xa
#define CSPI_CONTROLREG_POSTDIVIDER_DIV2048  0xb
#define CSPI_CONTROLREG_POSTDIVIDER_DIV4096  0xc
#define CSPI_CONTROLREG_POSTDIVIDER_DIV8192  0xd
#define CSPI_CONTROLREG_POSTDIVIDER_DIV16384 0xe
#define CSPI_CONTROLREG_POSTDIVIDER_DIV32768 0xf

#define CSPI_CONTROLREG_CHANNELMODE_SLAVE   0x0     // Slave mode
#define CSPI_CONTROLREG_CHANNELMODE_MASTER  0x1     // Master mode

#define CSPI_CONTROLREG_SMC_NORMAL_MODE     0x0     // XCH bit control SPI burst
#define CSPI_CONTROLREG_SMC_AUTOMATIC_MODE  0x1     // Immediately start SPI
                                                    // burst when data is
                                                    // written to the TXFIFO

#define CSPI_CONTROLREG_XCH_IDLE            0x0     // Idle
#define CSPI_CONTROLREG_XCH_EN              0x1     // Initiates exchange

#define CSPI_CONTROLREG_HW_HTMODE_DISABLE   0x0     // HW Trigger mode disabled
#define CSPI_CONTROLREG_HW_HTMODE_ENABLE    0x1     // HW Trigger mode enabled

#define CSPI_CONTROLREG_EN_DISABLE          0x0     // SPI disabled
#define CSPI_CONTROLREG_EN_ENABLE           0x1     // SPI enabled

// CONFIGREG
#define CSPI_CONFIGREG_SCLKCTL_STAYHIGH     0x0
#define CSPI_CONFIGREG_SCLKCTL_STAYLOW      0x1

#define CSPI_CONFIGREG_DATACTL_STAYHIGH     0x0
#define CSPI_CONFIGREG_DATACTL_STAYLOW      0x1

#define CSPI_CONFIGREG_SSBPOL_ACTIVELOW     0x0
#define CSPI_CONFIGREG_SSBPOL_ACTIVEHIGH    0x1

#define CSPI_CONFIGREG_SSBCTRL_SINGLEBURST          0x0
#define CSPI_CONFIGREG_SSBCTRL_MULTIPLEBURSTS       0x1

#define CSPI_CONFIGREG_SSBCTRL_COMPLETEBITSRECEIVED 0x0
#define CSPI_CONFIGREG_SSBCTRL_COMPLETESSBNEGATED   0x1

#define CSPI_CONFIGREG_SCLKPOL_ACTIVEHIGH   0x0
#define CSPI_CONFIGREG_SCLKPOL_ACTIVELOW    0x1

#define CSPI_CONFIGREG_SCLKPHA_PHASE0       0x0
#define CSPI_CONFIGREG_SCLKPHA_PHASE1       0x1

// INTREG
#define CSPI_INTREG_TCEN_DISABLE        0       // Transfer completed interrupt
#define CSPI_INTREG_TCEN_ENABLE         1       // Transfer completed interrupt

#define CSPI_INTREG_ROEN_DISABLE        0       // RxFIFO overflow interrupt
#define CSPI_INTREG_ROEN_ENABLE         1       // RxFIFO overflow interrupt

#define CSPI_INTREG_RFEN_DISABLE        0       // RxFIFO full interrupt
#define CSPI_INTREG_RFEN_ENABLE         1       // RxFIFO full interrupt

#define CSPI_INTREG_RDFEN_DISABLE       0       // RxFIFO data full interrupt
#define CSPI_INTREG_RDFEN_ENABLE        1       // RxFIFO data full interrupt

#define CSPI_INTREG_RREN_DISABLE        0       // RxFIFO data ready interrupt
#define CSPI_INTREG_RREN_ENABLE         1       // RxFIFO data ready interrupt

#define CSPI_INTREG_TFEN_DISABLE        0       // TxFIFO full interrupt
#define CSPI_INTREG_TFEN_ENABLE         1       // TxFIFO full interrupt

#define CSPI_INTREG_TDREN_DISABLE       0       // TxFIFO data request interrupt
#define CSPI_INTREG_TDREN_ENABLE        1       // TxFIFO data request interrupt

#define CSPI_INTREG_TEEN_DISABLE        0       // TxFIFO empty interrupt
#define CSPI_INTREG_TEEN_ENABLE         1       // TxFIFO empty interrupt

// DMAREG
#define CSPI_DMAREG_RXTDEN_DISABLE      0       // RxFIFO Tail DMA req disable
#define CSPI_DMAREG_RXTDEN_ENABLE       1       // RxFIFO Tail DMA req enable

#define CSPI_DMAREG_RXDEN_DISABLE       0       // RxFIFO DMA req disable
#define CSPI_DMAREG_RXDEN_ENABLE        1       // RxFIFO DMA req enable

#define CSPI_DMAREG_TXDEN_DISABLE       0       // TxFIFO DMA req disable
#define CSPI_DMAREG_TXDEN_ENABLE        1       // TxFIFO DMA req enable

// STATREG
#define CSPI_STATREG_TE_TXFIFOHASDATA   0
#define CSPI_STATREG_TE_TXFIFOEMPTY     1

#define CSPI_STATREG_TDR_LESSTHANEQUAL  0
#define CSPI_STATREG_TDR_GREATERTHAN    1

#define CSPI_STATREG_TF_TXFIFONOTFULL   0
#define CSPI_STATREG_TF_TXFIFOFULL      1

#define CSPI_STATREG_RR_RXFIFOEMPTY     0
#define CSPI_STATREG_RR_RXFIFONOTEMPTY  1

#define CSPI_STATREG_RDF_LESSTHAN         0
#define CSPI_STATREG_RDF_GREATERTHANEQUAL 1

#define CSPI_STATREG_RF_NOTFULL          0
#define CSPI_STATREG_RF_RXFIFOFULL       1

#define CSPI_STATREG_RO_RXFIFOAVAILABLE  0
#define CSPI_STATREG_RO_RXFIFOOVERLFOW   1

#define CSPI_STATREG_TC_BUSY              0
#define CSPI_STATREG_TC_TRANSFERCOMPLETED 1

// PERIODREG
#define CSPI_PERIODREG_CSRC_SPI         0x0     // SPI clock
#define CSPI_PERIODREG_CSRC_CKIL        0x1     // 32.768kHz

// TESTREG
#define CSPI_TESTREG_LBC_NOTCONNECTED        0x0     // Not connected
#define CSPI_TESTREG_LBC_INTERNALLYCONNECTED 0x1     // Internally connected
#define CSPI_TESTREG_CL_NORMALMODE           0x0
#define CSPI_TESTREG_CL_ONECYCLEDELAYMODE    0x2


#ifdef __cplusplus
}
#endif

#endif // __COMMON_ECSPI_H
