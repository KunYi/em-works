//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  idmac.h
//
//  Common IPU definitions
//
//-----------------------------------------------------------------------------

#ifndef __IPU_H__
#define __IPU_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define IDMAC_CH_0                                    0
#define IDMAC_CH_1                                    1
#define IDMAC_CH_2                                    2
#define IDMAC_CH_3                                    3
#define IDMAC_CH_8                                    8
#define IDMAC_CH_9                                    9
#define IDMAC_CH_10                                   10
#define IDMAC_CH_11                                   11
#define IDMAC_CH_12                                   12
#define IDMAC_CH_13                                   13
#define IDMAC_CH_14                                   14
#define IDMAC_CH_15                                   15
#define IDMAC_CH_17                                   17
#define IDMAC_CH_18                                   18
#define IDMAC_CH_20                                   20
#define IDMAC_CH_21                                   21
#define IDMAC_CH_22                                   22
#define IDMAC_CH_23                                   23
#define IDMAC_CH_24                                   24
#define IDMAC_CH_27                                   27
#define IDMAC_CH_28                                   28
#define IDMAC_CH_29                                   29
#define IDMAC_CH_31                                   31
#define IDMAC_CH_33                                   33
#define IDMAC_CH_40                                   40
#define IDMAC_CH_41                                   41
#define IDMAC_CH_42                                   42
#define IDMAC_CH_43                                   43
#define IDMAC_CH_44                                   44
#define IDMAC_CH_45                                   45
#define IDMAC_CH_46                                   46
#define IDMAC_CH_47                                   47
#define IDMAC_CH_48                                   48
#define IDMAC_CH_49                                   49
#define IDMAC_CH_50                                   50
#define IDMAC_CH_51                                   51
#define IDMAC_CH_52                                   52

#define IDMAC_CH_SMFC_CH0                          IDMAC_CH_0
#define IDMAC_CH_SMFC_CH1                          IDMAC_CH_1
#define IDMAC_CH_SMFC_CH2                          IDMAC_CH_2
#define IDMAC_CH_SMFC_CH3                          IDMAC_CH_3
#define IDMAC_CH_VDI_INPUT_PREV_FIELD              IDMAC_CH_8
#define IDMAC_CH_VDI_INPUT_CUR_FIELD               IDMAC_CH_9
#define IDMAC_CH_VDI_INPUT_NEXT_FIELD              IDMAC_CH_10
#define IDMAC_CH_PP_INPUT_VIDEO                    IDMAC_CH_11
#define IDMAC_CH_PRP_INPUT_VIDEO                   IDMAC_CH_12
#define IDMAC_CH_VDI_OUTPUT                        IDMAC_CH_13
#define IDMAC_CH_PRP_INPUT_GRAPHICS                IDMAC_CH_14
#define IDMAC_CH_PP_INPUT_GRAPHICS                 IDMAC_CH_15
#define IDMAC_CH_PRP_TRANSPARENCY                  IDMAC_CH_17
#define IDMAC_CH_PP_TRANSPARENCY                   IDMAC_CH_18
#define IDMAC_CH_PRP_OUTPUT_ENC                    IDMAC_CH_20
#define IDMAC_CH_PRP_OUTPUT_VF                     IDMAC_CH_21
#define IDMAC_CH_PP_OUTPUT                         IDMAC_CH_22
#define IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE        IDMAC_CH_23
#define IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE      IDMAC_CH_24
#define IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE         IDMAC_CH_27
#define IDMAC_CH_DC_SYNC_ASYNC_FLOW                IDMAC_CH_28
#define IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE       IDMAC_CH_29
#define IDMAC_CH_DP_PRIMARY_FLOW_AUX_PLANE_TRANSPARENCY    IDMAC_CH_31
#define IDMAC_CH_DP_SECONDARY_FLOW_AUX_PLANE_TRANSPARENCY  IDMAC_CH_33
#define IDMAC_CH_DC_READ                           IDMAC_CH_40
#define IDMAC_CH_DC_ASYNC_FLOW                     IDMAC_CH_41
#define IDMAC_CH_DC_COMMAND_STREAM1                IDMAC_CH_42
#define IDMAC_CH_DC_COMMAND_STREAM2                IDMAC_CH_43
#define IDMAC_CH_DC_OUTPUT_MASK                    IDMAC_CH_44
#define IDMAC_CH_IRT_INPUT_PRP_ENC                 IDMAC_CH_45
#define IDMAC_CH_IRT_INPUT_PRP_VF                  IDMAC_CH_46
#define IDMAC_CH_IRT_INPUT_PP                      IDMAC_CH_47
#define IDMAC_CH_IRT_OUTPUT_PRP_ENC                IDMAC_CH_48
#define IDMAC_CH_IRT_OUTPUT_PRP_VF                 IDMAC_CH_49
#define IDMAC_CH_IRT_OUTPUT_PP                     IDMAC_CH_50
#define IDMAC_CH_DP_PRIMARY_FLOW_MAIN_PLANE_TRANSPARENCY   IDMAC_CH_51
#define IDMAC_CH_DP_SECONDARY_FLOW_MAIN_PLANE_TRANSPARENCY IDMAC_CH_52

#define IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV444   0
#define IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422   1
#define IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420   2

#define IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV422   3
#define IDMAC_PARTIAL_INTERLEAVED_FORMAT_CODE_YUV420   4
#define IDMAC_INTERLEAVED_FORMAT_CODE_LUT          5
#define IDMAC_INTERLEAVED_FORMAT_CODE_GENERIC      6
#define IDMAC_INTERLEAVED_FORMAT_CODE_RGB          7
#define IDMAC_INTERLEAVED_FORMAT_CODE_YUV444       7
#define IDMAC_INTERLEAVED_FORMAT_CODE_YUY2V        8
#define IDMAC_INTERLEAVED_FORMAT_CODE_Y2UYV        9
#define IDMAC_INTERLEAVED_FORMAT_CODE_UYVY2        10
#define IDMAC_INTERLEAVED_FORMAT_CODE_UY2VY        11

#define GENERAL_IPU_INTR_SNOOPING1                 0   // Snooping 1 intr
#define GENERAL_IPU_INTR_SNOOPING2                 1   // Snooping 2 intr
#define GENERAL_IPU_INTR_DP_SF_START               2   // DP Sync Flow Start
#define GENERAL_IPU_INTR_DP_SF_END                 3   // DP Sync Flow End
#define GENERAL_IPU_INTR_DP_ASF_START              4   // DP Async Flow Start
#define GENERAL_IPU_INTR_DP_ASF_END                5   // DP Async Flow End
#define GENERAL_IPU_INTR_DP_SF_BRAKE               6   // DP Sync Flow Brake
#define GENERAL_IPU_INTR_DP_ASF_BRAKE              7   // DP Async Flow Brake
#define GENERAL_IPU_INTR_DC_FC_0                   8   // DC Frame Complete for Disp 0
#define GENERAL_IPU_INTR_DC_FC_1                   9   // DC Frame Complete for Disp 1
#define GENERAL_IPU_INTR_DC_FC_2                   10  // DC Frame Complete for Disp 2
#define GENERAL_IPU_INTR_DC_FC_3                   11  // DC Frame Complete for Disp 3
#define GENERAL_IPU_INTR_DC_FC_4                   12  // DC Frame Complete for Disp 4
#define GENERAL_IPU_INTR_DC_FC_6                   13  // DC Frame Complete for Disp 6
#define GENERAL_IPU_INTR_DI_VSYNC_PRE_0            14  // DI0 Pre-VSYNC (2 rows ahead)
#define GENERAL_IPU_INTR_DI_VSYNC_PRE_1            15  // DI1 Pre-VSYNC (2 rows ahead)
#define GENERAL_IPU_INTR_DP_START                  16  // DP Start (any flow)
#define GENERAL_IPU_INTR_ASYNC_STOP                17  // DP Stop Async, move to Sync flow
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_0          18  // DI0 Counter 0 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_1          19  // DI0 Counter 1 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_2          20  // DI0 Counter 2 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_3          21  // DI0 Counter 3 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_4          22  // DI0 Counter 4 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_5          23  // DI0 Counter 5 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_6          24  // DI0 Counter 6 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_7          25  // DI0 Counter 7 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_8          26  // DI0 Counter 8 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_9          27  // DI0 Counter 9 triggered
#define GENERAL_IPU_INTR_DI0_CNT_EN_PRE_10         28  // DI0 Counter 10 triggered
#define GENERAL_IPU_INTR_DI1_DISP_CLK              29  // DI1 Display Clock
#define GENERAL_IPU_INTR_DI1_CNT_EN_PRE_3          30  // DI1 Counter 3 triggered
#define GENERAL_IPU_INTR_DI1_CNT_EN_PRE_8          31  // DI1 Counter 8 triggered

//------------------------------------------------------------------------------
// Types

typedef enum {
    IDMAC_CH_PRIORITY_LOW = 0,
    IDMAC_CH_PRIORITY_HIGH = 1
} IDMAC_CH_PRIORITY;

typedef enum {
    IDMAC_CH_BAND_MODE_DISABLE = 0,
    IDMAC_CH_BAND_MODE_ENABLE = 1
} IDMAC_CH_BAND_MODE;

// Enumeration to select an interrupt type
typedef enum
{
    IPU_INTR_TYPE_EOF,     // End-of-frame
    IPU_INTR_TYPE_NFACK,   // New frame acknowledgement
    IPU_INTR_TYPE_NFB4EOF, // New frame before end-of-frame
    IPU_INTR_TYPE_EOS,     // End-of-scroll
    IPU_INTR_TYPE_EOBND,   // End-of-band
    IPU_INTR_TYPE_TH,      // Threshold crossing indication
    IPU_INTR_TYPE_GENERAL, // General Purpose interrupts
}IPU_INTR_TYPE, *PIPU_INTR_TYPE;


//------------------------------------------------------------------------------
// Functions

// IDMAC Functions
BOOL IDMACRegsInit(void);
void IDMACRegsCleanup(void);
void IDMACChannelEnable(DWORD dwChannel);
void IDMACChannelDisable(DWORD dwChannel);
void IDMACChannelSetPriority(DWORD dwChannel, IDMAC_CH_PRIORITY pri);
void IDMACChannelSetBandMode(DWORD dwChannel, IDMAC_CH_BAND_MODE band_mode);
BOOL IDMACChannelIsBusy(DWORD dwChannel);

// IDMAC Functions for IPU Common registers
void IDMACChannelSetSepAlpha(DWORD dwChannel, BOOL bEnable);
void IDMACChannelLock(DWORD dwChannel, BOOL bEnable);
void IDMACWaterMark(DWORD dwChannel, BOOL bEnable);
void IDMACChannelDBMODE(DWORD dwChannel, BOOL);
void IDMACChannelBUF0SetReady(DWORD dwChannel);
void IDMACChannelBUF1SetReady(DWORD dwChannel);
void IDMACChannelBUF0ClrReady(DWORD dwChannel);
void IDMACChannelBUF1ClrReady(DWORD dwChannel);
BOOL IDMACChannelBUF0IsReady(DWORD dwChannel);
BOOL IDMACChannelBUF1IsReady(DWORD dwChannel);
BOOL IDMACChannelCurrentBufIsBuf0(DWORD dwChannel);
BOOL IDMACChannelCurrentBufIsBuf1(DWORD dwChannel);
void IDMACChannelClearIntStatus(DWORD dwChannel, IPU_INTR_TYPE IntrType);
BOOL IDMACChannelGetIntStatus(DWORD dwChannel, IPU_INTR_TYPE IntrType);
BOOL IDMACChannelQueryIntEnable(DWORD dwChannel, IPU_INTR_TYPE IntrType);
void IDMACChannelIntCntrl(DWORD dwChannel, IPU_INTR_TYPE IntrType, BOOL enable);
void IDMACChannelClrCurrentBuf(DWORD dwChannel);
void IDMACChannelBUFReadyPush();
void IDMACChannelBUFReadyPop();
UINT8 IDMACChannelAltChIndex(DWORD dwChannel);
void IDMACChannelTRBMODE(DWORD dwChannel, BOOL bTripleBuf);
INT8 IDMACChannelQueryTripleCurrentBuf(DWORD dwChannel);
void IDMACChannelBUF2SetReady(DWORD dwChannel);
BOOL IDMACChannelBUF2IsReady(DWORD dwChannel);
void IDMACChannelBUF2ClrReady(DWORD dwChannel);





// Debug helper function
void IDMACDumpRegs();

#ifdef __cplusplus
}
#endif

#endif //__IPU_H__

