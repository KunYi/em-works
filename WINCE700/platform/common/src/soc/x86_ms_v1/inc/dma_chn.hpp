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

    dma_adpt.hpp

Abstract:  

    Contains DMA support routines.
    
Functions:

Notes:

--*/
#pragma once

#include <windows.h>
#include <types.h>
#include <Csync.h>
#include <CrefCon.h>
#include <dmamif.h>

class DmaAdapter;
class DmaChannel;
class DmaTransfer;
#include <dma_adpt.hpp>
#include <dma_chn.hpp>

#define DMA_CHANNEL_TAG 0xdaca5a3c
#define DMA_FREE_SPACE_TAG 0xdacaa5cc

typedef struct _FREE_DMA_TRANFER_SPACE *PFREE_DMA_TRANFER_SPACE;
typedef struct _FREE_DMA_TRANFER_SPACE {
    PFREE_DMA_TRANFER_SPACE pNextFreeTransfer;
    DWORD dwFreeSpaceTag;
    DWORD dwSpaceSize;
} FREE_DMA_TRANFER_SPACE, *PFREE_DMA_TRANFER_SPACE;

class DmaChannel :public DMA_MDD_CHANNEL_CONTEXT, public CRefObject, public CLockObject {
    friend class DmaAdapter;
    friend class DmaTransfer;
public:
    DmaChannel( DmaAdapter& dmaAdapter,PCE_DMA_ADAPTER pDmaAdapterInfo,DWORD dwChannelIndex,ULONG ulAddressSpace,PHYSICAL_ADDRESS phDeviceIoAddress );
    virtual BOOL Init();
    virtual ~DmaChannel( );
// ID function.
    DWORD   GetChanelTag() const { return m_dwTag; };
    DWORD   GetChannelIndex() const { return m_dwChannelIndex; };
// API Function.
    BOOL DMAIssueTransfer(IN OUT DMA_TRANSFER_HANDLE* ppDmaTransfer,  
          IN DWORD  dwFlags,
          IN PHYSICAL_ADDRESS  SystemMemoryPhysicalAddress,
          IN PVOID CurrentVa,
          IN ULONG Length,
          IN HANDLE NotifyHandle,
          IN PVOID NotifyContext1,
          IN PVOID NotifyContext2,
          IN PHYSICAL_ADDRESS  phOpDeviceIoAddress,
          IN DMA_TRANSFER_HANDLE pPreviousHandle = NULL

        );
    BOOL DMAIssueRawTransfer(IN OUT DMA_TRANSFER_HANDLE* ppDmaTransfer, 
          IN PVOID lpInPtr,
          IN DWORD nInLen,
          IN HANDLE NotifyHandle,
          IN PVOID NotifyContext1,
          IN PVOID NotifyContext2,
          IN DMA_TRANSFER_HANDLE pPreviousHandle = NULL
        );
    BOOL DMAIssueMultipleBufferTransfer(IN OUT DMA_TRANSFER_HANDLE* ppDmaTransfer, 
          IN DWORD  dwFlags,
          IN DWORD dwNumOfTransfer,
          IN CE_DMA_BUFFER_BLOCK pCeDmaBufferBlock[],
          IN HANDLE hNotifyHandle,
          IN PVOID Context1, IN PVOID Context2,
          IN PHYSICAL_ADDRESS  phOpDeviceIoAddress
        );

    BOOL DMACloseTransfer(DMA_TRANSFER_HANDLE DmaTransferHandle);
    BOOL StartTransfer(DMA_TRANSFER_HANDLE DmaTransferHandle);
    BOOL AbortTransfer(DMA_TRANSFER_HANDLE);
    // Services Transfer function.
    BOOL   GetDeviceAddress(ULONG& ulAddressSpace, PHYSICAL_ADDRESS& DeviceIoAddress) {
        ulAddressSpace = m_ulAddressSpace;
        DeviceIoAddress = m_phDeviceIoAddress ;
    }
    DmaAdapter& GetDmaAdapter() { return m_dmaAdapter; };
    BOOL    IsArmedTransferDone(DmaTransfer& dmaTransfer);
protected:
// Free Transfer Space Manager.
    PFREE_DMA_TRANFER_SPACE m_pFreeDmaTransferSpace;
    DWORD m_dwMinSize;
    PVOID AllocateTransfer(size_t stSize);
    void  DeleteTransfer(DmaTransfer *pTransfer);
    void  DeleteAllTransferSpace();
/* DMA_MDD_CHANNEL_CONTEXT member.
    CE_DMA_ADAPTER m_DmaAdapterInfo;
    DWORD   m_dwChannelIndex;
    ULONG       m_ulAddressSpace;
    PHYSICAL_ADDRESS m_phDeviceIoAddress;    
*/
protected:
// Object property.
    DmaAdapter& m_dmaAdapter;
    LIST_ENTRY  m_TransferList;
    PLIST_ENTRY m_pPendingTransfer;
public:    
    DmaTransfer * TransferAddRef(DMA_TRANSFER_HANDLE hTransfer);
protected:    
    static BOOL TransferCompleteNotifyStub (PDMA_MDD_CHANNEL_CONTEXT pDmaMDDChannelContext, PDMA_MDD_TRANFER_CONTEXT pDmaTransfer,DMA_STATUS_CODE status, DWORD dwRemaining) ;
    BOOL TransferCompleteNotify(DmaTransfer& dmaTransfer,DMA_STATUS_CODE status, DWORD dwRemaining);
    BOOL InsertTailTransfer(DmaTransfer *);
    BOOL RemoveTransfer(DmaTransfer *);
    BOOL TryToArmDMAFromHead();
    DWORD Adjust( IN ULONG Length, IN PHYSICAL_ADDRESS  SystemMemoryPhysicalAddress);
// Hardware Specific Function.
    PDMA_PDD_TRANFER_CONTEXT CreateDmaTransferPDD(DmaTransfer& aTransfer) ;
    PDMA_PDD_TRANFER_CONTEXT CreateRawDMATransferPDD(DmaTransfer& aTransfer,IN PVOID lpInPtr,IN DWORD nInLen);
    BOOL    FreeTransferPDD(DmaTransfer& aTransfer,PDMA_PDD_TRANFER_CONTEXT pPddTransferContext);

    DWORD   GetNumOfHardwareMappingRegister();
    DWORD   GetMaximunSizeOfEachRegister(); 
    DWORD   GetMaximunAddressBoundary();
    DWORD   GetAddressAligment();

    BOOL    CanArmDma() ;
    BOOL    ArmTransfer(DmaTransfer& aTransfer );
    virtual BOOL    TerminateTransfer(DmaTransfer& aTransfer);
    virtual BOOL    StartDmaTransfer();
    virtual BOOL    PollingForTransferDone() ;
    virtual BOOL    PollingTranferRemaining(DmaTransfer& dmaTransfer, DWORD& dwRemaining);
private:
    DmaChannel& operator=(DmaChannel&) { ASSERT(FALSE); }
    DWORD   m_dwTag;
    DWORD   m_dwFreeMappingRegister;

    PDMA_PDD_CHANNEL_CONTEXT m_pDMAPDDChannelContext ;
    
public:
// Help Function.
    PVOID AllocateCommonBuffer(ULONG Length, PPHYSICAL_ADDRESS LogicalAddress, BOOLEAN CacheEnabled);
    BOOL FreeCommonBuffer(ULONG Length, PHYSICAL_ADDRESS LogicalAddress, PVOID VirtualAddress, BOOLEAN CacheEnabled);

};


