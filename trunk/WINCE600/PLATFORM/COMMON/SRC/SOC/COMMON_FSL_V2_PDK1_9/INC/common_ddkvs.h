//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  common_ddkVS.h
//
//  This header contains common DDK definitions that are common that are common 
//  to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_DDKVS_H
#define __COMMON_DDKVS_H

#if __cplusplus
extern "C" {
#endif

#define DDK_DMA_COMMAND_NO_DMA_XFER  0x0
#define DDK_DMA_COMMAND_DMA_WRITE    0x1
#define DDK_DMA_COMMAND_DMA_READ     0x2

#define DDK_DMA_ERROR_NONE           0
#define DDK_DMA_ERROR_EARLYTERM      1
#define DDK_DMA_ERROR_BUS            2
#define DDK_DMA_ERROR_INCOMPLETE     3
#define DDK_DMA_ERROR_TIMEOUT        4
#define DDK_DMA_ERROR_UNKNOWN        0x7fffffff

// DDK APBH Definitions
BOOL DDKApbhStartDma(UINT8 Channel,PVOID memAddrPA, UINT8 semaphore);
BOOL DDKApbhStopDma(UINT8 Channel);
BOOL DDKApbhDmaInitChan(UINT8 Channel,BOOL bEnableIrq);
BOOL DDKApbhDmaChanCLKGATE(UINT8 Channel,BOOL bClockGate);
BOOL DDKApbhDmaClearCommandCmpltIrq(UINT8 Channel);
BOOL DDKApbhDmaEnableCommandCmpltIrq(UINT8 Channel,BOOL bEnable);
BOOL DDKApbhDmaResetChan(UINT8 Channel,BOOL bReset);
BOOL DDKApbhDmaFreezeChan(UINT8 Channel,BOOL bFreeze);
UINT32 DDKApbhDmaGetPhore(UINT32 Channel);

// DDK APBX Definitions
BOOL DDKApbxStartDma(UINT8 Channel,PVOID memAddrPA, UINT8 semaphore);
BOOL DDKApbxStopDma(UINT8 Channel);
BOOL DDKApbxDmaInitChan(UINT8 Channel,BOOL bEnableIrq);
BOOL DDKApbxDmaGetActiveIrq(UINT8 Channel);
BOOL DDKApbxDmaGetErrorIrq(UINT8 Channel);
BOOL DDKApbxDmaClearCommandCmpltIrq(UINT8 Channel);
BOOL DDKApbxDmaClearErrorIrq(UINT8 Channel);
BOOL DDKApbxDmaEnableCommandCmpltIrq(UINT8 Channel,BOOL bEnable);
UINT32 DDKApbxDmaGetErrorStatus(UINT8 Channel);
BOOL DDKApbxDmaResetChan(UINT8 Channel, BOOL bReset);
BOOL DDKApbxDmaFreezeChan(UINT8 Channel, BOOL bFreeze);
UINT32 DDKApbxGetNextCMDAR(UINT8 Channel);
BOOL DDKApbxDmaClearInterrupts(UINT8 Channel);
BOOL DDKApbhDmaClearErrorIrq(UINT8 Channel);

#if __cplusplus
}
#endif

#endif // __COMMON_DDKVS_H
