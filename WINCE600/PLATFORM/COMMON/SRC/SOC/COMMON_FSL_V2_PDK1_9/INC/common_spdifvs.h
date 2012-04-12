//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//  Header:  common_spdifvs.h
//
//  Provides definitions for SPDIF module that are common to Freescale SoCs.
//
//-----------------------------------------------------------------------------

#ifndef __COMMON_SPDIFVS_H
#define __COMMON_SPDIFVS_H

#if __cplusplus
extern "C" {
#endif


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
    UINT32 intTxUnFlNum;
    UINT32 intTxOvFlNum;
}  TX_INT_COUNTER;


//------------------------------------------------------------------------------
// Channel Status for Consumer use according to IEC60959-3
//------------------------------------------------------------------------------ 
#define IEC958_CON_CTRL_CONS                (0x0 << 0)  //Consumer use of channel status block
#define IEC958_CON_AUDIO_PCM                (0x0 << 1)
#define IEC958_CON_AUDIO_COMPRESSED         (0x1 << 1)
#define IEC958_CON_COPYRIGHT_NONE           (0x0 << 2) 
#define IEC958_CON_COPYRIGHT_WITH           (0x1 << 2)
#define IEC958_CON_EMPHASIS_NONE            (0x0 << 3)
#define IEC958_CON_EMPHASIS_5015            (0x1 << 3)
#define IEC958_CON_EMPHASIS_RESV            (0x2 << 3)
#define IEC958_CON_CHANNEL_MODE0            (0x0 << 6)

#define IEC958_CON_SOURCE_NUMBER_0          (0x0 << 0)
#define IEC958_CON_SOURCE_NUMBER_1          (0x1 << 0)
#define IEC958_CON_SOURCE_NUMBER_2          (0x2 << 0)
#define IEC958_CON_SOURCE_NUMBER_3          (0x3 << 0) 
#define IEC958_CON_SOURCE_NUMBER_4          (0x4 << 0)
#define IEC958_CON_SOURCE_NUMBER_5          (0x5 << 0)
#define IEC958_CON_SOURCE_NUMBER_6          (0x6 << 0)
#define IEC958_CON_SOURCE_NUMBER_7          (0x7 << 0)
#define IEC958_CON_SOURCE_NUMBER_8          (0x8 << 0)
#define IEC958_CON_SOURCE_NUMBER_9          (0x9 << 0)
#define IEC958_CON_SOURCE_NUMBER_10         (0x10 << 0)
#define IEC958_CON_SOURCE_NUMBER_11         (0x11 << 0)
#define IEC958_CON_SOURCE_NUMBER_12         (0x12 << 0)
#define IEC958_CON_SOURCE_NUMBER_13         (0x13 << 0)
#define IEC958_CON_SOURCE_NUMBER_14         (0x14 << 0)
#define IEC958_CON_SOURCE_NUMBER_15         (0x15 << 0)

#define IEC958_CON_SAMPLE_FREQ_44100        (0x0 << 0)
#define IEC958_CON_SAMPLE_FREQ_48000        (0x1 << 0)
#define IEC958_CON_SAMPLE_FREQ_32000        (0x3 << 0)

#define IEC958_CON_CLOCK_ACCURACY_LEVEL2    (0x0 << 4)
#define IEC958_CON_CLOCK_ACCURACY_LEVEL1    (0x1 << 4)
#define IEC958_CON_CLOCK_ACCURACY_LEVEL3    (0x2 << 4)
#define IEC958_CON_CLOCK_ACCURACY_RSV       (0x3 << 4)

#define IEC958_CON_MAX_LENGTH_20            (0x0 << 0)
#define IEC958_CON_MAX_LENGTH_24            (0x1 << 0)

#define IEC958_CON_SAMPLE_LENGTH_NODEFINED  (0x1 << 1)
#define IEC958_CON_SAMPLE_LENGTH_20_16      (0x1 << 1)
#define IEC958_CON_SAMPLE_LENGTH_22_18      (0x2 << 1)
#define IEC958_CON_SAMPLE_LENGTH_23_19      (0x4 << 1)
#define IEC958_CON_SAMPLE_LENGTH_24_20      (0x5 << 1)
#define IEC958_CON_SAMPLE_LENGTH_21_17      (0x6 << 1)

#define TST_PLL_LOCKED(reg) (INREG32(&(reg)) & CSP_BITFMASK(SPDIF_SRPC_LOCK))

//#define SPDIF_AUDIO_TYPE_PCM                         0x0
//#define SPDIF_AUDIO_TYPE_COMPRESSED           0x1

#ifdef __cplusplus
}
#endif

#endif // __COMMON_SPDIFVS_H

