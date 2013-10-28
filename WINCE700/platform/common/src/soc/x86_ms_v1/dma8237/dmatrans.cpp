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
    DmaTrans.Cpp

Abstract:  
    Implementation for DMA function--Transfer.

Notes: 

--*/
#include <windows.h>
#include <oaldma.h>
#include <dma_chn.hpp>
#include <dma_tran.hpp>
DmaTransfer::DmaTransfer(DmaChannel& dmaChannel,
          IN DWORD  dwFlags,
          IN PHYSICAL_ADDRESS  SystemMemoryPhysicalAddress,
          IN PVOID CurrentVa,
          IN ULONG Length,
          IN HANDLE NotifyHandle,
          IN PVOID NotifyContext1,IN PVOID NotifyContext2,
          IN PHYSICAL_ADDRESS  phDeviceIoAddress,
          IN DMA_TRANSFER_HANDLE hPrevAttachedTrans)
:   m_dwCompletionCode(DMA_TRANSFER_IN_QUEUE)
,   m_dwCompleteLength(0)
,   m_hNotifyHandle(NotifyHandle)
,   m_NotifyContext1(NotifyContext1), m_NotifyContext2(NotifyContext2)
,   m_dmaChannel(dmaChannel)
,   m_hPrevAttachedTransfer(hPrevAttachedTrans)
{
    // MDD Context.
    m_dwFlags = dwFlags;
    m_fRawTransfer = FALSE;
    m_pUserBufferPtr= CurrentVa;
    m_dwUserBufferLength = Length;
    m_UserBufferPhAddr = SystemMemoryPhysicalAddress;
    m_OptionalDeviceAddr= phDeviceIoAddress;
    m_lpInPtr = NULL;
    m_nInLen = 0 ;
    m_pDmaPDDTransferContext = NULL;
    m_dwTag = DMA_TRANSFER_TAG;
    m_lRefCount = 0;
    Flink = Blink = NULL ;
    m_pInternalBuffer = NULL;
    if (m_UserBufferPhAddr.QuadPart == 0 && m_pUserBufferPtr != 0 && m_dwUserBufferLength != 0  ) { 
        // Address not mapped yet.
        m_pInternalBuffer = dmaChannel.AllocateCommonBuffer(m_dwUserBufferLength,&m_UserBufferPhAddr, FALSE);
        if (!m_pInternalBuffer)
            m_UserBufferPhAddr.QuadPart = 0 ;
    }
    
}
DmaTransfer::DmaTransfer(DmaChannel& dmaChannel,
          IN PVOID lpInPtr,
          IN DWORD nInLen,
          IN HANDLE NotifyHandle,
          IN PVOID NotifyContext1, IN PVOID NotifyContext2,
          IN DMA_TRANSFER_HANDLE hPrevAttachedTrans)
:   m_dwCompletionCode(DMA_TRANSFER_IN_QUEUE)
,   m_dwCompleteLength(0)
,   m_hNotifyHandle(NotifyHandle)
,   m_NotifyContext1(NotifyContext1), m_NotifyContext2(NotifyContext2)
,   m_dmaChannel(dmaChannel)
,   m_hPrevAttachedTransfer(hPrevAttachedTrans)
{
    // MDD context
    m_dwFlags = 0 ;
    m_fRawTransfer = TRUE ;
    m_pUserBufferPtr = NULL ;
    m_dwUserBufferLength = NULL;
    m_lpInPtr = lpInPtr;
    m_nInLen = nInLen ;
    m_pDmaPDDTransferContext = NULL;
    m_UserBufferPhAddr.QuadPart = 0 ;
    m_OptionalDeviceAddr.QuadPart = 0 ;
    m_dwTag = DMA_TRANSFER_TAG;
    m_lRefCount = 0 ;
    Flink = Blink = NULL ;
    m_pInternalBuffer = NULL;
}
DmaTransfer::~DmaTransfer()
{
    if (m_pDmaPDDTransferContext) {
        m_dmaChannel.FreeTransferPDD(*this,m_pDmaPDDTransferContext);
    }
    if (m_dwCompletionCode== DMA_TRANSFER_IN_QUEUE || m_dwCompletionCode==DMA_TRANSFER_IN_PROGRESS ) {
        CompleteNotification(DMA_TRANSFER_COMPLETE_WITH_CANCELED, m_dwUserBufferLength );
    }
    if (m_pInternalBuffer)
        m_dmaChannel.FreeCommonBuffer(m_dwUserBufferLength, m_UserBufferPhAddr, m_pInternalBuffer, FALSE );
}
BOOL DmaTransfer::Init()
{
    if (!m_fRawTransfer) {
         m_pDmaPDDTransferContext= m_dmaChannel.CreateDmaTransferPDD(*this) ;

        if (!m_dmaChannel.GetDmaAdapter().IsPhysAddrSupported(m_UserBufferPhAddr,m_dwUserBufferLength)){
            SetLastError(ERROR_INVALID_ADDRESS);
            return FALSE;
        }
        if (m_dwUserBufferLength > m_dmaChannel.GetMaximunSizeOfEachRegister() ) {
            return FALSE;
        }

        if (m_dwFlags & DMA_FLAGS_INC_DEVICE_ADDRESS) {
            PHYSICAL_ADDRESS targetAddr;
            targetAddr.QuadPart = ((m_dwFlags & DMA_FLAGS_USER_OPTIONAL_DEVICE)?m_OptionalDeviceAddr.QuadPart : m_dmaChannel.m_phDeviceIoAddress.QuadPart);
            if (m_dmaChannel.m_ulAddressSpace)
                return FALSE;
            if (!m_dmaChannel.GetDmaAdapter().IsPhysAddrSupported(targetAddr,m_dwUserBufferLength)) {
                SetLastError(ERROR_INVALID_ADDRESS);
                return FALSE;
            }
        }
        if ( m_pInternalBuffer!=NULL && m_dwUserBufferLength!= 0 && m_pUserBufferPtr!=NULL 
                && (m_dwFlags & DMA_FLAGS_WRITE_TO_DEVICE)!=0) {
            CeSafeCopyMemory(m_pInternalBuffer,m_pUserBufferPtr, m_dwUserBufferLength ) ;
        }
        return (m_dwUserBufferLength!=0 && m_UserBufferPhAddr.QuadPart != 0) ;
    }
    else {
        m_pDmaPDDTransferContext = m_dmaChannel.CreateRawDMATransferPDD(*this,m_lpInPtr,m_nInLen);
        return m_pDmaPDDTransferContext!=NULL;
    }
};
void * DmaTransfer::operator new(size_t stSize, DmaChannel& dmaChannel)
{
    return dmaChannel.AllocateTransfer(stSize);
}
void DmaTransfer::operator delete(void * /*pointer*/)
{
    //m_dmaChannel.FreeTransfer(pointer);
}
DWORD DmaTransfer::DeRef( void )
{
    LONG lReturn = InterlockedDecrement(&m_lRefCount);
    if( lReturn <= 0 ) {
         m_dmaChannel.DeleteTransfer(this);
    }
    return (DWORD)lReturn;
}


BOOL DmaTransfer::AbortTransfer()
{
    if (m_dwCompletionCode == DMA_TRANSFER_IN_QUEUE || m_dwCompletionCode == DMA_TRANSFER_IN_PROGRESS) {
        CompleteNotification(DMA_TRANSFER_COMPLETE_WITH_CANCELED,m_dwUserBufferLength);
    }
    return TRUE;
}
BOOL DmaTransfer::GetStatus(  OUT PDWORD lpCompletedLength, OUT PDWORD lpCompletionCode)
{
    BOOL fRet = TRUE;
    m_dmaChannel.IsArmedTransferDone(*this);
    DWORD dwCompletedLength = m_dwCompleteLength;
    DWORD dwCompletionCode = m_dwCompletionCode;
    if (m_hPrevAttachedTransfer!=NULL) {
        DmaTransfer * pPrevTrans = m_dmaChannel.TransferAddRef(m_hPrevAttachedTransfer);
        if (pPrevTrans!=NULL) {
            if (pPrevTrans->GetStatus(&dwCompletedLength,&dwCompletionCode)) {
                if (dwCompletionCode == DMA_TRANSFER_COMPLETE) { 
                    // Previous has been completed success. So current one is used to determin final state.
                    if (m_dwCompletionCode== DMA_TRANSFER_IN_QUEUE || m_dwCompletionCode == DMA_TRANSFER_IN_PROGRESS) {
                        dwCompletionCode = DMA_TRANSFER_IN_PROGRESS;
                    }
                    else
                        dwCompletionCode = m_dwCompletionCode;
                }
                dwCompletedLength += m_dwCompleteLength;
            }
            else {
                fRet = FALSE;
            }
            pPrevTrans->DeRef();
        }
    }
    __try {
        if (lpCompletionCode) {
            *lpCompletionCode = dwCompletionCode;
        }
        if (lpCompletedLength) {
            *lpCompletedLength = dwCompletedLength;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        fRet = FALSE; 
    }
    return fRet;
}
BOOL DmaTransfer::GetContexts(  OUT PVOID * pNotifyContext1, OUT PVOID * pNotifyContext2)
{
    BOOL fRet = TRUE;
    __try {
        if (pNotifyContext1) {
            *pNotifyContext1 = m_NotifyContext1;
        }
        if (pNotifyContext2) {
            *pNotifyContext2 = m_NotifyContext2;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        fRet = FALSE; 
    }
    return fRet;
}

void DmaTransfer::CompleteNotification(DMA_STATUS_CODE status, DWORD dwRemaining)
{
    m_dwCompletionCode = status ;
    m_dwCompleteLength = (m_dwUserBufferLength > dwRemaining? m_dwUserBufferLength-dwRemaining: 0 );
    PVOID pUserBufferPtr = (PVOID)InterlockedExchange( (PLONG)&m_pUserBufferPtr, 0 );
    
    if (m_dwCompletionCode != DMA_TRANSFER_IN_QUEUE && m_dwCompletionCode != DMA_TRANSFER_IN_PROGRESS) {
        
        if ( m_pInternalBuffer!=NULL && m_dwCompleteLength!= 0 && pUserBufferPtr!=NULL 
                && (m_dwFlags & DMA_FLAGS_WRITE_TO_DEVICE) == 0) {
            CeSafeCopyMemory(pUserBufferPtr, m_pInternalBuffer, m_dwCompleteLength ) ;
        }
        NotifyClientDriver();
    }
}
void DmaTransfer::SetTransferProcessing()
{
    if (m_dwCompletionCode == DMA_TRANSFER_IN_QUEUE)
        m_dwCompletionCode = DMA_TRANSFER_IN_PROGRESS;
    else
        ASSERT(FALSE);
}
void DmaTransfer::UpdateRemaining(DWORD dwRemaining)
{
    if (m_dwCompletionCode == DMA_TRANSFER_IN_PROGRESS) {
        m_dwCompleteLength = (m_dwUserBufferLength > dwRemaining? m_dwUserBufferLength-dwRemaining: 0 );
    }
    else
        ASSERT(FALSE);
}
BOOL DmaTransfer::IoControl( IN DWORD dwIoControl, IN PVOID lpInPtr, IN DWORD nInLen, IN OUT LPVOID lpOutBuffer, IN DWORD nOutBufferSize, IN LPDWORD lpBytesReturned)
{
    if (m_pDmaPDDTransferContext && m_pDmaPDDTransferContext->lpDMAPDDTransferIoControl) {
        return m_pDmaPDDTransferContext->lpDMAPDDTransferIoControl(m_pDmaPDDTransferContext,
            dwIoControl, lpInPtr, nInLen, lpOutBuffer, nOutBufferSize, lpBytesReturned
        );
    }
    else
        return FALSE;
}

