//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
