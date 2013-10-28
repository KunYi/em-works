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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mxarm11_mu.h
//
//  Provides definitions for Messaging Unit (MU) 
//  module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_MU_H
#define __MXARM11_MU_H

#ifdef __cplusplus
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
     UINT32 MTR0;
     UINT32 MTR1;
     UINT32 MTR2;
     UINT32 MTR3;
     UINT32 MRR0;
     UINT32 MRR1;
     UINT32 MRR2;
     UINT32 MRR3;
     UINT32 MSR;
     UINT32 MCR;
} CSP_MU_REGS, *PCSP_MU_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define MU_MTR0_OFFSET          0x0000
#define MU_MTR1_OFFSET          0x0004
#define MU_MTR2_OFFSET          0x0008
#define MU_MTR3_OFFSET          0x000C
#define MU_MRR0_OFFSET          0x0010
#define MU_MRR1_OFFSET          0x0014
#define MU_MRR2_OFFSET          0x0018
#define MU_MRR3_OFFSET          0x001C
#define MU_MSR_OFFSET           0x0020
#define MU_MCR_OFFSET           0x000C


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define MU_MSR_MF0_LSH          0
#define MU_MSR_MF1_LSH          1
#define MU_MSR_MF2_LSH          2
#define MU_MSR_MNMIC_LSH        3
#define MU_MSR_MEP_LSH          4
#define MU_MSR_DPM_LSH          5
#define MU_MSR_DRS_LSH          7
#define MU_MSR_MFUP_LSH         8
#define MU_MSR_DRDIP_LSH        9
#define MU_MSR_MTE3_LSH         20
#define MU_MSR_MTE2_LSH         21
#define MU_MSR_MTE1_LSH         22
#define MU_MSR_MTE0_LSH         23
#define MU_MSR_MRF3_LSH         24
#define MU_MSR_MRF2_LSH         25
#define MU_MSR_MRF1_LSH         26
#define MU_MSR_MRF0_LSH         27
#define MU_MSR_MGIP3_LSH        28
#define MU_MSR_MGIP2_LSH        29
#define MU_MSR_MGIP1_LSH        30
#define MU_MSR_MGIP0_LSH        31


#define MU_MCR_MDF0_LSH         0
#define MU_MCR_MDF1_LSH         1
#define MU_MCR_MDF2_LSH         2
#define MU_MCR_DNMI_LSH         3
#define MU_MCR_DHR_LSH          4
#define MU_MCR_MMUR_LSH         5
#define MU_MCR_DRDIE_LSH        6
#define MU_MCR_MGIR3_LSH        16
#define MU_MCR_MGIR2_LSH        17
#define MU_MCR_MGIR1_LSH        18
#define MU_MCR_MGIR0_LSH        19
#define MU_MCR_MTIE3_LSH        20
#define MU_MCR_MTIE2_LSH        21
#define MU_MCR_MTIE1_LSH        22
#define MU_MCR_MTIE0_LSH        23
#define MU_MCR_MRIE3_LSH        24
#define MU_MCR_MRIE2_LSH        25
#define MU_MCR_MRIE1_LSH        26
#define MU_MCR_MRIE0_LSH        27
#define MU_MCR_MGIE3_LSH        28
#define MU_MCR_MGIE2_LSH        29
#define MU_MCR_MGIE1_LSH        30
#define MU_MCR_MGIE0_LSH        31


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define MU_MSR_MF0_WID          1
#define MU_MSR_MF1_WID          1
#define MU_MSR_MF2_WID          1
#define MU_MSR_MNMIC_WID        1
#define MU_MSR_MEP_WID          1
#define MU_MSR_DPM_WID          2
#define MU_MSR_DRS_WID          1
#define MU_MSR_MFUP_WID         1
#define MU_MSR_DRDIP_WID        1
#define MU_MSR_MTE3_WID         1
#define MU_MSR_MTE2_WID         1
#define MU_MSR_MTE1_WID         1
#define MU_MSR_MTE0_WID         1
#define MU_MSR_MRF3_WID         1
#define MU_MSR_MRF2_WID         1
#define MU_MSR_MRF1_WID         1
#define MU_MSR_MRF0_WID         1
#define MU_MSR_MGIP3_WID        1
#define MU_MSR_MGIP2_WID        1
#define MU_MSR_MGIP1_WID        1
#define MU_MSR_MGIP0_WID        1


#define MU_MCR_MDF0_WID         1
#define MU_MCR_MDF1_WID         1
#define MU_MCR_MDF2_WID         1
#define MU_MCR_DNMI_WID         1
#define MU_MCR_DHR_WID          1
#define MU_MCR_MMUR_WID         1
#define MU_MCR_DRDIE_WID        1
#define MU_MCR_MGIR3_WID        1
#define MU_MCR_MGIR2_WID        1
#define MU_MCR_MGIR1_WID        1
#define MU_MCR_MGIR0_WID        1
#define MU_MCR_MTIE3_WID        1
#define MU_MCR_MTIE2_WID        1
#define MU_MCR_MTIE1_WID        1
#define MU_MCR_MTIE0_WID        1
#define MU_MCR_MRIE3_WID        1
#define MU_MCR_MRIE2_WID        1
#define MU_MCR_MRIE1_WID        1
#define MU_MCR_MRIE0_WID        1
#define MU_MCR_MGIE3_WID        1
#define MU_MCR_MGIE2_WID        1
#define MU_MCR_MGIE1_WID        1
#define MU_MCR_MGIE0_WID        1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// MSR
#define MU_MSR_MF0_FLAGGED      1
#define MU_MSR_MF0_NOT_FLAGGED  0

#define MU_MSR_MF1_FLAGGED      1
#define MU_MSR_MF1_NOT_FLAGGED  0

#define MU_MSR_MF2_FLAGGED      1
#define MU_MSR_MF2_NOT_FLAGGED  0

#define MU_MSR_MNMIC_CLEAR      1

#define MU_MSR_MEP_PENDING      1
#define MU_MSR_MEP_NOT_PENDING  0

#define MU_MSR_DPM_RUN          0
#define MU_MSR_DPM_WAIT         1
#define MU_MSR_DPM_STOP         2
#define MU_MSR_DPM_DSM          3

#define MU_MSR_DRS_RESET        1
#define MU_MSR_DRS_NOT_RESET    0

#define MU_MSR_MFUP_PENDING     1
#define MU_MSR_MFUP_NOT_PENDING 0

#define MU_MSR_DRDIP_PENDING    1 // read value
#define MU_MSR_DRDIP_CLEAR      1 // write value

#define MU_MSR_MTE3_EMPTY       1
#define MU_MSR_MTE3_FULL        0

#define MU_MSR_MTE2_EMPTY       1
#define MU_MSR_MTE2_FULL        0

#define MU_MSR_MTE1_EMPTY       1
#define MU_MSR_MTE1_FULL        0

#define MU_MSR_MTE0_EMPTY       1
#define MU_MSR_MTE0_FULL        0

#define MU_MSR_MRF3_FULL        1
#define MU_MSR_MRF3_EMPTY       0

#define MU_MSR_MRF2_FULL        1
#define MU_MSR_MRF2_EMPTY       0

#define MU_MSR_MRF1_FULL        1
#define MU_MSR_MRF1_EMPTY       0

#define MU_MSR_MRF0_FULL        1
#define MU_MSR_MRF0_EMPTY       0

#define MU_MSR_MGIP3_PENDING     1
#define MU_MSR_MGIP3_NOT_PENDING 0

#define MU_MSR_MGIP2_PENDING     1
#define MU_MSR_MGIP2_NOT_PENDING 0

#define MU_MSR_MGIP1_PENDING     1
#define MU_MSR_MGIP1_NOT_PENDING 0

#define MU_MSR_MGIP0_PENDING     1
#define MU_MSR_MGIP0_NOT_PENDING 0


// MCR
#define MU_MCR_MDF0_SET_FLAG    1
#define MU_MCR_MDF0_CLEAR_FLAG  0

#define MU_MCR_MDF1_SET_FLAG    1
#define MU_MCR_MDF1_CLEAR_FLAG  0

#define MU_MCR_MDF2_SET_FLAG    1
#define MU_MCR_MDF2_CLEAR_FLAG  0

#define MU_MCR_DNMI_ISSUE_INTERRUPT 1
#define MU_MCR_DNMI_CLEARED         0

#define MU_MCR_DHR_ASSERT_RESET   1
#define MU_MCR_DHR_DEASSERT_RESET 0

#define MU_MCR_MMUR_ASSERT_RESET  1

#define MU_MCR_DRDIE_ENABLE     1
#define MU_MCR_DRDIE_DISABLE    0

#define MU_MCR_MGIR3_SET          1 // write value
#define MU_MCR_MGIR3_NOT_PENDING  0 // read value

#define MU_MCR_MGIR2_SET          1 // write value
#define MU_MCR_MGIR2_NOT_PENDING  0 // read value

#define MU_MCR_MGIR1_SET          1 // write value
#define MU_MCR_MGIR1_NOT_PENDING  0 // read value

#define MU_MCR_MGIR0_SET          1 // write value
#define MU_MCR_MGIR0_NOT_PENDING  0 // read value

#define MU_MCR_MTIE3_ENABLE       1
#define MU_MCR_MTIE3_DISABLE      0

#define MU_MCR_MTIE2_ENABLE       1
#define MU_MCR_MTIE2_DISABLE      0

#define MU_MCR_MTIE1_ENABLE       1
#define MU_MCR_MTIE1_DISABLE      0

#define MU_MCR_MTIE0_ENABLE       1
#define MU_MCR_MTIE0_DISABLE      0

#define MU_MCR_MRIE3_ENABLE       1
#define MU_MCR_MRIE3_DISABLE      0

#define MU_MCR_MRIE2_ENABLE       1
#define MU_MCR_MRIE2_DISABLE      0

#define MU_MCR_MRIE1_ENABLE       1
#define MU_MCR_MRIE1_DISABLE      0

#define MU_MCR_MRIE0_ENABLE       1
#define MU_MCR_MRIE0_DISABLE      0

#define MU_MCR_MGIE3_ENABLE       1
#define MU_MCR_MGIE3_DISABLE      0

#define MU_MCR_MGIE2_ENABLE       1
#define MU_MCR_MGIE2_DISABLE      0

#define MU_MCR_MGIE1_ENABLE       1
#define MU_MCR_MGIE1_DISABLE      0

#define MU_MCR_MGIE0_ENABLE       1
#define MU_MCR_MGIE0_DISABLE      0

#ifdef __cplusplus
}
#endif

#endif // __MXARM11_MU_H
