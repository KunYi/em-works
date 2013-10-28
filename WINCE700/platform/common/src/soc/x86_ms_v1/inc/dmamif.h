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
// Module Name:  
//     DMAMIF.h
// 
// Abstract: Modul interface between PDD and MDD..
// 
// Notes: 
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#define ZONE_INIT        DEBUGZONE(0)
#define ZONE_OPEN        DEBUGZONE(1)
#define ZONE_READ        DEBUGZONE(2)
#define ZONE_WRITE        DEBUGZONE(3)
#define ZONE_CLOSE        DEBUGZONE(4)
#define ZONE_IOCTL        DEBUGZONE(5)
#define ZONE_THREAD        DEBUGZONE(6)
#define ZONE_EVENTS        DEBUGZONE(7)
#define ZONE_CRITSEC    DEBUGZONE(8)
#define ZONE_IO            DEBUGZONE(9)
#define ZONE_PDD        DEBUGZONE(10)
#define ZONE_UNUSED        DEBUGZONE(11)
#define ZONE_ALLOC        DEBUGZONE(12)
#define ZONE_FUNCTION    DEBUGZONE(13)
#define ZONE_WARN        DEBUGZONE(14)
#define ZONE_ERROR        DEBUGZONE(15)
#endif

//
// Contexts.
typedef struct __DMA_MDD_CHANNEL_CONTEXT *PDMA_MDD_CHANNEL_CONTEXT;
typedef struct __DMA_MDD_TRANFER_CONTEXT *PDMA_MDD_TRANFER_CONTEXT;
typedef struct __DMA_PDD_ADAPTER_CONTEXT *PDMA_PDD_ADAPTER_CONTEXT;
typedef struct __DMA_PDD_CHANNEL_CONTEXT *PDMA_PDD_CHANNEL_CONTEXT;
typedef struct __DMA_PDD_TRANFER_CONTEXT *PDMA_PDD_TRANFER_CONTEXT;
//
// MDD Adapter Contexts
// No MDD Adapter function can't be access from PDD.
// PDD Adapter Contexts.

typedef BOOL (WINAPI *LPGetDmaAdapter) (PDMA_PDD_ADAPTER_CONTEXT lpAdapterContext,
        IN PDEVICE_DMA_REQUIREMENT_INFO pDeviceDmaRequirementInfo, //  Device Description. It descript what is capable Adapt supported.
        IN OUT PCE_DMA_ADAPTER pDmaAdapter
    );
typedef PDMA_PDD_CHANNEL_CONTEXT (WINAPI *LPAllocateChannel)(PDMA_PDD_ADAPTER_CONTEXT lpAdapterContext, IN PDMA_MDD_CHANNEL_CONTEXT lpDmaMDDChannelContext);
typedef BOOL (WINAPI *LPFreeDmaChannel)(PDMA_PDD_ADAPTER_CONTEXT lpAdapterContext,PDMA_PDD_CHANNEL_CONTEXT lpPDDChannelContext);

typedef void (WINAPI *LPPowerMgmtCallback)(PDMA_PDD_ADAPTER_CONTEXT lpAdapterContext, BOOL fOff);
typedef BOOL (WINAPI *LPIOControl)(PDMA_PDD_ADAPTER_CONTEXT lpAdapterContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,PDWORD pdwActualOut);

typedef PVOID(WINAPI *LPOALAllocateCommonBuffer)(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,
    PCE_DMA_ADAPTER DmaAdapter, ULONG Length, PPHYSICAL_ADDRESS LogicalAddress, BOOLEAN CacheEnabled);
typedef BOOL  (WINAPI *LPOALFreeCommonBuffer)(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,
    PCE_DMA_ADAPTER DmaAdapter, ULONG Length, PHYSICAL_ADDRESS LogicalAddress, PVOID VirtualAddress, BOOLEAN CacheEnabled);  

typedef void  (WINAPI * LPPowerOnReset)(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext);
typedef BOOL  (WINAPI * LPIsChannelSuitable)(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,
    CE_DMA_ADAPTER * dmaAdapterInfo,DWORD dwChannelIndex);
typedef DWORD  (WINAPI * LPUpdateFlags)(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext, DWORD dwFlags) ;
typedef BOOL  (WINAPI * LPIsPhysAddrSupported)(PDMA_PDD_ADAPTER_CONTEXT lpPddAdapterContext,PHYSICAL_ADDRESS BufferPhysAddress,DWORD dwLength);


typedef struct __DMA_PDD_ADAPTER_CONTEXT {

    DWORD   dwAdapterIndex;
    DWORD   dwNumOfChannel;
    DWORD   dwNumOfHardwareMappingRegister;
    DWORD   dwMaximumSizeOfEachRegister;
    DWORD   dwMaximumAddressBoundary;
    DWORD   dwAddressAligment;
    DWORD   dwDmaSystemMemoryRangeStart;
    DWORD   dwDmaSystemMemoryRangeLength;
    LPGetDmaAdapter         lpGetDmaAdapter; // Optional. THIS can be NULL
    LPAllocateChannel       lpAllocateChannel; 
    LPFreeDmaChannel        lpFreeDmaChannel;
    LPPowerMgmtCallback     lpPowerMgmtCallback;    // Optional. can be NULL
    LPOALAllocateCommonBuffer   lpOALAllocateCommonBuffer;// Optional, can be NULL.
    LPOALFreeCommonBuffer       lpOALFreeCommonBuffer; // Optional. can be NULL
    LPPowerOnReset          lpPowerOnReset;// Optional. can be NULL
    LPIsChannelSuitable     lpIsChannelSuitable;// Optional. can be NULL
    LPUpdateFlags           lpUpdateFlags;// Optional. can be NULL
    LPIsPhysAddrSupported   lpIsPhysAddrSupported;
} DMA_PDD_ADAPTER_CONTEXT, *PDMA_PDD_ADAPTER_CONTEXT;


typedef BOOL (WINAPI *LPTransferCompleteNotify)(PDMA_MDD_CHANNEL_CONTEXT pDmaMDDChannelContext, PDMA_MDD_TRANFER_CONTEXT pDmaTransfer,DMA_STATUS_CODE status, DWORD dwRemaining);

typedef struct __DMA_MDD_CHANNEL_CONTEXT{
    CE_DMA_ADAPTER  m_DmaAdapterInfo;
    ULONG           m_ulAddressSpace;
    PHYSICAL_ADDRESS m_phDeviceIoAddress;    
    DWORD           m_dwChannelIndex;
    LPTransferCompleteNotify m_lpTransferCompleteNotify;
} DMA_MDD_CHANNEL_CONTEXT, *PDMA_MDD_CHANNEL_CONTEXT;


typedef BOOL (WINAPI *LPCanArmDma)(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext);
typedef BOOL (WINAPI *LPArmTransfer)(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext, PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT pDmaPDDTransfer );
typedef BOOL (WINAPI *LPTerminateTransfer)(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext, PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT pDmaPDDTransfer );
typedef BOOL (WINAPI *LPStartDmaTransfer)(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext);
typedef BOOL (WINAPI *LPPollingForTransferDone)(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext) ;
typedef BOOL (WINAPI *LPPollingTransferRemaining)(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext,PDMA_MDD_TRANFER_CONTEXT pDmaMDDTransfer,PDMA_PDD_TRANFER_CONTEXT pDmaPDDTransfer,DWORD * dwRemaining);
typedef PDMA_PDD_TRANFER_CONTEXT (WINAPI *LPCreateDmaPDDTransfer)(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext,PDMA_MDD_TRANFER_CONTEXT pDmaMddTransferContext) ;
typedef PDMA_PDD_TRANFER_CONTEXT (WINAPI *LPCreateRawDmaPDDTransfer)(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext,PDMA_MDD_TRANFER_CONTEXT pDmaMddTransferContext, IN PVOID lpInPtr,IN DWORD nInLen);
typedef BOOL (WINAPI *LPFreeDmaTransfer)(PDMA_PDD_CHANNEL_CONTEXT pDmaPDDChannelContext,PDMA_PDD_TRANFER_CONTEXT pDmaContexts);

typedef struct __DMA_PDD_CHANNEL_CONTEXT { 
    LPCanArmDma                 lpCanArmDma;
    LPArmTransfer               lpArmTransfer;
    LPTerminateTransfer         lpTerminateTransfer;
    LPStartDmaTransfer          lpStartDmaTransfer;
    LPPollingForTransferDone    lpPollingForTransferDone;
    LPPollingTransferRemaining  lpPollingTransferRemaining;
    LPCreateDmaPDDTransfer      lpCreateDmaPDDTransfer;
    LPCreateRawDmaPDDTransfer   lpCreateRawDmaPDDTransfer;
    LPFreeDmaTransfer           lpFreeDmaTransfer;
} DMA_PDD_CHANNEL_CONTEXT,*PDMA_PDD_CHANNEL_CONTEXT;

typedef struct __DMA_MDD_TRANFER_CONTEXT{
    DWORD   m_dwFlags;
    PVOID   m_pUserBufferPtr;
    DWORD   m_dwUserBufferLength;
    PHYSICAL_ADDRESS    m_UserBufferPhAddr;
    PHYSICAL_ADDRESS    m_OptionalDeviceAddr;
    BOOL    m_fRawTransfer;
    PVOID m_lpInPtr;
    DWORD m_nInLen;
}DMA_MDD_TRANFER_CONTEXT, *PDMA_MDD_TRANFER_CONTEXT;

typedef BOOL (WINAPI *LPDMAPDDTransferIoControl)(PDMA_PDD_TRANFER_CONTEXT pDMAPDDTransferContext,
    IN DWORD dwIoControl, IN PVOID lpInPtr, IN DWORD nInLen, IN OUT LPVOID lpOutBuffer, IN DWORD nOutBufferSize, IN LPDWORD lpBytesReturned);

typedef struct __DMA_PDD_TRANFER_CONTEXT{
    LPDMAPDDTransferIoControl lpDMAPDDTransferIoControl;
} DMA_PDD_TRANFER_CONTEXT, *PDMA_PDD_TRANFER_CONTEXT;

extern PDMA_PDD_ADAPTER_CONTEXT CreateDMAPDDContext(LPCTSTR lpActiveRegPath, PVOID pReserved);
extern BOOL DeleteDMAPDDContext ( PDMA_PDD_ADAPTER_CONTEXT lpDmaPDDAdapterContext);

#ifdef __cplusplus
};
#endif




