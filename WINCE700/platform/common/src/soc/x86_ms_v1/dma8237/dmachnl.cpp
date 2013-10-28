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

    dmaChnl.cpp

Abstract:  

    Contains DMA support routines for Channel.
    
Functions:

Notes:

--*/
#include <windows.h>
#include <LinkList.h>
#include <oaldma.h>
#include <dma_adpt.hpp>
#include <dma_chn.hpp>
#include <dma_tran.hpp>


DmaChannel::DmaChannel( DmaAdapter& dmaAdapter,PCE_DMA_ADAPTER pDmaAdapter, DWORD dwChannelIndex,ULONG ulAddressSpace,PHYSICAL_ADDRESS phDeviceIoAddress )
:   m_dmaAdapter(dmaAdapter)
{
    // Initialize MDD context.
    m_dwChannelIndex = dwChannelIndex;
    m_ulAddressSpace = ulAddressSpace;
    m_phDeviceIoAddress = phDeviceIoAddress;
    m_lpTransferCompleteNotify = TransferCompleteNotifyStub;
    
    m_dwFreeMappingRegister = 0; 
    m_dwTag = DMA_CHANNEL_TAG ;
    InitializeListHead(&m_TransferList);
    m_pPendingTransfer = &m_TransferList ;
    
    m_pFreeDmaTransferSpace = NULL;
    m_dwMinSize = 0;
    
    m_pDMAPDDChannelContext = NULL;
    
    if (pDmaAdapter!=NULL)  {
        __try {
            m_DmaAdapterInfo = *pDmaAdapter;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            m_DmaAdapterInfo.Size = 0;
        }
    }
    else
        m_DmaAdapterInfo.Size = 0;
}
BOOL DmaChannel::Init()
{   
    m_pDMAPDDChannelContext = m_dmaAdapter.AllocateChannelPdd(*this);
    if (m_pDMAPDDChannelContext == NULL ||
        m_pDMAPDDChannelContext->lpArmTransfer==NULL ||
        m_pDMAPDDChannelContext->lpCanArmDma==NULL ||
        m_pDMAPDDChannelContext->lpPollingForTransferDone == NULL ||
        m_pDMAPDDChannelContext->lpPollingTransferRemaining == NULL ||
        m_pDMAPDDChannelContext->lpStartDmaTransfer == NULL ||
        m_pDMAPDDChannelContext->lpTerminateTransfer == NULL
        ) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (m_DmaAdapterInfo.Size >= sizeof(m_DmaAdapterInfo)) {
        m_dwFreeMappingRegister = GetNumOfHardwareMappingRegister();
        return TRUE;
    }
    else
        return FALSE;
}
DmaChannel::~DmaChannel()
{
    if (m_pDMAPDDChannelContext) {
        m_dmaAdapter.FreeChannelPdd(m_pDMAPDDChannelContext);
    }
    DWORD dwMaxCount = 0x1000;
    while (!IsListEmpty(&m_TransferList) && dwMaxCount -- ) {
        RemoveTransfer((DmaTransfer *)m_TransferList.Flink);
    }
    ASSERT(dwMaxCount);
    DeleteAllTransferSpace() ;
}
DWORD   DmaChannel::GetNumOfHardwareMappingRegister()
{    
    return m_dmaAdapter.GetNumOfHardwareMappingRegister(); 
};
DWORD   DmaChannel::GetMaximunSizeOfEachRegister()
{ 
    DWORD dwReturn =  m_dmaAdapter.GetMaximunSizeOfEachRegister(); 
    ASSERT(((dwReturn-1) & dwReturn) == 0);
    ASSERT(dwReturn>=GetMaximunAddressBoundary());
    return dwReturn ;
};
DWORD   DmaChannel::GetMaximunAddressBoundary()
{
    DWORD dwReturn = m_dmaAdapter.GetMaximunAddressBoundary();
    ASSERT(((dwReturn-1) & dwReturn) == 0);
    ASSERT(dwReturn>=GetAddressAligment());
    return dwReturn ;
}
DWORD   DmaChannel::GetAddressAligment()
{
    DWORD dwReturn = m_dmaAdapter.GetAddressAligment();
    ASSERT(((dwReturn-1) & dwReturn) == 0);
    return dwReturn ;
}

PVOID  DmaChannel::AllocateCommonBuffer(ULONG Length, PPHYSICAL_ADDRESS LogicalAddress, BOOLEAN CacheEnabled) 
{
    return m_dmaAdapter.OALAllocateCommonBuffer(&m_DmaAdapterInfo, Length, LogicalAddress, CacheEnabled) ;
}
BOOL DmaChannel::FreeCommonBuffer(ULONG Length, PHYSICAL_ADDRESS LogicalAddress, PVOID VirtualAddress, BOOLEAN CacheEnabled) 
{
    return m_dmaAdapter.OALFreeCommonBuffer(&m_DmaAdapterInfo, Length, LogicalAddress, VirtualAddress, CacheEnabled);
}
DWORD DmaChannel::Adjust(
          IN ULONG Length,
          IN PHYSICAL_ADDRESS  SystemMemoryPhysicalAddress
    )
{
    if ((SystemMemoryPhysicalAddress.LowPart & (GetAddressAligment() - 1))!=0) { // Physical Address is not align.
        return 0 ;
    };
    DWORD dwAddressBoundary = GetMaximunAddressBoundary() ;
    DWORD dwActualLength = dwAddressBoundary - (SystemMemoryPhysicalAddress.LowPart & (dwAddressBoundary-1));
    return min(dwActualLength,Length);    
}
BOOL DmaChannel::DMAIssueTransfer(IN OUT DMA_TRANSFER_HANDLE* ppDmaTransfer,  
          IN DWORD  dwFlags,
          IN PHYSICAL_ADDRESS  SystemMemoryPhysicalAddress,
          IN PVOID CurrentVa,
          IN ULONG Length,
          IN HANDLE NotifyHandle,
          IN PVOID NotifyContext1,
          IN PVOID NotifyContext2,
          IN PHYSICAL_ADDRESS  phOpDeviceIoAddress,
          IN DMA_TRANSFER_HANDLE pPreviousHandle
        )
{
    BOOL fRet = FALSE; 
    DMA_TRANSFER_HANDLE hCurrnDmaHandle = NULL; 
    
    if (ppDmaTransfer ) {
        
        while (Length) {
            
            DWORD dwLength;
            if (SystemMemoryPhysicalAddress.QuadPart!=0) 
                dwLength = Adjust(Length,SystemMemoryPhysicalAddress);
            else
                dwLength = min (Length, min(GetMaximunAddressBoundary(),GetMaximunSizeOfEachRegister()));
            ASSERT(dwLength <= Length);
            
            if (dwLength) {
                
                HANDLE hNotfy = (dwLength == Length? NotifyHandle: NULL);
                DmaTransfer* pDmaTransfer =  new(*this) DmaTransfer(*this,dwFlags,SystemMemoryPhysicalAddress, CurrentVa,
                    dwLength , hNotfy, NotifyContext1,NotifyContext2, phOpDeviceIoAddress, pPreviousHandle );
                
                if (pDmaTransfer && pDmaTransfer->Init()) {
                    InsertTailTransfer( pDmaTransfer );
                    hCurrnDmaHandle = (DMA_TRANSFER_HANDLE) pDmaTransfer;
                    if (dwFlags & (DMA_FLAGS_USER_OPTIONAL_DEVICE | DMA_FLAGS_INC_DEVICE_ADDRESS)) {
                        phOpDeviceIoAddress.QuadPart += dwLength ;
                    }
                    Length -= dwLength; 
                    SystemMemoryPhysicalAddress.QuadPart += dwLength ;
                    CurrentVa = (PBYTE)CurrentVa + dwLength;
                    pPreviousHandle = hCurrnDmaHandle ;
                        
                }
                else {
                    if (pDmaTransfer) {
                        delete pDmaTransfer;
                        SetLastError(ERROR_INVALID_PARAMETER);
                    }
                    else 
                        SetLastError(ERROR_OUTOFMEMORY);
                    ASSERT(FALSE);
                    break;
                }
                
            }
            else {
                SetLastError(ERROR_INVALID_PARAMETER);
                ASSERT(FALSE);
                break;
            }
        }
        
        if (!Length && hCurrnDmaHandle){ // 
            *ppDmaTransfer = hCurrnDmaHandle ;
            fRet = TRUE;
        }
        else if (hCurrnDmaHandle) {
            DMACloseTransfer(hCurrnDmaHandle);
            hCurrnDmaHandle = NULL ;
        }
        TryToArmDMAFromHead() ;
    }
    ASSERT(fRet);
    return fRet;
}
/* The DMAIssueMultipleBufferTransfer queue up multiple transfer in queue at once. 
*/
BOOL DmaChannel::DMAIssueMultipleBufferTransfer(
  IN PDMA_TRANSFER_HANDLE phDmaHandle,
  IN DWORD  dwFlags,
  IN DWORD dwNumOfTransfer,
  IN CE_DMA_BUFFER_BLOCK pCeDmaBufferBlock[],
  IN HANDLE hNotifyHandle,
  IN PVOID Context1, IN PVOID Context2,
  IN PHYSICAL_ADDRESS  phOpDeviceIoAddress
)
{
    BOOL fRet = FALSE; 
    DMA_TRANSFER_HANDLE hCurrnDmaHandle = NULL; 
    if (phDmaHandle && pCeDmaBufferBlock ) {
        for (DWORD dwIndex = 0 ; dwIndex < dwNumOfTransfer; dwIndex ++ ) {
            PHYSICAL_ADDRESS phSystemPhAddr =  pCeDmaBufferBlock[dwIndex].physicalAddress;
            PVOID pVirtAddr = pCeDmaBufferBlock[dwIndex].virtualAddress;
            DWORD dwLength = pCeDmaBufferBlock[dwIndex].dwLength;
            BOOL fLastOne = ((dwIndex+1)==dwNumOfTransfer);
            if (dwLength) {
                HANDLE hNotfy = (fLastOne? hNotifyHandle: NULL);
                DMA_TRANSFER_HANDLE hDmaTransfer = NULL;
                if (DMAIssueTransfer(&hDmaTransfer, dwFlags,phSystemPhAddr,pVirtAddr,dwLength,hNotfy,Context1,Context2,phOpDeviceIoAddress,hCurrnDmaHandle)){
                    if (dwFlags & (DMA_FLAGS_USER_OPTIONAL_DEVICE | DMA_FLAGS_INC_DEVICE_ADDRESS)) {
                        phOpDeviceIoAddress.QuadPart += dwLength ;
                    }
                    hCurrnDmaHandle = hDmaTransfer;                    
                }
                else
                    break;
            }
            else {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
        }
        if (dwIndex >= dwNumOfTransfer && hCurrnDmaHandle) { // Finished all of them
            *phDmaHandle = hCurrnDmaHandle ;
            fRet = TRUE;
        }
        else if (hCurrnDmaHandle) {
            DMACloseTransfer(hCurrnDmaHandle);
        }
    }
    ASSERT(fRet);
    return fRet;
        
    
}

BOOL DmaChannel::DMAIssueRawTransfer(IN OUT DMA_TRANSFER_HANDLE * ppDmaTransfer, 
          IN PVOID lpInPtr,
          IN DWORD nInLen,
          IN HANDLE NotifyHandle,
          IN PVOID NotifyContext1,
          IN PVOID NotifyContext2,
          IN DMA_TRANSFER_HANDLE pPreviousHandle
        )
{
    BOOL fRet = FALSE; 
    if (ppDmaTransfer) {
        DmaTransfer* pDmaTransfer = new(*this) DmaTransfer(*this,lpInPtr, nInLen,
            NotifyHandle,NotifyContext1,NotifyContext2,pPreviousHandle);
        if (pDmaTransfer && pDmaTransfer->Init()) {
            if (InsertTailTransfer( pDmaTransfer ) ) {
                *ppDmaTransfer =(DMA_TRANSFER_HANDLE) pDmaTransfer ;
                TryToArmDMAFromHead() ;
                fRet = TRUE;
            }
        }
        if (!fRet && pDmaTransfer!=NULL) {
            delete pDmaTransfer;
        }        
    }
    ASSERT(fRet);
    return fRet;
}

BOOL DmaChannel::DMACloseTransfer(DMA_TRANSFER_HANDLE DmaTransferHandle)
{
    BOOL fRet = FALSE;
    while (DmaTransferHandle) {
        DmaTransfer * pDmaTransfer = TransferAddRef(DmaTransferHandle);
        if (pDmaTransfer) {
            Lock();
            if (pDmaTransfer->GetCompletionCode()== DMA_TRANSFER_IN_PROGRESS) { 
                TerminateTransfer(*pDmaTransfer);
            }
            if (pDmaTransfer->GetCompletionCode()== DMA_TRANSFER_IN_PROGRESS ||
                    pDmaTransfer->GetCompletionCode() == DMA_TRANSFER_IN_QUEUE) {
                pDmaTransfer->AbortTransfer();
            }
            DmaTransferHandle = pDmaTransfer->GetPrevAttachedTransfer();
            RemoveTransfer(pDmaTransfer);
            Unlock();
            pDmaTransfer->DeRef();
            fRet = TRUE;
        }
        else
            break;
    }
    ASSERT(fRet);
    return fRet;
}
BOOL DmaChannel::AbortTransfer(DMA_TRANSFER_HANDLE DmaTransferHandle)
{
    while (DmaTransferHandle!=NULL) {
        DmaTransfer * pDmaTransfer = TransferAddRef(DmaTransferHandle);
        if (pDmaTransfer) {
            Lock();
            if (pDmaTransfer->GetCompletionCode()== DMA_TRANSFER_IN_PROGRESS) { 
                TerminateTransfer(*pDmaTransfer);
            }
            if (pDmaTransfer->GetCompletionCode()== DMA_TRANSFER_IN_PROGRESS ||
                    pDmaTransfer->GetCompletionCode() == DMA_TRANSFER_IN_QUEUE) {
                pDmaTransfer->AbortTransfer();
            }
            Unlock();
            DmaTransferHandle = pDmaTransfer->GetPrevAttachedTransfer();
            pDmaTransfer->DeRef();
        }
        else
            break;
    }
    return (DmaTransferHandle==NULL);
}
BOOL DmaChannel::StartTransfer(DMA_TRANSFER_HANDLE DmaTransferHandle)
{
    BOOL fCleaned = FALSE;
    while (DmaTransferHandle!=NULL) {
        DmaTransfer * pDmaTransfer = TransferAddRef(DmaTransferHandle);
        if (pDmaTransfer) {
            Lock();
            pDmaTransfer->m_dwFlags &= ~DMA_FLAGS_NOT_AUTO_START;
            fCleaned = TRUE;
            Unlock();
            DmaTransferHandle = pDmaTransfer->GetPrevAttachedTransfer();
            pDmaTransfer->DeRef();
        }
        else
            break;
    }
    if (fCleaned) {
        TryToArmDMAFromHead();
    }
    return (fCleaned);
}

DmaTransfer * DmaChannel::TransferAddRef(DMA_TRANSFER_HANDLE hTransfer)
{
    DmaTransfer * pReturnTransfer = NULL;
    Lock();
    if (!IsListEmpty(&m_TransferList)) {
        DmaTransfer *pCurTransfer = (DmaTransfer *) m_TransferList.Flink ;
        while(pCurTransfer->ConvertToLink() != &m_TransferList)  {
            if ((DMA_TRANSFER_HANDLE) pCurTransfer == hTransfer ) {
                pReturnTransfer = pCurTransfer;
                pReturnTransfer->AddRef();
                break;
            }
            else {
                pCurTransfer =(DmaTransfer *) (pCurTransfer->ConvertToLink()->Flink);
            }
        } ;
    }
    Unlock();
    ASSERT(pReturnTransfer!=NULL);
    return pReturnTransfer ;
}
BOOL DmaChannel::TransferCompleteNotifyStub (PDMA_MDD_CHANNEL_CONTEXT pDmaMDDChannelContext, PDMA_MDD_TRANFER_CONTEXT pDmaTransfer,DMA_STATUS_CODE status, DWORD dwRemaining)
{
    if (pDmaMDDChannelContext && pDmaTransfer) {
        return ((DmaChannel *)pDmaMDDChannelContext)->TransferCompleteNotify( *(DmaTransfer *)pDmaTransfer,status,dwRemaining);
    }
    else
        return FALSE;
}

BOOL DmaChannel::TransferCompleteNotify(DmaTransfer& dmaTransfer,DMA_STATUS_CODE status, DWORD dwRemaining)
{
    Lock();
    ASSERT(status!=DMA_TRANSFER_IN_PROGRESS && status!= DMA_TRANSFER_IN_QUEUE);
    dmaTransfer.CompleteNotification(status,dwRemaining);
    
    m_dwFreeMappingRegister ++ ; // We free one Mapping Register.
    ASSERT(m_dwFreeMappingRegister<=GetNumOfHardwareMappingRegister());
    if (status != DMA_TRANSFER_COMPLETE) {
        // Check Next ..
        DmaTransfer * pCurTransfer = &dmaTransfer;
        DmaTransfer * pNextTransfer = (DmaTransfer * )pCurTransfer->Flink;
        // We can do this because we are in CS.
        while (pNextTransfer!= &m_TransferList && pNextTransfer->GetPrevAttachedTransfer()==(DMA_TRANSFER_HANDLE )pCurTransfer) { // This is next one.
            ASSERT(pNextTransfer->GetTag() == DMA_TRANSFER_TAG);
            ASSERT(pCurTransfer->GetTag() == DMA_TRANSFER_TAG);
            pNextTransfer->CompleteNotification(status,pNextTransfer->GetUserBufferLength() );
            pCurTransfer = pNextTransfer;
            pNextTransfer =(DmaTransfer * ) pCurTransfer->Flink;
        }
    }
    Unlock();
    dmaTransfer.DeRef();
    TryToArmDMAFromHead();
    return TRUE;
}

BOOL DmaChannel::InsertTailTransfer(DmaTransfer *pTransfer)
{
    if (pTransfer) {
        ASSERT(pTransfer->GetTag() == DMA_TRANSFER_TAG);
        Lock();
        InsertTailList(&m_TransferList, pTransfer->ConvertToLink());
        pTransfer->AddRef();
        if (m_pPendingTransfer == &m_TransferList) {
            m_pPendingTransfer = pTransfer->ConvertToLink() ;
        }
        Unlock();
        return TRUE;
    }
    else
        return FALSE;
    
}
BOOL DmaChannel::RemoveTransfer(DmaTransfer *pTransfer )
{
    BOOL fRet = FALSE;
    if (pTransfer)  {
        Lock();
        PLIST_ENTRY pCurLink = m_TransferList.Flink ;
        while (pCurLink!= &m_TransferList) {
            if (pCurLink == pTransfer->ConvertToLink()) {
                if (m_pPendingTransfer == pCurLink) {
                    m_pPendingTransfer = pCurLink->Flink ;
                }
                RemoveEntryList(pCurLink);
                pTransfer->DeRef();
                fRet = TRUE;
                break;
            }
            else 
                pCurLink = pCurLink->Flink ;
        }
        Unlock();
    }
    ASSERT(fRet);
    return fRet;
}
BOOL DmaChannel::TryToArmDMAFromHead()
{
    BOOL  bRet = FALSE;
    Lock();
    if ( !IsListEmpty(&m_TransferList) && CanArmDma()) {
        while (m_pPendingTransfer!= &m_TransferList && m_dwFreeMappingRegister!=0 ) {
            DmaTransfer * pTransfer = (DmaTransfer * )m_pPendingTransfer;
            ASSERT(pTransfer->GetTag() == DMA_TRANSFER_TAG);
            
            if (pTransfer->GetCompletionCode()== DMA_TRANSFER_IN_QUEUE ) { 
                if ((pTransfer->GetTransferFlags() & DMA_FLAGS_NOT_AUTO_START)) { // we hit paused transfer.
                    break;
                }
                if (ArmTransfer(*pTransfer)) {
                    pTransfer->AddRef(); // We Add Ref Count for PDD.
                    pTransfer->SetTransferProcessing() ;
                    m_dwFreeMappingRegister--;
                    bRet = TRUE;
                }
                else { // Failed by PDD.
                    ASSERT(FALSE);
                    break;
                }

            }
            else {
                m_pPendingTransfer = m_pPendingTransfer->Flink ;
            }
        }
        if (bRet) { // We at least arm one. So trigger it to start.
            StartDmaTransfer() ;
        }
            
    }
    Unlock();
    return bRet;
}
BOOL DmaChannel::IsArmedTransferDone(DmaTransfer& dmaTransfer )
{
    BOOL bRet = FALSE;
    Lock();
    bRet = PollingForTransferDone() ;
    DWORD dwRemaining = MAXDWORD;
    if (dmaTransfer.GetCompletionCode() == DMA_TRANSFER_IN_PROGRESS &&
            PollingTranferRemaining( dmaTransfer, dwRemaining)) {
        dmaTransfer.UpdateRemaining(dwRemaining);
    }
    Unlock();
    return bRet;
}
//    PFREE_DMA_TRANFER_SPACE m_pFreeDmaTransferSpace;
//    DWORD m_dwMinSize;
PVOID DmaChannel::AllocateTransfer(size_t stSize)
{
    PVOID pReturn = NULL ;
    Lock();
    if (stSize>m_dwMinSize) {
        DEBUGMSG(ZONE_WARN && m_dwMinSize!=0,(L"TransferSize Changed From %d, to %d",m_dwMinSize,stSize) );
        DeleteAllTransferSpace();
        m_dwMinSize= stSize;
    }
    if (m_pFreeDmaTransferSpace==NULL) {
        m_pFreeDmaTransferSpace =(PFREE_DMA_TRANFER_SPACE) malloc(sizeof(FREE_DMA_TRANFER_SPACE)+m_dwMinSize);
        if (m_pFreeDmaTransferSpace) {
            m_pFreeDmaTransferSpace->dwFreeSpaceTag = DMA_FREE_SPACE_TAG;
            m_pFreeDmaTransferSpace->dwSpaceSize = m_dwMinSize;
            m_pFreeDmaTransferSpace->pNextFreeTransfer = NULL;
        }
    }
    if (m_pFreeDmaTransferSpace) {
        ASSERT(m_pFreeDmaTransferSpace->dwFreeSpaceTag == DMA_FREE_SPACE_TAG);
        ASSERT(m_pFreeDmaTransferSpace->dwSpaceSize>=stSize);
        pReturn = (PVOID)(m_pFreeDmaTransferSpace+1);
        m_pFreeDmaTransferSpace = m_pFreeDmaTransferSpace->pNextFreeTransfer ;
    };
    Unlock();
    return pReturn;
}
void  DmaChannel::DeleteTransfer(DmaTransfer *pTransfer)
{
    if (pTransfer) {
        delete pTransfer; // delete for DmaTransfer has been overwrite. It will not free the memory.
        Lock();
        PFREE_DMA_TRANFER_SPACE pDmaFreeSpace = (PFREE_DMA_TRANFER_SPACE)pTransfer;
        pDmaFreeSpace -- ; // move back to the header.
        if (pDmaFreeSpace->dwFreeSpaceTag== DMA_FREE_SPACE_TAG) {
            if (pDmaFreeSpace->dwSpaceSize != m_dwMinSize) { // Size has chnaged. So delete it directory.
                DEBUGMSG(ZONE_WARN,(L"free space because size changed.%x ",pDmaFreeSpace) );
                free(pDmaFreeSpace);
            }
            else {
                pDmaFreeSpace->pNextFreeTransfer = m_pFreeDmaTransferSpace;
                m_pFreeDmaTransferSpace = pDmaFreeSpace;
            }
        }
        else {// trashed point. Do know what to do except free it direct free it
            ASSERT(FALSE);
            DEBUGMSG(ZONE_ERROR,(L"DeleteTransfer detect garbage pointer %x ",pDmaFreeSpace) );
            free(pDmaFreeSpace);
        }
        Unlock();
    }
}
void  DmaChannel::DeleteAllTransferSpace()
{
    Lock();
    while (m_pFreeDmaTransferSpace) {
        PFREE_DMA_TRANFER_SPACE pDmaNextFreeSpace = m_pFreeDmaTransferSpace->pNextFreeTransfer;
        free(m_pFreeDmaTransferSpace);
        m_pFreeDmaTransferSpace = pDmaNextFreeSpace;
    }
    Unlock();
    
}
PDMA_PDD_TRANFER_CONTEXT DmaChannel::CreateDmaTransferPDD(DmaTransfer& aTransfer) 
{
    PREFAST_ASSERT(m_pDMAPDDChannelContext);
    if (m_pDMAPDDChannelContext->lpCreateDmaPDDTransfer)
        return (m_pDMAPDDChannelContext->lpCreateDmaPDDTransfer(m_pDMAPDDChannelContext,
            aTransfer.GetMDDContext()));
    else
        return NULL;
}
PDMA_PDD_TRANFER_CONTEXT DmaChannel::CreateRawDMATransferPDD(DmaTransfer& aTransfer,IN PVOID lpInPtr,IN DWORD nInLen)
{
    PREFAST_ASSERT(m_pDMAPDDChannelContext);
    if (m_pDMAPDDChannelContext->lpCreateRawDmaPDDTransfer)
        return (m_pDMAPDDChannelContext->lpCreateRawDmaPDDTransfer(m_pDMAPDDChannelContext,
            aTransfer.GetMDDContext(),lpInPtr,nInLen));
    else {
        SetLastError(ERROR_NOT_SUPPORTED);
        return NULL;
    }
}
BOOL    DmaChannel::FreeTransferPDD(DmaTransfer& aTransfer,PDMA_PDD_TRANFER_CONTEXT pPddTransferContext)
{
    PREFAST_ASSERT(m_pDMAPDDChannelContext);
    if (pPddTransferContext!=NULL && m_pDMAPDDChannelContext->lpFreeDmaTransfer) {
        return m_pDMAPDDChannelContext->lpFreeDmaTransfer(m_pDMAPDDChannelContext, pPddTransferContext) ;
    }
    else {
        SetLastError(ERROR_NOT_SUPPORTED);
        return NULL;
    }
        
    
}

BOOL    DmaChannel::CanArmDma() 
{
    PREFAST_ASSERT(m_pDMAPDDChannelContext);
    PREFAST_ASSERT(m_pDMAPDDChannelContext->lpCanArmDma);
    return (m_pDMAPDDChannelContext->lpCanArmDma(m_pDMAPDDChannelContext));
    
}
BOOL    DmaChannel::ArmTransfer(DmaTransfer& aTransfer )
{
    PREFAST_ASSERT(m_pDMAPDDChannelContext);
    PREFAST_ASSERT(m_pDMAPDDChannelContext->lpArmTransfer);
    return (m_pDMAPDDChannelContext->lpArmTransfer(m_pDMAPDDChannelContext,
        aTransfer.GetMDDContext(),aTransfer.GetPDDContext()));
}
BOOL    DmaChannel::TerminateTransfer(DmaTransfer& aTransfer)
{
    PREFAST_ASSERT(m_pDMAPDDChannelContext);
    PREFAST_ASSERT(m_pDMAPDDChannelContext->lpTerminateTransfer);
    return (m_pDMAPDDChannelContext->lpTerminateTransfer(m_pDMAPDDChannelContext,
        aTransfer.GetMDDContext(),aTransfer.GetPDDContext()));
}
BOOL    DmaChannel::StartDmaTransfer()
{
    PREFAST_ASSERT(m_pDMAPDDChannelContext);
    PREFAST_ASSERT(m_pDMAPDDChannelContext->lpStartDmaTransfer);
    return (m_pDMAPDDChannelContext->lpStartDmaTransfer(m_pDMAPDDChannelContext));
}
BOOL    DmaChannel::PollingForTransferDone()
{
    if (m_dwFreeMappingRegister< GetNumOfHardwareMappingRegister()) {
        PREFAST_ASSERT(m_pDMAPDDChannelContext);
        PREFAST_ASSERT(m_pDMAPDDChannelContext->lpPollingForTransferDone);
        return (m_pDMAPDDChannelContext->lpPollingForTransferDone(m_pDMAPDDChannelContext));
    }
    else
        return TRUE;
}
BOOL    DmaChannel::PollingTranferRemaining(DmaTransfer& dmaTransfer, DWORD& dwRemaining)
{
    PREFAST_ASSERT(m_pDMAPDDChannelContext);
    PREFAST_ASSERT(m_pDMAPDDChannelContext->lpPollingTransferRemaining);
    return (m_pDMAPDDChannelContext->lpPollingTransferRemaining(m_pDMAPDDChannelContext,
        dmaTransfer.GetMDDContext(),dmaTransfer.GetPDDContext(),&dwRemaining));
}

