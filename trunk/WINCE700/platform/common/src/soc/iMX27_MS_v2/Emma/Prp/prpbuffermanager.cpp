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
// Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//-----------------------------------------------------------------------------
//
// File: PrpBufferManager.cpp
//
// Implementation of preprocessor buffer manager.
//
//-----------------------------------------------------------------------------
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#include "csp.h"
#include "emmadbg.h"
#include "PrpBufferManager.h"

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
// Function: PrpBufferManager
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
PrpBufferManager::PrpBufferManager(void)
{
    MSGQUEUEOPTIONS queueOptions;

    BUFMAN_FUNCTION_ENTRY();

    InitializeCriticalSection(&m_csLockBufferQueues);

    // Create queues for read messages and write messages
    queueOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    queueOptions.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    queueOptions.dwMaxMessages = PRP_MAX_NUM_BUFFERS;
    queueOptions.cbMaxMessage = sizeof(prpBufferData);
    queueOptions.bReadAccess = TRUE; // we need read-access to msgqueue

    // We have 3 queues for each channel:  One for idle buffers,
    // one for busy buffers, and one for filled buffers.
    // For each of these buffers, a read and write handle is needed.
    // 6 handles total

    // Create read handles to idle and busy queues
    m_hReadIdleQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadIdleQueue) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error creating Read idle queue handle! \r\n"), 
            __WFUNCTION__));
    }

    m_hReadBusyQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadBusyQueue) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error creating Read busy queue handle! \r\n"), 
            __WFUNCTION__));
    }

    m_hReadFilledQueue = CreateMsgQueue(NULL, &queueOptions);
    if (!m_hReadFilledQueue) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error creating Read filled queue handle! \r\n"), 
            __WFUNCTION__));
    }

    queueOptions.bReadAccess = FALSE; // we need write-access to msgqueue

    // Create read handles to idle and busy queues
    m_hWriteIdleQueue = OpenMsgQueue(GetCurrentProcess(), 
        m_hReadIdleQueue, &queueOptions);
    if (!m_hWriteIdleQueue) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error opening idle queue for writing! \r\n"), 
            __WFUNCTION__));
    }

    m_hWriteBusyQueue = OpenMsgQueue(GetCurrentProcess(), 
        m_hReadBusyQueue, &queueOptions);
    if (!m_hWriteBusyQueue) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error opening busy queue for writing! \r\n"), 
            __WFUNCTION__));
    }

    m_hWriteFilledQueue = OpenMsgQueue(GetCurrentProcess(), 
        m_hReadFilledQueue, &queueOptions);
    if (!m_hWriteFilledQueue) {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Error opening filled queue for writing! \r\n"), 
            __WFUNCTION__));
    }

    BUFMAN_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: ~PrpBufferManager
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
PrpBufferManager::~PrpBufferManager(void)
{
    DeleteBuffers();

    CloseHandle(m_hReadIdleQueue);
    CloseHandle(m_hWriteIdleQueue);
    CloseHandle(m_hReadBusyQueue);
    CloseHandle(m_hWriteBusyQueue);
    CloseHandle(m_hReadFilledQueue);
    CloseHandle(m_hWriteFilledQueue);
}

//-----------------------------------------------------------------------------
//
// Function: AllocateBuffers
//
// This function allocates buffers and adds each to the idle buffer queue.
// This function must be called in order to allocate physically contiguous 
// buffers for use in the pre-processor.
//
// Parameters:
//      numBuffers
//          [in] Number of buffers to allocate and add to the queues.
//
//      bufSize
//          [in] Size of buffer to create and enqueue.
//
// Returns:
//      Returns virtual pointer to buffer created.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL PrpBufferManager::AllocateBuffers(ULONG numBuffers, ULONG bufSize)
{
    LPVOID tempVirtualAddress;
    ULONG tempPhysicalAddress;
    prpBufferData BufData;
    UINT16 i;

    BUFMAN_FUNCTION_ENTRY();

    EnterCriticalSection(&m_csLockBufferQueues);

    for (i=0; i<numBuffers; i++) {
        // Allocate buffer based on the size passed in.
        tempVirtualAddress = AllocPhysMem(bufSize, PAGE_EXECUTE_READWRITE,
            0, 0, &tempPhysicalAddress);
        if (tempVirtualAddress == NULL) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s : Buffer allocation failed!\r\n"), __WFUNCTION__));
            LeaveCriticalSection(&m_csLockBufferQueues);
            return FALSE;
        }

        BufData.pPhysAddr = (LPVOID)tempPhysicalAddress;
        BufData.pVirtAddr = tempVirtualAddress;

        DEBUGMSG(ZONE_FUNCTION,
            (TEXT("%s: Buffer %d - Size: %x, Virtual address: %x, Physical address: %x.\r\n"),
            __WFUNCTION__, i, bufSize, BufData.pVirtAddr, BufData.pPhysAddr));

        // Enqueue newly created buffer
        if (!WriteMsgQueue(m_hWriteIdleQueue, &BufData, 
            sizeof(prpBufferData), 0, 0)) {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s : Buffer allocation failed!\r\n"), __WFUNCTION__));
            LeaveCriticalSection(&m_csLockBufferQueues);
            return FALSE;
        }
    }

    m_iNumBuffers = numBuffers;

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    // Return virtual buffer address
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  SetActiveBuffer
//
// This function adds a buffer from the idle buffer queue to the message queue
// of busy buffers for preprocessing.  If the idle buffer queue is empty, we 
// take a buffer from the filled queue and report a dropped frame.
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
DWORD PrpBufferManager::SetActiveBuffer(LPVOID *ppPhysAddr, UINT32 timeout)
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead, retVal;
    prpBufferData bufData;
    MSGQUEUEINFO filledQInfo;

    if (m_iNumBuffers == PRP_MIN_NUM_BUFFERS)
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
        while(1) {
            // Test criteria 1)
            if (WaitForSingleObject(m_hReadIdleQueue, 0) == WAIT_OBJECT_0) {
                break;
            }

            // Test criteria 2)
            EnterCriticalSection(&m_csLockBufferQueues);

            GetMsgQueueInfo(m_hReadFilledQueue, &filledQInfo);
            if (filledQInfo.dwCurrentMessages > 1) {
                LeaveCriticalSection(&m_csLockBufferQueues);
                break;
            }

            LeaveCriticalSection(&m_csLockBufferQueues);

            // We failed both criteria, so we give up the current
            // timeslice and check again when we become active
            Sleep(1); 
        }
    }

    EnterCriticalSection(&m_csLockBufferQueues);

    // A buffer has been requested.  Attempt to read from the idle
    // queue to get an available buffer.  If one is not available,
    // we must take a buffer from the filled queue.
    if(ReadMsgQueue(m_hReadIdleQueue, &bufData, sizeof(prpBufferData), 
        &bytesRead, 0, &dwFlags)) {
        DEBUGMSG (ZONE_DEVICE, 
            (TEXT("%s: Buffer read from Idle queue\r\n"), 
            __WFUNCTION__));
        
        *ppPhysAddr = bufData.pPhysAddr;

        // Now send buffer to Busy queue
        if (!WriteMsgQueue(m_hWriteBusyQueue, &bufData, 
            sizeof(prpBufferData), 0, 0)) {
            DEBUGMSG(ZONE_ERROR, 
                (_T("%s: Filled buffer queue is full!\r\n"), __WFUNCTION__));
            LeaveCriticalSection(&m_csLockBufferQueues);
            return NULL;
        }

        retVal = 0;
    } else if (ReadMsgQueue(m_hReadFilledQueue, &bufData, 
        sizeof(prpBufferData), &bytesRead, 0, &dwFlags)) {
        DEBUGMSG (ZONE_DEVICE, 
            (TEXT("%s: No buffers available in the idle queue.  Buffer read from Filled queue.\r\n"), 
            __WFUNCTION__));
        
        *ppPhysAddr = bufData.pPhysAddr;

        // Now send buffer to Busy queue
        if (!WriteMsgQueue(m_hWriteBusyQueue, &bufData, 
            sizeof(prpBufferData), 0, 0)) {
            DEBUGMSG(ZONE_ERROR, 
                (_T("%s: Filled buffer queue is full!\r\n"), __WFUNCTION__));
            LeaveCriticalSection(&m_csLockBufferQueues);
            return NULL;
        }

        retVal = 1;
    } else {
        DEBUGMSG (ZONE_DEVICE, 
            (TEXT("%s: No buffers available?!\r\n"), __WFUNCTION__));
        retVal = -1;
    }

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return retVal;
}

//-----------------------------------------------------------------------------
//
// Function: SetFilledBuffer
//
// This function adds a buffer from the busy buffer queue to the message 
// queue of filled buffers.
//
// Parameters:
//      None.
//
// Returns:
//      Returns virtual pointer to buffer created.  
//      Returns NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
BOOL PrpBufferManager::SetFilledBuffer()
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    prpBufferData bufData;

    EnterCriticalSection(&m_csLockBufferQueues);

    // Read from Busy queue, and add to the Filled queue
    if (!ReadMsgQueue(m_hReadBusyQueue, &bufData, sizeof(prpBufferData), 
        &bytesRead, 0, &dwFlags)) {
        DEBUGMSG(ZONE_ERROR, 
            (_T("%s: No busy buffers available\r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        return FALSE;
    }

    if (!WriteMsgQueue(m_hWriteFilledQueue, &bufData, 
        sizeof(prpBufferData), 0, 0)) {
        DEBUGMSG(ZONE_ERROR, 
            (_T("%s: Filled buffer queue is full!  Aborting...\r\n"), 
            __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        return FALSE;
    }

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DeleteBuffers
//
// This function deletes all of the buffers in the 
// Idle, Busy, and Filled queues.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, otherwise FALSE.
//
//-----------------------------------------------------------------------------
BOOL PrpBufferManager::DeleteBuffers()
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    prpBufferData bufData;

    EnterCriticalSection(&m_csLockBufferQueues);

    // Read and deallocate all buffers from Idle queue.
    while (ReadMsgQueue(m_hReadIdleQueue, &bufData, sizeof(prpBufferData), 
        &bytesRead, 0, &dwFlags)) {
        // Deallocate all buffers
        if (bufData.pPhysAddr != NULL) {
            if (!FreePhysMem(bufData.pVirtAddr)) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Buffer Deallocation failed in idle queue! \r\n"), 
                    __WFUNCTION__));
                LeaveCriticalSection(&m_csLockBufferQueues);
                return FALSE;
            }
            bufData.pPhysAddr = NULL;
            bufData.pVirtAddr = NULL;
        }
    }

    // Read and deallocate all buffers from Busy queue.
    while (ReadMsgQueue(m_hReadBusyQueue, &bufData, sizeof(prpBufferData), 
        &bytesRead, 0, &dwFlags)) {
        // Deallocate all buffers in busy queue
        if (bufData.pPhysAddr != NULL) {
            if (!FreePhysMem(bufData.pVirtAddr)) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Buffer Deallocation failed in busy queue! \r\n"), 
                    __WFUNCTION__));
                LeaveCriticalSection(&m_csLockBufferQueues);
                return FALSE;
            }
            bufData.pPhysAddr = NULL;
            bufData.pVirtAddr = NULL;
        }
    }

    // Read and deallocate all buffers from Filled queue.
    while (ReadMsgQueue(m_hReadFilledQueue, &bufData, sizeof(prpBufferData), 
        &bytesRead, 0, &dwFlags)) {
        // Deallocate all buffers in filled queue
        if (bufData.pPhysAddr != NULL) {
            if (!FreePhysMem(bufData.pVirtAddr)) {
                DEBUGMSG(ZONE_ERROR, 
                    (TEXT("%s: Buffer Deallocation failed in filled queue! \r\n"), 
                    __WFUNCTION__));
                LeaveCriticalSection(&m_csLockBufferQueues);
                return FALSE;
            }
            bufData.pPhysAddr = NULL;
            bufData.pVirtAddr = NULL;
        }
    }

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: ResetBuffers
//
// This function moves all of the buffers in the Filled and Busy queue to 
// the Idle queue.  This resets the buffers to a ready-to-start state.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID PrpBufferManager::ResetBuffers()
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;
    prpBufferData bufData;

    EnterCriticalSection(&m_csLockBufferQueues);

    // Read all buffers from Filled queue and move 
    // them to the Busy queue.

    // Move filled queue buffers
    while (ReadMsgQueue(m_hReadFilledQueue, &bufData, sizeof(prpBufferData), 
        &bytesRead, 0, &dwFlags)) {
        WriteMsgQueue(m_hWriteIdleQueue, &bufData, sizeof(prpBufferData), 0, 0);
    }

    // Move busy queue buffers
    while (ReadMsgQueue(m_hReadBusyQueue, &bufData, sizeof(prpBufferData), 
        &bytesRead, 0, &dwFlags)) {
        WriteMsgQueue(m_hWriteIdleQueue, &bufData, sizeof(prpBufferData), 0, 0);
    }

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function:  GetBufFilled
//
// This function reads the queue of filled buffers and returns the buffer 
// at the top of the queue.
//
// Parameters:
//       pBufData
//           [out]  The virtual/physics address of the filled channel buffer.
//
// Returns:
//      TRUE for success, FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL PrpBufferManager::GetBufferFilled(pPrpBufferData pBufData)
{
    BUFMAN_FUNCTION_ENTRY();

    DWORD dwFlags, bytesRead;

    if (NULL == pBufData)
    {
       return FALSE;
    }

    EnterCriticalSection(&m_csLockBufferQueues);

    // Read from Filled queue, and add to the Idle queue
    if (!ReadMsgQueue(m_hReadFilledQueue, pBufData, sizeof(prpBufferData), 
        &bytesRead, 0, &dwFlags)) {
        DEBUGMSG(ZONE_ERROR, 
            (_T("%s: No filled buffers available!? \r\n"), __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        return FALSE;
    }

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  PutBufferIdle
//
// This function put the queue to idle buffers  queue.
//
// Parameters:
//       pBufData
//           [in]  The virtual/physics address of the filled channel buffer.
//
// Returns:
//      TRUE for success ,FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL PrpBufferManager::PutBufferIdle(pPrpBufferData pBufData)
{
    BUFMAN_FUNCTION_ENTRY();

    if (NULL == pBufData)
    {
       return FALSE;
    }

    EnterCriticalSection(&m_csLockBufferQueues);

    if (!WriteMsgQueue(m_hWriteIdleQueue, pBufData, 
        sizeof(prpBufferData), 0, 0)) {
        DEBUGMSG(ZONE_ERROR, 
            (_T("%s: Idle buffer queue is full! Aborting... \r\n"), 
            __WFUNCTION__));
        LeaveCriticalSection(&m_csLockBufferQueues);
        return FALSE;
    }

    LeaveCriticalSection(&m_csLockBufferQueues);

    BUFMAN_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PrintBufferInfo
//
// This function iterates through the buffers, returning returns the maximum 
// number of buffers supported by the preprocessor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID PrpBufferManager::PrintBufferInfo(void)
{
    DWORD dwFlags, bytesRead, i, j;
    prpBufferData bufData[100];

    EnterCriticalSection(&m_csLockBufferQueues);

    i = 0;
    // Read from idle queue and print buffer data
    while (ReadMsgQueue(m_hReadIdleQueue, &bufData[i], sizeof(prpBufferData),
        &bytesRead, 0, &dwFlags)) {
        DEBUGMSG(ZONE_DEVICE, 
            (_T("%s: Idle Buffer %d: Phys addr = %x, Virt addr = %x\r\n"), 
            __WFUNCTION__, i, bufData[i].pPhysAddr, bufData[i].pVirtAddr));
        i++;
    }

    // Write buffers back to idle queue
    for (j = 0; j < i; j++) {
        WriteMsgQueue(m_hWriteIdleQueue, &bufData[j], 
            sizeof(prpBufferData), 0, 0);
    }

    i = 0;
    // Read from busy queue and print buffer data
    while (ReadMsgQueue(m_hReadBusyQueue, &bufData[i], sizeof(prpBufferData),
        &bytesRead, 0, &dwFlags)) {
        DEBUGMSG(ZONE_DEVICE, 
            (_T("%s: Busy Buffer %d: Phys addr = %x, Virt addr = %x\r\n"), 
            __WFUNCTION__, i, bufData[i].pPhysAddr, bufData[i].pVirtAddr));
        i++;
    }

    // Write buffers back to busy queue
    for (j = 0; j < i; j++) {
        WriteMsgQueue(m_hWriteBusyQueue, &bufData[j], 
            sizeof(prpBufferData), 0, 0);
    }

    i = 0;
    // Read from filled queue and print buffer data
    while (ReadMsgQueue(m_hReadFilledQueue, &bufData[i], 
        sizeof(prpBufferData), &bytesRead, 0, &dwFlags)) {
        DEBUGMSG(ZONE_DEVICE, 
            (_T("%s: Filled Buffer %d: Phys addr = %x, Virt addr = %x\r\n"), 
            __WFUNCTION__, i, bufData[i].pPhysAddr, bufData[i].pVirtAddr));
        i++;
    }

    // Write buffers back to filled queue
    for (j = 0; j < i; j++) {
        WriteMsgQueue(m_hWriteFilledQueue, &bufData[j], 
            sizeof(prpBufferData), 0, 0);
    }

    LeaveCriticalSection(&m_csLockBufferQueues);
}

