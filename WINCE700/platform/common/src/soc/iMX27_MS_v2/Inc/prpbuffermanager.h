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
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
// File: PrpBufferManager.h
//
// Class definition for an object to manage multiple buffering
// in the pre-processor.
//
//------------------------------------------------------------------------------
#ifndef _PRP_BUFFER_MANAGER_H_
#define _PRP_BUFFER_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define PRP_MIN_NUM_BUFFERS             3
#define PRP_MAX_NUM_BUFFERS             100

//------------------------------------------------------------------------------
// Types
typedef struct {
    LPVOID pPhysAddr;
    LPVOID pVirtAddr;
} prpBufferData, *pPrpBufferData;

//------------------------------------------------------------------------------
// Functions
class PrpBufferManager {
public:
    PrpBufferManager();
    ~PrpBufferManager();
    BOOL AllocateBuffers(ULONG, ULONG);
    DWORD SetActiveBuffer(LPVOID *, UINT32);
    BOOL SetFilledBuffer();
    BOOL DeleteBuffers();
    VOID ResetBuffers();
    BOOL GetBufferFilled(pPrpBufferData);
    BOOL PutBufferIdle(pPrpBufferData);
    VOID PrintBufferInfo();

private:
    CRITICAL_SECTION m_csLockBufferQueues;
    DWORD m_iNumBuffers;

    // Queue handles
    HANDLE m_hReadIdleQueue, m_hWriteIdleQueue;
    HANDLE m_hReadBusyQueue, m_hWriteBusyQueue;
    HANDLE m_hReadFilledQueue, m_hWriteFilledQueue;
};

#ifdef __cplusplus
}
#endif

#endif  // _PRP_BUFFER_MANAGER_H_
