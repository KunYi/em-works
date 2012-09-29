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
--*/

/*---------------------------------------------------------------------------
* Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
// Module Name:
//
//    esdhcdma.hpp
//
// Abstract:
//
//    Freescale ESDHC DMA definitions
//
// Notes:
//
///////////////////////////////////////////////////////////////////////////////



#pragma once

#include <SDCardDDK.h>
#include <SDHCD.h>
#include <ceddk.h>

class CESDHCBase;

typedef enum _DMAEVENT {
    NO_DMA = 0,
    CMD_COMPLETE, // added to allow EDMA to start the DMA
    DMA_COMPLETE,
    DMA_ERROR_OCCOR,
    TRANSFER_COMPLETED
} DMAEVENT;
#define INITIAL_DMA_LIST 0x400 // 1K

typedef struct {
    DWORD   pSrcSize;
    PVOID   pSrcVirtualAddr;
    
    DWORD   dwBufferSize;
    PVOID   pBufferedVirtualAddr;
    DWORD   dwBufferOffset;
    PHYSICAL_ADDRESS physicalAddress;    
} DMA_BUFFERED_BUFFER, *PDMA_BUFFERED_BUFFER;

// extendable DMA class for different flavors of DMA
class CESDHCBaseDMA {
public: 
    CESDHCBaseDMA(CESDHCBase& ESDHCBase);
    virtual ~CESDHCBaseDMA();
    virtual BOOL Init() ;
    virtual BYTE DmaSelectBit() = 0; // 2.2.10 Bit 4-3.
    virtual BOOL ArmDMA(SD_BUS_REQUEST& Request, BOOL fToDevice);
    virtual BOOL CancelDMA(); 
    virtual BOOL DMANotifyEvent(SD_BUS_REQUEST& Request,DMAEVENT dmaEvent) = 0;
    virtual DMAEVENT IsDMACompleted(BOOL fToDevice) {
        UNREFERENCED_PARAMETER(fToDevice);
        return m_fDMAProcessing? NO_DMA : DMA_COMPLETE; 
    };
    DWORD   ReAllocateDMABufferList(DWORD dwRequired);
    DWORD DMAComletionCode();
protected:
    DMA_BUFFER_HANDLE m_hDma;
    DWORD           m_dwNumOfList;
    DWORD           m_dwNumOfAvailabe;
    PCE_DMA_BUFFER_BLOCK 
                    m_pDmaBufferList;
    DWORD           m_dwCurDMAListPos;
    CE_DMA_ADAPTER  m_dmaAdapter;
    
    BOOL            m_fDMAProcessing;
    DWORD           m_dwDMACompletionCode;

    DMA_BUFFERED_BUFFER m_StartBuffer;
    DMA_BUFFERED_BUFFER m_EndBuffer;
    CacheInfo       m_ceCacheInfo ;
    
    DWORD GetDataCacheSize() const {
        // max of L1 and L2 cache line size
        return (m_ceCacheInfo.dwL1DCacheLineSize!=0? max(m_ceCacheInfo.dwL1DCacheLineSize, m_ceCacheInfo.dwL2DCacheLineSize): 64); // Default to 64 bytes if it not avaiable.
    }

    CESDHCBase&  m_ESDHCBase;
#pragma warning(push)
#pragma warning(disable: 4512)
};
#pragma warning(pop)


// Simple DMA (not to be confused with Smart DMA -- external DMA engine on iMX platforms)
class CESDHCBaseSDMA : public  CESDHCBaseDMA {
public:
    CESDHCBaseSDMA(CESDHCBase& ESDHCBase) : CESDHCBaseDMA(ESDHCBase) {
        DEBUGMSG(SDCARD_ZONE_INIT, (_T("CESDHCBaseSDMA:Create DMA Object for SDMA\r\n")));    
        m_dwNextOffset = 0;
        m_fLocked = FALSE;
    };
    ~CESDHCBaseSDMA();
     BOOL Init() ;
     BYTE DmaSelectBit() { return 0; }; // SDMA 2.2.10.
     BOOL ArmDMA(SD_BUS_REQUEST& Request, BOOL fToDevice ) ;
     BOOL DMANotifyEvent(SD_BUS_REQUEST& Request, DMAEVENT dmaEvent);
protected:
    BOOL    GetDMABuffer(SD_BUS_REQUEST& Request,BOOL fToDevice);
    DWORD    GetDMALengthBit(DWORD dwPhysAdddr, DWORD dwDataLength) {
        DWORD dwOffset = (dwPhysAdddr & (PAGE_SIZE-1));    
        return min(dwDataLength, (PAGE_SIZE - dwOffset));
    }
    DWORD   m_dwNextOffset;

    // We handle DMA buffer directly because of SDMA LIMITATION.
    BOOL    m_fLocked;
    LPVOID  m_lpvLockedAddress;
    DWORD   m_dwLockedSize;
    
#pragma warning(push)
#pragma warning(disable: 4512)
};
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////
// For external DMA engine: Smart DMA (also called SDMA) on iMX platform

class CESDHCBaseEDMA : public  CESDHCBaseDMA {
public:
    CESDHCBaseEDMA(CESDHCBase& ESDHCBase);
    ~CESDHCBaseEDMA();

     BOOL Init() ;
     BYTE DmaSelectBit() { return 0; }; // SDMA 2.2.10.
     BOOL ArmDMA(SD_BUS_REQUEST& Request, BOOL fToDevice ) ;
     BOOL DMANotifyEvent(SD_BUS_REQUEST& Request, DMAEVENT dmaEvent);
     DMAEVENT IsDMACompleted(BOOL fToDevice);

    // BSP-specific callouts
     BOOL BspEDMAInit();
     BOOL BspEDMAArm(SD_BUS_REQUEST& Request, BOOL fToDevice);
     BOOL BspEDMANotifyEvent(SD_BUS_REQUEST& Request, DMAEVENT dmaEvent);
     DMAEVENT BspEDMACheckCompletion(BOOL fToDevice);
     
     UINT8 m_dwChanTx;
     UINT8 m_dwChanRx;

protected:
    BOOL    GetDMABuffer(SD_BUS_REQUEST& Request,BOOL fToDevice);
    DWORD    GetDMALengthBit(DWORD dwPhysAdddr, DWORD dwDataLength) {
        DWORD dwOffset = (dwPhysAdddr & (PAGE_SIZE-1));    
        return min(dwDataLength, (PAGE_SIZE - dwOffset));
    }
    DWORD   m_dwNextOffset;

    // We handle DMA buffer directly because of SDMA LIMITATION.
    BOOL    m_fLocked;
    LPVOID  m_lpvLockedAddress;
    DWORD   m_dwLockedSize;

    DWORD m_dwStrandedBytes;
    DWORD m_dwWML;

#pragma warning(push)
#pragma warning(disable: 4512)
};
#pragma warning(pop)


// For 32 bit ADMA.
typedef struct __ADMA_32_DESC {
    DWORD   Valid:1;
    DWORD   End:1;
    DWORD   Int:1;
    DWORD   :1;
    DWORD   Act:2;
    DWORD   :6;
    DWORD   AddrLength:20;
} ADMA_32_DESC,*PADMA_32_DESC;

#define DESC_ENTRY_PER_TABLE (PAGE_SIZE/sizeof(ADMA_32_DESC))
#define MAXIMUM_DESC_TABLES 0x10

class CESDHCBase32BitADMA: public CESDHCBaseDMA {
public:
    CESDHCBase32BitADMA(CESDHCBase& ESDHCBase);
     ~CESDHCBase32BitADMA();
     BOOL Init() ;
     BYTE DmaSelectBit() { return 1; }; // 32 bit ADMA
     BOOL ArmDMA(SD_BUS_REQUEST& Request, BOOL fToDevice ) ;
     BOOL DMANotifyEvent(SD_BUS_REQUEST& Request, DMAEVENT dmaEvent);
     BOOL GetDMABuffer(SD_BUS_REQUEST& Request,BOOL fToDevice);
protected:
    DWORD           m_dwNumOfTables;
    PADMA_32_DESC  m_pDmaDescTables[MAXIMUM_DESC_TABLES + 1]; //extra space to eliminate prefast warn 
    DWORD           m_dwDescTablePhysAddr[MAXIMUM_DESC_TABLES + 1];

    BOOL m_bToDevice;

    BOOL    m_fLocked;
    LPVOID  m_lpvLockedAddress;
    DWORD   m_dwLockedSize;

#pragma warning(push)
#pragma warning(disable: 4512)
};
#pragma warning(pop)

// For 32 bit ADMA2.
typedef struct __ADMA2_32_DESC {
    DWORD   Valid:1;
    DWORD   End:1;
    DWORD   Int:1;
    DWORD   :1;
    DWORD   Act:2;
    DWORD   :10;
    DWORD   Length:16;
    DWORD   Address:32;
} ADMA2_32_DESC,*PADMA2_32_DESC;

#define DESC_ENTRY_PER_TABLE_ADMA2 (PAGE_SIZE/sizeof(ADMA2_32_DESC))

class CESDHCBase32BitADMA2: public CESDHCBaseDMA {
public:
    CESDHCBase32BitADMA2(CESDHCBase& ESDHCBase);
    ~CESDHCBase32BitADMA2();
    BOOL Init() ;
    BYTE DmaSelectBit() { return 2; }; // 32 bit ADMA2 2.2.10.
    BOOL ArmDMA(SD_BUS_REQUEST& Request, BOOL fToDevice ) ;
    BOOL DMANotifyEvent(SD_BUS_REQUEST& Request, DMAEVENT dmaEvent);
    BOOL GetDMABuffer(SD_BUS_REQUEST& Request,BOOL fToDevice);    
protected:
    DWORD           m_dwNumOfTables;
    PADMA2_32_DESC  m_pDmaDescTables[MAXIMUM_DESC_TABLES];
    DWORD           m_dwDescTablePhysAddr[MAXIMUM_DESC_TABLES];

    BOOL    IsEnoughDescTable(DWORD dwNumOfBlock);

    BOOL    m_fLocked;
    LPVOID  m_lpvLockedAddress;
    DWORD   m_dwLockedSize;

#pragma warning(push)
#pragma warning(disable: 4512)
};
#pragma warning(pop)

