//------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  asrc_sdk.h
//
//------------------------------------------------------------------------------
#ifndef __ASRC_SDK_H__
#define __ASRC_SDK_H__

#include "asrc_dev.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Functions

HANDLE ASRCOpenHandle(DWORD* pPairIndex);

BOOL ASRCCloseHandle(HANDLE hASRC, DWORD dwPairIndex);

BOOL ASRCOpenPair(HANDLE hASRC,PASRC_OPEN_PARAM pOpenParam );

BOOL ASRCClosePair(HANDLE hASRC,DWORD dwPairIndex );

BOOL ASRCPrepareInputBuffer(HANDLE hASRC, PASRCHDR pHdrIn);

BOOL ASRCPrepareOutputBuffer(HANDLE hASRC, PASRCHDR pHdrOut);

BOOL ASRCUnprepareInputBuffer(HANDLE hASRC, PASRCHDR pHdrIn);

BOOL ASRCUnprepareOutputBuffer(HANDLE hASRC, PASRCHDR pHdrOut);

BOOL ASRCAddInputBuffer(HANDLE hASRC, PASRCHDR pHdrIn);

BOOL ASRCAddOutputBuffer(HANDLE hASRC, PASRCHDR pHdrOut);

BOOL ASRCStart(HANDLE hASRC, DWORD dwPairIndex);

BOOL ASRCStop(HANDLE hASRC, DWORD dwPairIndex);

BOOL ASRCReset(HANDLE hASRC, DWORD dwPairIndex);

BOOL ASRCConfig(HANDLE hASRC, PASRC_CONFIG_PARAM pConfigParam);

BOOL ASRCGetCapability(HANDLE hASRC, PASRC_CAP_PARAM pCapParam);

BOOL ASRCSuspend(HANDLE hASRC, DWORD dwPairIndex);

BOOL ASRCResume(HANDLE hASRC, DWORD dwPairIndex);

#ifdef __cplusplus
}
#endif

#endif // __ASRC_SDK_H

