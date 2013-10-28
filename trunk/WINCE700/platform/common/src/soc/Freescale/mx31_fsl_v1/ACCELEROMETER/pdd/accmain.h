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
// Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:           accpdd.h
//  Purpose:
//
//
//------------------------------------------------------------------------------
//

#ifndef __ACC_PDD_H__
#define __ACC_PDD_H__

#if __cplusplus
extern "C" {
#endif

#include "acc.h"

    bool  ACCInitialize(void);
    void  ACCDeinitialize(void);
    bool  ACC_SetMode(ACC_MODE eACCMode);
    bool  ACC_SetGSel(ACC_GSEL eACCGSel);
    bool  ACC_SetLDTH(INT8 iLDTH);
    bool  ACC_SetPDTH(INT8 iPDTH);
    bool  ACC_SetLDPOL(ACC_POL ePOL);
    bool  ACC_SetPDPOL(ACC_POL ePOL);
    bool  ACC_SetDFBW(ACC_DFBW eDFBW);
    bool  ACC_SetTHOPT(ACC_THRESHOLD_OPT eTHOPT);
    bool  ACC_SetDetection(BYTE byDetection);
    bool  ACC_SetXOffset(INT16 iXOffset);
    bool  ACC_SetYOffset(INT16 iYOffset);
    bool  ACC_SetZOffset(INT16 iZOffset);
    bool  ACC_SetLatencyTime(BYTE byLT);
    bool  ACC_SetPulseDuration(BYTE byPD);
    bool  ACC_SetTimeWindow(BYTE byTW);
    bool  ACC_SetOutputWidth(ACC_OUTPUT_WIDTH eOutputWidth);
    bool  ACC_GetMode(ACC_MODE* peMode);
    bool  ACC_GetGSel(ACC_GSEL* peGSel);
    bool  ACC_GetLDTH(INT8* piLDTH);
    bool  ACC_GetPDTH(INT8* piPDTH);
    bool  ACC_GetLDPOL(ACC_POL* pePOL);
    bool  ACC_GetPDPOL(ACC_POL* pePOL);
    bool  ACC_GetLatencyTime(PBYTE pbyLT);
    bool  ACC_GetPulseDuration(PBYTE pbyPD);
    bool  ACC_GetTimeWindow(PBYTE pbyTW);
    bool  ACC_GetOutputWidth(ACC_OUTPUT_WIDTH* peOutputWidth);
    bool  ACC_GetOutput(ACC_OUTPUT* pOutput);
    bool  ACC_GetStatus(ACC_STATUS* pStatus);
    bool  ACC_GetDFBW(ACC_DFBW* peDFBW);
    bool  ACC_GetTHOPT(ACC_THRESHOLD_OPT* peTHOPT);
    bool  ACC_GetXOffset(INT16* piXOffset);
    bool  ACC_GetYOffset(INT16* piYOffset);
    bool  ACC_GetZOffset(INT16* piZOffset);
    bool  ACC_SetDRPD(ACC_DRPD eDRPD);

#ifdef __cplusplus
}
#endif

#endif // __ACC_PDD_H__
