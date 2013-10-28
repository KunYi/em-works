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

    Contains DMA 8237 support routines.
    
Functions:

Notes:

--*/
#pragma once

#include <windows.h>
#include <types.h>
#include <Csync.h>
#include <CrefCon.h>
#include <dmamif.h>
#include <CRegedit.h>
#include <Cphysmem.hpp>

class Dma8237Adapter;
class Dma8237Channel;
#define DMA_ENABLEMEMORYTOMEMORY   TEXT("EnableMemoryToMemory")

class Dma8237Adapter: public DMA_PDD_ADAPTER_CONTEXT ,public CRegistryEdit {
    friend class Dma8237Channel;
private:
    DWORD m_fEnableMemoryToMemory;
    DWORD m_dwMaxNumOfChannel;
    DEVICEWINDOW m_DmaMemory;
    PBYTE       m_DmaMemoryVirt;
    CPhysMem *  m_pPhysMem;
public:
    Dma8237Adapter(LPCTSTR lpcRegistryPath) ;
    virtual BOOL Init();
    virtual ~Dma8237Adapter();
    virtual PVOID   OALAllocateCommonBuffer( PCE_DMA_ADAPTER pDmaAdapter, ULONG Length, PPHYSICAL_ADDRESS pLogicalAddress, BOOLEAN CacheEnabled) ;
    virtual BOOL    OALFreeCommonBuffer(PCE_DMA_ADAPTER pDmaAdapter, ULONG Length, PHYSICAL_ADDRESS LogicalAddress, PVOID VirtualAddress, BOOLEAN CacheEnabled) ;
    virtual BOOL    IsChannelSuitable(CE_DMA_ADAPTER& dmaAdapterInfo,DWORD dwChannelIndex) ;
    virtual DWORD   UpdateFlags(DWORD dwFlags);
    virtual Dma8237Channel * AllocaAdapterChannel(IN PDMA_MDD_CHANNEL_CONTEXT lpDmaMDDChannelContext);
    virtual BOOL    FreeDmaChannel(Dma8237Channel *pDmaChannel);
    virtual BOOL    IsPhysAddrSupported(PHYSICAL_ADDRESS BufferPhysAddress,DWORD dwLength);
    virtual void    PowerOnReset();
    
protected:
// PDD Function.
    virtual BOOL    InitializeAdapter(BOOL fInit) ;
private:
    static  PVOID   OALAllocateCommonBufferStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,
        PCE_DMA_ADAPTER DmaAdapter, ULONG Length, PPHYSICAL_ADDRESS LogicalAddress, BOOLEAN CacheEnabled);
    static BOOL     OALFreeCommonBufferStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,
        PCE_DMA_ADAPTER DmaAdapter, ULONG Length, PHYSICAL_ADDRESS LogicalAddress, PVOID VirtualAddress, BOOLEAN CacheEnabled);
    static BOOL     IsChannelSuitableStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,CE_DMA_ADAPTER * dmaAdapterInfo,DWORD dwChannelIndex);
    static DWORD    UpdateFlagsStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext, DWORD dwFlags) ;
    static PDMA_PDD_CHANNEL_CONTEXT AllocaAdapterChannelStub(PDMA_PDD_ADAPTER_CONTEXT pDmaPddAdapterContext, IN PDMA_MDD_CHANNEL_CONTEXT lpDmaMDDChannelContext);
    static BOOL FreeDmaChannelStub(PDMA_PDD_ADAPTER_CONTEXT pDmaPddAdapterContext, IN PDMA_PDD_CHANNEL_CONTEXT lpDmaMDDChannelContext);
    static void    PowerOnResetStub(PDMA_PDD_ADAPTER_CONTEXT pDmaPddAdapterContext);
    static BOOL  IsPhysAddrSupportedStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,PHYSICAL_ADDRESS BufferPhysAddress,DWORD dwLength);
    DWORD GetAddressAligment() const { return dwAddressAligment; };
    DWORD GetMaximunAddressBoundary() const { return dwMaximumAddressBoundary; };

    // 8237 DMA Implementatio.
    CLockObject m_HardwareLock;
    void LockHardware( ) {
        m_HardwareLock.Lock();
    }
    void UnlockHardware() {
        m_HardwareLock.Unlock();
    }
    void WriteAddrCount(DWORD wChannel,DWORD addr, WORD count) ;
    WORD CountRegister(DWORD wChannel);
    void SetClearRequest(DWORD dwChannel,BOOL fSet);
    void SetClearMask(DWORD dwChannel,BOOL fSet) ;
    void SetUpMode(DWORD dwChannel, CE_DMA_ADAPTER& dmaInfo);
    UCHAR m_StatusReg1,m_StatusReg2;
    void     UpdateTC() ;
    void ClearTC(DWORD dwChannel);
    BOOL IsTransferDone(DWORD dwChannel);
};
class Dma8237Channel : public DMA_PDD_CHANNEL_CONTEXT {
public:
    Dma8237Channel(Dma8237Adapter& DmaAdapter,PDMA_MDD_CHANNEL_CONTEXT pDmaMDDChannelContext) ;
    virtual BOOL Init() { return m_pDmaMDDChannelContext!=NULL ; };
    Dma8237Adapter& GetDmaAdapter() { return m_dmaAdapter; };    
protected:
    PDMA_MDD_TRANFER_CONTEXT m_Transfering;
    Dma8237Adapter& m_dmaAdapter;
    const PDMA_MDD_CHANNEL_CONTEXT m_pDmaMDDChannelContext;
    DWORD   GetChannelIndex() const  { return m_pDmaMDDChannelContext->m_dwChannelIndex; };
    virtual BOOL    CanArmDma() { return (m_Transfering==NULL); }
    virtual BOOL    StartDmaTransfer() {  return TRUE; };
    virtual BOOL    ArmTransfer(PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT pDmaPDDTransfer ) ;
    virtual BOOL    TerminateTransfer(PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT pDmaPDDTransfer );
    virtual BOOL    PollingForTransferDone();
    virtual BOOL    PollingTranferRemaining(PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT pDmaPDDTransfer , DWORD* dwRemaining);
private:
    operator=(const Dma8237Channel& /*rhs*/) { ASSERT(FALSE); }
    static BOOL     CanArmDmaStub(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext) {
        if (pDmaPDDChannelContext)
            return ((Dma8237Channel *)pDmaPDDChannelContext)->CanArmDma();
        else
            return FALSE;
    }
    static BOOL    StartDmaTransferStub(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext) {
        if (pDmaPDDChannelContext)
            return ((Dma8237Channel *)pDmaPDDChannelContext)->StartDmaTransfer();
        else
            return FALSE;
    }
    static BOOL ArmTransferStub (PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext,PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT pDmaPDDTransfer ){
        if (pDmaPDDChannelContext)
            return ((Dma8237Channel *)pDmaPDDChannelContext)->ArmTransfer(pDmaMDDTransfer,pDmaPDDTransfer);
        else
            return FALSE;
    }
    static BOOL    TerminateTransferStub(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext,PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT pDmaPDDTransfer ) {
        if (pDmaPDDChannelContext)
            return ((Dma8237Channel *)pDmaPDDChannelContext)->TerminateTransfer(pDmaMDDTransfer,pDmaPDDTransfer);
        else
            return FALSE;
    }
    static BOOL    PollingForTransferDoneStub(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext) {
        if (pDmaPDDChannelContext)
            return ((Dma8237Channel *)pDmaPDDChannelContext)->PollingForTransferDone();
        else
            return FALSE;
    }
    static BOOL    PollingTranferRemainingStub(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext,PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT pDmaPDDTransfer , DWORD* dwRemaining) {
        if (pDmaPDDChannelContext)
            return ((Dma8237Channel *)pDmaPDDChannelContext)->PollingTranferRemaining(pDmaMDDTransfer,pDmaPDDTransfer,dwRemaining);
        else
            return FALSE;
    }

};


