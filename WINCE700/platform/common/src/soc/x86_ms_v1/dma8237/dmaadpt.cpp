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

    dmaAdpt.cpp

Abstract:  

    Contains DMA support routines for Adapter.
    
Functions:

Notes:

--*/
#include <windows.h>
#include <oaldma.h>
#include <dma_adpt.hpp>
#include <dma_chn.hpp>
#include <dma_tran.hpp>
#include <cedma.h>
#include <dmamif.h>
#include <cregedit.h>

DmaAdapter::DmaAdapter(LPCTSTR lpActiveRegPath)
:   CMiniThread(  0, TRUE )
{
    m_pDmaChannelArray = NULL;
    m_pdmaPddAdapterContext = CreateDMAPDDContext(lpActiveRegPath, NULL );
    m_dwAdapterIndex = 0;
    m_dwNumOfChannel = 0;
    
    m_hResetOnResumed = NULL;
    CRegistryEdit deviceKey(lpActiveRegPath);
    if (deviceKey.IsKeyOpened()) {
        DWORD dwReinitOnResume = 0;
        if (deviceKey.GetRegValue(REG_DMA_RESETONRESUME,(PBYTE)&dwReinitOnResume,sizeof(dwReinitOnResume)) && 
                dwReinitOnResume!=0) {
            m_hResetOnResumed = CreateEvent(NULL, FALSE,FALSE, NULL);
            ASSERT(m_hResetOnResumed!=NULL);
        };
        DWORD dwPriority256 = 0; 
        if (deviceKey.GetRegValue(REG_DMA_PRIORITY_VAL_NAME,(PBYTE)&dwPriority256, sizeof(dwPriority256)) && 
                dwPriority256 >=5 )  {
            CeSetPriority(dwPriority256-1);
        }
        else
            CeSetPriority(DEFAULT_DMA_THREAD_PRIORITY-1);
    }
    
    if (m_pdmaPddAdapterContext) {
        m_dwAdapterIndex = m_pdmaPddAdapterContext->dwAdapterIndex;
        m_dwNumOfChannel = m_pdmaPddAdapterContext->dwNumOfChannel ;
    }
}

DmaAdapter::~DmaAdapter()
{
    WaitThreadComplete(5000);
    if (m_pdmaPddAdapterContext)
        DeleteDMAPDDContext (m_pdmaPddAdapterContext );
    
    if (m_pDmaChannelArray) {
        Lock();
        for (DWORD dwIndex = 0 ; dwIndex < m_dwNumOfChannel; dwIndex ++ ) {
            if (m_pDmaChannelArray[dwIndex] != NULL) {
                delete m_pDmaChannelArray[dwIndex];
                m_pDmaChannelArray[dwIndex] = NULL;
             }
        }
        free( m_pDmaChannelArray);
        Unlock();
    }
    if (m_hResetOnResumed)
        CloseHandle(m_hResetOnResumed);
}
BOOL DmaAdapter::Init()
{
    if (m_pdmaPddAdapterContext == NULL ||
        ((m_pdmaPddAdapterContext->lpOALAllocateCommonBuffer!=NULL)!= (m_pdmaPddAdapterContext->lpOALFreeCommonBuffer!=NULL)) ||
        m_pdmaPddAdapterContext->lpAllocateChannel== NULL ||
        m_pdmaPddAdapterContext->lpFreeDmaChannel == NULL
    ) {
        ASSERT(FALSE);
        return FALSE;
    }
    
    if (m_hResetOnResumed) {
        ThreadStart();
    }        
    
    if (m_dwNumOfChannel && m_dwNumOfChannel<0x1000)
        m_pDmaChannelArray = (DmaChannel **)malloc(m_dwNumOfChannel* sizeof(DmaChannel *));
    if (m_pDmaChannelArray) {
        for (DWORD dwIndex = 0 ; dwIndex < m_dwNumOfChannel; dwIndex ++ ) {
            m_pDmaChannelArray [dwIndex] = NULL;
        }
    }
    
    if (m_pDmaChannelArray)
        return TRUE;
    else
        return FALSE;
}

void DmaAdapter::PowerMgmtCallback(BOOL fOff)
{
    PREFAST_ASSERT(m_pdmaPddAdapterContext);
    if ( m_hResetOnResumed ) {
        if (!fOff)
            SetEvent(m_hResetOnResumed);
    }
    else 
    if (m_pdmaPddAdapterContext->lpPowerMgmtCallback) {
        m_pdmaPddAdapterContext->lpPowerMgmtCallback(m_pdmaPddAdapterContext,fOff);
    }
}
DWORD  DmaAdapter::ThreadRun()// Resume Reset Thread
{
    while (!m_bTerminated && m_hResetOnResumed!=NULL) {
        if ((WaitForSingleObject(m_hResetOnResumed,INFINITE) == WAIT_OBJECT_0) && !m_bTerminated) {
            Lock();
            if (m_pDmaChannelArray) {
                for (DWORD dwIndex = 0 ; dwIndex < m_dwNumOfChannel; dwIndex ++ ) {
                    if (m_pDmaChannelArray[dwIndex] != NULL) {
                        delete m_pDmaChannelArray[dwIndex];
                        m_pDmaChannelArray[dwIndex] = NULL;
                     }
                }
                if (m_pdmaPddAdapterContext && m_pdmaPddAdapterContext->lpPowerOnReset) {
                    m_pdmaPddAdapterContext->lpPowerOnReset(m_pdmaPddAdapterContext);
                }
            }
            Unlock();
        }
    }
    return 0;
}

BOOL DmaAdapter::GetDmaAdapter (
    IN PDEVICE_DMA_REQUIREMENT_INFO pDeviceDmaRequirementInfo, //  Device Description. It descript what is capable Adapt supported.
    IN OUT PCE_DMA_ADAPTER pDmaAdapter
)
{
    PREFAST_ASSERT(m_pdmaPddAdapterContext);
    if (m_pdmaPddAdapterContext->lpGetDmaAdapter) {
        return m_pdmaPddAdapterContext->lpGetDmaAdapter(m_pdmaPddAdapterContext,pDeviceDmaRequirementInfo,pDmaAdapter);
    }
    else  if (pDeviceDmaRequirementInfo!=NULL && pDmaAdapter!=NULL) {
        CE_DMA_ADAPTER DmaAdapterInfo;
        //
        // Generic implementation assume all channel has same capability
        // Low Channel number has highest priority.
        //
        DmaAdapterInfo.Size = sizeof(DmaAdapterInfo);
        DmaAdapterInfo.DemandMode = pDeviceDmaRequirementInfo->DemandMode;
        DmaAdapterInfo.BusMaster = pDeviceDmaRequirementInfo->BusMaster ;
        DmaAdapterInfo.DmaSpeed = pDeviceDmaRequirementInfo->DmaSpeed ;
        DmaAdapterInfo.CeDmaWidth = pDeviceDmaRequirementInfo->CeDmaWidth ;
        DmaAdapterInfo.DmaSpeed = pDeviceDmaRequirementInfo->DmaSpeed ;
        DmaAdapterInfo.DmaAdapter = m_dwAdapterIndex;
        DmaAdapterInfo.InterfaceType = pDeviceDmaRequirementInfo->InterfaceType ;
        DmaAdapterInfo.BusNumber = pDeviceDmaRequirementInfo->BusNumber ;
        DmaAdapterInfo.DeviceLocation= pDeviceDmaRequirementInfo->DeviceLocation ;
        DmaAdapterInfo.dwFlags = UpdateFlags(pDeviceDmaRequirementInfo->dwFlags);
        //DmaAdapterInfo.NumberOfMapRegisters = GetNumOfHardwareMappingRegister();
        DmaAdapterInfo.MaximumSizeOfMappingRegister = GetMaximunSizeOfEachRegister() ;
        DmaAdapterInfo.AddressBoundary = GetMaximunAddressBoundary();
        DmaAdapterInfo.AddressAlignment = GetAddressAligment();
        GetDmaSystemMemoryRange(&DmaAdapterInfo.DmaSystemMemoryRangeStart, &DmaAdapterInfo.DmaSystemMemoryRangeLength) ;
        *pDmaAdapter = DmaAdapterInfo;
        return TRUE;
    }
    else
        return FALSE;
}


DmaChannel * DmaAdapter::AllocaAdapterChannel(
        IN PCE_DMA_ADAPTER     pDmaAdapter,
        IN ULONG            ulRequestedChannel,
        IN ULONG            ulAddressSpace,
        IN PHYSICAL_ADDRESS phDeviceIoAddress
    )
{
    DmaChannel *  pFoundChannel = NULL ;
    if (pDmaAdapter && pDmaAdapter->Size >= sizeof(CE_DMA_ADAPTER) && !pDmaAdapter->BusMaster) {
        CE_DMA_ADAPTER dmaAdapterInfo = *pDmaAdapter ;
        Lock();
        if (ulRequestedChannel == DMA_CHANNEL_ANY) {
            for (DWORD dwIndex=0; dwIndex< m_dwNumOfChannel ; dwIndex++) {
                if (m_pDmaChannelArray[dwIndex] == NULL && IsChannelSuitable(dmaAdapterInfo,dwIndex)) {
                    ulRequestedChannel = dwIndex;
                    break;
                }
            }
        }
        if (IsChannelSuitable(dmaAdapterInfo,ulRequestedChannel) && ulRequestedChannel < m_dwNumOfChannel && m_pDmaChannelArray[ulRequestedChannel] == NULL ) {
            pFoundChannel = new DmaChannel(*this, &dmaAdapterInfo,ulRequestedChannel ,ulAddressSpace,phDeviceIoAddress);
            if (pFoundChannel!=NULL && pFoundChannel->Init()) {
                m_pDmaChannelArray[ulRequestedChannel] = pFoundChannel ;
                m_pDmaChannelArray[ulRequestedChannel]->AddRef();
            }
            else if (pFoundChannel) {
                delete pFoundChannel;
                pFoundChannel = NULL;
                SetLastError(ERROR_INVALID_PARAMETER);
            }
            else 
                SetLastError(ERROR_OUTOFMEMORY);
        }
        else
            SetLastError(ERROR_RESOURCE_NOT_AVAILABLE);
            
        Unlock();
    }
    ASSERT(pFoundChannel);
    return pFoundChannel;
}
BOOL DmaAdapter::FreeDmaChannel(DmaChannel *pChannel)
{
    BOOL fReturn = FALSE;
    Lock();
    if (pChannel!=NULL) {
        for (DWORD dwIndex = 0; dwIndex<m_dwNumOfChannel; dwIndex++) {
            if (m_pDmaChannelArray[dwIndex] == pChannel) {
                fReturn = TRUE;
                m_pDmaChannelArray[dwIndex]->DeRef();
                m_pDmaChannelArray[dwIndex] = NULL;
                break;
            }
            
        }
    }
    Unlock();
    ASSERT(fReturn);
    return fReturn;
}
DmaChannel * DmaAdapter::GetChannelRef(DWORD dwChannelIndex)
{
    DmaChannel *  pFoundChannel = NULL ;
    Lock();
    if (dwChannelIndex< m_dwNumOfChannel && m_pDmaChannelArray[dwChannelIndex]!=NULL) {
        pFoundChannel = m_pDmaChannelArray[dwChannelIndex];
        pFoundChannel->AddRef();
    }
    Unlock();
    return pFoundChannel ;
}

PVOID DmaAdapter::OALAllocateCommonBuffer( PCE_DMA_ADAPTER pDmaAdapter, ULONG Length, PPHYSICAL_ADDRESS LogicalAddress, BOOLEAN CacheEnabled)
{
    ASSERT(m_pdmaPddAdapterContext);
    if (m_pdmaPddAdapterContext->lpOALAllocateCommonBuffer && 
            pDmaAdapter && pDmaAdapter->Size >= sizeof(CE_DMA_ADAPTER) && Length && !pDmaAdapter->BusMaster) {
        return m_pdmaPddAdapterContext->lpOALAllocateCommonBuffer(m_pdmaPddAdapterContext,
            pDmaAdapter, Length, LogicalAddress, CacheEnabled);
    }
    else {
        ULONG ulProtect = 0;
        PHYSICAL_ADDRESS PhysicalAddress;
        PHYSICAL_ADDRESS VirtualAddress;

        // Assumption: we can allocate from anywhere in system memory (ignoring
        // cached/uncached for the moment) and it'll be within reach of the DMA
        // controller through the host bridge mapping register(s).  If we need
        // to place restrictions on the allocation based on the range mapped by
        // the host bridge, this routine will need to change.
        //
        if (!pDmaAdapter|| !Length || !LogicalAddress)
            return(NULL);

        memset(&PhysicalAddress, 0, sizeof(PHYSICAL_ADDRESS));
        memset(&VirtualAddress, 0, sizeof(PHYSICAL_ADDRESS));

        ulProtect = PAGE_READWRITE | ((CacheEnabled == TRUE) ? 0 : PAGE_NOCACHE);

        // Allocate buffer with default alignment (64KB aligned).
        //
        VirtualAddress.LowPart = (ULONG)AllocPhysMem(Length,
                                                     ulProtect,
                                                     0,
                                                     0,
                                                     &PhysicalAddress.LowPart);

        if (!VirtualAddress.LowPart)
        {
            RETAILMSG(1, (TEXT("HalAllocateCommonBuffer: memory allocation failed (error = 0x%x).\r\n"), GetLastError()));
            return(NULL);
        }

        // Translate physical address to logical (bus-relative) address.
        //
        if (!HalTranslateSystemAddress(pDmaAdapter->InterfaceType, pDmaAdapter->BusNumber, PhysicalAddress, LogicalAddress))
        {
            FreePhysMem((LPVOID)VirtualAddress.LowPart);
            RETAILMSG(1, (TEXT("HalAllocateCommonBuffer: failed to get logical address of buffer.\r\n")));
            return(NULL);
        }

        return((PVOID)VirtualAddress.LowPart);
    }
}
BOOL DmaAdapter::OALFreeCommonBuffer(PCE_DMA_ADAPTER pDmaAdapter, ULONG Length, PHYSICAL_ADDRESS LogicalAddress, PVOID VirtualAddress, BOOLEAN CacheEnabled)
{
    ASSERT(m_pdmaPddAdapterContext);
    if (m_pdmaPddAdapterContext->lpOALFreeCommonBuffer &&
            pDmaAdapter && pDmaAdapter->Size >= sizeof(CE_DMA_ADAPTER) && Length && !pDmaAdapter->BusMaster) {
        return m_pdmaPddAdapterContext->lpOALFreeCommonBuffer(m_pdmaPddAdapterContext,pDmaAdapter,Length,LogicalAddress,VirtualAddress,CacheEnabled);
    }
    else {
        if (!VirtualAddress)
            return FALSE;
        FreePhysMem((LPVOID)VirtualAddress);
        return TRUE;
    }
}
PDMA_PDD_CHANNEL_CONTEXT DmaAdapter::AllocateChannelPdd (DmaChannel& DmaChannel)
{
    ASSERT(m_pdmaPddAdapterContext);
    if (m_pdmaPddAdapterContext->lpAllocateChannel)
        return m_pdmaPddAdapterContext->lpAllocateChannel(m_pdmaPddAdapterContext,&DmaChannel);
    else
        return NULL;
}
void DmaAdapter::FreeChannelPdd(PDMA_PDD_CHANNEL_CONTEXT pDmaPddChannelContext)
{
    ASSERT(m_pdmaPddAdapterContext);
    if (m_pdmaPddAdapterContext->lpFreeDmaChannel && pDmaPddChannelContext)
        m_pdmaPddAdapterContext->lpFreeDmaChannel(m_pdmaPddAdapterContext,pDmaPddChannelContext);
}


BOOL DmaAdapter::IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,PDWORD pdwActualOut)
{
    BOOL bRet = FALSE;
    SetLastError(ERROR_INVALID_PARAMETER) ;
    switch (dwCode) {
      case IOCTL_CE_DMA_OALGETDMAADAPTER:
        if (pBufIn &&  dwLenIn>= sizeof(DEVICE_DMA_REQUIREMENT_INFO) &&
                pBufOut!=NULL && dwLenOut >= sizeof (CE_DMA_ADAPTER)) {
            bRet= GetDmaAdapter((PDEVICE_DMA_REQUIREMENT_INFO)pBufIn,(PCE_DMA_ADAPTER)pBufOut);
        }
        break;
      case IOCTL_CE_DMA_ALLOCATECHANNEL:
        if (pBufIn && dwLenIn>= sizeof(CE_DMA_OALALLOCADAPTERCHANNEL) &&
                pBufOut && dwLenOut>=sizeof(DMA_CHANNEL_HANDLE)) {
            PCE_DMA_OALALLOCADAPTERCHANNEL pDmaAllocAdapterChannel = (PCE_DMA_OALALLOCADAPTERCHANNEL)pBufIn ;
            DMA_CHANNEL_HANDLE hChannelHandle = 
                AllocaAdapterChannel(&pDmaAllocAdapterChannel->DmaAdapter, 
                    pDmaAllocAdapterChannel->ulRequestedChannel, 
                    pDmaAllocAdapterChannel->ulAddressSpace, 
                    pDmaAllocAdapterChannel->phDeviceIoAddress);
            if (hChannelHandle) {
                bRet = TRUE;
                *(DMA_CHANNEL_HANDLE *)pBufOut = hChannelHandle;
                if (pdwActualOut)
                    *pdwActualOut = sizeof(DMA_CHANNEL_HANDLE);
            }
        }
        break;
      case IOCTL_CE_DMA_FREEDMACHANNEL:
        if (pBufIn && dwLenIn>= sizeof(DMA_CHANNEL_HANDLE)) {
            DMA_CHANNEL_HANDLE Channel = * (DMA_CHANNEL_HANDLE *)pBufIn;
            if (Channel!=NULL && ((DmaChannel *)Channel)->GetChanelTag()==DMA_CHANNEL_TAG  ) {
                bRet = FreeDmaChannel((DmaChannel *)Channel) ;
            }
        }
        break;
      case IOCTL_CE_DMA_ISSUEDMATRANSFER:
        if (pBufIn && dwLenIn>= sizeof(CE_DMA_ISSUEDMATRANSFER) &&
                pBufOut && dwLenOut>=sizeof(DMA_TRANSFER_HANDLE)) {
            PCE_DMA_ISSUEDMATRANSFER pDmaIssueDmaTransfer = (PCE_DMA_ISSUEDMATRANSFER)pBufIn;
            DmaChannel * pDmaChannel = (DmaChannel *)pDmaIssueDmaTransfer->hDmaChannel ;
            if (pDmaChannel->GetChanelTag()==DMA_CHANNEL_TAG) {
                DmaChannel *pRefDmaChannel = GetChannelRef(pDmaChannel->GetChannelIndex());
                if (pRefDmaChannel) {
                    __try {
                        bRet =pRefDmaChannel->DMAIssueTransfer((DMA_TRANSFER_HANDLE *)pBufOut,
                            pDmaIssueDmaTransfer->dwFlags, 
                            pDmaIssueDmaTransfer->SystemMemoryPhysicalAddress,
                            pDmaIssueDmaTransfer->CurrentVa, 
                            pDmaIssueDmaTransfer->Length,
                            pDmaIssueDmaTransfer->hNotifyHandle, 
                            pDmaIssueDmaTransfer->NotifyContext1, 
                            pDmaIssueDmaTransfer->NotifyContext2,
                            pDmaIssueDmaTransfer->phOpDeviceIoAddress);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        bRet = FALSE;
                    };
                    pRefDmaChannel->DeRef();
                }
            }
        }
        break;
      case IOCTL_CE_DMA_ISSUEMULTIDMATRANSFER:
        if (pBufIn && dwLenIn>= sizeof(CE_DMA_ISSUEMULTIDMATRANSFER) &&
                pBufOut && dwLenOut>=sizeof(DMA_TRANSFER_HANDLE)) {
            PCE_DMA_ISSUEMULTIDMATRANSFER pIssueMultiDmaTransfer = (PCE_DMA_ISSUEMULTIDMATRANSFER)pBufIn;
            DmaChannel * pDmaChannel = (DmaChannel *)pIssueMultiDmaTransfer->hDmaChannel ;
            if (pDmaChannel->GetChanelTag()==DMA_CHANNEL_TAG) {
                DmaChannel *pRefDmaChannel = GetChannelRef(pDmaChannel->GetChannelIndex());
                if (pRefDmaChannel) {
                    __try {
                        bRet =pRefDmaChannel->DMAIssueMultipleBufferTransfer((DMA_TRANSFER_HANDLE *)pBufOut,
                            pIssueMultiDmaTransfer->dwFlags,
                            pIssueMultiDmaTransfer->dwNumOfTransfer,
                            pIssueMultiDmaTransfer->pDmaBufferBlock,
                            pIssueMultiDmaTransfer->hNotifyHandle, 
                            pIssueMultiDmaTransfer->NotifyContext1, 
                            pIssueMultiDmaTransfer->NotifyContext2,
                            pIssueMultiDmaTransfer->phOpDeviceIoAddress);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        bRet = FALSE;
                    };
                    pRefDmaChannel->DeRef();
                }
            }
        }
        break;
      case IOCTL_CE_DMA_OALCANCELDMATRANSFER:
      case IOCTL_CE_DMA_OALCLOSEDMATRANSFER:
      case IOCTL_CE_DMA_STARTTRANSFER:
        if (pBufIn && dwLenIn >= sizeof(DMA_TRANSFER_HANDLE)) {
            DmaTransfer* pDmaTransfer = (DmaTransfer* )*(PDMA_TRANSFER_HANDLE)pBufIn;
            if (pDmaTransfer && pDmaTransfer->GetTag() == DMA_TRANSFER_TAG ) {
                if (pDmaTransfer->GetDmaChannel().GetChanelTag() == DMA_CHANNEL_TAG) {
                    DmaChannel *pRefDmaChannel = GetChannelRef(pDmaTransfer->GetDmaChannel().GetChannelIndex());
                    if (pRefDmaChannel) {
                        __try {
                            switch (dwCode) {
                              case IOCTL_CE_DMA_OALCANCELDMATRANSFER:
                                bRet = pRefDmaChannel->AbortTransfer(*(PDMA_TRANSFER_HANDLE)pBufIn);
                                break;
                              case IOCTL_CE_DMA_OALCLOSEDMATRANSFER:
                                bRet =  pRefDmaChannel->DMACloseTransfer(*(PDMA_TRANSFER_HANDLE)pBufIn);
                                break;
                              case IOCTL_CE_DMA_STARTTRANSFER:
                                bRet = pRefDmaChannel->StartTransfer(*(PDMA_TRANSFER_HANDLE)pBufIn);
                                break;
                            }
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER) {
                            bRet = FALSE;
                        };
                        pRefDmaChannel->DeRef();
                    }
                }
            }
        }
        break;
      case IOCTL_CE_DMA_GETDMASTATUS:
        if (pBufIn && dwLenIn >= sizeof(DMA_TRANSFER_HANDLE) &&
                pBufOut && dwLenOut>= sizeof(CE_DMA_GETDMASTATUS)) {
            DMA_TRANSFER_HANDLE hDmaTransferHandle = *(PDMA_TRANSFER_HANDLE)pBufIn ;
            PCE_DMA_GETDMASTATUS pCeDmaGetStatus = (PCE_DMA_GETDMASTATUS) pBufOut;
            DmaTransfer* pDmaTransfer = (DmaTransfer* )hDmaTransferHandle;
            if (pDmaTransfer->GetTag() == DMA_TRANSFER_TAG ) {
                if (pDmaTransfer->GetDmaChannel().GetChanelTag() == DMA_CHANNEL_TAG) {
                    DmaChannel *pRefDmaChannel = GetChannelRef(pDmaTransfer->GetDmaChannel().GetChannelIndex());
                    if (pRefDmaChannel) {
                        DmaTransfer * pRefDmaTransfer = pRefDmaChannel->TransferAddRef(hDmaTransferHandle);
                        if (pRefDmaTransfer) {
                            bRet = pRefDmaTransfer->GetStatus(&pCeDmaGetStatus->CompletedLength,&pCeDmaGetStatus->CompletionCode);
                            pRefDmaTransfer->DeRef();
                        }
                        pRefDmaChannel->DeRef();
                    }
                }
            }
            
        }
        break;
      case IOCTL_CE_DMA_GETDMACONTEXTS:
        if (pBufIn && dwLenIn >= sizeof(DMA_TRANSFER_HANDLE) &&
                pBufOut && dwLenOut>= sizeof(CE_DMA_GETDMASTATUS)) {
            DMA_TRANSFER_HANDLE hDmaTransferHandle = *(PDMA_TRANSFER_HANDLE)pBufIn ;
            PCE_DMA_GETDMACONTEXTS pCeDmaGetContexts = (PCE_DMA_GETDMACONTEXTS) pBufOut;
            DmaTransfer* pDmaTransfer = (DmaTransfer* )hDmaTransferHandle;
            if (pDmaTransfer->GetTag() == DMA_TRANSFER_TAG ) {
                if (pDmaTransfer->GetDmaChannel().GetChanelTag() == DMA_CHANNEL_TAG) {
                    DmaChannel *pRefDmaChannel = GetChannelRef(pDmaTransfer->GetDmaChannel().GetChannelIndex());
                    if (pRefDmaChannel) {
                        DmaTransfer * pRefDmaTransfer = pRefDmaChannel->TransferAddRef(hDmaTransferHandle);
                        if (pRefDmaTransfer) {
                            bRet = pRefDmaTransfer->GetContexts(&pCeDmaGetContexts->dmaContext1,&pCeDmaGetContexts->dmaContext2);
                            pRefDmaTransfer->DeRef();
                        }
                        pRefDmaChannel->DeRef();
                    }
                }
            }
            
        }
        break;
      case IOCTL_CE_DMA_ISSUERAWDMATRANSFER:
        if (pBufIn && dwLenIn>=sizeof(CE_DMA_ISSUERAWDMATRANSFER) &&
                pBufOut && dwLenOut>=sizeof(DMA_TRANSFER_HANDLE)) {
            PCE_DMA_ISSUERAWDMATRANSFER pceDmaIssueRawTransfer = (PCE_DMA_ISSUERAWDMATRANSFER)pBufIn;
            DmaChannel * pDmaChannel = (DmaChannel *)pceDmaIssueRawTransfer->hDmaChannel ;
            if (pDmaChannel->GetChanelTag()==DMA_CHANNEL_TAG) {
                DmaChannel *pRefDmaChannel = GetChannelRef(pDmaChannel->GetChannelIndex());
                if (pRefDmaChannel) {
                    __try {
                        bRet =pRefDmaChannel->DMAIssueRawTransfer((DMA_TRANSFER_HANDLE *)pBufOut,
                                pceDmaIssueRawTransfer->lpInPtr,
                                pceDmaIssueRawTransfer->nInLen,
                                pceDmaIssueRawTransfer->hNotifyHandle,
                                pceDmaIssueRawTransfer->NotifyContext1,
                                pceDmaIssueRawTransfer->NotifyContext2);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        bRet = FALSE;
                    };
                    pRefDmaChannel->DeRef();
                }
            }

        }
        break;
      case IOCTL_CE_DMA_RAWDMATRANSFERCTRL:
        if (pBufIn && dwLenIn>=sizeof(CE_DMA_RAWDMATRANSFERCTRL)) {
            PCE_DMA_RAWDMATRANSFERCTRL pDmaRawTranCtrl = (PCE_DMA_RAWDMATRANSFERCTRL)pBufIn ;
            DmaTransfer* pDmaTransfer = (DmaTransfer* ) pDmaRawTranCtrl->hDmaHandle ;
            if (pDmaTransfer->GetTag() == DMA_TRANSFER_TAG ) {
                if (pDmaTransfer->GetDmaChannel().GetChanelTag() == DMA_CHANNEL_TAG) {
                    DmaChannel *pRefDmaChannel = GetChannelRef(pDmaTransfer->GetDmaChannel().GetChannelIndex());
                    if (pRefDmaChannel) {
                        DmaTransfer * pRefDmaTransfer = pRefDmaChannel->TransferAddRef(pDmaRawTranCtrl->hDmaHandle);
                        if (pRefDmaTransfer) {
                            __try {
                            bRet = pRefDmaTransfer->IoControl(
                                pDmaRawTranCtrl->dwIoControl, 
                                pDmaRawTranCtrl->lpInPtr,
                                pDmaRawTranCtrl->nInLen,
                                pBufOut,dwLenOut,pdwActualOut) ;                            
                            }
                            __except (EXCEPTION_EXECUTE_HANDLER) {
                                bRet = FALSE;
                            };
                            pRefDmaTransfer->DeRef();
                        }
                        pRefDmaChannel->DeRef();
                    }
                }
            }
        }
        break;
      case IOCTL_CE_DMA_DMAALLOCBUFFER:
        if (pBufIn && dwLenIn>=sizeof(CE_DMA_DMAALLOCBUFFER_IN) &&
                pBufOut && dwLenOut>=sizeof(CE_DMA_DMAALLOCBUFFER_OUT)) {
            PCE_DMA_DMAALLOCBUFFER_IN pDmaBufferIn = (PCE_DMA_DMAALLOCBUFFER_IN)pBufIn;
            PCE_DMA_DMAALLOCBUFFER_OUT pDmaBufferOut = (PCE_DMA_DMAALLOCBUFFER_OUT)pBufOut;
            pDmaBufferOut->VirtualAddress = OALAllocateCommonBuffer(&pDmaBufferIn->Adapter,pDmaBufferIn->Length, 
                &pDmaBufferOut->PhysicalAddress,pDmaBufferIn->CacheEnabled);
            if (pDmaBufferOut->VirtualAddress!=NULL) {
                bRet = TRUE;
                if (pdwActualOut)
                    *pdwActualOut = sizeof(CE_DMA_DMAALLOCBUFFER_OUT);
            }
        }
        break;
      case IOCTL_CE_DMA_DMAFREEBUFFER:
        if (pBufIn && dwLenIn>=sizeof(CE_DMA_DMAFREEBUFFER) ) {
            PCE_DMA_DMAFREEBUFFER pDmaFreeBuffer = (PCE_DMA_DMAFREEBUFFER)pBufIn;
            bRet = OALFreeCommonBuffer(&pDmaFreeBuffer->Adapter,pDmaFreeBuffer->Length,pDmaFreeBuffer->LogicalAddress,pDmaFreeBuffer->VirtualAddress,pDmaFreeBuffer->CacheEnabled);
        }
        break;
      default:
        bRet = FALSE;
        SetLastError(ERROR_NOT_SUPPORTED) ;
        break;
    }
    return bRet;
}

