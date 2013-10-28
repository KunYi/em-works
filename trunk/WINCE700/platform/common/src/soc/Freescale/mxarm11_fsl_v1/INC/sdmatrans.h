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
//-----------------------------------------------------------------------------
//
//  Header:  sdmatrans.h
//
//  Provides definitions for specifying a data transfer handled by the SDMA.
//
//-----------------------------------------------------------------------------
#ifndef __SDMATRANS_H__
#define __SDMATRANS_H__

#if __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// Defines

 
//-----------------------------------------------------------------------------
// Types

typedef struct
{
    BOOL bDoneIntr;
    UINT16 numBytes;
    UINT32 srcAddrPA;
    UINT32 destAddrPA;
} SDMA_MEM_TRANS, *PSDMA_MEM_TRANS;

typedef struct
{
    BOOL bReady;
    BOOL bError;
} SDMA_TRANS_STATUS, *PSDMA_TRANS_STATUS;


//-----------------------------------------------------------------------------
// Functions

BOOL SdmaInit(void);
BOOL SdmaDeinit(void);
UINT8 SdmaOpenChan(UINT32 dmaReq, UINT8 priority);
BOOL SdmaCloseChan(UINT8 chan);
BOOL SdmaMapMemTransList(UINT8 chan, UINT32 numTrans, 
    PSDMA_MEM_TRANS pTransList);
BOOL SdmaUpdateMemTrans(UINT8 chan, UINT32 transIndex, PSDMA_MEM_TRANS pTrans);
BOOL SdmaResetTransStatus(UINT8 chan, UINT32 transIndex);
BOOL SdmaGetTransStatus(UINT8 chan, UINT32 transIndex, 
    PSDMA_TRANS_STATUS pStatus); 
BOOL SdmaResetTransList(UINT8 chan);
BOOL SdmaUnmapTransList(UINT8 chan);
BOOL SdmaStartChan(UINT8 chan);
BOOL SdmaStopChan(UINT8 chan, BOOL bKill);

#ifdef __cplusplus
}
#endif

#endif // __SDMATRANS_H__
