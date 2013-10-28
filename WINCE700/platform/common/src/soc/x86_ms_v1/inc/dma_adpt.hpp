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
#include <CRefCon.h>
#include <cmthread.h>
#include <dmamif.h>

class DmaAdapter;
class DmaChannel;
#include <dma_chn.hpp>
#include <dma_tran.hpp>

#define REG_DMA_RESETONRESUME TEXT("ResetOnResume")
#define REG_DMA_PRIORITY_VAL_NAME TEXT("Priority256")
#define DEFAULT_DMA_THREAD_PRIORITY 110

class DmaAdapter : public CLockObject, public CMiniThread {
    friend class DmaChannel;
public:
    DmaAdapter(LPCTSTR lpActiveRegPath);
    virtual ~DmaAdapter();
    virtual BOOL Init();
// PM entry.
    void PowerMgmtCallback(BOOL fOff);
// IO Entry.
    BOOL IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,PDWORD pdwActualOut);
// MDD first own the default implementation, If it is not fit, PDD can overwrite this.
    BOOL GetDmaAdapter (
        IN PDEVICE_DMA_REQUIREMENT_INFO pDeviceDmaRequirementInfo, //  Device Description. It descript what is capable Adapt supported.
        IN OUT PCE_DMA_ADAPTER pDmaAdapter
    );
    DmaChannel * AllocaAdapterChannel(
        IN PCE_DMA_ADAPTER     pDmaAdapter,
        IN ULONG            ulRequestedChannel,
        IN ULONG            ulAddressSpace,
        IN PHYSICAL_ADDRESS phDeviceIoAddress
    );
    BOOL FreeDmaChannel(DmaChannel *pChannel);
    DmaChannel * GetChannelRef(DWORD dwChannelIndex);
    PVOID   OALAllocateCommonBuffer( PCE_DMA_ADAPTER DmaAdapter, ULONG Length, PPHYSICAL_ADDRESS LogicalAddress, BOOLEAN CacheEnabled);
    BOOL    OALFreeCommonBuffer(PCE_DMA_ADAPTER DmaAdapter, ULONG Length, PHYSICAL_ADDRESS LogicalAddress, PVOID VirtualAddress, BOOLEAN CacheEnabled);    
protected:
//object property.
    DWORD   m_dwAdapterIndex;
    DWORD   m_dwNumOfChannel;
    DmaChannel ** m_pDmaChannelArray;
public:
    DWORD   GetNumOfHardwareMappingRegister() {
        PREFAST_ASSERT(m_pdmaPddAdapterContext);
        return (m_pdmaPddAdapterContext->dwNumOfHardwareMappingRegister);
            
    }
    DWORD   GetMaximunSizeOfEachRegister()  {
        PREFAST_ASSERT(m_pdmaPddAdapterContext);
        return (m_pdmaPddAdapterContext->dwMaximumSizeOfEachRegister);
    }
    DWORD   GetMaximunAddressBoundary() {
        PREFAST_ASSERT(m_pdmaPddAdapterContext);
        return (m_pdmaPddAdapterContext->dwMaximumAddressBoundary) ;
    }
    DWORD   GetAddressAligment() {
        PREFAST_ASSERT(m_pdmaPddAdapterContext);
        return (m_pdmaPddAdapterContext->dwAddressAligment);
    }
    void    GetDmaSystemMemoryRange(PPHYSICAL_ADDRESS pPhysAddr, PULONG pdwLength) {
        ASSERT(m_pdmaPddAdapterContext);
        if (pPhysAddr)
            pPhysAddr->QuadPart = m_pdmaPddAdapterContext->dwDmaSystemMemoryRangeStart;
        if (pdwLength)
            *pdwLength = m_pdmaPddAdapterContext->dwDmaSystemMemoryRangeLength;
    }
protected:
    BOOL    IsChannelSuitable(CE_DMA_ADAPTER& dmaAdapterInfo,DWORD dwChannelIndex) { 
        PREFAST_ASSERT(m_pdmaPddAdapterContext);
        if (m_pdmaPddAdapterContext->lpIsChannelSuitable)
            return m_pdmaPddAdapterContext->lpIsChannelSuitable(m_pdmaPddAdapterContext,&dmaAdapterInfo,dwChannelIndex);
        else
            return TRUE;
        
    };
    DWORD   UpdateFlags(DWORD dwFlags) { 
        PREFAST_ASSERT(m_pdmaPddAdapterContext);
        if (m_pdmaPddAdapterContext->lpUpdateFlags)
            return m_pdmaPddAdapterContext->lpUpdateFlags(m_pdmaPddAdapterContext,dwFlags);
        else
            return dwFlags;
    }
    PDMA_PDD_CHANNEL_CONTEXT AllocateChannelPdd (DmaChannel& DmaChannel);
    void FreeChannelPdd(PDMA_PDD_CHANNEL_CONTEXT pDmaPddChannelContext);

public:
    BOOL   IsPhysAddrSupported(PHYSICAL_ADDRESS BufferPhysAddress,DWORD dwLength) { 
        PREFAST_ASSERT(m_pdmaPddAdapterContext);
        if (m_pdmaPddAdapterContext->lpIsPhysAddrSupported)
            return m_pdmaPddAdapterContext->lpIsPhysAddrSupported(m_pdmaPddAdapterContext,BufferPhysAddress,dwLength);
        else
            return TRUE; 
    }
private:
    PDMA_PDD_ADAPTER_CONTEXT m_pdmaPddAdapterContext;
    HANDLE              m_hResetOnResumed;
    virtual DWORD       ThreadRun() ;

};

