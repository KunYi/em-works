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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

    dma_tran.hpp

Abstract:  

    Contains DMA support routines.
    
Functions:

Notes:

--*/
#pragma once

#include <windows.h>
#include <types.h>
#include <Csync.h>
#include <oaldma.h>
#include <dmamif.h>

class DmaChannel;
class DmaTransfer;

#define DMA_TRANSFER_TAG 0xdafa55aa

class DmaTransfer :public LIST_ENTRY, public DMA_MDD_TRANFER_CONTEXT {
    friend class DmaChannel;
public:
    // Constructor.
    DmaTransfer(DmaChannel& dmaChannel,
          IN DWORD  dwFlags,
          IN PHYSICAL_ADDRESS  SystemMemoryPhysicalAddress,
          IN PVOID CurrentVa,
          IN ULONG Length,
          IN HANDLE NotifyHandle,
          IN PVOID NotifyContext1,IN PVOID NotifyContext2,
          IN PHYSICAL_ADDRESS  phDeviceIoAddress,
          IN DMA_TRANSFER_HANDLE hPrevAttachedTrans = NULL );
    DmaTransfer(DmaChannel& dmaChannel,
          IN PVOID lpInPtr,
          IN DWORD nInLen,
          IN HANDLE NotifyHandle,
          IN PVOID NotifyContext1, IN PVOID NotifyContext2,
          IN DMA_TRANSFER_HANDLE hPrevAttachedTrans = NULL );
    virtual ~DmaTransfer();
    void * operator new(size_t stSize, DmaChannel& dmaChannel);
    void operator delete (void *pointer);
    // Reference function
private:
    LONG   m_lRefCount;
public:
    DWORD AddRef( void )
    {
        return (DWORD)InterlockedIncrement(&m_lRefCount);
    };
    DWORD DeRef( void );
    // Intialize
    virtual BOOL Init();
    // Function
    DWORD   GetTag() const { return m_dwTag; };
    DmaChannel& GetDmaChannel() { return m_dmaChannel; };
    LIST_ENTRY * ConvertToLink() { return (PLIST_ENTRY)this; };
    BOOL AbortTransfer();
    BOOL GetStatus(  OUT PDWORD lpCompletedLength, OUT PDWORD lpCompletionCode);
    BOOL GetContexts(  OUT PVOID * pNotifyContext1, OUT PVOID * pNotifyContext2);
    DMA_TRANSFER_HANDLE GetPrevAttachedTransfer() { return m_hPrevAttachedTransfer; };
    BOOL    GetCompletionCode () { return m_dwCompletionCode; } ;
    BOOL IoControl( IN DWORD dwIoControl, IN PVOID lpInPtr, IN DWORD nInLen, IN OUT LPVOID lpOutBuffer, IN DWORD nOutBufferSize, IN LPDWORD lpBytesReturned);
    PDMA_PDD_TRANFER_CONTEXT GetPDDContext() { return m_pDmaPDDTransferContext; }
    PDMA_MDD_TRANFER_CONTEXT GetMDDContext() { return (PDMA_MDD_TRANFER_CONTEXT)this;};
protected:
    virtual void CompleteNotification(DMA_STATUS_CODE status, DWORD dwRemaining);
    void SetTransferProcessing();
    void UpdateRemaining(DWORD dwRemaining);
// Properties
protected:
    BOOL IsSupportRawTransfer() { return m_pDmaPDDTransferContext!=NULL; };
public:
    DWORD   GetTransferFlags() { return m_dwFlags; };
protected:
    DWORD   m_dwCompletionCode;
    DWORD   m_dwCompleteLength;
    HANDLE  m_hNotifyHandle;
    PVOID   m_NotifyContext1, m_NotifyContext2;
    void NotifyClientDriver() { // Make sure only call once.
        HANDLE hNotifyHandle = 
            (HANDLE)InterlockedExchange( (PLONG)&m_hNotifyHandle,NULL);
        if (hNotifyHandle != NULL) {
            SetEvent(hNotifyHandle);
        }
            
    }
    // Buffer.
/*  DMA_MDD_TRANFER_CONTEXT
    DWORD   m_dwFlags;
    PVOID   m_pUserBufferPtr;
    DWORD   m_dwUserBufferLength;
    PHYSICAL_ADDRESS    m_UserBufferPhAddr;
    PHYSICAL_ADDRESS    m_OptionalDeviceAddr;
    BOOL    m_fRawTransfer;
    PVOID m_lpInPtr;
    DWORD m_nInLen;
*/
public:
    PHYSICAL_ADDRESS    GetUserBufferPhysAddr() { return m_UserBufferPhAddr; };
    PHYSICAL_ADDRESS    GetOptionalDeviceAddr() { return m_UserBufferPhAddr; };
    DWORD               GetUserBufferLength() { return m_dwUserBufferLength; };
protected:
    // Channel that hold this trasfer.
    DmaChannel& m_dmaChannel;
    
private:
    DmaTransfer& operator=(DmaTransfer&) { ASSERT(FALSE); }

    DWORD   m_dwTag;
    PVOID   m_pInternalBuffer;
    const DMA_TRANSFER_HANDLE   m_hPrevAttachedTransfer;
    PDMA_PDD_TRANFER_CONTEXT    m_pDmaPDDTransferContext;

};

