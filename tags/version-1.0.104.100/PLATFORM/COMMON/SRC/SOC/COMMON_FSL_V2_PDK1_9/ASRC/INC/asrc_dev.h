//------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  asrc_dev.h
//
//------------------------------------------------------------------------------
#ifndef __ASRCDEV_H__
#define __ASRCDEV_H__

#include <winioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define ASRC_IOCTL_OPEN_PAIR    CTL_CODE(FILE_DEVICE_SOUND , 3000, METHOD_BUFFERED, FILE_ANY_ACCESS)     
#define ASRC_IOCTL_CLOSE_PAIR   CTL_CODE(FILE_DEVICE_SOUND , 3001, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define ASRC_IOCTL_SET_CONFIG   CTL_CODE(FILE_DEVICE_SOUND , 3002, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define ASRC_IOCTL_PREPARE_BUFFER   CTL_CODE(FILE_DEVICE_SOUND , 3003, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define ASRC_IOCTL_ADD_INPUT_BUFFER   CTL_CODE(FILE_DEVICE_SOUND , 3004, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define ASRC_IOCTL_ADD_OUTPUT_BUFFER   CTL_CODE(FILE_DEVICE_SOUND , 3005, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define ASRC_IOCTL_START_CONVERT    CTL_CODE(FILE_DEVICE_SOUND , 3006, METHOD_BUFFERED, FILE_ANY_ACCESS)  
#define ASRC_IOCTL_STOP_CONVERT    CTL_CODE(FILE_DEVICE_SOUND , 3007, METHOD_BUFFERED, FILE_ANY_ACCESS)    
#define ASRC_IOCTL_REQUEST_PAIR    CTL_CODE(FILE_DEVICE_SOUND , 3008, METHOD_BUFFERED, FILE_ANY_ACCESS)    
#define ASRC_IOCTL_RELEASE_PAIR    CTL_CODE(FILE_DEVICE_SOUND , 3009, METHOD_BUFFERED, FILE_ANY_ACCESS)   
#define ASRC_IOCTL_PREPARE_INPUT_BUFFER    CTL_CODE(FILE_DEVICE_SOUND , 3010, METHOD_BUFFERED, FILE_ANY_ACCESS)   
#define ASRC_IOCTL_PREPARE_OUTPUT_BUFFER    CTL_CODE(FILE_DEVICE_SOUND , 3011, METHOD_BUFFERED, FILE_ANY_ACCESS)   
#define ASRC_IOCTL_UNPREPARE_INPUT_BUFFER    CTL_CODE(FILE_DEVICE_SOUND , 3012, METHOD_BUFFERED, FILE_ANY_ACCESS)   
#define ASRC_IOCTL_UNPREPARE_OUTPUT_BUFFER    CTL_CODE(FILE_DEVICE_SOUND , 3013, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define ASRC_IOCTL_RESET_PAIR    CTL_CODE(FILE_DEVICE_SOUND , 3014, METHOD_BUFFERED, FILE_ANY_ACCESS)   
#define ASRC_IOCTL_GET_CAPALITY    CTL_CODE(FILE_DEVICE_SOUND , 3015, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define ASRC_IOCTL_SUSPEND_CONVERT   CTL_CODE(FILE_DEVICE_SOUND , 3016, METHOD_BUFFERED, FILE_ANY_ACCESS)   
#define ASRC_IOCTL_RESUME_CONVERT   CTL_CODE(FILE_DEVICE_SOUND , 3017, METHOD_BUFFERED, FILE_ANY_ACCESS) 


#define ASRC_HDR_INPUT_QUEUED 0x1
#define ASRC_HDR_OUTPUT_QUEUED 0x2
#define ASRC_HDR_INPUT_PROCESSED 0x4
#define ASRC_HDR_OUTPUT_READY 0x8

//------------------------------------------------------------------------------
// Types

typedef enum {
    ASRC_PAIR_NONE = -1,    
  ASRC_PAIR_A = 0,
  ASRC_PAIR_B,
  ASRC_PAIR_C,
  ASRC_PAIR_ANY,
}ASRC_PAIR_INDEX;

typedef enum {
  ASRC_DATA_WIDTH_16BIT = 0,
  ASRC_DATA_WIDTH_24BIT,
  ASRC_DATA_WIDTH_32BIT
}ASRC_DATA_WIDTH;

typedef enum {
  ASRC_OCLK_SRC_ESAI_TX = 0,
  ASRC_OCLK_SRC_SSI1_TX,
  ASRC_OCLK_SRC_SSI2_TX,
  ASRC_OCLK_SRC_RSV1,
  ASRC_OCLK_SRC_SPDIF_TX,
  ASRC_OCLK_SRC_MLB,
  ASRC_OCLK_SRC_RSV2,
  ASRC_OCLK_SRC_RSV3,
  ASRC_OCLK_SRC_ESAI_RX,
  ASRC_OCLK_SRC_SSI1_RX,
  ASRC_OCLK_SRC_SSI2_RX,
  ASRC_OCLK_SRC_RSV4,
  ASRC_OCLK_SRC_SPDIF_RX,
  ASRC_OCLK_SRC_ASRCK1
}ASRC_OUTPUT_CLK_SRC;

typedef enum {
  ASRC_ICLK_SRC_ESAI_TX = 0,
  ASRC_ICLK_SRC_SSI1_TX,
  ASRC_ICLK_SRC_SSI2_TX,
  ASRC_ICLK_SRC_RSV1,
  ASRC_ICLK_SRC_SPDIF_TX,
  ASRC_ICLK_SRC_MLB,
  ASRC_ICLK_SRC_RSV2,
  ASRC_ICLK_SRC_RSV3,
  ASRC_ICLK_SRC_ESAI_RX,
  ASRC_ICLK_SRC_SSI1_RX,
  ASRC_ICLK_SRC_SSI2_RX,
  ASRC_ICLK_SRC_RSV4,
  ASRC_ICLK_SRC_SPDIF_RX,
  ASRC_ICLK_SRC_ASRCK1
}ASRC_INPUT_CLK_SRC;


typedef enum {
  ASRC_CLK_TWO_SRC = 0,            //we have both input and output clk from audio peri
  ASRC_CLK_ONE_SRC_INPUT ,     // we have only input clk from audio peri
  ASRC_CLK_ONE_SRC_OUTPUT ,    // we have only output clk from auido peri,
  ASRC_CLK_ONE_SRC_OUTPUT_AUTO_SEL,  //System assign appropriate clk source for output accoding to the output sample rate
  ASRC_CLK_NONE_SRC // we don't have any clk from audio peri, use asrc clk and ideal ratio
}ASRC_CLK_MODE;

typedef struct{
    ASRC_PAIR_INDEX pairIndex;
    UINT32 inputChnNum;
    UINT32 outputChnNum;
    HANDLE  hEventInputDone;
    HANDLE hEventOutputDone;
    TCHAR strInputEventName[16];
    TCHAR strOutputEventName[16];
    UINT32 lenInputEventName;
    UINT32 lenOutputEventName;
}ASRC_OPEN_PARAM,*PASRC_OPEN_PARAM;

typedef struct {
 // ASRC_DATA_WIDTH effectDataWidth;
  //ASRC_DATA_WIDTH packedDataWidth;
  // maybe we need aligned mode here 
  ASRC_PAIR_INDEX pairIndex;
  ASRC_OUTPUT_CLK_SRC outClkSrc;
  ASRC_INPUT_CLK_SRC inClkSrc;
  ASRC_CLK_MODE  clkMode;
  
  UINT32  inputSampleRate;  // unit of 1k samples
  UINT32  outputSampleRate;
  UINT32 inputBitClkRate; 
  UINT32 outputBitClkRate;    
}ASRC_CONFIG_PARAM,*PASRC_CONFIG_PARAM;

typedef enum {
  ASRC_TRANS_MEM2MEM = 0,
  ASRC_TRANS_MEM2ESAI,
}ASRC_TRANS_MODE;

typedef enum {
  ASRC_PROCESS_OPITION_SET = 0, 
  ASRC_PROCESS_OPITION_AUTO, 
}ASRC_PROCESS_OPITION_MODE;


typedef struct asrchdrf_tag{
    WAVEHDR wavHdr;
    PVOID pMashalledBuf;
    PVOID pAsyncBuf;
    PVOID pUser;
    DWORD dwPairIndex;
    DWORD dwValidOffset;    //num of the bytes for the offset of valid recorded data in the buffer,will be 0 in most case
    DWORD dwHeadFlags;
    struct asrchdrf_tag *lpNext;
}ASRCHDR, *PASRCHDR;    


typedef struct asrcCapability{
    UINT32 dwMaxInputSampleRate;
    UINT32 dwMinInputSampleRate;
    UINT32 dwMaxOutputSampleRate;
    UINT32 dwMinOutputSampleRate;
    UINT32 dwMaxChnNum;
    UINT32 dwInputBlockSize;
    UINT32 dwOutputBlockSize;
    BOOL bSupportOddChnNum;
    BOOL bSupportMultiPair;
    UINT16 pad;
    ASRC_PAIR_INDEX pairIndexDefault;
}ASRC_CAP_PARAM, *PASRC_CAP_PARAM;

//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif // __ASRCDEV_H__
