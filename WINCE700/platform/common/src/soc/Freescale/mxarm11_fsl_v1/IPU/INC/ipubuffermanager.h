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
//  File:  IpuBufferManager.h
//
//  Class definition for an object to manage multiple buffering
//  in the IDMAC of the IPU.
//
//------------------------------------------------------------------------------

#ifndef __IPUBUFFERMANAGER_H__
#define __IPUBUFFERMANAGER_H__

//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types

#ifdef __cplusplus
extern "C" {
#endif


typedef struct ipuBufferDataStruct
{
    UINT32* pPhysAddr;
    UINT32* pVirtAddr;
} ipuBufferData, *pIpuBufferData;

//------------------------------------------------------------------------------
// Functions
class IpuBufferManager
{
    public:
        IpuBufferManager();
        ~IpuBufferManager();

        BOOL AllocateBuffers(ULONG, ULONG);
        DWORD SetActiveBuffer(UINT32**, UINT32);
        BOOL SetFilledBuffer();
        BOOL DeleteBuffers();
        void ResetBuffers();
        UINT32* GetBufferFilled();

        UINT32 GetMaxBuffers(void);

        void PrintBufferInfo();

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

#endif  // __IPUBUFFERMANAGER_H__
