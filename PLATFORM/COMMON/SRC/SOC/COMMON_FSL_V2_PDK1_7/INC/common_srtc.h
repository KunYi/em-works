//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: comon_srtc.h
//
//  Provides definitions for the SRTC (Secure Real-Time Clock) 
//  module that are common to Freescale SoCs.
//
//-----------------------------------------------------------------------------
#ifndef __COMMON_SRTC_H
#define __COMMON_SRTC_H

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
    UINT32 LPSCMR;          // 0x000
    UINT32 LPSCLR;          // 0x004
    UINT32 LPSAR;           // 0x008
    UINT32 LPSMCR;          // 0x00C
    UINT32 LPCR;            // 0x010
    UINT32 LPSR;            // 0x014
    UINT32 LPPDR;           // 0x018
    UINT32 LPGR;            // 0x01C
    UINT32 HPCMR;           // 0x020
    UINT32 HPCLR;           // 0x024
    UINT32 HPAMR;           // 0x028
    UINT32 HPALR;           // 0x02C
    UINT32 HPCR;            // 0x030
    UINT32 HPISR;           // 0x034
    UINT32 HPIENR;          // 0x038
} CSP_SRTC_REGS, *PCSP_SRTC_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define  SRTC_LPSCMR_OFFSET             0x0000
#define  SRTC_LPSCLR_OFFSET             0x0004
#define  SRTC_LPSAR_OFFSET              0x0008
#define  SRTC_LPSMCR_OFFSET             0x000C
#define  SRTC_LPCR_OFFSET               0x0010
#define  SRTC_LPSR_OFFSET               0x0014
#define  SRTC_LPPDR_OFFSET              0x0018
#define  SRTC_LPGR_OFFSET               0x001C
#define  SRTC_HPCMR_OFFSET              0x0020
#define  SRTC_HPCLR_OFFSET              0x0024
#define  SRTC_HPAMR_OFFSET              0x0028
#define  SRTC_HPALR_OFFSET              0x002C
#define  SRTC_HPCR_OFFSET               0x0030
#define  SRTC_HPISR_OFFSET              0x0034
#define  SRTC_HPIENR_OFFSET             0x0038


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define  SRTC_LPCR_SWR_LP_LSH           0
#define  SRTC_LPCR_EN_LP_LSH            3
#define  SRTC_LPCR_WAE_LSH              4
#define  SRTC_LPCR_SAE_LSH              5
#define  SRTC_LPCR_SI_LSH               6
#define  SRTC_LPCR_ALP_LSH              7
#define  SRTC_LPCR_LTC_LSH              8
#define  SRTC_LPCR_LMC_LSH              9
#define  SRTC_LPCR_SV_LSH               10
#define  SRTC_LPCR_NSA_LSH              11
#define  SRTC_LPCR_NVEIE_LSH            12
#define  SRTC_LPCR_IEIE_LSH             13
#define  SRTC_LPCR_NVE_LSH              14
#define  SRTC_LPCR_IE_LSH               15

#define  SRTC_LPSR_TRI_LSH              0
#define  SRTC_LPSR_PGD_LSH              1
#define  SRTC_LPSR_CTD_LSH              2
#define  SRTC_LPSR_ALP_LSH              3
#define  SRTC_LPSR_MR_LSH               4
#define  SRTC_LPSR_TR_LSH               5
#define  SRTC_LPSR_EAD_LSH              6
#define  SRTC_LPSR_IT_LSH               7
#define  SRTC_LPSR_SM_LSH               10
#define  SRTC_LPSR_STATE_LP_LSH         12
#define  SRTC_LPSR_NVES_LSH             14
#define  SRTC_LPSR_IES_LSH              15

#define  SRTC_LPGR_LPGR_LSH             0
#define  SRTC_LPGR_SW_ISO_LSH           31

#define  SRTC_HPCR_SWR_HP_LSH           0
#define  SRTC_HPCR_EN_HP_LSH            3
#define  SRTC_HPCR_TS_LSH               4

#define  SRTC_HPISR_PI0_LSH             0
#define  SRTC_HPISR_PI1_LSH             1
#define  SRTC_HPISR_PI2_LSH             2
#define  SRTC_HPISR_PI3_LSH             3
#define  SRTC_HPISR_PI4_LSH             4
#define  SRTC_HPISR_PI5_LSH             5
#define  SRTC_HPISR_PI6_LSH             6
#define  SRTC_HPISR_PI7_LSH             7
#define  SRTC_HPISR_PI8_LSH             8
#define  SRTC_HPISR_PI9_LSH             9
#define  SRTC_HPISR_PI10_LSH            10
#define  SRTC_HPISR_PI11_LSH            11
#define  SRTC_HPISR_PI12_LSH            12
#define  SRTC_HPISR_PI13_LSH            13
#define  SRTC_HPISR_PI14_LSH            14
#define  SRTC_HPISR_PI15_LSH            15
#define  SRTC_HPISR_AHP_LSH             16
#define  SRTC_HPISR_WDHP_LSH            18
#define  SRTC_HPISR_WDLP_LSH            19
#define  SRTC_HPISR_WPHP_LSH            20
#define  SRTC_HPISR_WPLP_LSH            21

#define  SRTC_HPIENR_PI0_LSH            0
#define  SRTC_HPIENR_PI1_LSH            1
#define  SRTC_HPIENR_PI2_LSH            2
#define  SRTC_HPIENR_PI3_LSH            3
#define  SRTC_HPIENR_PI4_LSH            4
#define  SRTC_HPIENR_PI5_LSH            5
#define  SRTC_HPIENR_PI6_LSH            6
#define  SRTC_HPIENR_PI7_LSH            7
#define  SRTC_HPIENR_PI8_LSH            8
#define  SRTC_HPIENR_PI9_LSH            9
#define  SRTC_HPIENR_PI10_LSH           10
#define  SRTC_HPIENR_PI11_LSH           11
#define  SRTC_HPIENR_PI12_LSH           12
#define  SRTC_HPIENR_PI13_LSH           13
#define  SRTC_HPIENR_PI14_LSH           14
#define  SRTC_HPIENR_PI15_LSH           15
#define  SRTC_HPIENR_AHP_LSH            16
#define  SRTC_HPIENR_WDHP_LSH           18
#define  SRTC_HPIENR_WDLP_LSH           19


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define  SRTC_LPCR_SWR_LP_WID           1
#define  SRTC_LPCR_EN_LP_WID            1
#define  SRTC_LPCR_WAE_WID              1
#define  SRTC_LPCR_SAE_WID              1
#define  SRTC_LPCR_SI_WID               1
#define  SRTC_LPCR_ALP_WID              1
#define  SRTC_LPCR_LTC_WID              1
#define  SRTC_LPCR_LMC_WID              1
#define  SRTC_LPCR_SV_WID               1
#define  SRTC_LPCR_NSA_WID              1
#define  SRTC_LPCR_NVEIE_WID            1
#define  SRTC_LPCR_IEIE_WID             1
#define  SRTC_LPCR_NVE_WID              1
#define  SRTC_LPCR_IE_WID               1

#define  SRTC_LPSR_TRI_WID              1
#define  SRTC_LPSR_PGD_WID              1
#define  SRTC_LPSR_CTD_WID              1
#define  SRTC_LPSR_ALP_WID              1
#define  SRTC_LPSR_MR_WID               1
#define  SRTC_LPSR_TR_WID               1
#define  SRTC_LPSR_EAD_WID              1
#define  SRTC_LPSR_IT_WID               3
#define  SRTC_LPSR_SM_WID               2
#define  SRTC_LPSR_STATE_LP_WID         2
#define  SRTC_LPSR_NVES_WID             1
#define  SRTC_LPSR_IES_WID              1

#define  SRTC_LPGR_LPGR_WID             31
#define  SRTC_LPGR_SW_ISO_WID           1

#define  SRTC_HPCR_SWR_HP_WID           1
#define  SRTC_HPCR_EN_HP_WID            1
#define  SRTC_HPCR_TS_WID               1

#define  SRTC_HPISR_PI0_WID             1
#define  SRTC_HPISR_PI1_WID             1
#define  SRTC_HPISR_PI2_WID             1
#define  SRTC_HPISR_PI3_WID             1
#define  SRTC_HPISR_PI4_WID             1
#define  SRTC_HPISR_PI5_WID             1
#define  SRTC_HPISR_PI6_WID             1
#define  SRTC_HPISR_PI7_WID             1
#define  SRTC_HPISR_PI8_WID             1
#define  SRTC_HPISR_PI9_WID             1
#define  SRTC_HPISR_PI10_WID            1
#define  SRTC_HPISR_PI11_WID            1
#define  SRTC_HPISR_PI12_WID            1
#define  SRTC_HPISR_PI13_WID            1
#define  SRTC_HPISR_PI14_WID            1
#define  SRTC_HPISR_PI15_WID            1
#define  SRTC_HPISR_AHP_WID             1
#define  SRTC_HPISR_WDHP_WID            1
#define  SRTC_HPISR_WDLP_WID            1
#define  SRTC_HPISR_WPHP_WID            1
#define  SRTC_HPISR_WPLP_WID            1

#define  SRTC_HPIENR_PI0_WID            1
#define  SRTC_HPIENR_PI1_WID            1
#define  SRTC_HPIENR_PI2_WID            1
#define  SRTC_HPIENR_PI3_WID            1
#define  SRTC_HPIENR_PI4_WID            1
#define  SRTC_HPIENR_PI5_WID            1
#define  SRTC_HPIENR_PI6_WID            1
#define  SRTC_HPIENR_PI7_WID            1
#define  SRTC_HPIENR_PI8_WID            1
#define  SRTC_HPIENR_PI9_WID            1
#define  SRTC_HPIENR_PI10_WID           1
#define  SRTC_HPIENR_PI11_WID           1
#define  SRTC_HPIENR_PI12_WID           1
#define  SRTC_HPIENR_PI13_WID           1
#define  SRTC_HPIENR_PI14_WID           1
#define  SRTC_HPIENR_PI15_WID           1
#define  SRTC_HPIENR_AHP_WID            1
#define  SRTC_HPIENR_WDHP_WID           1
#define  SRTC_HPIENR_WDLP_WID           1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// LPCR
#define  SRTC_LPCR_SWR_LP_NORMAL        0
#define  SRTC_LPCR_SWR_LP_RESET         1

#define  SRTC_LPCR_EN_LP_DISABLE        0
#define  SRTC_LPCR_EN_LP_ENABLE         1

#define  SRTC_LPCR_WAE_DISABLE          0
#define  SRTC_LPCR_WAE_ENABLE           1

#define  SRTC_LPCR_SAE_DISABLE          0
#define  SRTC_LPCR_SAE_ENABLE           1

#define  SRTC_LPCR_SI_DISABLE           0
#define  SRTC_LPCR_SI_ENABLE            1

#define  SRTC_LPCR_ALP_DISABLE          0
#define  SRTC_LPCR_ALP_ENABLE           1

#define  SRTC_LPCR_LTC_UNLOCK           0
#define  SRTC_LPCR_LTC_LOCK             1

#define  SRTC_LPCR_LMC_UNLOCK           0
#define  SRTC_LPCR_LMC_LOCK             1

#define  SRTC_LPCR_SV_NORMAL            0
#define  SRTC_LPCR_SV_VIOLATION         1

#define  SRTC_LPCR_NSA_SECURE           0
#define  SRTC_LPCR_NSA_NONSECURE        1

#define  SRTC_LPCR_NVEIE_DISABLE        0
#define  SRTC_LPCR_NVEIE_ENABLE         1

#define  SRTC_LPCR_IEIE_DISABLE         0
#define  SRTC_LPCR_IEIE_ENABLE          1

#define  SRTC_LPCR_NVE_INVALID          0
#define  SRTC_LPCR_NVE_VALID            1

#define  SRTC_LPCR_IE_INIT              0
#define  SRTC_LPCR_IE_NORMAL            1

// LPSR
#define  SRTC_LPSR_TRI_VALID            0
#define  SRTC_LPSR_TRI_INVALID          1

#define  SRTC_LPSR_PGD_NORMAL           0
#define  SRTC_LPSR_PGD_GLITCH           1

#define  SRTC_LPSR_CTD_NORMAL           0
#define  SRTC_LPSR_CTD_TAMPER           1

#define  SRTC_LPSR_ALP_NORMAL           0
#define  SRTC_LPSR_ALP_ALARM            1

#define  SRTC_LPSR_MR_NORMAL            0
#define  SRTC_LPSR_MR_MAX               1

#define  SRTC_LPSR_TR_NORMAL            0
#define  SRTC_LPSR_TR_MAX               1

#define  SRTC_LPSR_EAD_NORMAL           0
#define  SRTC_LPSR_EAD_ALARM            1

#define  SRTC_LPSR_SM_LOW               0
#define  SRTC_LPSR_SM_MEDIUM            1
#define  SRTC_LPSR_SM_HIGH              2

#define  SRTC_LPSR_STATE_LP_INIT        0
#define  SRTC_LPSR_STATE_LP_INVALID     1
#define  SRTC_LPSR_STATE_LP_VALID       2
#define  SRTC_LPSR_STATE_LP_FAILURE     3

#define  SRTC_LPSR_NVES_INVALID         0
#define  SRTC_LPSR_NVES_VALID           1

#define  SRTC_LPSR_IES_INIT             0
#define  SRTC_LPSR_IES_NORMAL           1

// LPGR
#define  SRTC_LPGR_SW_ISO_INCLUDED      0
#define  SRTC_LPGR_SW_ISO_ISOLATED      1

// HPCR
#define  SRTC_HPCR_SWR_HP_NORMAL        0
#define  SRTC_HPCR_SWR_HP_RESET         1

#define  SRTC_HPCR_EN_HP_DISABLE        0
#define  SRTC_HPCR_EN_HP_ENABLE         1

#define  SRTC_HPCR_TS_NORMAL            0
#define  SRTC_HPCR_TS_SYNC              1

// HPISR
#define  SRTC_HPISR_AHP_NORMAL          0
#define  SRTC_HPISR_AHP_ALARM           1

#define  SRTC_HPISR_WDHP_PENDING        0
#define  SRTC_HPISR_WDHP_DONE           1

#define  SRTC_HPISR_WDLP_PENDING        0
#define  SRTC_HPISR_WDLP_DONE           1

#define  SRTC_HPISR_WPHP_DONE           0
#define  SRTC_HPISR_WPHP_PENDING        1

#define  SRTC_HPISR_WPLP_DONE           0
#define  SRTC_HPISR_WPLP_PENDING        1

// HPIENR
#define  SRTC_HPIENR_AHP_DISABLE        0
#define  SRTC_HPIENR_AHP_ENABLE         1

#define  SRTC_HPIENR_WDHP_DISABLE        0
#define  SRTC_HPIENR_WDHP_ENABLE         1

#define  SRTC_HPIENR_WDLP_DISABLE        0
#define  SRTC_HPIENR_WDLP_ENABLE         1

#ifdef __cplusplus
}
#endif


#endif    // __COMMON_SRTC_H
