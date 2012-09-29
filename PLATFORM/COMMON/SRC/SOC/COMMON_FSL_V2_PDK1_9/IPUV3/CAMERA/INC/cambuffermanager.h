//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CamBufferManager.h
//
//  Class definition for an object to manage multiple buffering
//  in the IDMAC of the IPUv3.
//
//------------------------------------------------------------------------------

#ifndef __CAMBUFFERMANAGER_H__
#define __CAMBUFFERMANAGER_H__

//------------------------------------------------------------------------------
// Defines
#define NUM_PIN_BUFFER_DRIVER 3
#define NUM_PIN_BUFFER_CLIENT 5
#define NUM_PIN_BUFFER_MAX    5

//------------------------------------------------------------------------------
// Types

#ifdef __cplusplus
extern "C" {
#endif


#define BUFFER_MODE_CLIENT                         1
#define BUFFER_MODE_DRIVER                         0

typedef struct CamBufferDataStruct
{
    UINT32* pPhysAddr;
    UINT32* pVirtAddr;
} CamBufferData, *pCamBufferData;

//------------------------------------------------------------------------------
// Functions
class CamBufferManager
{
    public:
        CamBufferManager();
        ~CamBufferManager();

        BOOL AllocateBuffers(ULONG, ULONG);
        DWORD SetActiveBuffer(UINT32**, UINT32);
        int SetFilledBuffer();
        BOOL DeleteBuffers();
        void ResetBuffers();
        void ResetBuffersToLocked();
        UINT32* GetBufferFilled();

        UINT32 GetMaxBuffers(void);
        UINT32 GetCurrentBufferNum(void);

        void PrintBufferInfo();

        DWORD GetAllocBufferPhyAddr(UINT32*[]);

        // add for register
        BOOL RegisterBuffer(LPVOID pVirtualAddress, UINT32 pPhysicalAddress);
        BOOL UnregisterBuffer(LPVOID pVirtualAddress, UINT32 pPhysicalAddress);
        BOOL UnregisterAllBuffers();
        BOOL EnQueue();
        UINT32 GetEnqueuedBufferNum();

    private:
        CRITICAL_SECTION m_csLockBufferQueues;

        volatile DWORD m_iNumBuffers;
        volatile DWORD m_iNumBuffersEnqueued;
        DWORD m_dwMode;         // 0 driver mode   1 client mode

        // Queue handles
        HANDLE m_hReadIdleQueue, m_hWriteIdleQueue;
        HANDLE m_hReadBusyQueue, m_hWriteBusyQueue;
        HANDLE m_hReadFilledQueue, m_hWriteFilledQueue;
        HANDLE m_hReadLockedQueue, m_hWriteLockedQueue;

        IpuBuffer *m_hCamBuffer[NUM_PIN_BUFFER_DRIVER];
        ipuBufferData m_sIpubufferData[NUM_PIN_BUFFER_CLIENT];    // only used for client mode
        HANDLE m_hIPUBase;
};

#ifdef __cplusplus
}
#endif

#endif  // __CAMBUFFERMANAGER_H__
