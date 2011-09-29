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
//    esdhcdma.cpp
//
// Abstract:
//
//    Freescale ESDHC DMA implementation
//
// Notes:
//
///////////////////////////////////////////////////////////////////////////////


#include "esdhc.h"

CESDHCBaseDMA::CESDHCBaseDMA (CESDHCBase& ESDHCBase)
:   m_ESDHCBase (ESDHCBase)
{
    m_dwNumOfList = 0;
    m_dwNumOfAvailabe = 0;
    m_pDmaBufferList = NULL;
    m_dwCurDMAListPos = 0;
    m_fDMAProcessing = FALSE;
    m_dwDMACompletionCode = ERROR_SUCCESS;
    
    m_hDma = NULL;
    memset(&m_dmaAdapter,0,sizeof(m_dmaAdapter));
    m_dmaAdapter.Size = sizeof(m_dmaAdapter);
    m_dmaAdapter.BusMaster = TRUE;
    m_dmaAdapter.BusNumber = m_ESDHCBase.m_dwControllerIndex;
    m_dmaAdapter.InterfaceType = ProcessorInternal;
};
BOOL CESDHCBaseDMA::Init() 
{
    if (!CeGetCacheInfo(sizeof(m_ceCacheInfo), &m_ceCacheInfo)) {
        ASSERT(FALSE);
    }

    else {

        m_pDmaBufferList = new CE_DMA_BUFFER_BLOCK[INITIAL_DMA_LIST];
        if (m_pDmaBufferList) {
            m_dwNumOfList = INITIAL_DMA_LIST ;
        }

        m_StartBuffer.pBufferedVirtualAddr = OALDMAAllocBuffer(&m_dmaAdapter, PAGE_SIZE , &m_StartBuffer.physicalAddress, FALSE );
        m_StartBuffer.dwBufferOffset = 0;
        m_StartBuffer.dwBufferSize = PAGE_SIZE;
        m_StartBuffer.pSrcVirtualAddr = NULL;
        m_StartBuffer.pSrcSize = 0;
        
        m_EndBuffer.pBufferedVirtualAddr = OALDMAAllocBuffer(&m_dmaAdapter, PAGE_SIZE , &m_EndBuffer.physicalAddress, FALSE );
        m_EndBuffer.dwBufferOffset = 0;
        m_EndBuffer.dwBufferSize = PAGE_SIZE;
        m_EndBuffer.pSrcVirtualAddr = NULL;
        m_EndBuffer.pSrcSize = 0;
    }
    
    return (m_pDmaBufferList != NULL && m_dwNumOfList != 0 && m_StartBuffer.pBufferedVirtualAddr != NULL &&  m_EndBuffer.pBufferedVirtualAddr != NULL);
}

CESDHCBaseDMA::~CESDHCBaseDMA()
{

    if (m_StartBuffer.pBufferedVirtualAddr!=NULL) {
        OALDMAFreeBuffer(&m_dmaAdapter, PAGE_SIZE,m_StartBuffer.physicalAddress,m_StartBuffer.pBufferedVirtualAddr,FALSE);
    }
    if (m_EndBuffer.pBufferedVirtualAddr!=NULL) {
        OALDMAFreeBuffer(&m_dmaAdapter, PAGE_SIZE,m_EndBuffer.physicalAddress,m_EndBuffer.pBufferedVirtualAddr,FALSE);
    }

    if (m_pDmaBufferList)
        delete [] m_pDmaBufferList;
    if (m_hDma) {
        DMACloseBuffer(m_hDma);
    }
    
}

DWORD   CESDHCBaseDMA::ReAllocateDMABufferList(DWORD dwRequired)
{
    if (m_pDmaBufferList && m_dwNumOfList < dwRequired) {
        delete [] m_pDmaBufferList;
        m_dwNumOfList = 0;
    }
    if (m_pDmaBufferList == NULL && dwRequired!= 0) {
        m_pDmaBufferList = new CE_DMA_BUFFER_BLOCK[dwRequired];
        if (m_pDmaBufferList) {
            m_dwNumOfList = dwRequired;
        }
    }
    return m_dwNumOfList;
}
BOOL CESDHCBaseDMA::CancelDMA()
{
    if (!m_fDMAProcessing) {
        m_fDMAProcessing = FALSE;
        m_dwCurDMAListPos = m_dwNumOfAvailabe;
        m_dwDMACompletionCode = ERROR_CANCELLED ;
    }
    if (!m_fDMAProcessing && m_hDma) {
        DMACloseBuffer(m_hDma);
        m_hDma = NULL;
    }
    return TRUE;
}

BOOL CESDHCBaseDMA::ArmDMA(SD_BUS_REQUEST& Request,BOOL fToDevice) 
{
    ASSERT(!m_fDMAProcessing);
    ASSERT(m_hDma==NULL);
    BOOL fReturn = FALSE;
    // Check for the limitaion.
    if (Request.NumBlocks > 1 && (Request.BlockSize & (sizeof(DWORD)-1)) == 0 
            && ((DWORD)Request.pBlockBuffer & (sizeof(DWORD)-1)) == 0
            && m_hDma == NULL) {
        // We are going to transfer this block as DMA.
        if ( (Request.Flags & SD_BUS_REQUEST_PHYS_BUFFER)!=0 && 
            Request.cbSizeOfPhysList!=0 && Request.pPhysBuffList!=NULL) {
            // We have user passed in Physical Buffer List.
            if (ReAllocateDMABufferList(Request.cbSizeOfPhysList) >= Request.cbSizeOfPhysList) {
                //Copy the Buffer.
                ASSERT(Request.cbSizeOfPhysList <= m_dwNumOfList);
                m_dwNumOfAvailabe = Request.cbSizeOfPhysList ;
                PBYTE pVirtualAddr = Request.pBlockBuffer ;
                for (DWORD dwIndex = 0; dwIndex < Request.cbSizeOfPhysList && dwIndex< m_dwNumOfList; dwIndex ++) {
                    m_pDmaBufferList[dwIndex].dwLength = Request.pPhysBuffList[dwIndex].PhysLen;
                    m_pDmaBufferList[dwIndex].physicalAddress = Request.pPhysBuffList[dwIndex].PhysAddr;
                    m_pDmaBufferList[dwIndex].virtualAddress = pVirtualAddr + m_pDmaBufferList[dwIndex].dwLength;
                }
                fReturn = TRUE;
                
            }
        }
        else { // We need figure out the physical address by using CEDDK DMA function.
            DWORD dwLength = Request.BlockSize * Request.NumBlocks;
            m_dmaAdapter.dwFlags =(fToDevice? DMA_FLAGS_WRITE_TO_DEVICE: 0);
//            if (!fToDevice) {
//                memset(Request.pBlockBuffer,0xc5,dwLength);
//            }
            m_hDma = DMAOpenBuffer(&m_dmaAdapter,1, (PVOID *)&Request.pBlockBuffer,&dwLength);
            if (m_hDma) {
                m_dwNumOfAvailabe = DMAGetBufferPhysAddr(m_hDma, m_dwNumOfList, m_pDmaBufferList);
                if (m_dwNumOfAvailabe>m_dwNumOfList && ReAllocateDMABufferList(m_dwNumOfAvailabe)>=m_dwNumOfAvailabe) {
                    m_dwNumOfAvailabe = DMAGetBufferPhysAddr(m_hDma, m_dwNumOfList, m_pDmaBufferList);
                }
                if (m_dwNumOfAvailabe<= m_dwNumOfList) {
                    fReturn = TRUE;
                }
                else { // FAILED.
                    m_dwNumOfAvailabe = 0;
                    DMACloseBuffer(m_hDma);
                    m_hDma = NULL;
                }
            }
        }
        ASSERT(fReturn);        
    }
    
    return fReturn;
}

// Simple DMA protocol for SD (not to be confused with Smart DMA IP block on iMX platforms)
/************* SIMPLE DMA UNTESTED SO FAR *****************************/

CESDHCBaseSDMA::~CESDHCBaseSDMA()
{
    if (m_fLocked )
        UnlockPages( m_lpvLockedAddress, m_dwLockedSize);
    m_fLocked = FALSE;
}

BOOL CESDHCBaseSDMA::Init()  
{
    m_fLocked= FALSE;
    return (CESDHCBaseDMA::Init());
};


BOOL CESDHCBaseSDMA::GetDMABuffer(SD_BUS_REQUEST& Request,BOOL fToDevice)
{
// We shouldn't use PAGE_SHIFT in CEDDK because it is too confusing and too dangerouse.
#define PFN_SHIEFT UserKInfo[KINX_PFN_SHIFT]
#define MAX_SUPPORTED_PFN (MAXDWORD>>PFN_SHIEFT)
#define PAGE_MASK (PAGE_SIZE-1)
    ASSERT(!m_fDMAProcessing);
    ASSERT(m_hDma==NULL);
    ASSERT(m_fLocked == FALSE);
    m_StartBuffer.pSrcVirtualAddr = NULL;
    m_StartBuffer.pSrcSize = 0;
    m_EndBuffer.pSrcVirtualAddr = NULL;
    m_EndBuffer.pSrcSize = 0;
    BOOL fReturn = FALSE;
    // Check for the limitaion.
    if (Request.NumBlocks > 1 && (Request.BlockSize & (sizeof(DWORD)-1)) == 0 
            && ((DWORD)Request.pBlockBuffer & (sizeof(DWORD)-1)) == 0
            && m_hDma == NULL) {
        // We are going to transfer this block as DMA.
        if ( (Request.Flags & SD_BUS_REQUEST_PHYS_BUFFER)!=0 && 
            Request.cbSizeOfPhysList!=0 && Request.pPhysBuffList!=NULL) {
            // We have user passed in Physical Buffer List.
            if (ReAllocateDMABufferList(Request.cbSizeOfPhysList) >= Request.cbSizeOfPhysList) {
                //Copy the Buffer.
                ASSERT(Request.cbSizeOfPhysList <= m_dwNumOfList);
                m_dwNumOfAvailabe = Request.cbSizeOfPhysList ;
                PBYTE pVirtualAddr = Request.pBlockBuffer ;
                for (DWORD dwIndex = 0; dwIndex < Request.cbSizeOfPhysList && dwIndex< m_dwNumOfList; dwIndex ++) {
                    m_pDmaBufferList[dwIndex].dwLength = Request.pPhysBuffList[dwIndex].PhysLen;
                    m_pDmaBufferList[dwIndex].physicalAddress = Request.pPhysBuffList[dwIndex].PhysAddr;
                    m_pDmaBufferList[dwIndex].virtualAddress = pVirtualAddr + m_pDmaBufferList[dwIndex].dwLength;
                }
                fReturn = TRUE;
                
            }
        }
        else { // We need figure out the physical address by using CEDDK DMA function.
            DWORD dwLength = Request.BlockSize * Request.NumBlocks;
            PVOID pUseBufferPtr = Request.pBlockBuffer;
            m_dmaAdapter.dwFlags =(fToDevice? DMA_FLAGS_WRITE_TO_DEVICE: 0);
//            if (!fToDevice) {
//                memset(Request.pBlockBuffer,0xc5,dwLength);
//            }
            m_dwNumOfAvailabe = ADDRESS_AND_SIZE_TO_SPAN_PAGES( pUseBufferPtr, dwLength );
            PDWORD pdwPhysAddress = new DWORD[m_dwNumOfAvailabe];
            if (pdwPhysAddress) {
                if (m_dwNumOfAvailabe> m_dwNumOfList) {
                    ReAllocateDMABufferList(m_dwNumOfAvailabe);
                }
                if (m_dwNumOfAvailabe<= m_dwNumOfList ) {
                    m_fLocked = LockPages( pUseBufferPtr,dwLength,pdwPhysAddress, // m_pdwPhysAddress to stall PFN temperory.
                        fToDevice? LOCKFLAG_READ: LOCKFLAG_WRITE);
                    if (m_fLocked) { // Create table for Physical Address and length.
                        m_lpvLockedAddress = pUseBufferPtr;
                        m_dwLockedSize = dwLength;
                        
                        fReturn = TRUE;
                        for (DWORD dwIndex = 0; dwIndex< m_dwNumOfAvailabe; dwIndex++) {
                            if (pdwPhysAddress[dwIndex] > MAX_SUPPORTED_PFN) {
                                ASSERT(FALSE);
                                fReturn = FALSE;
                                break;
                            }
                            else {
                                m_pDmaBufferList[dwIndex].dwLength = min((PAGE_SIZE - ((DWORD)pUseBufferPtr & PAGE_MASK)),dwLength) ;
                                m_pDmaBufferList[dwIndex].physicalAddress.LowPart = (pdwPhysAddress[dwIndex]<<PFN_SHIEFT) + ((DWORD)pUseBufferPtr & PAGE_MASK);
                                m_pDmaBufferList[dwIndex].physicalAddress.HighPart = 0;
                                m_pDmaBufferList[dwIndex].virtualAddress = (PBYTE)pUseBufferPtr;
                                dwLength -= m_pDmaBufferList[dwIndex].dwLength;
                                pUseBufferPtr = (PBYTE)pUseBufferPtr+m_pDmaBufferList[dwIndex].dwLength;
                            }
                        };
                    }
                    
                }
                delete[] pdwPhysAddress;
            }
            if (fReturn && m_dwNumOfAvailabe) { // Check for Cache aligh begin and end.
                if (!fToDevice) {
                    DWORD dwDCacheLineSize = GetDataCacheSize() ;
                    ASSERT ((dwDCacheLineSize & dwDCacheLineSize-1) == 0 ); // I has to be nature align.
                    // First Block
                    if ((m_pDmaBufferList[0].physicalAddress.LowPart & (dwDCacheLineSize-1)) != 0) { // Not cache align. we have to do something.
                        m_StartBuffer.dwBufferOffset = (m_pDmaBufferList[0].physicalAddress.LowPart & PAGE_MASK);
                        m_StartBuffer.pSrcSize = m_pDmaBufferList[0].dwLength;
                        m_StartBuffer.pSrcVirtualAddr = m_pDmaBufferList[0].virtualAddress;
                        m_pDmaBufferList[0].physicalAddress.QuadPart = m_StartBuffer.physicalAddress.QuadPart + m_StartBuffer.dwBufferOffset;
                        //m_pDmaBufferList[0].virtualAddress = (PBYTE)m_StartBuffer.pBufferedVirtualAddr + m_StartBuffer.dwBufferOffset;
                    }

                    // check tail buffer only if there is more than 1 buffer, or if with 1 buffer the StartBuffer is not substituted for it
                    if((m_dwNumOfAvailabe > 1 || m_StartBuffer.pSrcVirtualAddr == NULL)
                        && (m_pDmaBufferList[m_dwNumOfAvailabe-1].dwLength & (dwDCacheLineSize-1))!=0) {
                        m_EndBuffer.dwBufferOffset = 0 ;
                        m_EndBuffer.pSrcSize = m_pDmaBufferList[m_dwNumOfAvailabe-1].dwLength;
                        m_EndBuffer.pSrcVirtualAddr = m_pDmaBufferList[m_dwNumOfAvailabe-1].virtualAddress;
                        m_pDmaBufferList[m_dwNumOfAvailabe-1].physicalAddress = m_EndBuffer.physicalAddress;
                        //m_pDmaBufferList[m_dwNumOfAvailabe-1].virtualAddress = m_EndBuffer.pBufferedVirtualAddr;
                    }
                }
            }
            else {
                fReturn = FALSE;
                if (m_fLocked) {
                    UnlockPages(m_lpvLockedAddress, m_dwLockedSize);
                };
                m_fLocked = FALSE;
            }
            if (fReturn) {

                // for writes
                if (fToDevice)
                {
                    for (DWORD dwIndex = 0 ; dwIndex<m_dwNumOfAvailabe ; dwIndex++) {
                        CacheRangeFlush(m_pDmaBufferList[dwIndex].virtualAddress,m_pDmaBufferList[dwIndex].dwLength, CACHE_SYNC_WRITEBACK );
                    }
                }

                // for reads
                else
                {
                    for (DWORD dwIndex = 0 ; dwIndex<m_dwNumOfAvailabe ; dwIndex++) {
                        CacheRangeFlush(m_pDmaBufferList[dwIndex].virtualAddress,m_pDmaBufferList[dwIndex].dwLength, CACHE_SYNC_DISCARD );
                    }
                }
            }
        }
        ASSERT(fReturn);        
    }
    return fReturn;
}

BOOL CESDHCBaseSDMA::ArmDMA(SD_BUS_REQUEST& Request,BOOL fToDevice )
{
    BOOL fResult = GetDMABuffer(Request,fToDevice);//CESDHCBaseDMA::ArmDMA(Request,fToDevice);
    if (fResult) {
        m_fDMAProcessing = TRUE;
        m_dwDMACompletionCode = ERROR_IO_PENDING ;
        m_dwCurDMAListPos = 0;
        ASSERT(m_dwNumOfAvailabe!=0);
        m_dwNextOffset = GetDMALengthBit(m_pDmaBufferList[m_dwCurDMAListPos].physicalAddress.LowPart, m_pDmaBufferList[m_dwCurDMAListPos].dwLength);
        ASSERT(m_dwNextOffset<= m_pDmaBufferList[m_dwCurDMAListPos].dwLength);
        // Arm the first buffer.
        OUTREG32(&(m_ESDHCBase.m_pESDHCReg->DSADDR), m_pDmaBufferList[m_dwCurDMAListPos].physicalAddress.LowPart );
    }
    return fResult;
}
BOOL CESDHCBaseSDMA::DMANotifyEvent(SD_BUS_REQUEST& Request, DMAEVENT dmaEvent)
{
    switch (dmaEvent) {
      case DMA_COMPLETE:
        if (m_fDMAProcessing && m_pDmaBufferList && m_dwCurDMAListPos<m_dwNumOfAvailabe) {
            if (m_dwNextOffset < m_pDmaBufferList[m_dwCurDMAListPos].dwLength) { // re-arm the same.
                DWORD dwNewAddr = m_pDmaBufferList[m_dwCurDMAListPos].physicalAddress.LowPart + m_dwNextOffset;
                DWORD dwLength = GetDMALengthBit(dwNewAddr, m_pDmaBufferList[m_dwCurDMAListPos].dwLength-m_dwNextOffset);
                ASSERT(dwLength + m_dwNextOffset <= m_pDmaBufferList[m_dwCurDMAListPos].dwLength);
                m_dwNextOffset +=  dwLength;
                OUTREG32(&(m_ESDHCBase.m_pESDHCReg->DSADDR),dwNewAddr);
                
            }
            else { // next entry,
                // Update HCParam            
                Request.HCParam += m_pDmaBufferList[m_dwCurDMAListPos].dwLength ;
                m_dwNextOffset = 0;
                
                m_dwCurDMAListPos++;
                if (m_dwCurDMAListPos < m_dwNumOfAvailabe) { // Continue for next
                    m_dwNextOffset = GetDMALengthBit(m_pDmaBufferList[m_dwCurDMAListPos].physicalAddress.LowPart, m_pDmaBufferList[m_dwCurDMAListPos].dwLength);
                    ASSERT(m_dwNextOffset<= m_pDmaBufferList[m_dwCurDMAListPos].dwLength);
                    OUTREG32(&(m_ESDHCBase.m_pESDHCReg->DSADDR), m_pDmaBufferList[m_dwCurDMAListPos].physicalAddress.LowPart);
                }
                else {
                    m_fDMAProcessing = FALSE;
                    ASSERT(FALSE); // DMA has been completed.
                }
            }
        }
        else {
            ASSERT(m_dwNumOfAvailabe == m_dwCurDMAListPos);
            m_fDMAProcessing = FALSE;
        }
        break;
      case TRANSFER_COMPLETED:
        ASSERT(m_dwCurDMAListPos <= m_dwNumOfAvailabe);
        if (m_fDMAProcessing && m_pDmaBufferList && m_dwCurDMAListPos<m_dwNumOfAvailabe) {
            Request.HCParam += m_pDmaBufferList[m_dwCurDMAListPos].dwLength ;
            m_dwCurDMAListPos ++;
        }
        m_fDMAProcessing = FALSE;
        break;
      case DMA_ERROR_OCCOR:
      default:
        ASSERT(FALSE);
        m_dwDMACompletionCode = ERROR_NOT_READY;
        m_fDMAProcessing = FALSE;
        break;
    }
    if (!m_fDMAProcessing && m_hDma) {
        DMACloseBuffer(m_hDma);
        m_hDma = NULL;
    }
    else if (!m_fDMAProcessing && m_fLocked && m_pDmaBufferList) {
        if (Request.TransferClass != SD_WRITE) {

            if (m_StartBuffer.pSrcVirtualAddr!=NULL && m_StartBuffer.pSrcSize!=0) {
                CeSafeCopyMemory (m_StartBuffer.pSrcVirtualAddr, (PBYTE)m_StartBuffer.pBufferedVirtualAddr+ m_StartBuffer.dwBufferOffset,m_StartBuffer.pSrcSize);
            }
            if (m_EndBuffer.pSrcVirtualAddr!=NULL && m_EndBuffer.pSrcSize!=0) {
                CeSafeCopyMemory (m_EndBuffer.pSrcVirtualAddr, m_EndBuffer.pBufferedVirtualAddr,m_EndBuffer.pSrcSize);
            }
        }
        if (m_fLocked )
            UnlockPages( m_lpvLockedAddress, m_dwLockedSize);
        m_fLocked = FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////
// External DMA engine (Smart DMA (SDMA)) implementation

BOOL CESDHCBaseEDMA::Init()
{
    m_fLocked= FALSE;

    return (BspEDMAInit() && CESDHCBaseDMA::Init());
}

BOOL CESDHCBaseEDMA::GetDMABuffer(SD_BUS_REQUEST& Request,BOOL fToDevice)
{
// We shouldn't use PAGE_SHIFT in CEDDK because it is too confusing and too dangerouse.
#define PFN_SHIEFT UserKInfo[KINX_PFN_SHIFT]
#define MAX_SUPPORTED_PFN (MAXDWORD>>PFN_SHIEFT)
#define PAGE_MASK (PAGE_SIZE-1)
    ASSERT(!m_fDMAProcessing);
    ASSERT(m_hDma==NULL);
    ASSERT(m_fLocked == FALSE);
    m_StartBuffer.pSrcVirtualAddr = NULL;
    m_StartBuffer.pSrcSize = 0;
    m_EndBuffer.pSrcVirtualAddr = NULL;
    m_EndBuffer.pSrcSize = 0;
    BOOL fReturn = FALSE;
    // Check for the limitaion.
    if (Request.NumBlocks > 1 && (Request.BlockSize & (sizeof(DWORD)-1)) == 0 
            && ((DWORD)Request.pBlockBuffer & (sizeof(DWORD)-1)) == 0
            && m_hDma == NULL) {
        // We are going to transfer this block as DMA.
        if ( (Request.Flags & SD_BUS_REQUEST_PHYS_BUFFER)!=0 && 
            Request.cbSizeOfPhysList!=0 && Request.pPhysBuffList!=NULL) {
            // We have user passed in Physical Buffer List.
            if (ReAllocateDMABufferList(Request.cbSizeOfPhysList) >= Request.cbSizeOfPhysList) {
                //Copy the Buffer.
                ASSERT(Request.cbSizeOfPhysList <= m_dwNumOfList);
                m_dwNumOfAvailabe = Request.cbSizeOfPhysList ;
                PBYTE pVirtualAddr = Request.pBlockBuffer ;
                for (DWORD dwIndex = 0; dwIndex < Request.cbSizeOfPhysList && dwIndex< m_dwNumOfList; dwIndex ++) {
                    m_pDmaBufferList[dwIndex].dwLength = Request.pPhysBuffList[dwIndex].PhysLen;
                    m_pDmaBufferList[dwIndex].physicalAddress = Request.pPhysBuffList[dwIndex].PhysAddr;
                    m_pDmaBufferList[dwIndex].virtualAddress = pVirtualAddr + m_pDmaBufferList[dwIndex].dwLength;
                }
                fReturn = TRUE;
                
            }
        }
        else { // We need figure out the physical address by using CEDDK DMA function.
            DWORD dwLength = Request.BlockSize * Request.NumBlocks;
            PVOID pUseBufferPtr = Request.pBlockBuffer;
            m_dmaAdapter.dwFlags =(fToDevice? DMA_FLAGS_WRITE_TO_DEVICE: 0);

            // total number of physical pages spanned by this virt buffer
            m_dwNumOfAvailabe = ADDRESS_AND_SIZE_TO_SPAN_PAGES( pUseBufferPtr, dwLength );
            PDWORD pdwPhysAddress = new DWORD[m_dwNumOfAvailabe];
            if (pdwPhysAddress) {
                if (m_dwNumOfAvailabe> m_dwNumOfList) {
                    ReAllocateDMABufferList(m_dwNumOfAvailabe);
                }
                if (m_dwNumOfAvailabe<= m_dwNumOfList ) {
                    m_fLocked = LockPages( pUseBufferPtr,dwLength,pdwPhysAddress, // m_pdwPhysAddress to stall PFN temperory.
                        fToDevice? LOCKFLAG_READ: LOCKFLAG_WRITE);
                    if (m_fLocked) { // Create table for Physical Address and length.
                        m_lpvLockedAddress = pUseBufferPtr;
                        m_dwLockedSize = dwLength;
                        
                        fReturn = TRUE;
                        for (DWORD dwIndex = 0; dwIndex< m_dwNumOfAvailabe; dwIndex++) {
                            if (pdwPhysAddress[dwIndex] > MAX_SUPPORTED_PFN) {
                                ASSERT(FALSE);
                                fReturn = FALSE;
                                break;
                            }

                            // populate each buffer entry with phys addresses and length to read
                            else {

                                // size of data = min ( (page size - offset(= virt addr lower addr bits)) (=rem data in this page), length of virt buffer) 
                                m_pDmaBufferList[dwIndex].dwLength = min((PAGE_SIZE - ((DWORD)pUseBufferPtr & PAGE_MASK)),dwLength) ;

                                // start of physAddr = result of LockPages shifted left by PFN_SHIFT + offset (= virt addr lower addr bits)
                                m_pDmaBufferList[dwIndex].physicalAddress.LowPart = (pdwPhysAddress[dwIndex]<<PFN_SHIEFT) + ((DWORD)pUseBufferPtr & PAGE_MASK);
                                m_pDmaBufferList[dwIndex].physicalAddress.HighPart = 0;
                                m_pDmaBufferList[dwIndex].virtualAddress = (PBYTE)pUseBufferPtr;
                                dwLength -= m_pDmaBufferList[dwIndex].dwLength;
                                pUseBufferPtr = (PBYTE)pUseBufferPtr+m_pDmaBufferList[dwIndex].dwLength;
                            }
                        };
                    }
                    
                }
                delete[] pdwPhysAddress;
            }
            if (fReturn && m_dwNumOfAvailabe) { // Check for Cache align begin and end

                // for read from device only
                if (!fToDevice) {
                    DWORD dwDCacheLineSize = GetDataCacheSize() ;
                    ASSERT ((dwDCacheLineSize & dwDCacheLineSize-1) == 0 ); // I has to be nature align.
                    // First page
                    if ((m_pDmaBufferList[0].physicalAddress.LowPart & (dwDCacheLineSize-1)) != 0) { // Addr not cache aligned. we have to do something.
                        m_StartBuffer.dwBufferOffset = (m_pDmaBufferList[0].physicalAddress.LowPart & PAGE_MASK);
                        m_StartBuffer.pSrcSize = m_pDmaBufferList[0].dwLength;
                        m_StartBuffer.pSrcVirtualAddr = m_pDmaBufferList[0].virtualAddress;
                        m_pDmaBufferList[0].physicalAddress.QuadPart = m_StartBuffer.physicalAddress.QuadPart + m_StartBuffer.dwBufferOffset;
                        //m_pDmaBufferList[0].virtualAddress = (PBYTE)m_StartBuffer.pBufferedVirtualAddr + m_StartBuffer.dwBufferOffset;
                    }

                    // pages in between first and last are guaranteed to be cache aligned b/c they have to start on page boundary
                    // (because they have to be contiguous)
                    
                    // check tail buffer only if there is more than 1 buffer, or if with 1 buffer the StartBuffer is not substituted for it
                    if((m_dwNumOfAvailabe > 1 || m_StartBuffer.pSrcVirtualAddr == NULL)
                        &&  (m_pDmaBufferList[m_dwNumOfAvailabe-1].dwLength & (dwDCacheLineSize-1))!=0) {
                        m_EndBuffer.dwBufferOffset = 0 ;
                        m_EndBuffer.pSrcSize = m_pDmaBufferList[m_dwNumOfAvailabe-1].dwLength;
                        m_EndBuffer.pSrcVirtualAddr = m_pDmaBufferList[m_dwNumOfAvailabe-1].virtualAddress;
                        m_pDmaBufferList[m_dwNumOfAvailabe-1].physicalAddress = m_EndBuffer.physicalAddress;

                        // uncommented the following line because we may be accessing the virtual addr of last buffer for stranded bytes
                        m_pDmaBufferList[m_dwNumOfAvailabe-1].virtualAddress = m_EndBuffer.pBufferedVirtualAddr;  
                    }
                }
            }
            else {
                fReturn = FALSE;
                if (m_fLocked) {
                    UnlockPages(m_lpvLockedAddress, m_dwLockedSize);
                };
                m_fLocked = FALSE;
            }

            //  for write: no reason to check alignment, just sync the buffer with latest data from cache
            if (fReturn) {

                // for writes
                if (fToDevice)
                {
                    for (DWORD dwIndex = 0 ; dwIndex<m_dwNumOfAvailabe ; dwIndex++) {
                        CacheRangeFlush(m_pDmaBufferList[dwIndex].virtualAddress,m_pDmaBufferList[dwIndex].dwLength, CACHE_SYNC_WRITEBACK );
                    }
                }

                // for reads
                else
                {
                    for (DWORD dwIndex = 0 ; dwIndex<m_dwNumOfAvailabe ; dwIndex++) {
                        CacheRangeFlush(m_pDmaBufferList[dwIndex].virtualAddress,m_pDmaBufferList[dwIndex].dwLength, CACHE_SYNC_DISCARD );
                    }
                }
            }
        }
        ASSERT(fReturn);        
    }
    
    return fReturn;
}


BOOL CESDHCBaseEDMA::ArmDMA(SD_BUS_REQUEST& Request, BOOL fToDevice)
{
    // do not use the external DMA engine when the request is fast path due to overhead of DMA setup
    if (m_ESDHCBase.m_fCurrentRequestFastPath)
        return FALSE;
    
    // get the cache aligned physical pages involved in the DMA
    BOOL fResult = GetDMABuffer(Request,fToDevice);

    // initialize and start the external DMA chain for the TX or RX channel
    if (fResult && BspEDMAArm(Request, fToDevice)) {
        //m_fDMAProcessing = TRUE;
        //m_dwDMACompletionCode = ERROR_IO_PENDING ;
        //m_dwCurDMAListPos = 0;
    }

    else
    {
        fResult = FALSE;
    }

    return fResult;
}

BOOL CESDHCBaseEDMA::DMANotifyEvent(SD_BUS_REQUEST& Request, DMAEVENT dmaEvent)
{

    switch (dmaEvent) {

      case CMD_COMPLETE:

        // External DMA can start the channel now
        if (BspEDMANotifyEvent(Request, CMD_COMPLETE))
        {
            m_fDMAProcessing = TRUE;
            m_dwDMACompletionCode = ERROR_IO_PENDING ;            
        }
        
        break;
        
      case DMA_COMPLETE:

            // entire chain has completed for EDMA, get starndedBytes, if any
            BspEDMANotifyEvent(Request, DMA_COMPLETE);
            m_dwStrandedBytes = 0;
            
            // Update HCParam            
            Request.HCParam += Request.NumBlocks * Request.BlockSize;
            m_fDMAProcessing = FALSE;
        break;

      case TRANSFER_COMPLETED:
        m_fDMAProcessing = FALSE;
        break;
        
      case DMA_ERROR_OCCOR:

        if (m_fDMAProcessing)
            BspEDMANotifyEvent(Request, DMA_ERROR_OCCOR);

        m_fDMAProcessing = FALSE;
        break;
        
      default:
        ASSERT(FALSE);
        m_dwDMACompletionCode = ERROR_NOT_READY;
        m_fDMAProcessing = FALSE;
        break;
        
    }
    if (!m_fDMAProcessing && m_hDma) {
        DMACloseBuffer(m_hDma);
        m_hDma = NULL;
    }
    else if (!m_fDMAProcessing && m_fLocked) {

        // for reads, copy the cache aligned buffer to original destination (non-cache aligned buffer)
        if (Request.TransferClass != SD_WRITE) {
            if (m_StartBuffer.pSrcVirtualAddr!=NULL && m_StartBuffer.pSrcSize!=0) {
                CeSafeCopyMemory (m_StartBuffer.pSrcVirtualAddr, (PBYTE)m_StartBuffer.pBufferedVirtualAddr+ m_StartBuffer.dwBufferOffset,m_StartBuffer.pSrcSize);
            }
            if (m_EndBuffer.pSrcVirtualAddr!=NULL && m_EndBuffer.pSrcSize!=0) {
                CeSafeCopyMemory (m_EndBuffer.pSrcVirtualAddr, m_EndBuffer.pBufferedVirtualAddr,m_EndBuffer.pSrcSize);
            }
        }
        if (m_fLocked )
            UnlockPages( m_lpvLockedAddress, m_dwLockedSize);
        m_fLocked = FALSE;
    }
    return TRUE;

}

DMAEVENT CESDHCBaseEDMA::IsDMACompleted(BOOL fToDevice)
{

    if (m_fDMAProcessing)
        return BspEDMACheckCompletion(fToDevice);

    else
        return NO_DMA;
}


CESDHCBase32BitADMA::CESDHCBase32BitADMA(CESDHCBase& ESDHCBase)
:    CESDHCBaseDMA(ESDHCBase)
{
    DEBUGMSG(SDCARD_ZONE_INIT, (_T("CESDHCBase32BitADMA:Create DMA Object for ADMA\r\n")));        
    m_dwNumOfTables =0;
    for (DWORD dwIndex=0; dwIndex < MAXIMUM_DESC_TABLES; dwIndex++) {
        m_pDmaDescTables[dwIndex] = NULL;
        m_dwDescTablePhysAddr[dwIndex] = 0 ;
    }
        
};
CESDHCBase32BitADMA::~CESDHCBase32BitADMA()
{
    for (DWORD dwIndex=0; dwIndex< m_dwNumOfTables; dwIndex++) {
        ASSERT(m_pDmaDescTables[dwIndex]);
        ASSERT(m_dwDescTablePhysAddr[dwIndex]);
        PHYSICAL_ADDRESS LogicalAddress = {m_dwDescTablePhysAddr[dwIndex],0};
        OALDMAFreeBuffer(&m_dmaAdapter,PAGE_SIZE,LogicalAddress,m_pDmaDescTables[dwIndex],FALSE);
    }

    if (m_fLocked )
        UnlockPages( m_lpvLockedAddress, m_dwLockedSize);
}
BOOL CESDHCBase32BitADMA::Init()
{
    m_fLocked= FALSE;

    if (CESDHCBaseDMA::Init()) {
        PHYSICAL_ADDRESS LogicalAddress;
        ASSERT(m_dwNumOfTables==0);
        if (m_dwNumOfTables<MAXIMUM_DESC_TABLES) {
            m_pDmaDescTables[m_dwNumOfTables] = (PADMA_32_DESC) OALDMAAllocBuffer(&m_dmaAdapter, PAGE_SIZE , &LogicalAddress, FALSE );
            if (m_pDmaDescTables[m_dwNumOfTables]) {
                m_dwDescTablePhysAddr[m_dwNumOfTables] = LogicalAddress.LowPart; // We are using 32 bit address.
                m_dwNumOfTables++;
            }
        }
        ASSERT(m_dwNumOfTables!=0);
        return (m_dwNumOfTables!=0);
    }
    return FALSE;
}



BOOL CESDHCBase32BitADMA::GetDMABuffer(SD_BUS_REQUEST& Request,BOOL fToDevice)
{
// We shouldn't use PAGE_SHIFT in CEDDK because it is too confusing and too dangerouse.
#define PFN_SHIEFT UserKInfo[KINX_PFN_SHIFT]
#define MAX_SUPPORTED_PFN (MAXDWORD>>PFN_SHIEFT)
#define PAGE_MASK (PAGE_SIZE-1)
    ASSERT(!m_fDMAProcessing);
    ASSERT(m_hDma==NULL);
    ASSERT(m_fLocked == FALSE);
    m_StartBuffer.pSrcVirtualAddr = NULL;
    m_StartBuffer.pSrcSize = 0;
    m_EndBuffer.pSrcVirtualAddr = NULL;
    m_EndBuffer.pSrcSize = 0;
    BOOL fReturn = FALSE;
    // Check for the limitaion.
    if (Request.NumBlocks > 1 && (Request.BlockSize & (sizeof(DWORD)-1)) == 0 
            && ((DWORD)Request.pBlockBuffer & (sizeof(DWORD)-1)) == 0
            && m_hDma == NULL) {
        // We are going to transfer this block as DMA.
        if ( (Request.Flags & SD_BUS_REQUEST_PHYS_BUFFER)!=0 && 
            Request.cbSizeOfPhysList!=0 && Request.pPhysBuffList!=NULL) {
            // We have user passed in Physical Buffer List.
            if (ReAllocateDMABufferList(Request.cbSizeOfPhysList) >= Request.cbSizeOfPhysList) {
                //Copy the Buffer.
                ASSERT(Request.cbSizeOfPhysList <= m_dwNumOfList);
                m_dwNumOfAvailabe = Request.cbSizeOfPhysList ;
                PBYTE pVirtualAddr = Request.pBlockBuffer ;
                for (DWORD dwIndex = 0; dwIndex < Request.cbSizeOfPhysList && dwIndex< m_dwNumOfList; dwIndex ++) {
                    m_pDmaBufferList[dwIndex].dwLength = Request.pPhysBuffList[dwIndex].PhysLen;
                    m_pDmaBufferList[dwIndex].physicalAddress = Request.pPhysBuffList[dwIndex].PhysAddr;
                    m_pDmaBufferList[dwIndex].virtualAddress = pVirtualAddr + m_pDmaBufferList[dwIndex].dwLength;
                }
                fReturn = TRUE;
                
            }
        }
        else { // We need figure out the physical address by using CEDDK DMA function.
            DWORD dwLength = Request.BlockSize * Request.NumBlocks;
            PVOID pUseBufferPtr = Request.pBlockBuffer;
            m_dmaAdapter.dwFlags =(fToDevice? DMA_FLAGS_WRITE_TO_DEVICE: 0);

            m_dwNumOfAvailabe = ADDRESS_AND_SIZE_TO_SPAN_PAGES( pUseBufferPtr, dwLength );
            
            PDWORD pdwPhysAddress = new DWORD[m_dwNumOfAvailabe];
            if (pdwPhysAddress) {
                if (m_dwNumOfAvailabe> m_dwNumOfList) {
                    ReAllocateDMABufferList(m_dwNumOfAvailabe);
                }
                if (m_dwNumOfAvailabe<= m_dwNumOfList ) {
                    m_fLocked = LockPages( pUseBufferPtr,dwLength,pdwPhysAddress, // m_pdwPhysAddress to stall PFN temperory.
                        fToDevice? LOCKFLAG_READ: LOCKFLAG_WRITE);
                    if (m_fLocked) { // Create table for Physical Address and length.
                        m_lpvLockedAddress = pUseBufferPtr;
                        m_dwLockedSize = dwLength;
                        
                        fReturn = TRUE;
                        for (DWORD dwIndex = 0; dwIndex< m_dwNumOfAvailabe; dwIndex++) {
                            if (pdwPhysAddress[dwIndex] > MAX_SUPPORTED_PFN) {
                                ASSERT(FALSE);
                                fReturn = FALSE;
                                break;
                            }
                            else {
                                m_pDmaBufferList[dwIndex].dwLength = min((PAGE_SIZE - ((DWORD)pUseBufferPtr & PAGE_MASK)),dwLength) ;

                                m_pDmaBufferList[dwIndex].physicalAddress.LowPart = (pdwPhysAddress[dwIndex]<<PFN_SHIEFT) + ((DWORD)pUseBufferPtr & PAGE_MASK);
                                m_pDmaBufferList[dwIndex].physicalAddress.HighPart = 0;
                                m_pDmaBufferList[dwIndex].virtualAddress = (PBYTE)pUseBufferPtr;
                                dwLength -= m_pDmaBufferList[dwIndex].dwLength;
                                pUseBufferPtr = (PBYTE)pUseBufferPtr+m_pDmaBufferList[dwIndex].dwLength;

                            }
                            
                        };

                    }
                }
                delete[] pdwPhysAddress;
            }
            if (fReturn && m_dwNumOfAvailabe) { // Check for Cache aligh begin and end.
            
                if (!fToDevice) {
                    DWORD dwDCacheLineSize = GetDataCacheSize() ;

                    ASSERT ((dwDCacheLineSize & dwDCacheLineSize-1) == 0 ); // I has to be nature align.
                    
                    // First Block
                    if ((m_pDmaBufferList[0].physicalAddress.LowPart & PAGE_MASK)){// Not PAGE align. we have to do something.
                        m_StartBuffer.dwBufferOffset = (m_StartBuffer.physicalAddress.LowPart & PAGE_MASK);
                        m_StartBuffer.pSrcSize = m_pDmaBufferList[0].dwLength;
                        m_StartBuffer.pSrcVirtualAddr = m_pDmaBufferList[0].virtualAddress;
                        m_pDmaBufferList[0].physicalAddress.QuadPart = m_StartBuffer.physicalAddress.QuadPart + m_StartBuffer.dwBufferOffset;
                        //m_pDmaBufferList[0].virtualAddress = (PBYTE)m_StartBuffer.pBufferedVirtualAddr + m_StartBuffer.dwBufferOffset;
                    }

                    // check tail buffer only if there is more than 1 buffer, or if with 1 buffer the StartBuffer is not substituted for it
                    if((m_dwNumOfAvailabe > 1 || m_StartBuffer.pSrcVirtualAddr == NULL)
                        &&  (m_pDmaBufferList[m_dwNumOfAvailabe-1].dwLength & (dwDCacheLineSize-1))!=0) {
                        m_EndBuffer.dwBufferOffset = 0 ;
                        m_EndBuffer.pSrcSize = m_pDmaBufferList[m_dwNumOfAvailabe-1].dwLength;
                        m_EndBuffer.pSrcVirtualAddr = m_pDmaBufferList[m_dwNumOfAvailabe-1].virtualAddress;
                        m_pDmaBufferList[m_dwNumOfAvailabe-1].physicalAddress = m_EndBuffer.physicalAddress;
                        //m_pDmaBufferList[m_dwNumOfAvailabe-1].virtualAddress = m_EndBuffer.pBufferedVirtualAddr;
                    }


                    // For Reads: Flush and invalidate buffers BEFORE dma starts in case of any dirty data (that may overwrite dma data)
                    for (DWORD dwIndex = 0 ; dwIndex<m_dwNumOfAvailabe ; dwIndex++) 
                    {
                        CacheRangeFlush(m_pDmaBufferList[dwIndex].virtualAddress,m_pDmaBufferList[dwIndex].dwLength, CACHE_SYNC_DISCARD );
                    }
                    
                }
            }
            else {
                fReturn = FALSE;
                if (m_fLocked) {
                    UnlockPages(m_lpvLockedAddress, m_dwLockedSize);
                };
                m_fLocked = FALSE;
            }
            if (fReturn && fToDevice) {

                //check for page aligned
                // First Block
                if ((m_pDmaBufferList[0].physicalAddress.LowPart & PAGE_MASK) != 0) { // Not PAGE align. we have to do something.
                        m_StartBuffer.dwBufferOffset = (m_StartBuffer.physicalAddress.LowPart & PAGE_MASK);
                        m_StartBuffer.pSrcSize = m_pDmaBufferList[0].dwLength;
                        m_StartBuffer.pSrcVirtualAddr = m_pDmaBufferList[0].virtualAddress;
                        m_pDmaBufferList[0].physicalAddress.QuadPart = m_StartBuffer.physicalAddress.QuadPart + m_StartBuffer.dwBufferOffset;
                        
                        CeSafeCopyMemory ((PBYTE)m_StartBuffer.pBufferedVirtualAddr+ m_StartBuffer.dwBufferOffset,
                                          m_StartBuffer.pSrcVirtualAddr, m_StartBuffer.pSrcSize);

                        // update address of 1st buffer to StartBuffer because we need to flush out this data to memory before starting DMA
                        m_pDmaBufferList[0].virtualAddress = (PBYTE)m_StartBuffer.pBufferedVirtualAddr + m_StartBuffer.dwBufferOffset;
                
                }

                // for writes, flush out data to memory before starting DMA to card
                for (DWORD dwIndex = 0 ; dwIndex<m_dwNumOfAvailabe ; dwIndex++) {
                    CacheRangeFlush(m_pDmaBufferList[dwIndex].virtualAddress,m_pDmaBufferList[dwIndex].dwLength, CACHE_SYNC_WRITEBACK );
                }                


                // we don't need to check tail because only the address needs to be page aligned, not length. Cache line misalignment is ok
            }
        }
        ASSERT(fReturn);        
    }
    return fReturn;

}




BOOL CESDHCBase32BitADMA::ArmDMA(SD_BUS_REQUEST& Request,BOOL fToDevice )
{
    BOOL fResult = FALSE ;
    DWORD dwAddr,dwLength;
    DWORD dwMaxLenPerDesc = 0x10000;
    fResult = GetDMABuffer(Request,fToDevice);//CESDHCBaseDMA::ArmDMA(Request,fToDevice);

    if (fResult){
        DWORD dwCurTable = 0 ;
        DWORD dwCurEntry = 0 ;
        DWORD dwCurPhysicalPage = 0;
        PADMA_32_DESC pCurTable = m_pDmaDescTables[dwCurTable];
        PADMA_32_DESC pCurEntry = pCurTable + dwCurEntry;
        while (dwCurPhysicalPage < m_dwNumOfAvailabe) {
            pCurTable = m_pDmaDescTables[dwCurTable];
            pCurEntry = pCurTable + dwCurEntry;
                  //= (UINT32*)pCurEntry;
            
            // Setup Descriptor
            dwLength = m_pDmaBufferList[dwCurPhysicalPage].dwLength;
            dwAddr = m_pDmaBufferList[dwCurPhysicalPage].physicalAddress.LowPart;
            while (dwLength > dwMaxLenPerDesc ){
              pCurEntry->Valid = 1 ;
              pCurEntry->End = 0 ;
              pCurEntry->Int = 0 ;
              pCurEntry->Act = 1 ; // set length.
              pCurEntry->AddrLength = dwMaxLenPerDesc;
              
              dwCurEntry++;
              pCurEntry = pCurTable+dwCurEntry;
              
              pCurEntry->Valid = 1 ;
              pCurEntry->End = 0 ;
              pCurEntry->Int = 0 ;
              pCurEntry->Act = 2 ; // trans.
              pCurEntry->AddrLength = dwAddr >> 12;
              
              dwCurEntry++;
              pCurEntry = pCurTable+dwCurEntry;
            
              dwLength -= dwMaxLenPerDesc;
              dwAddr += dwMaxLenPerDesc;
            }
            
            //set the last block
            
            pCurEntry->Valid = 1 ;
            pCurEntry->End = 0 ;
            pCurEntry->Int = 0 ;
            pCurEntry->Act = 1 ; // set length.
            pCurEntry->AddrLength = dwLength;
              
            dwCurEntry++;
            pCurEntry = pCurTable+dwCurEntry;
              
            pCurEntry->Valid = 1 ;
            pCurEntry->End = 0 ;
            pCurEntry->Int = 0 ;
            pCurEntry->Act = 2 ; // trans.
            pCurEntry->AddrLength = dwAddr >> 12;
            
            //dwLength -= dwMaxLenPerDesc;
            //dwAddr += dwMaxLenPerDesc;
            //set length
            
            /*pCurEntry->Valid = 1 ;
            pCurEntry->End = 0 ;
            pCurEntry->Int = 0 ;
            pCurEntry->Act = 2 ; // Transfer.
            pCurEntry->Length = m_pDmaBufferList[dwCurPhysicalPage].dwLength;
            pCurEntry->Address = m_pDmaBufferList[dwCurPhysicalPage].physicalAddress.LowPart;*/


            dwCurPhysicalPage++;
            
            if (dwCurPhysicalPage < m_dwNumOfAvailabe) { // We have more
                dwCurEntry++;
                /* assume we have only one table here
                if (dwCurEntry>= DESC_ENTRY_PER_TABLE -1 ) { // We reserv last one for Link Descriptor.
                    pCurEntry = pCurTable+dwCurEntry;
                    // Setup link.
                    pCurEntry->Valid = 1 ;
                    pCurEntry->End = 0 ;
                    pCurEntry->Int = 0 ;
                    pCurEntry->Act = 3 ; // Link
                    pCurEntry->Length = 0;
                    pCurEntry->Address = m_dwDescTablePhysAddr[dwCurTable+1];
                    dwCurTable ++; 
                    dwCurEntry = 0;
                    
                    if (dwCurTable>=m_dwNumOfTables) { // For some reason we exceed.
                        ASSERT(FALSE);
                        break;
                    }
                }*/
            }
            else { // We finished here.
                // Change this link to end
                pCurEntry->End = 1;
                fResult = TRUE;

                break;
            }

        }
        
        if (fResult) {
            OUTREG32(&(m_ESDHCBase.m_pESDHCReg->ADSADDR), m_dwDescTablePhysAddr[0]);
            m_fDMAProcessing = TRUE;
        }
        else {
            ASSERT(FALSE);
        }

    }
    return fResult;
}

BOOL CESDHCBase32BitADMA::DMANotifyEvent(SD_BUS_REQUEST& Request, DMAEVENT dmaEvent)
{

    switch (dmaEvent) {
      case DMA_COMPLETE:
        //ASSERT(FALSE);
        m_dwDMACompletionCode = ERROR_NOT_READY;
        m_fDMAProcessing = FALSE;
            Request.HCParam = Request.BlockSize*Request.NumBlocks;
        break;
       
      case TRANSFER_COMPLETED:
        ASSERT(m_dwCurDMAListPos <= m_dwNumOfAvailabe);
        if (m_fDMAProcessing && m_pDmaBufferList && m_dwCurDMAListPos<m_dwNumOfAvailabe) {
            Request.HCParam = Request.BlockSize*Request.NumBlocks;
        }
        m_fDMAProcessing = FALSE;
        break;
      case DMA_ERROR_OCCOR:
        m_fDMAProcessing = FALSE; 
        m_dwDMACompletionCode = ERROR_NOT_READY;
        
        //BYTE ADMAErrorStatus = m_ESDHCBase.ReadByte(SDHC_ADMA_ERROR_STATUS);

        //UINT32 ADMAErrorStatus = INREG32(&(m_ESDHCBase.m_pESDHCReg->ADMAES));
        //RETAILMSG(1,(TEXT("ADMA Erorr Status 0x%x: Refer to 2.2.30"), ADMAErrorStatus));
        
    
        break;
      default:
        break;
    }
    if (!m_fDMAProcessing && m_hDma) {
        DMACloseBuffer(m_hDma);
        m_hDma = NULL;
    }
    else if (!m_fDMAProcessing && m_fLocked && m_pDmaBufferList) {
        if (Request.TransferClass != SD_WRITE) {
            if (m_StartBuffer.pSrcVirtualAddr!=NULL && m_StartBuffer.pSrcSize!=0) {
                CeSafeCopyMemory (m_StartBuffer.pSrcVirtualAddr, (PBYTE)m_StartBuffer.pBufferedVirtualAddr + m_StartBuffer.dwBufferOffset,m_StartBuffer.pSrcSize);
            }
            if (m_EndBuffer.pSrcVirtualAddr!=NULL && m_EndBuffer.pSrcSize!=0) {
                CeSafeCopyMemory (m_EndBuffer.pSrcVirtualAddr, m_EndBuffer.pBufferedVirtualAddr,m_EndBuffer.pSrcSize);
            }  
        }
        if (m_fLocked )
            UnlockPages( m_lpvLockedAddress, m_dwLockedSize);
        m_fLocked = FALSE;
    }
    return TRUE;
}


CESDHCBase32BitADMA2::CESDHCBase32BitADMA2(CESDHCBase& ESDHCBase)
:    CESDHCBaseDMA(ESDHCBase)
{
    DEBUGMSG(SDCARD_ZONE_INIT, (_T("CESDHCBase32BitADMA2:Create DMA Object for ADMA2\r\n")));        
    m_dwNumOfTables =0;
    for (DWORD dwIndex=0; dwIndex < MAXIMUM_DESC_TABLES; dwIndex++) {
        m_pDmaDescTables[dwIndex] = NULL;
        m_dwDescTablePhysAddr[dwIndex] = 0 ;
    }
        
};
CESDHCBase32BitADMA2::~CESDHCBase32BitADMA2()
{
    for (DWORD dwIndex=0; dwIndex< m_dwNumOfTables; dwIndex++) {
        ASSERT(m_pDmaDescTables[dwIndex]);
        ASSERT(m_dwDescTablePhysAddr[dwIndex]);
        PHYSICAL_ADDRESS LogicalAddress = {m_dwDescTablePhysAddr[dwIndex],0};
        OALDMAFreeBuffer(&m_dmaAdapter,PAGE_SIZE,LogicalAddress,m_pDmaDescTables[dwIndex],FALSE);
    }

    if (m_fLocked )
        UnlockPages( m_lpvLockedAddress, m_dwLockedSize);    
}
BOOL CESDHCBase32BitADMA2::Init()
{
    m_fLocked= FALSE;

    if (CESDHCBaseDMA::Init()) {
        PHYSICAL_ADDRESS LogicalAddress;
        ASSERT(m_dwNumOfTables==0);
        if (m_dwNumOfTables<MAXIMUM_DESC_TABLES) {
            m_pDmaDescTables[m_dwNumOfTables] = (PADMA2_32_DESC) OALDMAAllocBuffer(&m_dmaAdapter, PAGE_SIZE , &LogicalAddress, FALSE );
            if (m_pDmaDescTables[m_dwNumOfTables]) {
                m_dwDescTablePhysAddr[m_dwNumOfTables] = LogicalAddress.LowPart; // We are using 32 bit address.
                m_dwNumOfTables++;
            }
        }
        ASSERT(m_dwNumOfTables!=0);
        return (m_dwNumOfTables!=0);
    }
    return FALSE;
}


BOOL CESDHCBase32BitADMA2::GetDMABuffer(SD_BUS_REQUEST& Request,BOOL fToDevice)
{
// We shouldn't use PAGE_SHIFT in CEDDK because it is too confusing and too dangerouse.
#define PFN_SHIEFT UserKInfo[KINX_PFN_SHIFT]
#define MAX_SUPPORTED_PFN (MAXDWORD>>PFN_SHIEFT)
#define PAGE_MASK (PAGE_SIZE-1)
    ASSERT(!m_fDMAProcessing);
    ASSERT(m_hDma==NULL);
    ASSERT(m_fLocked == FALSE);

    m_StartBuffer.pSrcVirtualAddr = NULL;
    m_StartBuffer.pSrcSize = 0;
    m_EndBuffer.pSrcVirtualAddr = NULL;
    m_EndBuffer.pSrcSize = 0;

    BOOL fReturn = FALSE;
    // Check for the limitaion.
    if (Request.NumBlocks > 1 && (Request.BlockSize & (sizeof(DWORD)-1)) == 0 
            && ((DWORD)Request.pBlockBuffer & (sizeof(DWORD)-1)) == 0
            && m_hDma == NULL) {
        // We are going to transfer this block as DMA.
        if ( (Request.Flags & SD_BUS_REQUEST_PHYS_BUFFER)!=0 && 
            Request.cbSizeOfPhysList!=0 && Request.pPhysBuffList!=NULL) {
            // We have user passed in Physical Buffer List.
            if (ReAllocateDMABufferList(Request.cbSizeOfPhysList) >= Request.cbSizeOfPhysList) {
                //Copy the Buffer.
                ASSERT(Request.cbSizeOfPhysList <= m_dwNumOfList);
                m_dwNumOfAvailabe = Request.cbSizeOfPhysList ;
                PBYTE pVirtualAddr = Request.pBlockBuffer ;
                for (DWORD dwIndex = 0; dwIndex < Request.cbSizeOfPhysList && dwIndex< m_dwNumOfList; dwIndex ++) {
                    m_pDmaBufferList[dwIndex].dwLength = Request.pPhysBuffList[dwIndex].PhysLen;
                    m_pDmaBufferList[dwIndex].physicalAddress = Request.pPhysBuffList[dwIndex].PhysAddr;
                    m_pDmaBufferList[dwIndex].virtualAddress = pVirtualAddr + m_pDmaBufferList[dwIndex].dwLength;
                }
                fReturn = TRUE;
                
            }
        }
        else { // We need figure out the physical address by using CEDDK DMA function.
            DWORD dwLength = Request.BlockSize * Request.NumBlocks;
            PVOID pUseBufferPtr = Request.pBlockBuffer;
            m_dmaAdapter.dwFlags =(fToDevice? DMA_FLAGS_WRITE_TO_DEVICE: 0);

            m_dwNumOfAvailabe = ADDRESS_AND_SIZE_TO_SPAN_PAGES( pUseBufferPtr, dwLength );
            
            PDWORD pdwPhysAddress = new DWORD[m_dwNumOfAvailabe];
            if (pdwPhysAddress) {
                if (m_dwNumOfAvailabe> m_dwNumOfList) {
                    ReAllocateDMABufferList(m_dwNumOfAvailabe);
                }
                if (m_dwNumOfAvailabe<= m_dwNumOfList ) {
                    m_fLocked = LockPages( pUseBufferPtr,dwLength,pdwPhysAddress, // m_pdwPhysAddress to stall PFN temperory.
                        fToDevice? LOCKFLAG_READ: LOCKFLAG_WRITE);
                    if (m_fLocked) { // Create table for Physical Address and length.
                        m_lpvLockedAddress = pUseBufferPtr;
                        m_dwLockedSize = dwLength;
                        
                        fReturn = TRUE;
                        for (DWORD dwIndex = 0; dwIndex< m_dwNumOfAvailabe; dwIndex++) {
                            if (pdwPhysAddress[dwIndex] > MAX_SUPPORTED_PFN) {
                                ASSERT(FALSE);
                                fReturn = FALSE;
                                break;
                            }
                            else {
                                m_pDmaBufferList[dwIndex].dwLength = min((PAGE_SIZE - ((DWORD)pUseBufferPtr & PAGE_MASK)),dwLength) ;

                                m_pDmaBufferList[dwIndex].physicalAddress.LowPart = (pdwPhysAddress[dwIndex]<<PFN_SHIEFT) + ((DWORD)pUseBufferPtr & PAGE_MASK);
                                m_pDmaBufferList[dwIndex].physicalAddress.HighPart = 0;
                                m_pDmaBufferList[dwIndex].virtualAddress = (PBYTE)pUseBufferPtr;
                                dwLength -= m_pDmaBufferList[dwIndex].dwLength;
                                pUseBufferPtr = (PBYTE)pUseBufferPtr+m_pDmaBufferList[dwIndex].dwLength;

                            }
                            
                        };

                    }
                }
                delete[] pdwPhysAddress;
            }
            if (fReturn && m_dwNumOfAvailabe) { // Check for Cache aligh begin and end.

                if (!fToDevice) {

                    DWORD dwDCacheLineSize = GetDataCacheSize() ;
                    ASSERT ((dwDCacheLineSize & dwDCacheLineSize-1) == 0 ); // I has to be nature align.
                    
                    // First page
                    if ((m_pDmaBufferList[0].physicalAddress.LowPart & (dwDCacheLineSize-1)) != 0) { // Addr not cache aligned. we have to do something.

                        m_StartBuffer.dwBufferOffset = (m_pDmaBufferList[0].physicalAddress.LowPart & PAGE_MASK);
                        m_StartBuffer.pSrcSize = m_pDmaBufferList[0].dwLength;
                        m_StartBuffer.pSrcVirtualAddr = m_pDmaBufferList[0].virtualAddress;
                        m_pDmaBufferList[0].physicalAddress.QuadPart = m_StartBuffer.physicalAddress.QuadPart + m_StartBuffer.dwBufferOffset;
                        //m_pDmaBufferList[0].virtualAddress = (PBYTE)m_StartBuffer.pBufferedVirtualAddr + m_StartBuffer.dwBufferOffset;

                    }

                    // check tail buffer only if there is more than 1 buffer, or if with 1 buffer the StartBuffer is not substituted for it
                    if((m_dwNumOfAvailabe > 1 || m_StartBuffer.pSrcVirtualAddr == NULL)
                        &&  (m_pDmaBufferList[m_dwNumOfAvailabe-1].dwLength & (dwDCacheLineSize-1))!=0) {
                        m_EndBuffer.dwBufferOffset = 0 ;
                        m_EndBuffer.pSrcSize = m_pDmaBufferList[m_dwNumOfAvailabe-1].dwLength;
                        m_EndBuffer.pSrcVirtualAddr = m_pDmaBufferList[m_dwNumOfAvailabe-1].virtualAddress;
                        m_pDmaBufferList[m_dwNumOfAvailabe-1].physicalAddress = m_EndBuffer.physicalAddress;
                        //m_pDmaBufferList[m_dwNumOfAvailabe-1].virtualAddress = m_EndBuffer.pBufferedVirtualAddr;
                    }

                    // For Reads: Flush and invalidate buffers BEFORE dma starts in case of any dirty data (that may overwrite dma data after it has completed)
                    for (DWORD dwIndex = 0 ; dwIndex<m_dwNumOfAvailabe ; dwIndex++) 
                    {
                        CacheRangeFlush(m_pDmaBufferList[dwIndex].virtualAddress,m_pDmaBufferList[dwIndex].dwLength, CACHE_SYNC_DISCARD );
                    }

                }
                
                else {
                    //for writes, flush out any data before DMA starts
                    for (DWORD dwIndex = 0 ; dwIndex<m_dwNumOfAvailabe ; dwIndex++) 
                    {
                        CacheRangeFlush(m_pDmaBufferList[dwIndex].virtualAddress,m_pDmaBufferList[dwIndex].dwLength, CACHE_SYNC_WRITEBACK );
                    }
                }
            }
            else {
                fReturn = FALSE;
                if (m_fLocked) {
                    UnlockPages(m_lpvLockedAddress, m_dwLockedSize);
                };
                m_fLocked = FALSE;
            }
        }
        ASSERT(fReturn);        
    }
    return fReturn;

}


BOOL CESDHCBase32BitADMA2::ArmDMA(SD_BUS_REQUEST& Request,BOOL fToDevice )
{
    BOOL fResult = GetDMABuffer(Request,fToDevice);
    
    if (fResult && IsEnoughDescTable(m_dwNumOfAvailabe) ) {

        DWORD dwCurTable = 0 ;
        DWORD dwCurEntry = 0 ;
        DWORD dwCurPhysicalPage = 0;
        while (dwCurPhysicalPage < m_dwNumOfAvailabe) {
            PADMA2_32_DESC pCurTable = m_pDmaDescTables[dwCurTable];
            PADMA2_32_DESC pCurEntry = pCurTable + dwCurEntry;
            // Setup Descriptor
            pCurEntry->Valid = 1 ;
            pCurEntry->End = 0 ;
            pCurEntry->Int = 0 ;
            pCurEntry->Act = 2 ; // Transfer.
            pCurEntry->Length = m_pDmaBufferList[dwCurPhysicalPage].dwLength;
            pCurEntry->Address = m_pDmaBufferList[dwCurPhysicalPage].physicalAddress.LowPart;

            dwCurPhysicalPage++;
            
            if (dwCurPhysicalPage < m_dwNumOfAvailabe) { // We have more
                dwCurEntry++;
                if (dwCurEntry>= DESC_ENTRY_PER_TABLE_ADMA2 -1 ) { // We reserv last one for Link Descriptor.
                    pCurEntry = pCurTable+dwCurEntry;
                    // Setup link.
                    pCurEntry->Valid = 1 ;
                    pCurEntry->End = 0 ;
                    pCurEntry->Int = 0 ;
                    pCurEntry->Act = 3 ; // Link
                    pCurEntry->Length = 0;
                    pCurEntry->Address = m_dwDescTablePhysAddr[dwCurTable+1];
                    dwCurTable ++; 
                    dwCurEntry = 0;
                    
                    if (dwCurTable>=m_dwNumOfTables) { // For some reason we exceed.
                        ASSERT(FALSE);
                        break;
                    }
                }
            }
            else { // We finished here.
                // Change this link to end
                pCurEntry->End = 1;
                pCurEntry->Int = 1;
                fResult = TRUE;
                break;
            }

        }
        // Arm the first buffer.
        if (fResult) {
            OUTREG32(&(m_ESDHCBase.m_pESDHCReg->ADSADDR), m_dwDescTablePhysAddr[0] ); // 32-bit address.
            m_fDMAProcessing = TRUE;
        }
        else {
            ASSERT(FALSE);
        }

    }
    return fResult;
}
BOOL CESDHCBase32BitADMA2::IsEnoughDescTable(DWORD dwNumOfBlock)
{
    DWORD dwNumOfEntryPerTable = DESC_ENTRY_PER_TABLE_ADMA2 -1; // we reserv one for the link.
    DWORD dwNumOfTable = (dwNumOfBlock+dwNumOfEntryPerTable-1)/dwNumOfEntryPerTable;
    if (dwNumOfTable> MAXIMUM_DESC_TABLES)
        return FALSE;
    if (dwNumOfTable> m_dwNumOfTables && dwNumOfTable < MAXIMUM_DESC_TABLES) { // we need allocate more
        for (DWORD dwIndex = m_dwNumOfTables; dwIndex< dwNumOfTable; dwIndex++) {
            PHYSICAL_ADDRESS LogicalAddress;
            m_pDmaDescTables[m_dwNumOfTables] = (PADMA2_32_DESC) OALDMAAllocBuffer(&m_dmaAdapter, PAGE_SIZE , &LogicalAddress, FALSE );
            if (m_pDmaDescTables[m_dwNumOfTables]) {
                m_dwDescTablePhysAddr[m_dwNumOfTables] = LogicalAddress.LowPart; // We are using 32 bit address.
                m_dwNumOfTables++;
            }
            else
                break;
        }
        if (dwNumOfTable!=m_dwNumOfTables) {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    return TRUE;
    
}
BOOL CESDHCBase32BitADMA2::DMANotifyEvent(SD_BUS_REQUEST& Request, DMAEVENT dmaEvent)
{
    switch (dmaEvent) {

      case TRANSFER_COMPLETED:
        ASSERT(m_dwCurDMAListPos <= m_dwNumOfAvailabe);
        if (m_fDMAProcessing && m_pDmaBufferList && m_dwCurDMAListPos<m_dwNumOfAvailabe) {
            Request.HCParam = Request.BlockSize*Request.NumBlocks;
        }
        m_fDMAProcessing = FALSE;
        break;
      case DMA_ERROR_OCCOR:
        m_fDMAProcessing = FALSE; {
        m_dwDMACompletionCode = ERROR_NOT_READY;

#if DEBUG        
        DWORD ADMAErrorStatus = INREG32(&(m_ESDHCBase.m_pESDHCReg->ADMAES));
        DEBUGMSG(SDCARD_ZONE_ERROR,(TEXT("ADMA Error Status 0x%x"), ADMAErrorStatus));
#endif
        }
        break;

      case DMA_COMPLETE:
        Request.HCParam = Request.BlockSize*Request.NumBlocks;
        m_dwDMACompletionCode = ERROR_NOT_READY;
        m_fDMAProcessing = FALSE;
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    if (!m_fDMAProcessing && m_fLocked) {

        if (Request.TransferClass != SD_WRITE) {
            if (m_StartBuffer.pSrcVirtualAddr!=NULL && m_StartBuffer.pSrcSize!=0) {
                CeSafeCopyMemory (m_StartBuffer.pSrcVirtualAddr, (PBYTE)m_StartBuffer.pBufferedVirtualAddr + m_StartBuffer.dwBufferOffset,m_StartBuffer.pSrcSize);
            }
            if (m_EndBuffer.pSrcVirtualAddr!=NULL && m_EndBuffer.pSrcSize!=0) {
                CeSafeCopyMemory (m_EndBuffer.pSrcVirtualAddr, m_EndBuffer.pBufferedVirtualAddr,m_EndBuffer.pSrcSize);
            }  
        }

        if (m_fLocked )
            UnlockPages( m_lpvLockedAddress, m_dwLockedSize);
        m_fLocked = FALSE;
    }
    
    return TRUE;
}

