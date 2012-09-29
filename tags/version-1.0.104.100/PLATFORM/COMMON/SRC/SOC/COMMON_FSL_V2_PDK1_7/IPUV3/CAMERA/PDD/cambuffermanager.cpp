//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  camerabuffermanager.cpp
//
//  Implementation of camera buffer manager driver methods.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"

#include "CamBufferManager.h"
#include "CameraPDDProps.h"
#include "cameradbg.h"

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define BUFMAN_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define BUFMAN_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#define THREAD_PRIORITY                            250

#define IPU_MAX_NUM_BUFFERS                        100


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: CamBufferManager
//
// Buffer manager class constructor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
CamBufferManager::CamBufferManager(void)
{
    MSGQUEUEOPTIONS queueOptions;

    BUFMAN_FUNCTION_ENTRY();

    InitializeCriticalSection(&m_csLockBufferQueues);

    // open handle to the IPU_BASE driver in order to enable IC module
    // First, create handle to IPU_BASE driver
    m_hIPUBase = IPUV3BaseOpenHandle();
    if (m_hIPUBase == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
    }   

    for (UINT i =0; i< NUM_PIN_BUFFER_DRIVER; i++)
    {
        m_hCamBuffer[i] = NULL;
    }

    // Create queues for read messages and write messages
    queueOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    queueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    queueOptions.dwMaxMessages = IPU_MAX_NUM_BUFFERS;
    queueOptions.cbMaxMessage = sizeof(ipuBufferData);
    queueOptions.bReadAccess = TRUE; // we need read-access to msgqueue

    // For allocated buffer in driver mode,We have 3 queues for each channel:  
    // One for idle buffers, one for busy buffers, and one for filled buffers.
    // For each of these buffers, a read and write handle is needed.
    // 6 handles total
    // For allocated buffer in client mode,adds one queue for each channel:
    // added one for locked buffers.
    // 8 handles total

    // Create read handles to idle,busy,filled and locked queues
    m_hReadIdleQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadIdleQueue)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Error creating Read idle queue handle.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    m_hReadBusyQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadBusyQueue)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Error creating Read busy queue handle.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    m_hReadFilledQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadFilledQueue)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Error creating Read filled queue handle.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    m_hReadLockedQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadLockedQueue)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Error creating Read locked queue handle.  Initialization failed! \r\n"), __WFUNCTION__));
    }
    queueOptions.bReadAccess = FALSE; // we need write-access to msgqueue

    // Create read handles to idle,busy,filled and locked queues
    m_hWriteIdleQueue = OpenMsgQueue(GetCurrentProcess(), m_hReadIdleQueue, &queueOptions);
    if (!m_hWriteIdleQueue)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Error opening idle queue for writing.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    m_hWriteBusyQueue = OpenMsgQueue(GetCurrentProcess(), m_hReadBusyQueue, &queueOptions);
    if (!m_hWriteBusyQueue)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Error opening busy queue for writing.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    m_hWriteFilledQueue = OpenMsgQueue(GetCurrentProcess(), m_hReadFilledQueue, &queueOptions);
    if (!m_hWriteFilledQueue)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Error opening filled queue for writing.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    m_hWriteLockedQueue = OpenMsgQueue(GetCurrentProcess(), m_hReadLockedQueue, &queueOptions);
    if (!m_hWriteFilledQueue)
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Error opening locked queue for writing.  Initialization failed! \r\n"), __WFUNCTION__));
    }

    m_dwMode = BUFFER_MODE_DRIVER;   // driver mode
    m_iNumBuffers = 0;
    m_iNumBuffersEnqueued = 0;

    for(int i = 0; i < NUM_PIN_BUFFER_CLIENT; i++)
    {
        m_sIpubufferData[i].pPhysAddr = NULL;
        m_sIpubufferData[i].pVirtAddr = NULL;
    }

    BUFMAN_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: ~CamBufferManager
//
// The destructor for Class.  Deletes buffes and closes queue handles.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
CamBufferManager::~CamBufferManager(void)
{
    if( m_dwMode == 0 )
    {
        DeleteBuffers();
    }
    else
    {
        UnregisterAllBuffers();
    }
    DeleteCriticalSection(&m_csLockBufferQueues); 

    CloseHandle(m_hReadIdleQueue);
    m_hReadIdleQueue = NULL;
    CloseHandle(m_hWriteIdleQueue);
    m_hWriteIdleQueue = NULL;
    CloseHandle(m_hReadBusyQueue);
    m_hReadBusyQueue = NULL;
    CloseHandle(m_hWriteBusyQueue);
    m_hWriteBusyQueue = NULL;
    CloseHandle(m_hReadFilledQueue);
    m_hReadFilledQueue = NULL;
    CloseHandle(m_hWriteFilledQueue);
    m_hWriteFilledQueue = NULL;
    CloseHandle(m_hReadLockedQueue);
    m_hReadLockedQueue = NULL;
    CloseHandle(m_hWriteLockedQueue);
    m_hWriteLockedQueue = NULL;
 
    IPUV3BaseCloseHandle(m_hIPUBase);
    m_hIPUBase = NULL;
    m_dwMode = BUFFER_MODE_DRIVER;   // driver mode
    m_iNumBuffers = 0;
    m_iNumBuffersEnqueued = 0;
    for(int i = 0; i < NUM_PIN_BUFFER_CLIENT; i++)
    {
        m_sIpubufferData[i].pPhysAddr = NULL;
        m_sIpubufferData[i].pVirtAddr = NULL;
    }
}


//-----------------------------------------------------------------------------
//
// Function:  AllocateBuffers
//
// This function allocates buffers and adds each to the idle buffer queue.
// This function must be called in order to allocate physically contiguous 
// buffers for use in the IPU's IDMAC.
//
// Parameters:
//      numBuffers
//          [in] Number of buffers to allocate and add
//          to the queues.
//      bufSize
//          [in] Size of buffer to create and enqueue.
//
// Returns:
//      Returns virtual pointer to buffer created.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL CamBufferManager::AllocateBuffers(ULONG numBuffers, ULONG bufSize)
{
    ipuBufferData bufData;
    UINT16 i;
    IPUBufferInfo IPUBufInfo;

    BUFMAN_FUNCTION_ENTRY();

    EnterCriticalSection(&m_csLockBufferQueues);

    if( numBuffers > NUM_PIN_BUFFER_DRIVER )
    {
        ERRORMSG(TRUE,(TEXT("%s: too many buffer need to allocate ! \r\n"), __WFUNCTION__)); 
        BUFMAN_FUNCTION_EXIT();
        return FALSE;
    }
    
    for (i = 0; i < numBuffers; i++)
    {        
        IPUBufInfo.MemType = MEM_TYPE_VIDEOMEMORY;
        IPUBufInfo.dwBufferSize = bufSize;
        
        //Allocate buffer 
        m_hCamBuffer[i] = new IpuBuffer(IPUBufInfo.dwBufferSize, IPUBufInfo.MemType);        
        IPUV3AllocateBuffer(m_hIPUBase, &IPUBufInfo, m_hCamBuffer[i]);

        //Physical address needn't CEOpenCallerBuffer().
        if(m_hCamBuffer[i] == NULL)
        {
            ERRORMSG(TRUE,
                    (TEXT("%s: Fail to get IpuBuffer ! \r\n"), __WFUNCTION__));    
            BUFMAN_FUNCTION_EXIT();
            return FALSE;             
        }
        
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s(0x%x): Buffer %d - Size: %x, Virtual address: %x, Physical address: %x.\r\n"),
            __WFUNCTION__, this, i, bufSize, m_hCamBuffer[i]->VirtAddr(), m_hCamBuffer[i]->PhysAddr()));
        
        bufData.pPhysAddr = m_hCamBuffer[i]->PhysAddr();
        bufData.pVirtAddr = m_hCamBuffer[i]->VirtAddr();
        // Enqueue newly created buffer
        if (!WriteMsgQueue(m_hWriteIdleQueue, &bufData, sizeof(ipuBufferData), 0, 0))
        {
            ERRORMSG(TRUE, 
                (TEXT("%s: Enqueue Buffer failed!\r\n"), __WFUNCTION__));
            LeaveCriticalSection(&m_csLockBufferQueues);
            BUFMAN_FUNCTION_EXIT();
            return FALSE;
        }
    }

    m_iNumBuffers = numBuffers;
    m_dwMode = BUFFER_MODE_DRIVER;

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  GetAllocBufferPhyAddr
//
// This function returns the phusical address of allocate buffer.
//
// Parameters:
//      pPhyAddr[]
//            [in] the pointer for allocated buffer
//
// Returns:
//      the allocated buffers' number.
//
//-----------------------------------------------------------------------------
DWORD CamBufferManager::GetAllocBufferPhyAddr(UINT32* pPhyAddr[])
{
    UINT32 i; 
        
    BUFMAN_FUNCTION_ENTRY();

    if(m_dwMode == BUFFER_MODE_DRIVER)
    {
        for (i = 0; i < (DWORD)m_iNumBuffers; i++)
        {
            pPhyAddr[i] = m_hCamBuffer[i]->PhysAddr();
            DEBUGMSG(ZONE_FUNCTION,
                (TEXT("%s: Buffer %d - Return address: %x, Physical address: %x.\r\n"),
                __WFUNCTION__, i, pPhyAddr[i], m_hCamBuffer[i]->PhysAddr()));
        }
    }
    else
    {
        for (i = 0; i < (DWORD)m_iNumBuffers; i++)
        {
            pPhyAddr[i] = m_sIpubufferData[i].pPhysAddr;
            DEBUGMSG(ZONE_FUNCTION,
                (TEXT("%s: Buffer %d - Return address: %x, Physical address: %x.\r\n"),
                __WFUNCTION__, i, pPhyAddr[i], m_sIpubufferData[i].pPhysAddr));
        }
    }

    BUFMAN_FUNCTION_EXIT();

    // Return physical buffer num
    return m_iNumBuffers;
}

//-----------------------------------------------------------------------------
//
// Function:  SetActiveBuffer
//
// This function adds a buffer from the idle buffer queue 
// to the message queue of busy buffers for preprocessing.  If
// the idle buffer queue is empty, we take a buffer from the filled
// queue and report a dropped frame.
//
// Parameters:
//      pPhysAddr
//          [out] Pointer to physical address to hold return value.
//      
//      timeout
//          [in] Time to wait for a busy buffer.
//
// Returns:
//      Returns 1 if a frame was dropped.
//      Returns 0 if no frame was dropped.
//      Returns -1 if an error occurred.
//
//-----------------------------------------------------------------------------
DWORD CamBufferManager::SetActiveBuffer(UINT32** ppPhysAddr, UINT32 timeout)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(timeout);

    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead, retVal;
    ipuBufferData bufData;

    *ppPhysAddr = NULL;
#if 0
    if (m_iNumBuffers < 4)
    {
        // If we are using less than 4 buffers, a buffer is only
        // considered free, and thus capable of being moved
        // to the busy queue, if it meets either of two criteria:
        //      1) It is in the Idle queue
        //      2) There is more than one buffer in the
        //         filled queue, indicating that one buffer
        //         can be removed without causing an error
        //         when the application attempts to read
        //         from the filled queue.
        //
        // If we have more than 4 buffer, one of these two criteria
        // will always be met.
        for (;;)
        {
            if(m_dwMode == BUFFER_MODE_CLIENT)
            {
                DEBUGMSG(ZONE_FUNCTION,(TEXT("%x,SetActiveBuffer m_iNumBuffersEnqueued = %d!\r\n"),this,m_iNumBuffersEnqueued));
                if( m_iNumBuffersEnqueued == 0 )
                {
                    retVal = (DWORD) -1;
                    BUFMAN_FUNCTION_EXIT();
                    return retVal;
                }
            }

            // Test criteria 1)
            DEBUGMSG(ZONE_FUNCTION,(TEXT("CamBufferManager::SetActiveBuffer WaitforSingleObject\r\n")));
            if (WaitForSingleObject(m_hReadIdleQueue, 0) == WAIT_OBJECT_0)
            {
                break;
            }

            // Test criteria 2)
            EnterCriticalSection(&m_csLockBufferQueues);

            filledQInfo.dwSize = sizeof(MSGQUEUEINFO);
            GetMsgQueueInfo(m_hReadFilledQueue, &filledQInfo);
            if (filledQInfo.dwCurrentMessages > 1)
            {
                LeaveCriticalSection(&m_csLockBufferQueues);
                break;
            }

            LeaveCriticalSection(&m_csLockBufferQueues);

            // We failed both criteria, so we give up the current
            // timeslice and check again when we become active
            Sleep(1);
        }
    }
#endif
    EnterCriticalSection(&m_csLockBufferQueues);

    // A buffer has been requested.  Attempt to read from the idle
    // queue to get an available buffer.  If one is not available,
    // we must take a buffer from the filled queue.
    if(ReadMsgQueue(m_hReadIdleQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: Buffer read from Idle queue\r\n"), __WFUNCTION__));
        *ppPhysAddr = bufData.pPhysAddr;

        // Now send buffer to Busy queue
        if (!WriteMsgQueue(m_hWriteBusyQueue, &bufData, sizeof(ipuBufferData), 0, 0))
        {
            DEBUGMSG(ZONE_ERROR, (_T("%s: Filled buffer queue is full!\r\n"), __WFUNCTION__));
            LeaveCriticalSection(&m_csLockBufferQueues);
            BUFMAN_FUNCTION_EXIT();
            return NULL;
        }
        RETAILMSG (0, (TEXT("%s(0x%x): Buffer read from Idle queue address = 0x%x\r\n"), __WFUNCTION__,this,*ppPhysAddr));
        retVal = 0;
    }
    else if (ReadMsgQueue(m_hReadFilledQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("%s: No buffers available in the idle queue.  Buffer read from Filled queue.\r\n"), __WFUNCTION__));
        *ppPhysAddr = bufData.pPhysAddr;

        // Now send buffer to Busy queue
        if (!WriteMsgQueue(m_hWriteBusyQueue, &bufData, sizeof(ipuBufferData), 0, 0))
        {
            DEBUGMSG(ZONE_ERROR, (_T("%s: Filled buffer queue is full!\r\n"), __WFUNCTION__));
            LeaveCriticalSection(&m_csLockBufferQueues);
            BUFMAN_FUNCTION_EXIT();
            return NULL;
        }
        DEBUGMSG(ZONE_FUNCTION, (TEXT("%s(0x%x): Buffer read from Filled queue address = 0x%x\r\n"), __WFUNCTION__,this,*ppPhysAddr));
        retVal = 1;
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s: No buffers available?!\r\n"), __WFUNCTION__));
        BUFMAN_FUNCTION_EXIT();
        retVal = (DWORD) -1;
    }

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function:  SetFilledBuffer
//
// This function adds a buffer from the busy buffer queue 
// to the message queue of filled buffers.
//
// Parameters:
//      None.
//
// Returns:
//      Returns number to buffer created.  
//      Returns -1 if unsuccessful.
//
//-----------------------------------------------------------------------------
int CamBufferManager::SetFilledBuffer()
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    ipuBufferData bufData;
    int bufNum = -1;

    EnterCriticalSection(&m_csLockBufferQueues);
    DEBUGMSG(ZONE_FUNCTION,(TEXT("CamBufferManager::SetFilledBuffer\r\n")));

    // Read from Busy queue, and add to the Filled queue
    if (!ReadMsgQueue(m_hReadBusyQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: No busy buffers available\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        BUFMAN_FUNCTION_EXIT();
        return bufNum;
    }
    DEBUGMSG(ZONE_FUNCTION, (_T("%s: Busy Buffer: Phys addr = %x, Virt addr = %x\r\n"), 
            __WFUNCTION__, bufData.pPhysAddr, bufData.pVirtAddr));

    if (!WriteMsgQueue(m_hWriteFilledQueue, &bufData, sizeof(ipuBufferData), 0, 0))
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: Filled buffer queue is full!  Aborting...\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        BUFMAN_FUNCTION_EXIT();
        return bufNum;
    }
    if(m_dwMode == BUFFER_MODE_DRIVER)
    {
        for (DWORD i = 0; i < m_iNumBuffers; i++)
        {
            if(bufData.pVirtAddr == m_hCamBuffer[i]->VirtAddr())
                bufNum = i;
        }
    }
    else
    {
        for (DWORD i = 0; i < m_iNumBuffers; i++)
        {
            if(bufData.pVirtAddr == m_sIpubufferData[i].pVirtAddr)
                bufNum = i;
        }
    }
    DEBUGMSG(ZONE_FUNCTION, (_T("%s(0x%x): Filled Buffer: Phys addr = %x, Virt addr = %x\r\n"), 
            __WFUNCTION__, this, bufData.pPhysAddr, bufData.pVirtAddr));

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return bufNum;
}


//-----------------------------------------------------------------------------
//
// Function:  DeleteBuffers
//
// This function deletes all of the buffers in the Idle, 
// Busy, and Filled queues.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL CamBufferManager::DeleteBuffers()
{
    BUFMAN_FUNCTION_ENTRY();

    UINT i = 0;
    ipuBufferData bufData;
    DWORD dwFlags, bytesRead;
    
    for (i =0; i< NUM_PIN_BUFFER_DRIVER; i++)
    {
        DEBUGMSG(ZONE_FUNCTION,(TEXT("%s: m_hCamBuffer[%d]= %x.\r\n"),
                            __WFUNCTION__,i, m_hCamBuffer[i]));
        
        if(NULL != m_hCamBuffer[i])
        {
            DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Virtual address: %x, Physical address: %x, i %x.\r\n"),
            __WFUNCTION__, m_hCamBuffer[i]->VirtAddr(), m_hCamBuffer[i]->PhysAddr(),i));
            IPUV3DeallocateBuffer(m_hIPUBase,m_hCamBuffer[i]);
            m_hCamBuffer[i] = NULL;
        }
    }

    EnterCriticalSection(&m_csLockBufferQueues);

    // Read and deallocate all buffers from Idle queue.
    while (ReadMsgQueue(m_hReadIdleQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("CamBufferManager::DeleteBuffers remove 0x%x(0x%x) from idle\r\n"),
                    bufData.pVirtAddr,bufData.pPhysAddr));
        // Deallocate all buffers
        if(bufData.pPhysAddr != NULL)
        {
            bufData.pPhysAddr = NULL;
            bufData.pVirtAddr = NULL;
        }
    }

    m_iNumBuffers = 0;
    
    LeaveCriticalSection(&m_csLockBufferQueues);
    
    BUFMAN_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  ResetBuffers
//
// This function moves all of the buffers in the Filled 
// and Busy queue to the Idle queue.  This resets the buffers
// to a ready-to-start state.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CamBufferManager::ResetBuffers()
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    ipuBufferData bufData;

    EnterCriticalSection(&m_csLockBufferQueues);

    // Read all buffers from Filled queue and move 
    // them to the Busy queue.

    // Move filled queue buffers
    while (ReadMsgQueue(m_hReadFilledQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        WriteMsgQueue(m_hWriteIdleQueue, &bufData, sizeof(ipuBufferData), 0, 0);
    }

    // Move busy queue buffers
    while (ReadMsgQueue(m_hReadBusyQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        WriteMsgQueue(m_hWriteIdleQueue, &bufData, sizeof(ipuBufferData), 0, 0);
    }
    
    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function:  ResetBuffersToLocked
//
// This function moves all of the buffers in the Filled 
// and Busy queue to the Idle queue.  This resets the buffers
// to a ready-to-start state.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CamBufferManager::ResetBuffersToLocked()
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    ipuBufferData bufData;

    EnterCriticalSection(&m_csLockBufferQueues);

    // Read all buffers from Filled queue and move 
    // them to the Busy queue.

    // Move filled queue buffers
    while (ReadMsgQueue(m_hReadFilledQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        WriteMsgQueue(m_hWriteLockedQueue, &bufData, sizeof(ipuBufferData), 0, 0);
    }

    // Move busy queue buffers
    while (ReadMsgQueue(m_hReadBusyQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        WriteMsgQueue(m_hWriteLockedQueue, &bufData, sizeof(ipuBufferData), 0, 0);
    }

    // Move Idel queue buffers
    while (ReadMsgQueue(m_hReadIdleQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        WriteMsgQueue(m_hWriteLockedQueue, &bufData, sizeof(ipuBufferData), 0, 0);
    }
    m_iNumBuffersEnqueued = 0;
    
    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function:  GetBufFilled
//
// This function reads the queue of filled
// buffers and returns the buffer at the top of the queue.
//
// Parameters:
//      None.
//
// Returns:
//      Pointer to filled buffer, or NULL if failure.
//
//-----------------------------------------------------------------------------
UINT32* CamBufferManager::GetBufferFilled()
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    ipuBufferData bufData;

    EnterCriticalSection(&m_csLockBufferQueues);

    // Read from Filled queue, and add to the Idle queue
    if (!ReadMsgQueue(m_hReadFilledQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: No filled buffers available!?\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        BUFMAN_FUNCTION_EXIT();
        return NULL;
    }

    if ( m_dwMode == BUFFER_MODE_DRIVER)
    {
        if (!WriteMsgQueue(m_hWriteIdleQueue, &bufData, sizeof(ipuBufferData), 0, 0))
        {
            DEBUGMSG(ZONE_ERROR, (_T("%s: Idle buffer queue is full!  Aborting...\r\n"), __WFUNCTION__));
            LeaveCriticalSection(&m_csLockBufferQueues);
            BUFMAN_FUNCTION_EXIT();
            return NULL;
        }
    }
    else
    {
        if (!WriteMsgQueue(m_hWriteLockedQueue, &bufData, sizeof(ipuBufferData), 0, 0))
        {
            DEBUGMSG(ZONE_ERROR, (_T("%s: Locked buffer queue is full!  Aborting...\r\n"), __WFUNCTION__));
            LeaveCriticalSection(&m_csLockBufferQueues);
            BUFMAN_FUNCTION_EXIT();
            return NULL;
        }
        m_iNumBuffersEnqueued--;
    }

    LeaveCriticalSection(&m_csLockBufferQueues);
    DEBUGMSG(ZONE_FUNCTION,(TEXT("CamBufferManager::GetBufferFilled ----- 0x%x \r\n"),bufData.pVirtAddr));
    BUFMAN_FUNCTION_EXIT();

   
    return bufData.pVirtAddr;
}

//-----------------------------------------------------------------------------
//
// Function:  EnQueue
//
// This function reads the queue of Locked
// buffers and returns the buffer at the top of the queue to Idle.
//
// Parameters:
//      None.
//
// Returns:
//      Pointer to filled buffer, or NULL if failure.
//
//-----------------------------------------------------------------------------
BOOL CamBufferManager::EnQueue()
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    ipuBufferData bufData;

    if(m_dwMode != BUFFER_MODE_CLIENT)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT(" m_dwMode is not client\r\n")));
        return FALSE;
    }
    EnterCriticalSection(&m_csLockBufferQueues);

    // Read from Filled queue, and add to the Idle queue
    if (!ReadMsgQueue(m_hReadLockedQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: No Locked buffers available!?\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        BUFMAN_FUNCTION_EXIT();
        return FALSE;
    }

    if (!WriteMsgQueue(m_hWriteIdleQueue, &bufData, sizeof(ipuBufferData), 0, 0))
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: Idle buffer queue is full!  Aborting...\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        BUFMAN_FUNCTION_EXIT();
        return FALSE;
    }
    
    m_iNumBuffersEnqueued++;
    DEBUGMSG(ZONE_FUNCTION, (_T("%s(0x%x): Enqueue buffer(%d) address = 0x%x\r\n"), __WFUNCTION__,this, m_iNumBuffersEnqueued,bufData.pVirtAddr));
    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  GetEnqueuedBufferNum
//
// This function get the number of enqueued buffer.
//
// Parameters:
//      None.
//
// Returns:
//      Number of buffer enqueued
//
//-----------------------------------------------------------------------------
UINT32 CamBufferManager::GetEnqueuedBufferNum()
{
    return m_iNumBuffersEnqueued;
}

//-----------------------------------------------------------------------------
//
// Function:  RegisterBuffer
//
// This function register buffer allocated by other module; including its virtual
// memory address and physically contiguous memory address for use in the IPU's IDMAC.
//
// Parameters:
//      pVirtualAddress
//          [in] the virtual address of buffer want to be registered.
//      pPhysicalAddress
//          [in] the physical address of buffer want to be registered.
//
// Returns:
//      Returns TRUE if successfule.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL CamBufferManager::RegisterBuffer(LPVOID pVirtualAddress, UINT32 pPhysicalAddress)
{
    ipuBufferData bufData;

    BUFMAN_FUNCTION_ENTRY();

    if( m_iNumBuffers >= NUM_PIN_BUFFER_CLIENT )
    {
        ERRORMSG(TRUE,(TEXT("%s: too many buffer need to register ! \r\n"), __WFUNCTION__));   
        BUFMAN_FUNCTION_EXIT();
        return FALSE;
    }

    EnterCriticalSection(&m_csLockBufferQueues);

    if(pVirtualAddress == NULL || pPhysicalAddress == NULL)    
    {
        ERRORMSG(TRUE, (TEXT("%s : Buffer register failed!\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        BUFMAN_FUNCTION_EXIT();
        return FALSE;
    }

    m_dwMode = BUFFER_MODE_CLIENT;

    bufData.pPhysAddr = (UINT32 *) pPhysicalAddress;
    bufData.pVirtAddr = (UINT32 *) pVirtualAddress;
    
    if (!WriteMsgQueue(m_hWriteLockedQueue, &bufData, sizeof(ipuBufferData), 0, 0))
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s : WriteMsgQueue failed!\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        BUFMAN_FUNCTION_EXIT();
        return FALSE;
    }
    
    m_sIpubufferData[m_iNumBuffers].pPhysAddr = bufData.pPhysAddr;
    m_sIpubufferData[m_iNumBuffers].pVirtAddr = bufData.pVirtAddr;
    m_iNumBuffers++;

    DEBUGMSG(ZONE_FUNCTION,
        (TEXT("%s: Register Buffer %d : Virtual address: %x, Physical address: %x.\r\n"),
        __WFUNCTION__, m_iNumBuffers, bufData.pVirtAddr, bufData.pPhysAddr));

    LeaveCriticalSection(&m_csLockBufferQueues);
    
    BUFMAN_FUNCTION_EXIT();

    // Return virtual buffer address
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  UnregisterBuffer
//
// This function Unregister buffers
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL CamBufferManager::UnregisterBuffer(LPVOID pVirtualAddress, UINT32 pPhysicalAddress)
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    ipuBufferData bufData;

    EnterCriticalSection(&m_csLockBufferQueues);

    //if user unregister buffer, we will reset all other queue buffer to Locked
    ResetBuffersToLocked();

    // because when unregisterbuffer called, all the buffer should be Locked queue.
    // Read and deallocate all buffers from Locked queue.
    while (ReadMsgQueue(m_hReadLockedQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        if( (bufData.pPhysAddr == (UINT32*)pPhysicalAddress) &&
            (bufData.pVirtAddr == pVirtualAddress))
        {
            m_iNumBuffers--;
            for(int i = 0; i < NUM_PIN_BUFFER_CLIENT; i++)
            {
                if( (m_sIpubufferData[i].pPhysAddr == (UINT32*)pPhysicalAddress) &&
                    (m_sIpubufferData[i].pVirtAddr == pVirtualAddress))
                {
                    m_sIpubufferData[i].pPhysAddr = NULL;
                    m_sIpubufferData[i].pVirtAddr = NULL;
                    break;
                }
            }
            LeaveCriticalSection(&m_csLockBufferQueues);
            DEBUGMSG(ZONE_FUNCTION,
                    (TEXT("%s: Deregister Buffer :  Virtual address: %x, Physical address: %x.\r\n"),
                    __WFUNCTION__, bufData.pVirtAddr, bufData.pPhysAddr));
            BUFMAN_FUNCTION_EXIT();
            return TRUE;
        }
        else
        {
            if (!WriteMsgQueue(m_hWriteLockedQueue, &bufData, sizeof(ipuBufferData), 0, 0))
            {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s : WriteMsgQueue failed!\r\n"), __WFUNCTION__));
                LeaveCriticalSection(&m_csLockBufferQueues);
                BUFMAN_FUNCTION_EXIT();
                return FALSE;
            }
        }    
    }

    LeaveCriticalSection(&m_csLockBufferQueues);
    
    BUFMAN_FUNCTION_EXIT();

    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function:  UnregisterAllBuffers
//
// This function deregister all of the buffers in the Idle, 
// Busy, and Filled queues.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL CamBufferManager::UnregisterAllBuffers()
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    ipuBufferData bufData;
    
    EnterCriticalSection(&m_csLockBufferQueues);

    // Read and deallocate all buffers from Idle queue.
    while (ReadMsgQueue(m_hReadIdleQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Deregister Buffer : %x, Virtual address: %x, Physical address: %x.\r\n"),
            __WFUNCTION__, bufData.pVirtAddr, bufData.pPhysAddr));
        m_iNumBuffers--;
    }  

    // Read and deallocate all buffers from Busy queue.
    while (ReadMsgQueue(m_hReadBusyQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Deregister Buffer : %x, Virtual address: %x, Physical address: %x.\r\n"),
            __WFUNCTION__, bufData.pVirtAddr, bufData.pPhysAddr));
        m_iNumBuffers--;
    } 
    
    // Read and deallocate all buffers from Filled queue.
    while (ReadMsgQueue(m_hReadFilledQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Deregister Buffer : %x, Virtual address: %x, Physical address: %x.\r\n"),
            __WFUNCTION__, bufData.pVirtAddr, bufData.pPhysAddr));
        m_iNumBuffers--;
    } 
    
    // Read and deallocate all buffers from Locked queue.
    while (ReadMsgQueue(m_hReadLockedQueue, &bufData, sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Deregister Buffer : %x, Virtual address: %x, Physical address: %x.\r\n"),
            __WFUNCTION__, bufData.pVirtAddr, bufData.pPhysAddr));
        m_iNumBuffers--;
    } 
    
    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function:  GetMaxBuffers
//
// This function returns the maximum number of buffers supported
// by the preprocessor.
// with the IPU hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns the Max buffer number.
//
//-----------------------------------------------------------------------------
UINT32 CamBufferManager::GetMaxBuffers(void)
{
    return IPU_MAX_NUM_BUFFERS;
}

//-----------------------------------------------------------------------------
//
// Function:  GetCurrentBufferNum
//
// This function returns the number of current buffers in buffermanager
//
// Parameters:
//      None.
//
// Returns:
//      Returns the buffer number.
//
//-----------------------------------------------------------------------------
UINT32 CamBufferManager::GetCurrentBufferNum(void)
{
    return m_iNumBuffers;
}

//-----------------------------------------------------------------------------
//
// Function:  PrintBufferInfo
//
// This function iterates through the buffers, returning returns the maximum number of buffers supported
// by the preprocessor.
// with the IPU hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns the Max buffer number.
//
//-----------------------------------------------------------------------------
void CamBufferManager::PrintBufferInfo(void)
{
    DWORD dwFlags, bytesRead, i, j;
    ipuBufferData bufData[100];

    EnterCriticalSection(&m_csLockBufferQueues);

    i = 0;
    // Read from queue and print buffer data
    while (ReadMsgQueue(m_hReadIdleQueue, &bufData[i], sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_DEVICE, (_T("%s: Idle Buffer %d: Phys addr = %x, Virt addr = %x\r\n"), 
            __WFUNCTION__, i, bufData[i].pPhysAddr, bufData[i].pVirtAddr));
        i++;
    }

    // Write buffers back to queue
    for (j = 0; j < i; j++)
    {
        WriteMsgQueue(m_hWriteIdleQueue, &bufData[j], sizeof(ipuBufferData), 0, 0);
    }

    i = 0;
    // Read from queue and print buffer data
    while (ReadMsgQueue(m_hReadBusyQueue, &bufData[i], sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_DEVICE, (_T("%s: Busy Buffer %d: Phys addr = %x, Virt addr = %x\r\n"), 
            __WFUNCTION__, i, bufData[i].pPhysAddr, bufData[i].pVirtAddr));
        i++;
    }

    // Write buffers back to queue
    for (j = 0; j < i; j++)
    {
        WriteMsgQueue(m_hWriteBusyQueue, &bufData[j], sizeof(ipuBufferData), 0, 0);
    }

    i = 0;
    // Read from queue and print buffer data
    while (ReadMsgQueue(m_hReadFilledQueue, &bufData[i], sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_DEVICE, (_T("%s: Filled Buffer %d: Phys addr = %x, Virt addr = %x\r\n"), 
            __WFUNCTION__, i, bufData[i].pPhysAddr, bufData[i].pVirtAddr));
        i++;
    }

    // Write buffers back to queue
    for (j = 0; j < i; j++)
    {
        WriteMsgQueue(m_hWriteFilledQueue, &bufData[j], sizeof(ipuBufferData), 0, 0);
    }

    i = 0;
    // Read from queue and print buffer data
    while (ReadMsgQueue(m_hReadLockedQueue, &bufData[i], sizeof(ipuBufferData), &bytesRead, 0, &dwFlags))
    {
        DEBUGMSG(ZONE_DEVICE, (_T("%s: Locked Buffer %d: Phys addr = %x, Virt addr = %x\r\n"), 
            __WFUNCTION__, i, bufData[i].pPhysAddr, bufData[i].pVirtAddr));
        i++;
    }

    // Write buffers back to queue
    for (j = 0; j < i; j++)
    {
        WriteMsgQueue(m_hWriteLockedQueue, &bufData[j], sizeof(ipuBufferData), 0, 0);
    }

    LeaveCriticalSection(&m_csLockBufferQueues);
}


