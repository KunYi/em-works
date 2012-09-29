//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  sdma_p2p.h
//
//  This header contains ss15 sdma script definitions for P2P Audio DMA.
//
//------------------------------------------------------------------------------
#ifndef __SDMA_P2P_H
#define __SDMA_P2P_H

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define SDMA_P2PINFO_LWML_LSH       0
#define SDMA_P2PINFO_PS_LSH         8
#define SDMA_P2PINFO_PA_LSH         9
#define SDMA_P2PINFO_SPDIF_LSH      10
#define SDMA_P2PINFO_SP_LSH         11
#define SDMA_P2PINFO_DP_LSH         12
#define SDMA_P2PINFO_HWML_LSH       14
#define SDMA_P2PINFO_N_LSH          24
#define SDMA_P2PINFO_LWE_LSH        28
#define SDMA_P2PINFO_HWE_LSH        29
#define SDMA_P2PINFO_CONT_LSH       31

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define SDMA_P2PINFO_LWML_WID       8
#define SDMA_P2PINFO_PS_WID         1
#define SDMA_P2PINFO_PA_WID         1
#define SDMA_P2PINFO_SPDIF_WID      1
#define SDMA_P2PINFO_SP_WID         1
#define SDMA_P2PINFO_DP_WID         1
#define SDMA_P2PINFO_HWML_WID       10
#define SDMA_P2PINFO_N_WID          4
#define SDMA_P2PINFO_LWE_WID        1
#define SDMA_P2PINFO_HWE_WID        1
#define SDMA_P2PINFO_CONT_WID       1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define SDMA_P2PINFO_PS_PAD_NOT_SWALLOW  0
#define SDMA_P2PINFO_PS_PAD_SWALLOW      1

#define SDMA_P2PINFO_PA_PAD_NOT_ADD      0
#define SDMA_P2PINFO_PA_PAD_ADD          1

#define SDMA_P2PINFO_SP_AIPS_PER         0
#define SDMA_P2PINFO_SP_SPBA_PER         1

#define SDMA_P2PINFO_DP_AIPS_PER         0
#define SDMA_P2PINFO_DP_SPBA_PER         1

#define SDMA_P2PINFO_LWE_IN_EVENT1_REG        0
#define SDMA_P2PINFO_LWE_IN_EVENT2_REG        1

#define SDMA_P2PINFO_HWE_IN_EVENT1_REG        0
#define SDMA_P2PINFO_HWE_IN_EVENT2_REG        1

#define SDMA_P2PINFO_CONT_NO_CONTINUOUS_TRANSFER      0
#define SDMA_P2PINFO_CONT_CONTINUOUS_TRANSFER         1

#endif // __SDMA_P2P_H
