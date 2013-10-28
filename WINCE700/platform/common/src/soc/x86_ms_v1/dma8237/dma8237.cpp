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
#include <CRegedit.h>
#include <Dma8237.hpp>
#include "cphysmem.hpp"
Dma8237Adapter::Dma8237Adapter(LPCTSTR lpcRegistryPath) 
:   CRegistryEdit(lpcRegistryPath)
{
    // Intiaize PDD context.
    dwAdapterIndex = 0 ;
    dwNumOfChannel = 8;
    dwNumOfHardwareMappingRegister = 1;
    dwMaximumSizeOfEachRegister = 0x10000;
    dwMaximumAddressBoundary = 0x10000;
    dwAddressAligment = 1 ;
    dwDmaSystemMemoryRangeStart = 0 ;
    dwDmaSystemMemoryRangeLength =0x1000000L;
    lpGetDmaAdapter = NULL ;
    lpAllocateChannel = AllocaAdapterChannelStub;
    lpFreeDmaChannel = FreeDmaChannelStub ;
    lpPowerMgmtCallback = NULL ;
    lpOALAllocateCommonBuffer = OALAllocateCommonBufferStub;// Optional, can be NULL.
    lpOALFreeCommonBuffer = OALFreeCommonBufferStub;
    lpPowerOnReset = PowerOnResetStub;
    lpIsChannelSuitable = IsChannelSuitableStub;
    lpUpdateFlags = UpdateFlagsStub;// Optional. can be NULL
    lpIsPhysAddrSupported = IsPhysAddrSupportedStub ;
    m_fEnableMemoryToMemory = 0;
    m_dwMaxNumOfChannel = 8 ;
    m_StatusReg1 = m_StatusReg2 = 0;
    m_DmaMemory.dwBase = 0;
    m_DmaMemory.dwLen = 0;
    m_pPhysMem = NULL;
    m_DmaMemoryVirt = NULL;
    if (!(IsKeyOpened() && GetRegValue( DMA_ENABLEMEMORYTOMEMORY,(LPBYTE)&m_fEnableMemoryToMemory, sizeof(m_fEnableMemoryToMemory)))){
        m_fEnableMemoryToMemory = 0;
    }
    DDKWINDOWINFO dwi ;
    if (IsKeyOpened() && GetWindowInfo(&dwi)==ERROR_SUCCESS && dwi.dwNumMemWindows!=0 ) {
        m_DmaMemory = dwi.memWindows[0] ;
    }
    else
        m_DmaMemory.dwLen = 0;
};
BOOL Dma8237Adapter::Init() 
{
    if  (m_DmaMemory.dwBase!=0 && m_DmaMemory.dwLen!=0 ) {
        PHYSICAL_ADDRESS PhysicalAddress = {
            m_DmaMemory.dwBase,0
        };
        ULONGLONG SourcePhys = PhysicalAddress.QuadPart & ~(PAGE_SIZE - 1);
        PREFAST_SUPPRESS( 12011,"Align with Page");
        ULONG   SourceSize = m_DmaMemory.dwLen + (PhysicalAddress.LowPart & (PAGE_SIZE - 1));
        m_DmaMemoryVirt = (PBYTE)VirtualAlloc(0, SourceSize, MEM_RESERVE, PAGE_NOACCESS);
        if (m_DmaMemoryVirt != NULL)
        {
            BOOL bSuccess = VirtualCopy(
                m_DmaMemoryVirt, (PVOID)(SourcePhys >> 8), SourceSize,
                PAGE_PHYSICAL | PAGE_READWRITE | PAGE_NOCACHE);

            if (bSuccess)
            {
                m_DmaMemoryVirt += PhysicalAddress.LowPart & (PAGE_SIZE - 1);
                m_pPhysMem = new CPhysMem(m_DmaMemory.dwLen, m_DmaMemory.dwLen>>12, m_DmaMemoryVirt,(PUCHAR) m_DmaMemory.dwBase);
            }
            else
            {
                VirtualFree(m_DmaMemoryVirt, 0, MEM_RELEASE);
                m_DmaMemoryVirt = NULL;
            }
        }
        
    }
    return (m_pPhysMem!=NULL && m_pPhysMem->InittedOK() && InitializeAdapter(TRUE)) ;
}
Dma8237Adapter::~Dma8237Adapter()
{
    if (m_pPhysMem!=NULL)
        delete m_pPhysMem;
    if (m_DmaMemoryVirt)
        VirtualFree((PVOID)((ULONG)m_DmaMemoryVirt & ~(ULONG)(PAGE_SIZE - 1)), 0, MEM_RELEASE);
    
};
void  Dma8237Adapter::PowerOnReset()
{
    BOOL fReturn = InitializeAdapter(FALSE) ;
    ASSERT(fReturn);
    fReturn = InitializeAdapter(TRUE) ;
    ASSERT(fReturn);
}

void Dma8237Adapter::PowerOnResetStub(PDMA_PDD_ADAPTER_CONTEXT lpDmaPddAdapterContext)
{
    if (lpDmaPddAdapterContext)
        ((Dma8237Adapter*)lpDmaPddAdapterContext)->PowerOnReset();
}

BOOL   Dma8237Adapter::IsPhysAddrSupported(PHYSICAL_ADDRESS BufferPhysAddress,DWORD dwLength)
{
    return (BufferPhysAddress.QuadPart +dwLength  < 0x1000000L);
}
BOOL  Dma8237Adapter::IsPhysAddrSupportedStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,PHYSICAL_ADDRESS BufferPhysAddress,DWORD dwLength)
{
    if (lpPddAdapterContext)
       return ((Dma8237Adapter*)lpPddAdapterContext)->IsPhysAddrSupported( BufferPhysAddress, dwLength);
    else
        return FALSE;
}

PVOID   Dma8237Adapter::OALAllocateCommonBuffer( PCE_DMA_ADAPTER pDmaAdapter, ULONG Length, PPHYSICAL_ADDRESS pLogicalAddress, BOOLEAN /*CacheEnabled*/) 
{
    if (pDmaAdapter && pDmaAdapter->Size >= sizeof(CE_DMA_ADAPTER) && Length && !pDmaAdapter->BusMaster && m_pPhysMem && pLogicalAddress) {
        PBYTE pVirAddr = NULL;
        ASSERT(GetAddressAligment()<=GetMaximunAddressBoundary());
        ASSERT(((GetAddressAligment()-1) & GetAddressAligment())==0);
        ASSERT(((GetMaximunAddressBoundary()-1) & GetMaximunAddressBoundary())==0);
        DWORD dwNatureAlign = Length;
        DWORD dwTemp;
        while ((dwTemp = (dwNatureAlign & (dwNatureAlign-1)))!=0)  {
            dwNatureAlign = dwTemp;
        }
        if (dwNatureAlign < Length) {
            dwNatureAlign <<= 1;
        }
        if (dwNatureAlign < Length) {
            ASSERT(FALSE); // Overflow
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }
        dwNatureAlign = max(dwNatureAlign, GetAddressAligment());
        dwNatureAlign = min (dwNatureAlign, GetMaximunAddressBoundary());
        if (!m_pPhysMem->AllocateMemory( Length, &pVirAddr, CPHYSMEM_FLAG_NOBLOCK,dwNatureAlign)) {
            pVirAddr = NULL;
            SetLastError(ERROR_OUTOFMEMORY);
        }
        else {
            PHYSICAL_ADDRESS SystemAddress = {m_pPhysMem->VaToPa(pVirAddr), 0 };
/* We don't need translate the system address because this is slave dma so, the address is used by DMA Controller .
            if (!HalTranslateSystemAddress(pDmaAdapter->InterfaceType, pDmaAdapter->BusNumber, SystemAddress, pLogicalAddress)) {
                m_pPhysMem->FreeMemory(pVirAddr,m_pPhysMem->VaToPa(pVirAddr),CPHYSMEM_FLAG_NOBLOCK);
                pVirAddr = NULL;
            }
*/
            *pLogicalAddress = SystemAddress;
        }
        return pVirAddr;
    }
    else 
        return NULL;
}
PVOID   Dma8237Adapter::OALAllocateCommonBufferStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,
        PCE_DMA_ADAPTER DmaAdapter, ULONG Length, PPHYSICAL_ADDRESS LogicalAddress, BOOLEAN CacheEnabled)
{
    if (lpPddAdapterContext)
       return ((Dma8237Adapter*)lpPddAdapterContext)->OALAllocateCommonBuffer( DmaAdapter,Length,LogicalAddress,CacheEnabled);
    else
        return NULL;
}

BOOL    Dma8237Adapter::OALFreeCommonBuffer(PCE_DMA_ADAPTER pDmaAdapter, ULONG /*Length*/, PHYSICAL_ADDRESS LogicalAddress, PVOID VirtualAddress, BOOLEAN /*CacheEnabled*/) {
    if (VirtualAddress && pDmaAdapter && !pDmaAdapter->BusMaster && m_pPhysMem ) {
#ifdef DEBUG            
        ULONG ulAddressSpace = 0;
        PHYSICAL_ADDRESS SystemAddress;
        ASSERT(HalTranslateBusAddress(pDmaAdapter->InterfaceType,pDmaAdapter->BusNumber,LogicalAddress,&ulAddressSpace, &SystemAddress));
        ASSERT(SystemAddress.LowPart == m_pPhysMem->VaToPa((PBYTE)VirtualAddress));
#endif
        m_pPhysMem->FreeMemory((PBYTE)VirtualAddress,m_pPhysMem->VaToPa((PBYTE)VirtualAddress),CPHYSMEM_FLAG_NOBLOCK);
        return TRUE;
    }
    else
        return NULL;
}
BOOL     Dma8237Adapter::OALFreeCommonBufferStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,
        PCE_DMA_ADAPTER DmaAdapter, ULONG Length, PHYSICAL_ADDRESS LogicalAddress, PVOID VirtualAddress, BOOLEAN CacheEnabled)
{
    if (lpPddAdapterContext)
        return ((Dma8237Adapter *)lpPddAdapterContext)->OALFreeCommonBuffer(DmaAdapter,Length,LogicalAddress,VirtualAddress,CacheEnabled);
    else
        return FALSE;
}
BOOL    Dma8237Adapter::InitializeAdapter(BOOL fInit) 
{
    if (fInit) {
        WRITE_PORT_UCHAR((PUCHAR)0xd,0); // Mast Reset.
        if (m_dwMaxNumOfChannel> 4) {
            WRITE_PORT_UCHAR((PUCHAR)0xda,0);
        }
        LockHardware();
        BYTE  bMode = (m_fEnableMemoryToMemory?1:0); // Enable MMT for memory to memory transfer.
        WRITE_PORT_UCHAR((PUCHAR)0xf,0xf); 
        WRITE_PORT_UCHAR((PUCHAR)0x8,bMode);
        if (m_dwMaxNumOfChannel>4) {
            WRITE_PORT_UCHAR((PUCHAR)0xde,0xf); 
            WRITE_PORT_UCHAR((PUCHAR)0xd0,bMode);
        }
        for (DWORD dwIndex= 0; dwIndex< m_dwMaxNumOfChannel; dwIndex++) {
            //SetClearRequest(dwIndex,FALSE);
            SetClearMask(dwIndex,TRUE);
        }
        UnlockHardware();
        
    }
    else {
        WRITE_PORT_UCHAR((PUCHAR)0xd,0); // Mast Reset.
        if (m_dwMaxNumOfChannel> 4) {
            WRITE_PORT_UCHAR((PUCHAR)0xda,0);
        }
        BYTE  bMode = 0x4; // COND = 1; disable the controller.
        WRITE_PORT_UCHAR((PUCHAR)0x8,bMode);
        if (m_dwMaxNumOfChannel>4) {
            WRITE_PORT_UCHAR((PUCHAR)0xd0,bMode);
        }

    }
    return TRUE;
}
BOOL    Dma8237Adapter::IsChannelSuitable(CE_DMA_ADAPTER& dmaAdapterInfo,DWORD dwChannelIndex) { 
    if (dwChannelIndex>=m_dwMaxNumOfChannel)
        return FALSE;
    
    if (m_fEnableMemoryToMemory) {
        if (dwChannelIndex == 1 || dwChannelIndex == 5) { // It reserved for memory to memory pair with the channal 0 and 4.
            return FALSE;
        }
    }
    if ((dmaAdapterInfo.dwFlags & DMA_FLAGS_INC_DEVICE_ADDRESS)) {
        if ( m_fEnableMemoryToMemory && (dwChannelIndex == 0 || dwChannelIndex == 4) ) {
            return TRUE;
        }
        else
            return FALSE;
    }
    else 
        return TRUE;
};
BOOL     Dma8237Adapter::IsChannelSuitableStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,CE_DMA_ADAPTER * dmaAdapterInfo,DWORD dwChannelIndex)
{
    if (lpPddAdapterContext && dmaAdapterInfo)
        return ((Dma8237Adapter *)lpPddAdapterContext)->IsChannelSuitable(*dmaAdapterInfo,dwChannelIndex);
    else
        return FALSE;
}

DWORD   Dma8237Adapter::UpdateFlags(DWORD dwFlags) 
{
    if (!m_fEnableMemoryToMemory) {
        dwFlags &= ~(DMA_FLAGS_INC_DEVICE_ADDRESS); // Indicates does not support memory to memory            
    }
    return dwFlags;
}
DWORD    Dma8237Adapter::UpdateFlagsStub(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext, DWORD dwFlags)
{
    if (lpPddAdapterContext )
        return ((Dma8237Adapter *)lpPddAdapterContext)->UpdateFlags(dwFlags);
    else
        return dwFlags;
}
Dma8237Channel * Dma8237Adapter::AllocaAdapterChannel(IN PDMA_MDD_CHANNEL_CONTEXT lpDmaMDDChannelContext)
{
    Dma8237Channel *pChannel = new Dma8237Channel(*this,lpDmaMDDChannelContext);
    if (pChannel && pChannel->Init())
        return pChannel;
    else if (pChannel)
        delete pChannel;
    return NULL;
}
PDMA_PDD_CHANNEL_CONTEXT Dma8237Adapter::AllocaAdapterChannelStub(PDMA_PDD_ADAPTER_CONTEXT pDmaPddAdapterContext, IN PDMA_MDD_CHANNEL_CONTEXT lpDmaMDDChannelContext)
{
    if (pDmaPddAdapterContext)
        return ((Dma8237Adapter *)pDmaPddAdapterContext)->AllocaAdapterChannel(lpDmaMDDChannelContext);
    else
        return NULL;
}
BOOL    Dma8237Adapter::FreeDmaChannel(Dma8237Channel *pDmaChannel)
{
    if (pDmaChannel)
        delete pDmaChannel;
    return TRUE;
}
BOOL Dma8237Adapter::FreeDmaChannelStub(PDMA_PDD_ADAPTER_CONTEXT pDmaPddAdapterContext, IN PDMA_PDD_CHANNEL_CONTEXT lpDmaMDDChannelContext)
{
    if (pDmaPddAdapterContext && lpDmaMDDChannelContext)
        return ((Dma8237Adapter *)pDmaPddAdapterContext)->FreeDmaChannel((Dma8237Channel *)lpDmaMDDChannelContext);
    else
        return NULL;
}
void Dma8237Adapter::WriteAddrCount(DWORD wChannel,DWORD addr, WORD count) 
{
    DWORD dwAddrPort = ( wChannel&3 )*2;
    DWORD dwPagePort = 0x87;
    count --;
    switch(wChannel & 3 ) {
      case 0: default:
        dwPagePort = 0x87;
        break;
      case 1:
        dwPagePort = 0x83;
        break;
      case 2:
        dwPagePort = 0x81;
        break;
      case 3:
        dwPagePort = 0x82 ;
        break;            
    }
    DWORD dwFlipFlopPort = 0x0c;
    if (wChannel>=4 ) {
        dwAddrPort += 0xc0;
        dwFlipFlopPort = 0xd8 ;
        dwPagePort += 0x8;
    }
    DWORD dwCountPort = dwAddrPort+1;
    DEBUGMSG(ZONE_WRITE,(L"dwFlipFlopPort=%x,dwAddrPort=%x dwCountPort=%x,addr=%x,count=%x",
        dwFlipFlopPort,dwAddrPort,dwCountPort,addr,count));
    
    WRITE_PORT_UCHAR((PUCHAR)dwFlipFlopPort, 0 ); // Reset Flip-Flop.
    WRITE_PORT_UCHAR((PUCHAR)dwAddrPort,(UCHAR)addr);
    WRITE_PORT_UCHAR((PUCHAR)dwAddrPort,(UCHAR)(addr>>8));
    
    WRITE_PORT_UCHAR((PUCHAR)dwFlipFlopPort, 0 ); // Reset Flip-Flop.
    WRITE_PORT_UCHAR((PUCHAR)dwCountPort,(UCHAR)count);
    WRITE_PORT_UCHAR((PUCHAR)dwCountPort,(UCHAR)(count>>8));
    
    WRITE_PORT_UCHAR((PUCHAR)dwPagePort,(UCHAR)(addr>>16)); // It only support upto 16 Mg.
};
WORD Dma8237Adapter::CountRegister(DWORD wChannel) 
{
    DWORD dwAddrPort = ( wChannel&3 )*2;
    DWORD dwFlipFlopPort = 0x0c;
    if (wChannel>=4 ) {
        dwAddrPort += 0xc0;
        dwFlipFlopPort = 0xd8 ;
    }
    DWORD dwCountPort = dwAddrPort+1;
    WRITE_PORT_UCHAR((PUCHAR)dwFlipFlopPort, 0 ); // Reset Flip-Flop.
    WORD wRet = READ_PORT_UCHAR((PUCHAR)dwCountPort);
    wRet += ((WORD)READ_PORT_UCHAR((PUCHAR)dwCountPort))<<8;
    DEBUGMSG(ZONE_WRITE,(L"CountRegister(%x)=%x",dwCountPort,wRet));
    return wRet;
    
}
void Dma8237Adapter::SetClearRequest(DWORD dwChannel,BOOL fSet) 
{
    if (dwChannel<dwNumOfChannel) {
        BYTE bReq = (BYTE)(dwChannel & 3);
        bReq |= (fSet? 0x4: 0);
        if (dwChannel< 4)  {
            WRITE_PORT_UCHAR((PUCHAR)0x9,bReq);
        }
        else {
            WRITE_PORT_UCHAR((PUCHAR)0xd2,bReq);
        }
        DEBUGMSG(ZONE_WRITE,(L"SetClearRequest:bReq=%x,ReqReg1=%x, ReqReg2=%x",bReq,READ_PORT_UCHAR((PUCHAR)0x9),READ_PORT_UCHAR((PUCHAR)0xd2)));
    }
    
};
void Dma8237Adapter::SetClearMask(DWORD dwChannel,BOOL fSet) 
{
    if (dwChannel<dwNumOfChannel) {
        BYTE bReq = (BYTE)(dwChannel & 3);
        bReq |= (fSet? 0x4: 0);
        if (dwChannel< 4)  {
            WRITE_PORT_UCHAR((PUCHAR)0xa,bReq);
        }
        else
            WRITE_PORT_UCHAR((PUCHAR)0xd4,bReq);
        DEBUGMSG(ZONE_WRITE,(L"SetClearMask:bReq=%x, MaskReg1=%x, MaskReg2=%x",bReq,READ_PORT_UCHAR((PUCHAR)0xf),READ_PORT_UCHAR((PUCHAR)0xde)));
    }
};
void Dma8237Adapter::SetUpMode(DWORD dwChannel, CE_DMA_ADAPTER& dmaInfo) 
{
    if (dwChannel<dwNumOfChannel) {
        BYTE bReq =  (BYTE)(dwChannel & 3); // Increase Address,
        // Read Mode is reading memory to device....
        bReq |= (((dmaInfo.dwFlags & DMA_FLAGS_WRITE_TO_DEVICE)?2:1) << 2);  
        if ((dmaInfo.dwFlags & DMA_FLAGS_INC_DEVICE_ADDRESS)!=0) { // memory to memory
            bReq |= (2<<6); // Block Mode.
        }
        else
            bReq |= ((dmaInfo.DemandMode? 0: 1)<<6); // Either demand mode or Single mode.
            
        if (dwChannel< 4)  {
            WRITE_PORT_UCHAR((PUCHAR)0xb,bReq);
        }
        else
            WRITE_PORT_UCHAR((PUCHAR)0xd6,bReq);
        DEBUGMSG(ZONE_WRITE,(L"SetUpMode:bReq=%x,Mode1=%x,Mode2=%x ",bReq,READ_PORT_UCHAR((PUCHAR)0xb),READ_PORT_UCHAR((PUCHAR)0xd6)));
    }
}
void     Dma8237Adapter::UpdateTC() 
{
    m_HardwareLock.Lock();
    m_StatusReg1 &= 0xf;
    m_StatusReg1 |= READ_PORT_UCHAR((PUCHAR)0x8);
    if (dwNumOfChannel > 4) {
        m_StatusReg2 &= 0xf;
        m_StatusReg2 |=  READ_PORT_UCHAR((PUCHAR)0xd0) ;
    }
    DEBUGMSG(ZONE_WRITE,(L"TC: m_StatusReg1=%x, m_StatusReg2=%x",m_StatusReg1,m_StatusReg2));
    m_HardwareLock.Unlock();
}
void Dma8237Adapter::ClearTC(DWORD dwChannel) 
{
    if (dwChannel<dwNumOfChannel) {
        m_HardwareLock.Lock();
        UpdateTC();
        BYTE bBit = (1<< (dwChannel&3));
        if (dwChannel<4) 
            m_StatusReg1 &= ~bBit;
        else
            m_StatusReg2 &= ~bBit;
        m_HardwareLock.Unlock();
    }
    
}
BOOL Dma8237Adapter::IsTransferDone(DWORD dwChannel) 
{
    BOOL fRet = TRUE;
    if (dwChannel< dwNumOfChannel) {
        UpdateTC();
        BYTE statusReg = (dwChannel< 4?  m_StatusReg1 : m_StatusReg2);
        fRet =  (( statusReg & (1<<(dwChannel&3)))!=0 );
    }
    return fRet;
}

Dma8237Channel::Dma8237Channel(Dma8237Adapter& DmaAdapter,PDMA_MDD_CHANNEL_CONTEXT pDmaMDDChannelContext) 
:   m_pDmaMDDChannelContext(pDmaMDDChannelContext)
,   m_dmaAdapter(DmaAdapter)
{
    lpCanArmDma = CanArmDmaStub;
    lpArmTransfer = ArmTransferStub;
    lpTerminateTransfer = TerminateTransferStub;
    lpStartDmaTransfer = StartDmaTransferStub;
    lpPollingForTransferDone = PollingForTransferDoneStub;
    lpPollingTransferRemaining = PollingTranferRemainingStub;
    lpCreateDmaPDDTransfer = NULL;
    lpCreateRawDmaPDDTransfer = NULL;
    lpFreeDmaTransfer = NULL ;
    
    m_Transfering = NULL;
}
BOOL    Dma8237Channel::ArmTransfer(PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT /*pDmaPDDTransfer*/ )
{
    if (m_Transfering!=NULL || pDmaMDDTransfer == NULL)
        return FALSE;
    GetDmaAdapter().LockHardware();
    m_Transfering = pDmaMDDTransfer;
    
    DWORD dwTransferFlags = m_pDmaMDDChannelContext->m_DmaAdapterInfo.dwFlags;
    PHYSICAL_ADDRESS sourceAddr = pDmaMDDTransfer->m_UserBufferPhAddr ; //dmaTransfer.GetUserBufferPhysAddr();
    PHYSICAL_ADDRESS targetAddr = ((pDmaMDDTransfer->m_dwFlags  & DMA_FLAGS_USER_OPTIONAL_DEVICE)?
            pDmaMDDTransfer->m_OptionalDeviceAddr : m_pDmaMDDChannelContext->m_phDeviceIoAddress);
    if ((DMA_FLAGS_WRITE_TO_DEVICE & m_pDmaMDDChannelContext->m_DmaAdapterInfo.dwFlags)==0 && 
            (m_pDmaMDDChannelContext->m_DmaAdapterInfo.dwFlags & DMA_FLAGS_INC_DEVICE_ADDRESS)!=0) { // Switch Memory <- Memory to Memory -> Memory
        m_pDmaMDDChannelContext->m_DmaAdapterInfo.dwFlags |= DMA_FLAGS_WRITE_TO_DEVICE;
        PHYSICAL_ADDRESS temp = sourceAddr;
        sourceAddr = targetAddr;
        targetAddr = temp;
    }
    // Setup Mode.
    GetDmaAdapter().SetClearMask(GetChannelIndex(),TRUE);
    GetDmaAdapter().ClearTC(GetChannelIndex());
    GetDmaAdapter().SetUpMode(GetChannelIndex(),m_pDmaMDDChannelContext->m_DmaAdapterInfo);
    ASSERT(pDmaMDDTransfer->m_UserBufferPhAddr.QuadPart<0x1000000);
    ASSERT(pDmaMDDTransfer->m_dwUserBufferLength<=0x10000);
    GetDmaAdapter().WriteAddrCount(GetChannelIndex(), sourceAddr.LowPart,
        (WORD)pDmaMDDTransfer->m_dwUserBufferLength);
    
    if (m_pDmaMDDChannelContext->m_DmaAdapterInfo.dwFlags & DMA_FLAGS_INC_DEVICE_ADDRESS ) {
        GetDmaAdapter().SetClearMask(GetChannelIndex()+1,TRUE);
//            GetDmaAdapter().ClearTC(GetChannelIndex()+1);
//            m_DmaAdapterInfo.dwFlags ^= DMA_FLAGS_WRITE_TO_DEVICE;
//            GetDmaAdapter().SetUpMode(GetChannelIndex()+1,m_DmaAdapterInfo);            
//            ASSERT(m_ulAddressSpace==0);
        GetDmaAdapter().WriteAddrCount(GetChannelIndex()+1,targetAddr.LowPart,(WORD)pDmaMDDTransfer->m_dwUserBufferLength);
//            GetDmaAdapter().SetClearMask(GetChannelIndex()+1,FALSE);
//            GetDmaAdapter().SetClearRequest(GetChannelIndex()+1, TRUE );

    };
    GetDmaAdapter().SetClearMask(GetChannelIndex(),FALSE);
    if (m_pDmaMDDChannelContext->m_DmaAdapterInfo.dwFlags & DMA_FLAGS_INC_DEVICE_ADDRESS ) {
        GetDmaAdapter().SetClearRequest(GetChannelIndex(), TRUE );
    }
    m_pDmaMDDChannelContext->m_DmaAdapterInfo.dwFlags = dwTransferFlags;
    GetDmaAdapter().UnlockHardware();
    return TRUE;
}
BOOL   Dma8237Channel:: TerminateTransfer(PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT /*pDmaPDDTransfer*/ ) 
{
    if (pDmaMDDTransfer!=NULL && pDmaMDDTransfer == m_Transfering) {
        if (GetDmaAdapter().IsTransferDone(GetChannelIndex())) {
            m_Transfering = NULL;
            m_pDmaMDDChannelContext->m_lpTransferCompleteNotify(m_pDmaMDDChannelContext,pDmaMDDTransfer,DMA_TRANSFER_COMPLETE,0);
        }
        else {
            GetDmaAdapter().LockHardware();
            DWORD dwCount = GetDmaAdapter().CountRegister(GetChannelIndex());
            GetDmaAdapter().SetClearMask(GetChannelIndex(),TRUE);
            GetDmaAdapter().SetClearRequest(GetChannelIndex(), FALSE );
            if ((m_pDmaMDDChannelContext->m_DmaAdapterInfo.dwFlags & DMA_FLAGS_INC_DEVICE_ADDRESS)!=0) {
                GetDmaAdapter().SetClearMask(GetChannelIndex()+1,TRUE);
                GetDmaAdapter().SetClearRequest(GetChannelIndex()+1, FALSE );
            }
            GetDmaAdapter().UnlockHardware();
            m_Transfering = NULL;
            m_pDmaMDDChannelContext->m_lpTransferCompleteNotify(m_pDmaMDDChannelContext,pDmaMDDTransfer,DMA_TRANSFER_COMPLETE_WITH_CANCELED,(WORD)(dwCount+1));
        }
        return TRUE;
    }
    else
        return FALSE;
}
BOOL   Dma8237Channel:: PollingForTransferDone() 
{
    // For Debugging.
    if (m_Transfering!=NULL) {
        if (GetDmaAdapter().IsTransferDone(GetChannelIndex())) {
            PDMA_MDD_TRANFER_CONTEXT lTransferring = m_Transfering;
            m_Transfering = NULL;
            m_pDmaMDDChannelContext->m_lpTransferCompleteNotify(m_pDmaMDDChannelContext,lTransferring,
                DMA_TRANSFER_COMPLETE, 
                (WORD)(GetDmaAdapter().CountRegister(GetChannelIndex())+1));
            return TRUE;
        }
    }
    else
        ASSERT(FALSE);
    return FALSE;
    
}
BOOL    Dma8237Channel::PollingTranferRemaining(PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT /*pDmaPDDTransfer*/, DWORD* dwRemaining)
{
    if (pDmaMDDTransfer == m_Transfering) {
        DWORD dwCount = GetDmaAdapter().CountRegister(GetChannelIndex());
        DEBUGMSG(ZONE_WRITE,(L"Count=%x",dwCount));
        if (dwRemaining)
            *dwRemaining = (WORD)(dwCount+1);
        return TRUE;
    }
    else
        return FALSE;
};

