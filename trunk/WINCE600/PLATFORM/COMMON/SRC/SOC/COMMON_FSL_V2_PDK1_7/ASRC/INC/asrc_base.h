//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  asrc_base.h
//
//   This module provides a stream interface for the ASRC
//   driver. 
//
//------------------------------------------------------------------------------

#ifndef _ASRC_BASE_H_
#define _ASRC_BASE_H_

#pragma warning(push)
#pragma warning(disable: 4100 4115 4127 4189 4201 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#pragma warning(pop)

#include "common_asrc.h"
#include "common_ddk.h"
#include "common_macros.h"
#include "asrc_dev.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASRC_MAX_INPUT_SAMPLE_RATE  96000  //100 k
#define ASRC_MIN_INPUT_SAMPLE_RATE  8000  //8k

#define ASRC_MAX_OUTPUT_SAMPLE_RATE  96000  //100 k
#define ASRC_MIN_OUTPUT_SAMPLE_RATE  32000  //30k

#define ASRC_DMA_BUFFER_NUM  2
#define ASRC_INPUT_EVENT_NUM 1
#define ASRC_OUTPUT_EVENT_NUM 1

#define ASRC_BUF_INPUT_DONE_A   1
#define ASRC_BUF_INPUT_DONE_B   2
#define ASRC_BUF_OUTPUT_DONE_A  4
#define ASRC_BUF_OUTPUT_DONE_B  8

#define ASRC_BUF_A  0
#define ASRC_BUF_B  1

#define ASRC_VERSION_1  1
#define ASRC_VERSION_2  2


typedef struct {
  BOOL bOpened;  
  BOOL bIdealRatioMode;
  ASRC_PAIR_INDEX index;
  ASRC_TRANS_MODE transMode;  
  ASRC_PROCESS_OPITION_MODE opitionMode;
  UINT32  inputChnNum; 
  UINT32  outputChnNum;
  UINT32  pairChnNum;
  UINT8 inputDMAChan;
  UINT8 outputDMAChan;
  UINT8 inputDMAPriority;
  UINT8 outputDMAPriority;
  UINT32    inputWaterMark;
  UINT32    outputWaterMark;
  DDK_DMA_REQ   inputDMAReq;
  DDK_DMA_REQ   outputDMAReq;
  DWORD dwInputSysintr;
  DWORD dwInputThreadID;
  DWORD dwInputIRQ;
  HANDLE    hInputIST;
  HANDLE    hInputISTEvent;
  DWORD dwOutputSysintr;
  DWORD dwOutputThreadID;
  DWORD dwOutputIRQ;
  HANDLE hOutputIST;
  HANDLE hOutputISTEvent;
  PHYSICAL_ADDRESS  phyAddrInputBuf[ASRC_DMA_BUFFER_NUM];
  PHYSICAL_ADDRESS  phyAddrOutputBuf[ASRC_DMA_BUFFER_NUM];
  UINT32 inputBufSize[ASRC_DMA_BUFFER_NUM];  //num in bytes
  UINT32 outputBufSize[ASRC_DMA_BUFFER_NUM];  //num in bytes
  HANDLE inputEvent[ASRC_INPUT_EVENT_NUM];
  HANDLE outputEvent[ASRC_OUTPUT_EVENT_NUM]; 
  CRITICAL_SECTION lockInput;
  CRITICAL_SECTION lockOutput;
  UINT32    inputBufStatus;
  UINT32    outputBufStatus;
  UINT32    deviceWML;
  ASRC_CONFIG_PARAM configParam;
}ASRC_PAIR_CONTEXT,*PASRC_PAIR_CONTEXT;

typedef struct {
    PCSP_ASRC_REG   pAsrcReg;
    DWORD   dwVersion;
    DWORD   dwIrqAsrc;
    DWORD   dwSysintrAsrc ; 
    HANDLE hControllerIST;    
    HANDLE hControllerISTEvent;   
    PASRC_PAIR_CONTEXT pPairArray[3];
}ASRC_CONTROLLER_CONTEXT, *PASRC_CONTROLLER_CONTEXT;


BOOL AsrcDeinit(void);
BOOL AsrcInit(UINT32  *ppContext);

ASRC_PAIR_INDEX AsrcOpenPair(ASRC_PAIR_INDEX pairIndex,
                UINT32 inputChnNum,UINT32 outputChnNum, 
                HANDLE hInputEvent, HANDLE hOutputEvent,
                ASRC_TRANS_MODE transMode);

void AsrcClosePair(ASRC_PAIR_INDEX indexPair);
BOOL AsrcConfigPair(ASRC_PAIR_INDEX indexPair,PASRC_CONFIG_PARAM pParam);
BOOL AsrcSetInputBuf(ASRC_PAIR_INDEX indexPair, PHYSICAL_ADDRESS phyAddr,
    UINT32 numBytes, UINT32 index, PBYTE pVirtBuf );
BOOL AsrcSetOutputBuf(ASRC_PAIR_INDEX indexPair, PHYSICAL_ADDRESS phyAddr,
    UINT32 numBytes, UINT32 index, PBYTE pVirtBuf);
BOOL AsrcSetInputEvent(ASRC_PAIR_INDEX indexPair,HANDLE inputEvent);
BOOL AsrcSetOutputEvent(ASRC_PAIR_INDEX indexPair,HANDLE inputEvent);
BOOL AsrcStartConv(ASRC_PAIR_INDEX indexPair,BOOL bEnableChannel,
BOOL bEanbleInput,BOOL bEnableOutput);
BOOL AsrcStopConv(ASRC_PAIR_INDEX indexPair,BOOL bStopChannel,
    BOOL bStopInput,BOOL bStopOutput);
UINT32 AsrcGetInputBufStatus(ASRC_PAIR_INDEX indexPair);
UINT32 AsrcGetOutputBufStatus(ASRC_PAIR_INDEX indexPair);
BOOL AsrcPairUnderrun(ASRC_PAIR_INDEX indexPair);
BOOL AsrcSetP2PDeviceWML(ASRC_PAIR_INDEX indexPair,UINT32 deviceWML);
BOOL AsrcSuspendConvert(ASRC_PAIR_INDEX indexPair);
BOOL AsrcResumeConvert(ASRC_PAIR_INDEX indexPair);
BOOL AsrcSupportOddChnNum(void);


UINT32 AsrcGetBaseRegAddr(void);
UINT32 AsrcGetIRQ(void);
UINT8 AsrcGetInputDMAReq(ASRC_PAIR_INDEX index);
UINT8 AsrcGetOutputDMAReq(ASRC_PAIR_INDEX index);
UINT8 AsrcGetOutputP2PDMAReq(ASRC_PAIR_INDEX index);
UINT8 AsrcGetInputDMAPriority(ASRC_PAIR_INDEX index);
UINT8 AsrcGetOutputDMAPriority(ASRC_PAIR_INDEX index);
UINT32 AsrcGetSDMABaseIrq(void);
BOOL AsrcEnableClock(void);
UINT32 AsrcGetP2PInfo(PASRC_PAIR_CONTEXT pPairContext);
UINT32 AsrcGetVersion(void);



#ifdef __cplusplus
}
#endif


#endif // _ASRC_BASE_H_
