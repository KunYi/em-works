//--------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//--------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Header:  common_spdif.h
//
//  Provides definitions for SPDIF
//  module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_SPDIF_H
#define __COMMON_SPDIF_H

#if __cplusplus
extern "C" {
#endif


typedef struct 
{
    UINT32 SCR;      // SPDIF configuration register 24 [23:0] 24' h000400
    UINT32 SRCD;    // CDText configuration register 24 [1] 24'h000000
    UINT32 SRPC;    // FreqMeas configuration register 24 [5:0] 24'h000000
    UINT32 SIE;       // Interrupt enable register 24 [23:0] 24'h000000
    UINT32 SIS;       // Interrupt status/clear register 24 [23:0] 24'h000002
    UINT32 SRL;      // SPDIF Receive Data - left channel 24 [23:0] 24'h000000
    UINT32 SRR;      // SPDIF Receive Data - right channel 24 [23:0] 24'h000000
    UINT32 SRCSH; // SPDIF Receive C channel, bits[47:24] 24 [23:0] 24'h000000
    UINT32 SRCSL; // SPDIF Receive C channel, bits[23:0] 24 [23:0] 24'h000000
    UINT32 SRU;     // SPDIF Receive U channel 24 [23:0] 24'h000000
    UINT32 SRQ;     // SPDIF Receive Q channel 24 [23:0] 24'h000000
    UINT32 STL;     // SPDIF Transmit Left channel 24 [23:0] 24'h000000
    UINT32 STR;     // SPDIF Transmit Rightchannel 24 [23:0] 24'h000000
    UINT32 STCSCH; //SPDIF Transmit Cons. C channel, bits [47:24] 24'h000000
    UINT32 STCSCL; // SPDIF Transmit Cons. C channel, bits [23:0] 24'h000000
    UINT32 RESERVED1[2];
    UINT32 SRFM;    // FreqMeasurement 24 [23:0] 24'h000000
    UINT32 RESERVED2[2];
    UINT32 STC;      // Transmit clock control register 24 [23:0] 24'h020f00
} SPDIF_REG, *PSPDIF_REG;


typedef struct
{  
    union
    {
        UINT32 Data;  
        struct
        {
            UINT32 ChannelStatus : 1;
            UINT32 AudioFormat   : 1; 
            UINT32 Copyright     : 1;
            UINT32 AddInfo       : 3;
            UINT32 ChannelMode   : 2;
            UINT32 CategoryCode  : 8;
            UINT32 SourceNumber  : 4;
            UINT32 ChannelNumber : 4;
            UINT32 Dummy         : 8;  
        } Ctrl;
     } H; //bits [47:24] of C Channel  
     //TODO : Need check if place of H,L right?
    union 
    {
        UINT32 Data;  
        struct
        {
            UINT32 SampleFreq    : 4;  
            UINT32 ClockAccuracy : 2;
            UINT32 Dummy1        : 2;
            UINT32 MaxLenth      : 1;
            UINT32 Samplenth     : 3;
            UINT32 Dummy2        : 20;  
        } Ctrl;
    } L;  //bits [23:0] of C Channel
      
} CCHANNEL, *P_CCHANNEL;

typedef UINT32 UCHANNEL, *P_UCHANNEL;
typedef UINT32 QCHANNEL, *P_QCHANNEL;

typedef struct
{
    CCHANNEL CChannel;  // C Channel status of receiver
    UCHANNEL UChannel;  // U Channel status of receiver
    QCHANNEL QChannel; // Q Channel status of receiver
} RX_CHANNEL_STATUS;

typedef struct
{
    RX_CHANNEL_STATUS RxStatus;  // Get channel status when receiver works
    UINT32 FreqMesa;                         // Get the receiver frequency 
} RX_CONTROL_DETAILS, *PRX_CONTROL_DETAILS;

typedef struct
{
    CCHANNEL CChannel;  //Consumer C Channel of transmitter
} TX_CHANNEL_STATUS;


typedef struct
{
    TX_CHANNEL_STATUS TxStatus;
} TX_CONTROL_DETAILS, *PTX_CONTROL_DETAILS;

typedef struct
{
    UINT32 intRxFIFOFulNum;
    UINT32 intRxLockLossNum;
    UINT32 intRxFIFOResynNum;
    UINT32 intRxFIFOUnOvNum;
    UINT32 intRxUQErrNum;
    UINT32 intRxUQSyncNum;
    UINT32 intRxQRxOvNum;
    UINT32 intRxQRxFulNum;
    UINT32 intRxURxOvNum;
    UINT32 intRxURxFulNum;
    UINT32 intRxBitErrNum;
    UINT32 intRxSymErrNum;
    UINT32 intRxValNoGoodNum;
    UINT32 intRxCNewNum;
    UINT32 intRxLockNum;
}  RX_INT_COUNTER;

typedef struct
{
    UINT32 intTxEmNum;
    UINT32 intTxResynNum;
    UINT32 intTxUnOvNum;
}  TX_INT_COUNTER;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define SPDIF_SCR_OFFSET          0x0000
#define SPDIF_SRCD_OFFSET         0x0004
#define SPDIF_SRPC_OFFSET         0x0008
#define SPDIF_SIE_OFFSET          0x000C
#define SPDIF_SIS_OFFSET          0x0010
#define SPDIF_SRL_OFFSET          0x0014
#define SPDIF_SRR_OFFSET          0x0018
#define SPDIF_SRCSH_OFFSET        0x001C
#define SPDIF_SRCSL_OFFSET        0x0020
#define SPDIF_SQU_OFFSET          0x0024
#define SPDIF_SRQ_OFFSET          0x0028
#define SPDIF_STL_OFFSET          0x002C
#define SPDIF_STR_OFFSET          0x0030
#define SPDIF_STCSCH_OFFSET       0x0034
#define SPDIF_STCSCL_OFFSET       0x0038
#define SPDIF_SRFM_OFFSET         0x0044
#define SPDIF_STC_OFFSET          0x0050

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT) & REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// Define for SPDIF Configuration Register(SCR)
#define SPDIF_SCR_USRCSEL_LSH           0
#define SPDIF_SCR_TXSEL_LSH               2
#define SPDIF_SCR_VALCTRL_LSH           5
#define SPDIF_SCR_DMATX_LSH             8
#define SPDIF_SCR_DMARX_LSH           9
#define SPDIF_SCR_TXFIFOCTRL_LSH     10
#define SPDIF_SCR_SOFTRESET_LSH       12
#define SPDIF_SCR_LOWPOWER_LSH       13
#define SPDIF_SCR_TXFIFOEMPTYSEL_LSH     15
#define SPDIF_SCR_TXAUTOSYNC_LSH    17
#define SPDIF_SCR_RCVAUTOSYNC_LSH  18
#define SPDIF_SCR_RCVFIFOFULLSEL_LSH      19
#define SPDIF_SCR_RCVFIFORST_LSH     21
#define SPDIF_SCR_RCVFIFOOFF_LSH     22
#define SPDIF_SCR_RCVFIFOCTRL_LSH   23

#define SPDIF_SCR_USRCSEL_WID           2
#define SPDIF_SCR_TXSEL_WID               3
#define SPDIF_SCR_VALCTRL_WID           2
#define SPDIF_SCR_DMARX_WID             1
#define SPDIF_SCR_DMATX_WID           1
#define SPDIF_SCR_TXFIFOCTRL_WID     2
#define SPDIF_SCR_SOFTRESET_WID       1
#define SPDIF_SCR_LOWPOWER_WID       1
#define SPDIF_SCR_TXFIFOEMPTYSEL_WID     2
#define SPDIF_SCR_TXAUTOSYNC_WID    1
#define SPDIF_SCR_RCVAUTOSYNC_WID  1
#define SPDIF_SCR_RCVFIFOFULLSEL_WID      2
#define SPDIF_SCR_RCVFIFORST_WID     1
#define SPDIF_SCR_RCVFIFOOFF_WID     1
#define SPDIF_SCR_RCVFIFOCTRL_WID   1

// Define for CDText Control Register(SRCD)
#define SPDIF_SRCD_USYNCMODE_LSH           1

#define SPDIF_SRCD_USYNCMODE_WID           1

// Define for PhaseConfig Register(SRPC)
#define SPDIF_SRPC_GAINSEL_LSH           3
#define SPDIF_SRPC_LOCK_LSH                 6
#define SPDIF_SRPC_CLKSRCSEL_LSH        7 

#define SPDIF_SRPC_GAINSEL_WID           3
#define SPDIF_SRPC_LOCK_WID                 1
#define SPDIF_SRPC_CLKSRCSEL_WID       4 

// Define for InterruptEN(SIE)/InterruptStat(SIS)/InterruptClear(SIC)
#define SPDIF_SIS_RXFIFOFUL_LSH           0
#define SPDIF_SIS_TXEM_LSH                1
#define SPDIF_SIS_LOCKLOSS_LSH         2
#define SPDIF_SIS_RXFIFORESYNC_LSH     3
#define SPDIF_SIS_RXFIFOUNOV_LSH        4
#define SPDIF_SIS_UQERR_LSH              5
#define SPDIF_SIS_UQSYNC_LSH            6
#define SPDIF_SIS_QRXOV_LSH              7
#define SPDIF_SIS_QRXFUL_LSH            8
#define SPDIF_SIS_URXOV_LSH              9
#define SPDIF_SIS_URXFUL_LSH            10
#define SPDIF_SIS_BITERR_LSH             14
#define SPDIF_SIS_SYMERR_LSH            15
#define SPDIF_SIS_VALNOGOOD_LSH     16
#define SPDIF_SIS_CNEW_LSH               17
#define SPDIF_SIS_TXRESYN_LSH          18
#define SPDIF_SIS_TXUNOV_LSH            19
#define SPDIF_SIS_LOCK_LSH                 20

#define SPDIF_SIS_RXFIFOFUL_WID           1
#define SPDIF_SIS_TXEM_WID                1
#define SPDIF_SIS_LOCKLOSS_WID         1
#define SPDIF_SIS_RXFIFORESYNC_WID     1
#define SPDIF_SIS_RXFIFOUNOV_WID        1
#define SPDIF_SIS_UQERR_WID              1
#define SPDIF_SIS_UQSYNC_WID            1
#define SPDIF_SIS_QRXOV_WID              1
#define SPDIF_SIS_QRXFUL_WID            1
#define SPDIF_SIS_URXOV_WID              1
#define SPDIF_SIS_URXFUL_WID            1
#define SPDIF_SIS_BITERR_WID             1
#define SPDIF_SIS_SYMERR_WID            1
#define SPDIF_SIS_VALNOGOOD_WID     1
#define SPDIF_SIS_CNEW_WID               1
#define SPDIF_SIS_TXRESYN_WID          1
#define SPDIF_SIS_TXUNOV_WID            1
#define SPDIF_SIS_LOCK_WID                 1

// Define for SPDIFRcvLeft Register(SRL)
#define SPDIF_SRL_RCVDATALEFT_LSH   0

#define SPDIF_SRL_RCVDATALEFT_WID   24

// Define for SPDIFRcvRight Register(SRR)
#define SPDIF_SRR_RCVDATARIGHT_LSH   0

#define SPDIF_SRR_RCVDATARIGHT_WID   24

// Define for SPDIFRcvCChannel_l Register(SRCSL)
#define SPDIF_SRCSL_RXCCHANNELL_LSH   0

#define SPDIF_SRCSL_RXCCHANNELL_WID   24

// Define for SPDIFRcvCChannel_h Register(SRCSH)
#define SPDIF_SRCSH_RXCCHANNELH_LSH   0

#define SPDIF_SRCSH_RXCCHANNELH_WID   24

// Define for SPDIFRcvCChannel_l Register(SRCSL)
#define SPDIF_SRCSL_RXCCHANNELL_LSH   0

#define SPDIF_SRCSL_RXCCHANNELL_WID   24


// Define for UChannelRcv Register(SRU)
#define SPDIF_SRU_RXUCHANNEL_LSH   0

#define SPDIF_SRU_RXUCHANNEL_WID   24

// Define for QChannelRcv Register(SRQ)
#define SPDIF_SRQ_RXQCHANNEL_LSH   0

#define SPDIF_SRQ_RXQCHANNEL_WID   24

// Define for SPDIFTxLeft Register(STL)
#define SPDIF_STL_TXDATALEFT_LSH   0

#define SPDIF_STL_TXDATALEFT_WID   24

// Define for SPDIFTxRight Register(STR)
#define SPDIF_STR_TXDATARIGHT_LSH   0

#define SPDIF_STR_TXDATARIGHT_WID   24

// Define for SPDIFTxCChannelCons_h Register(STCSCH)
#define SPDIF_STCSCH_TXCCHANNELCONSH_LSH   0

#define SPDIF_STCSCH_TXCCHANNELCONSH_WID   24

// Define for SPDIFTxCChannelCons_l Register(STCSCL)
#define SPDIF_STCSCL_TXCCHANNELCONSL_LSH   0

#define SPDIF_STCSCL_TXCCHANNELCONSL_WID   24

// Define for SPDIFTxCChannelProf_h Register(STCSPH)
#define SPDIF_STCSPH_TXCCHANNELPROFH_LSH   0

#define SPDIF_STCSPH_TXCCHANNELPROFH_WID   8

// Define for SPDIFTxCChannelProf_l Register(STCSPL)
#define SPDIF_STCSPL_TXCCHANNELPROFL_LSH   0

#define SPDIF_STCSPL_TXCCHANNELPROFL_WID   24

// Define for FreqMeas Register(SFM)
#define SPDIF_SRFM_FREQMEAS_LSH    0

#define SPDIF_SRFM_FREQMEAS_WID   24

// Define for SPDIFTxClk Register(STC)
#define SPDIF_STC_TXCLKDF_LSH      0
#define SPDIF_STC_TXCLKSRC_LSH   8
#define SPDIF_STC_SYSCLKDF_LSH   11


#define SPDIF_STC_TXCLKDF_WID    7
#define SPDIF_STC_TXCLKSRC_WID   3
#define SPDIF_STC_SYSCLKDF_WID   9


//------------------------------------------------------------------------------
// Channel Status for Consumer use according to IEC60959-3
//------------------------------------------------------------------------------ 
#define IEC958_CON_CTRL_CONS        (0x0 << 0)  //Consumer use of channel status block
#define IEC958_CON_AUDIO_PCM        (0x0 << 1)
#define IEC958_CON_AUDIO_COMPRESSED (0x1 << 1)
#define IEC958_CON_COPYRIGHT_NONE   (0x0 << 2) 
#define IEC958_CON_COPYRIGHT_WITH   (0x1 << 2)
#define IEC958_CON_EMPHASIS_NONE    (0x0 << 3)
#define IEC958_CON_EMPHASIS_5015    (0x1 << 3)
#define IEC958_CON_EMPHASIS_RESV    (0x2 << 3)
#define IEC958_CON_CHANNEL_MODE0    (0x0 << 6)

#define IEC958_CON_SOURCE_NUMBER_0  (0x0 << 0)
#define IEC958_CON_SOURCE_NUMBER_1  (0x1 << 0)
#define IEC958_CON_SOURCE_NUMBER_2  (0x2 << 0)
#define IEC958_CON_SOURCE_NUMBER_3  (0x3 << 0) 
#define IEC958_CON_SOURCE_NUMBER_4  (0x4 << 0)
#define IEC958_CON_SOURCE_NUMBER_5  (0x5 << 0)
#define IEC958_CON_SOURCE_NUMBER_6  (0x6 << 0)
#define IEC958_CON_SOURCE_NUMBER_7  (0x7 << 0)
#define IEC958_CON_SOURCE_NUMBER_8  (0x8 << 0)
#define IEC958_CON_SOURCE_NUMBER_9  (0x9 << 0)
#define IEC958_CON_SOURCE_NUMBER_10 (0x10 << 0)
#define IEC958_CON_SOURCE_NUMBER_11 (0x11 << 0)
#define IEC958_CON_SOURCE_NUMBER_12 (0x12 << 0)
#define IEC958_CON_SOURCE_NUMBER_13 (0x13 << 0)
#define IEC958_CON_SOURCE_NUMBER_14 (0x14 << 0)
#define IEC958_CON_SOURCE_NUMBER_15 (0x15 << 0)

#define IEC958_CON_SAMPLE_FREQ_44100 (0x0 << 0)
#define IEC958_CON_SAMPLE_FREQ_48000 (0x1 << 0)
#define IEC958_CON_SAMPLE_FREQ_32000 (0x3 << 0)

#define IEC958_CON_CLOCK_ACCURACY_LEVEL2 (0x0 << 4)
#define IEC958_CON_CLOCK_ACCURACY_LEVEL1 (0x1 << 4)
#define IEC958_CON_CLOCK_ACCURACY_LEVEL3 (0x2 << 4)
#define IEC958_CON_CLOCK_ACCURACY_RSV    (0x3 << 4)

#define IEC958_CON_MAX_LENGTH_20         (0x0 << 0)
#define IEC958_CON_MAX_LENGTH_24         (0x1 << 0)

#define IEC958_CON_SAMPLE_LENGTH_NODEFINED  (0x1 << 1)
#define IEC958_CON_SAMPLE_LENGTH_20_16   (0x1 << 1)
#define IEC958_CON_SAMPLE_LENGTH_22_18   (0x2 << 1)
#define IEC958_CON_SAMPLE_LENGTH_23_19   (0x4 << 1)
#define IEC958_CON_SAMPLE_LENGTH_24_20   (0x5 << 1)
#define IEC958_CON_SAMPLE_LENGTH_21_17   (0x6 << 1)

#define TST_PLL_LOCKED(reg) (INREG32(&(reg)) & CSP_BITFMASK(SPDIF_SRPC_LOCK))

//#define SPDIF_AUDIO_TYPE_PCM                0x0
//#define SPDIF_AUDIO_TYPE_COMPRESSED 0x1

#ifdef __cplusplus
}
#endif

#endif // __COMMON_CSPI_H

