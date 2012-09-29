//------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  asrc_main.h
//
//------------------------------------------------------------------------------
#ifndef __ASRC_MAIN_H__
#define __ASRC_MAIN_H__

#include "asrc_base.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define ASRC_DMA_INPUT_BUF_SIZE   (4096*3)
#define ASRC_DMA_OUTPUT_BUF_SIZE   (4096*3)
#define ASRC_DMA_BUF_A 0
#define ASRC_DMA_BUF_B 1

typedef struct {
    ASRC_PAIR_INDEX pairIndex;
    BOOL bOpened;
    BOOL bNeedInit;
    BOOL bAsrcRunning;
    BOOL bAsrcSuspend;
    BOOL bInputDMARunning;
    BOOL bOutputDMARunning;
    BOOL bInputBufUnderrun;
    BOOL bOutputBufUnderrun;
    BOOL bExitInputIST;
    BOOL bExitOutputIST;
    ASRCHDR *pInputListHead;
    ASRCHDR *pInputListTail;
    ASRCHDR *pOutputListHead;
    ASRCHDR *pOutputListTail;
    PBYTE pInputBufPos; //the point to the position of current head buffer, for next copy
    PBYTE pOutputBufPos;
    HANDLE hInputEvent;
    HANDLE hOutputEvent;
    HANDLE hTriggerInputEvent;
    HANDLE hTriggerOutputEvent;
    HANDLE hInputIST;
    HANDLE hOutputIST;
    DWORD dwInputThreadID;
    DWORD dwOutputThreadID;
    PHYSICAL_ADDRESS    phyAddrInputBuf[2];
    PHYSICAL_ADDRESS    phyAddrOutputBuf[2];
    PBYTE pVirtInputBuf[2];
    PBYTE pVirtOutputBuf[2]; 
    BYTE indexNextInputDMABuf;
    BYTE indexNextOutputDMABuf;    
    BYTE availInputDMABufNum;
    BYTE availOutputDMABufNum;
    FLOAT dwUnProcessedSample;   // numble of sample not converted
    FLOAT dwInputSampleNum;  //tatol sample number ,for debug purpose
    FLOAT dwOutputSampleNum;
    DWORD dwInputChnNum;
    DWORD dwOutputChnNum;
    DWORD dwInputSampleRate;
    DWORD dwOutputSampleRate; 
    DWORD dwInitLenth; //data length to cut at begin when bNeedinit is set
    CRITICAL_SECTION lockInput;   // lock for input buffer queue
    CRITICAL_SECTION lockOutput;  // lock for output buffer queue
    CRITICAL_SECTION lockContext; // lock for general data access for the open context
    HANDLE hUserEvent[2];
}ASRC_OPEN_CONTEXT, * PASRC_OPEN_CONTEXT;
//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif // __ASRC_MAIN_H__
