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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
// 
// Module Name:  
//     Transfer.cpp
#include "cpipe.hpp"
#include "cphysmem.hpp"
#include "chw.hpp"
#include "cohcd.hpp"

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

// !!!NOTE!!!
// 4KB system SRAM memory which is shared between CPU and USB must be accessed
// by 4 bytes, so the general memcpy function cann't be used in this situation,
// make a special function to conform the restriction.
static void CopyToShareMemory(PVOID pDst, PCVOID pSrc, DWORD dwLen)
{
    PDWORD pdwDst = (PDWORD)pDst;
    PBYTE pbSrc = (PBYTE)pSrc;
    DWORD dwCnt1, dwCnt2;
    union {
        DWORD dwData;
        BYTE bDatas[4];
    } dwTemp;

    ASSERT((((DWORD)pDst)&0x03)==0); // must 4 bytes alignment

    for (dwCnt2 = 0; dwCnt2 < dwLen;) {
        dwTemp.dwData = 0;
        for (dwCnt1 = 0; dwCnt1<4 && dwCnt2<dwLen; dwCnt1++, dwCnt2++)
            dwTemp.bDatas[dwCnt1] = *(pbSrc++);
        *(pdwDst++) = dwTemp.dwData; // no problem, memory block always round up to 32 bytes alignment
    }
}

static void CopyFromShareMemory(PVOID pDst, PCVOID pSrc, DWORD dwLen)
{
    PBYTE pbDst = (PBYTE)pDst;
    PDWORD pdwSrc = (PDWORD)pSrc;
    DWORD dwCnt1, dwCnt2;
    union {
        DWORD dwData;
        BYTE bDatas[4];
    } dwTemp;

    ASSERT((((DWORD)pSrc)&0x03)==0); // must 4 bytes alignment

    for (dwCnt2 = 0; dwCnt2 < dwLen;) {
        dwTemp.dwData = *(pdwSrc++); // no problem, memory block always round up to 32 bytes alignment
        for (dwCnt1 = 0; dwCnt1<4 && dwCnt2<dwLen; dwCnt1++, dwCnt2++)
            *(pbDst++) = dwTemp.bDatas[dwCnt1];
    }
}

CTransfer::CTransfer(CQueuedPipe* const  rPipe, STransfer sTransfer,CTransfer *lpNextTransfer) 
: m_rPipe(rPipe)
, m_lpNextTransfer(lpNextTransfer)
{
    m_Transfer = sTransfer;
    // HD64465 doesn't support custom physical memory
    m_Transfer.paClientBuffer = 0;
    m_Transfer.paClientBuffer = 0;

    if ( m_Transfer.lpvControlHeader!=NULL ) { // Control Transfer
        m_dwNextToQueueIndex = m_dwNextToCompleteIndex=-1; // Need Setup.
        m_bNeedStatus = m_bNeedStatusComplete = TRUE;
        m_bZeroLengthCompleteNeeded = m_bZeroLengthNeeded = FALSE ;
    }
    else {
        m_dwNextToQueueIndex=m_dwNextToCompleteIndex=0; // Need Setup.
        m_bNeedStatus = m_bNeedStatusComplete = FALSE;
        m_bZeroLengthCompleteNeeded = m_bZeroLengthNeeded = (m_Transfer.dwBufferSize == 0 ) ;
    };
    m_dwQueueTD =0;
    m_dwFirstError=USB_NO_ERROR;
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    __try { // initializing transfer status parameters
        *m_Transfer.lpfComplete = FALSE;
        *m_Transfer.lpdwBytesTransferred = 0;
        *m_Transfer.lpdwError = USB_NOT_COMPLETE_ERROR;
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
    }
#pragma prefast(pop)
};
// ******************************************************************               
// Scope: public
BOOL CTransfer::ScheduleTD()
//
// Purpose: Schedule TD for this transfer.
//
// return : TRUE: success schedule TD, FALSE: Error happens.
//
// Parameters: Nothing
//
// ******************************************************************
{
    DWORD dwMaxTDDataSize = m_rPipe->GetMaxTDDataSize() ;
    if (m_rPipe->IsTDQueueFull())
        return FALSE;
    if (m_dwNextToQueueIndex == (DWORD)-1) { // QUEUE Control header.
        if (m_Transfer.lpvControlHeader !=NULL) {
            PBYTE pSetupData = m_rPipe->GetTDQueueTailBufferVirtAddr();
            __try {
                CopyToShareMemory(pSetupData,m_Transfer.lpvControlHeader,sizeof(USB_DEVICE_REQUEST));
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::ScheduleTD()(%s):Copy lpvControlHeader(%x) exception !!!\n"), m_rPipe->GetPipeType(),m_Transfer.lpvControlHeader ) );
            }
            BOOL bReturn =m_rPipe->InitTDQueueTailTD(
                  this,
                  m_rPipe,
                  1, // InterruptOnComplete,
                  TD_SETUP_PID, // PID 
                  TD::DATATOG_0, // Data Toggle (USB spec 8.5.2)
                  0,
                  sizeof( USB_DEVICE_REQUEST ), // MaxLength in (n-1) form
                  FALSE
                );
            ASSERT(bReturn==TRUE);
            if (bReturn){
                m_dwNextToQueueIndex=0;
                m_rPipe->AdvanceTDQueueTail();
            }
            else
                return FALSE;                
        
        }
        else {
            ASSERT(FALSE);
            return FALSE;
        }
    }
    while (!m_rPipe->IsTDQueueFull() && (m_dwNextToQueueIndex < m_Transfer.dwBufferSize || m_bZeroLengthNeeded)) { // if this is TD not full yet.
        DWORD dwLength = (m_bZeroLengthNeeded? 0 : min(dwMaxTDDataSize,m_Transfer.dwBufferSize - m_dwNextToQueueIndex));
        m_bZeroLengthNeeded = FALSE ;
        if ( m_Transfer.paClientBuffer == 0 && (m_Transfer.dwFlags & USB_IN_TRANSFER) ==0 && dwLength ) { // We need Copy data
            PBYTE pUserData = m_rPipe->GetTDQueueTailBufferVirtAddr();
            __try { // setting transfer status and executing callback function
                CopyToShareMemory( pUserData, (PBYTE)(m_Transfer.lpvClientBuffer) + m_dwNextToQueueIndex,dwLength);
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::ScheduleTD()(%s)::CheckForDoneTransfers - exception setting transfer status to complete\n"), m_rPipe->GetPipeType() ) );
            }
        }
        BOOL bReturn =m_rPipe->InitTDQueueTailTD( 
              this,
              m_rPipe,
              1, // InterruptOnComplete,
              ((m_Transfer.dwFlags & USB_IN_TRANSFER)==0)?TD_OUT_PID:TD_IN_PID, // PID 
              TD::DATATOG_ED, // Data Toggle (USB spec 8.5.2)
              (m_Transfer.paClientBuffer == 0)?0:m_Transfer.paClientBuffer+m_dwNextToQueueIndex ,
              dwLength, // MaxLength in (n-1) form
              FALSE
            );
        ASSERT(bReturn==TRUE);
        if (bReturn){
            m_dwNextToQueueIndex +=dwLength;
            m_rPipe->AdvanceTDQueueTail();
        }
        else
            return FALSE;                
    }
    if (m_bNeedStatus &&  m_dwNextToQueueIndex >= m_Transfer.dwBufferSize && !m_rPipe->IsTDQueueFull()) {        
        // We need Status for Controller Endpoint.
        BOOL bReturn =m_rPipe->InitTDQueueTailTD( 
              this,
              m_rPipe,
              1, // InterruptOnComplete,
              ((m_Transfer.dwFlags & USB_IN_TRANSFER)==0)?TD_IN_PID:TD_OUT_PID, // Reversed
              TD::DATATOG_1, // Data Toggle (USB spec 8.5.2)
              0 ,
              0, // MaxLength in (n-1) form
              FALSE
            );
        ASSERT(bReturn==TRUE);
        if (bReturn){
            m_rPipe->AdvanceTDQueueTail();
            m_bNeedStatus = FALSE;
        }
        else
            return FALSE;                
    }
    return TRUE;
}
// ******************************************************************               
// Scope: public
BOOL CTransfer::DoneTD()
//
// Purpose: One TD Done for this Transfer..
//
// return : TRUE: success Done TD, FALSE: Error happens.
//
// Parameters: Nothing
//
// ******************************************************************
{
    // Update the TD status.
    P_TD pCur= m_rPipe->GetTDQueueHead();
    if (m_dwNextToCompleteIndex == -1) { // If completed is setup packet, move to data stage.
        m_dwNextToCompleteIndex=0;        
    }
    else
    if (m_dwNextToCompleteIndex <m_Transfer.dwBufferSize || m_bZeroLengthCompleteNeeded ) { //Data completed 
        DWORD dwMaxTDDataSize = m_rPipe->GetMaxTDDataSize() ;
        DWORD dwLength = (m_bZeroLengthCompleteNeeded? 0 : min(dwMaxTDDataSize,m_Transfer.dwBufferSize - m_dwNextToCompleteIndex));
        m_bZeroLengthCompleteNeeded = FALSE;
        if (pCur->paCurBuffer) { // This is mean Short Packet completed or error happens in this TD.
            DWORD dwLeft = pCur->paBufferEnd + 1 - pCur->paCurBuffer;
            if (dwLeft <= dwLength)
                dwLength -= dwLeft;
        }
        if (m_Transfer.paClientBuffer == 0 && (m_Transfer.dwFlags & USB_IN_TRANSFER)!=0 && dwLength ) { // In transfer , we need copy.
            PBYTE pUserData = m_rPipe->GetTDQueueHeadBufferVirtAddr();
            __try { // initializing transfer status parameters
                CopyFromShareMemory ((PBYTE)(m_Transfer.lpvClientBuffer) + m_dwNextToCompleteIndex , pUserData,dwLength);
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::DoneTD()(%s)::exception setting transfer status to complete\n"), m_rPipe->GetPipeType() ) );
            }
        }
        m_dwNextToCompleteIndex += dwLength;
    }
    else 
    if (m_bNeedStatusComplete) { // Status Stage completed for Control Transfer.
         m_bNeedStatusComplete = FALSE;
    }
    if (m_dwFirstError== USB_NO_ERROR) {
        if (pCur->bfConditionCode == USB_DATA_UNDERRUN_ERROR && (m_Transfer.dwFlags & USB_SHORT_TRANSFER_OK)) {
            // For Control Endpoint, We need send status for short thransfer error.
            if (m_Transfer.lpvControlHeader!=NULL  && m_rPipe->GetED()->bfHalted!=0) {
                BOOL bSkip = (m_rPipe->GetED()->bfSkip!=0);
                ASSERT(m_bNeedStatusComplete==TRUE);
                m_Transfer.dwBufferSize = m_dwNextToQueueIndex = m_dwNextToCompleteIndex ;
                m_bNeedStatus = TRUE;
                m_rPipe->QueueInit();
                m_rPipe->GetED()->paTdQueueHead = m_rPipe->GetED()->paTdQueueTail = m_rPipe->GetTDQueueHeadPhys();
                m_rPipe->GetED()->bfHalted = 0; // let the HC process the ED again
                return FALSE;
            }
        } else {
            // This is really an error.
            m_dwFirstError = pCur->bfConditionCode;
        }
    }
    DWORD bReturn=FALSE;
    if (pCur->bfConditionCode !=  USB_NO_ERROR || 
            ((pCur->paCurBuffer!=0 || (m_dwNextToCompleteIndex>=m_Transfer.dwBufferSize && !m_bZeroLengthCompleteNeeded)) && !m_bNeedStatusComplete)) { // We are completed.
        bReturn=TRUE;
        __try { // initializing transfer status parameters
            *m_Transfer.lpfComplete = TRUE;
            *m_Transfer.lpdwBytesTransferred = m_dwNextToCompleteIndex;
            *m_Transfer.lpdwError = m_dwFirstError;
            if (m_Transfer.lpfnCallback ) {
                ( *m_Transfer.lpfnCallback )( m_Transfer.lpvCallbackParameter );
                m_Transfer.lpfnCallback =NULL;
            }
            
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::DoneTD()(%s)::exception setting transfer status to complete\n"), m_rPipe->GetPipeType() ) );
        }
        
    }    
    m_rPipe->AdvanceTDQueueHead();
    return bReturn;
};

BOOL CTransfer::Canceled()
{
    __try { // initializing transfer status parameters
        m_dwNextToQueueIndex=m_dwNextToCompleteIndex=m_Transfer.dwBufferSize;
        *m_Transfer.lpfComplete = TRUE;
        *m_Transfer.lpdwError = USB_CANCELED_ERROR;
        if (m_Transfer.lpfnCallback ) {
            ( *m_Transfer.lpfnCallback )( m_Transfer.lpvCallbackParameter );
            m_Transfer.lpfnCallback =NULL;
        }
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::DoneTD()(%s)::exception setting transfer status to complete\n"), m_rPipe->GetPipeType() ) );
    }
        
    return TRUE;

}

TDQueue::TDQueue(IN CPhysMem* const pCPhysMem, DWORD, DWORD) 
: m_dwNumOfTD(2) // one buffer due to limited memory on HD64465
, m_dwTDBufferSize(64) // maximum packet size for USB 1.1 control, bulk and interrupt transfer
, m_pCPhysMem( pCPhysMem)
{
    m_pTDQueue = NULL;
    m_pBuffer = NULL;
};
TDQueue::~TDQueue()
{
    QueueDeInit();
}
void TDQueue::QueueDeInit()
{
    if (m_pBuffer) {
        m_pCPhysMem->FreeMemory(m_pBuffer,m_pBufferPhysAddr,CPHYSMEM_FLAG_NOBLOCK);
        m_pBuffer=NULL;
        m_pBufferPhysAddr = 0;
    }
    if (m_pTDQueue) {
        m_pCPhysMem->FreeMemory((PBYTE)m_pTDQueue,m_pTDQueuePhysAddr,CPHYSMEM_FLAG_NOBLOCK);
        m_pTDQueue = NULL;
        m_pTDQueuePhysAddr = 0;
    }
}
BOOL TDQueue::QueueInit()
{
    if (m_pTDQueue==NULL) {
        if (!m_pCPhysMem->AllocateMemory( DEBUG_PARAM( TEXT("IssueTransfer TDs") )
                                m_dwNumOfTD *sizeof(TD),
                                (PUCHAR *) &m_pTDQueue,
                                CPHYSMEM_FLAG_NOBLOCK))  {
            DEBUGMSG( ZONE_WARNING, (TEXT("TDQueue::TDQueue - no memory for TD list\n") ) );
            m_pTDQueue= NULL;
        }
        else {
            m_pTDQueuePhysAddr = m_pCPhysMem->VaToPa((PBYTE) m_pTDQueue );
            memset (  m_pTDQueue , 0, m_dwNumOfTD *sizeof(TD));
        }
    }
    if (m_pBuffer==NULL) {
        if (!m_pCPhysMem->AllocateMemory( DEBUG_PARAM( TEXT("IssueTransfer TDs") )
                                m_dwNumOfTD * m_dwTDBufferSize ,
                                (PUCHAR *) &m_pBuffer,
                                CPHYSMEM_FLAG_NOBLOCK))  {
            DEBUGMSG( ZONE_WARNING, (TEXT("TDQueue::TDQueue - no memory for TD list\n") ) );
            m_pBuffer= NULL;
        }
        else
            m_pBufferPhysAddr =  m_pCPhysMem->VaToPa(m_pBuffer);
    }
    m_dwHeadIndex = m_dwTailIndex = 0;
    if  ( m_pTDQueue!=NULL && m_pBuffer!=NULL ) {
        PREFAST_SUPPRESS(12009, "false positive, the dwNumOfTD is less than 0x1000");
        for (DWORD dwIndex=0;dwIndex< m_dwNumOfTD;dwIndex++) {
            (m_pTDQueue+dwIndex)->bfDiscard=1;
            (m_pTDQueue+dwIndex)->paNextTd.phys =  m_pTDQueuePhysAddr + sizeof(TD) * IncIndex(dwIndex);
            (m_pTDQueue+dwIndex)->paCurBuffer = m_pBufferPhysAddr + m_dwTDBufferSize * m_dwTailIndex;
        }
        m_dwHeadIndex = m_dwTailIndex = 0;
        return TRUE;
    }
    return FALSE;
}
BOOL TDQueue::QueueDisable()
{
    if  ( m_pTDQueue!=NULL && m_pBuffer!=NULL ) {
        for (DWORD dwIndex=0;dwIndex< m_dwNumOfTD;dwIndex++) {
            (m_pTDQueue+dwIndex)->bfDiscard=1;
        }
    }
    return TRUE;
}


BOOL TDQueue::InitTDQueueTailTD( IN       CTransfer *pTransfer,
                              IN CQueuedPipe * pPipe,
                              IN const UCHAR InterruptOnComplete,
                              IN const DWORD PID,
                              IN const USHORT DataToggle,
                              IN const DWORD paBuffer,
                              IN const DWORD MaxLength,
                              IN const BOOL bShortPacketOk )
    
{
    if (IsTDQueueFull() || m_pTDQueue==NULL || m_pBuffer == NULL)
        return FALSE;
    TD * pTD = m_pTDQueue + m_dwTailIndex;
    
    // not really part of the TD
    pTD->pTransfer = pTransfer;
    pTD->pNextTd =  m_pTDQueue + IncIndex(m_dwTailIndex);
    pTD->pPipe = pPipe;
    pTD->bfIsIsoch = 0;
    pTD->bfDiscard = 0;

    // the actual TD (null is legal for the last TD)
    pTD->paNextTd.phys = m_pTDQueuePhysAddr + sizeof(TD) * IncIndex(m_dwTailIndex);

//    DEBUGCHK( InterruptOnComplete == 0 || InterruptOnComplete == 7 );
    pTD->bfShortPacketOk = bShortPacketOk;
    pTD->bfDelayInterrupt = InterruptOnComplete ? gcTdInterruptOnComplete : gcTdNoInterrupt;
    pTD->bfDataToggle = DataToggle;
    pTD->bfErrorCount = 0;
    pTD->bfConditionCode = USB_NOT_ACCESSED_ERROR;

    DEBUGCHK( PID == TD_IN_PID ||
              PID == TD_OUT_PID ||
              PID == TD_SETUP_PID );
    pTD->bfPID = PID;
    DWORD phAddr = (paBuffer==0? (m_pBufferPhysAddr + m_dwTDBufferSize * m_dwTailIndex):paBuffer);
    if (MaxLength == 0 ) {
        // zero-length transfer
        pTD->paCurBuffer = 0;
        pTD->paBufferEnd = 0;
    } else {
        DEBUGCHK( MaxLength <= 0x2000 /*8K*/ );
        pTD->paCurBuffer = phAddr;
        pTD->paBufferEnd = phAddr+MaxLength-1;
    }
    return TRUE;
};

CITransfer::CITransfer(CIsochronousPipe* const  rPipe, STransfer sTransfer,CITransfer *lpNextTransfer) 
: m_rPipe(rPipe)
, m_lpNextTransfer(lpNextTransfer)
{
    m_Transfer = sTransfer;
    // HD64465 doesn't support custom physical memory
    m_Transfer.paClientBuffer = 0;
    m_Transfer.paClientBuffer = 0;

    m_dwNextToQueueFrameIndex= m_dwNextToCompleteFrameIndex = 0;
    m_dwNextToQueueBufferIndex = m_dwNextToCompleteBufferIndex = 0;
    
    m_dwQueueTD =0;
    m_dwFirstError = USB_NO_ERROR;
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
    __try { // initializing transfer status parameters
        *m_Transfer.lpfComplete = FALSE;
        *m_Transfer.lpdwBytesTransferred = 0;
        *m_Transfer.lpdwError = USB_NOT_COMPLETE_ERROR;
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
    }
#pragma prefast(pop)

};
// ******************************************************************               
// Scope: public
BOOL CITransfer::ScheduleITD()
//
// Purpose: Schedule ITD for this transfer.
//
// return : TRUE: success schedule TD, FALSE: Error happens.
//
// Parameters: Nothing
//
// ******************************************************************
{
    if (m_rPipe->IsITDQueueFull())
        return FALSE;
    
    while (m_rPipe->IsITDQueueFull()==FALSE && m_dwNextToQueueFrameIndex < m_Transfer.dwFrames ) { // if this is TD not full yet.
        DWORD dwFrame = min(  m_rPipe->GetFrameNumberPerITD(),m_Transfer.dwFrames - m_dwNextToQueueFrameIndex);
        DWORD dwDataBufferSize =0;
        for (DWORD dwIndex =0;dwIndex< dwFrame; dwIndex ++) {
            dwDataBufferSize += m_Transfer.aLengths[m_dwNextToQueueFrameIndex + dwIndex];
        }
        if ( m_Transfer.paClientBuffer == 0 && (m_Transfer.dwFlags & USB_IN_TRANSFER) ==0 ) { // We need Copy data
            PBYTE pUserData = m_rPipe->GetITDQueueTailBufferVirtAddr();
            __try { // setting transfer status and executing callback function
                CopyToShareMemory( pUserData, (PBYTE)(m_Transfer.lpvClientBuffer) +  m_dwNextToQueueBufferIndex ,dwDataBufferSize );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::ScheduleTD()(%s)::CheckForDoneTransfers - exception setting transfer status to complete\n"), m_rPipe->GetPipeType() ) );
            }
        };
        BOOL bReturn =m_rPipe->InitITDQueueTailITD(this,
                              m_rPipe,
                              (WORD)(m_Transfer.dwStartingFrame + m_dwNextToQueueFrameIndex),
                              (WORD)dwFrame,
                              m_Transfer.aLengths+m_dwNextToQueueFrameIndex,
                              gcTdInterruptOnComplete,
                              ((m_Transfer.paClientBuffer!=0)?m_Transfer.paClientBuffer +  m_dwNextToQueueBufferIndex:0),
                              dwDataBufferSize)   ;
        ASSERT(bReturn==TRUE);
        if (bReturn){
            m_dwNextToQueueFrameIndex += dwFrame ;
            m_dwNextToQueueBufferIndex += dwDataBufferSize;
            m_rPipe->AdvanceITDQueueTail();
        }
        else
            return FALSE;                
    }
    return TRUE;
}
// ******************************************************************               
// Scope: public
BOOL CITransfer::DoneITD()
//
// Purpose: One ITD Done for this Transfer..
//
// return : TRUE: success Done TD, FALSE: Error happens.
//
// Parameters: Nothing
//
// ******************************************************************
{
    P_ITD pCur= m_rPipe->GetITDQueueHead();
    if (m_dwNextToCompleteFrameIndex < m_Transfer.dwFrames ) {
        DWORD dwFrame =min ( pCur->bfFrameCount +1, m_rPipe->GetFrameNumberPerITD());
        ASSERT(dwFrame<=m_Transfer.dwFrames - m_dwNextToCompleteFrameIndex );
        dwFrame = min (dwFrame,m_Transfer.dwFrames - m_dwNextToCompleteFrameIndex); // This is for protection.
        DWORD dwDataBufferSize =0;
        // must use buffer here, because all ITDFrame element is WORD(2 bytes) type!
        ITDFrame offsetPsw[gcITdNumOffsets];
        ASSERT(sizeof(offsetPsw) == sizeof(pCur->offsetPsw));
        CopyFromShareMemory(offsetPsw, pCur->offsetPsw, sizeof(offsetPsw));
        for (DWORD dwIndex =0;dwIndex< dwFrame; dwIndex ++) {
            DWORD cc = offsetPsw[dwIndex].uConditionCode;
            DWORD sz = offsetPsw[dwIndex].uSize;
            if (cc == USB_NOT_ACCESSED_ALT)
                cc = USB_NOT_ACCESSED_ERROR;            
            __try { // initializing transfer status parameters
                if (m_Transfer.adwIsochErrors!=NULL)
                    m_Transfer.adwIsochErrors[m_dwNextToCompleteFrameIndex + dwIndex] = cc;
                // OHCI 4.3.2.3.3 
                if (m_Transfer.adwIsochLengths!=NULL) 
                    m_Transfer.adwIsochLengths[m_dwNextToCompleteFrameIndex + dwIndex] =
                        ((m_Transfer.dwFlags & USB_IN_TRANSFER)!=0?sz:m_Transfer.aLengths[m_dwNextToCompleteFrameIndex + dwIndex]); 
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::DoneTD()(%s)::exception setting transfer status to complete\n"), m_rPipe->GetPipeType() ) );
            }
            dwDataBufferSize += m_Transfer.aLengths[m_dwNextToCompleteFrameIndex + dwIndex];
        }
        if (m_Transfer.paClientBuffer == 0 && (m_Transfer.dwFlags & USB_IN_TRANSFER)!=0) { // In transfer , we need copy.
            PBYTE pUserData = m_rPipe->GetITDQueueHeadBufferVirtAddr();
            __try { // initializing transfer status parameters
                CopyFromShareMemory ((PBYTE)(m_Transfer.lpvClientBuffer) + m_dwNextToCompleteBufferIndex , pUserData,dwDataBufferSize);
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::DoneTD()(%s)::exception setting transfer status to complete\n"), m_rPipe->GetPipeType() ) );
            }
        }
        m_dwNextToCompleteBufferIndex += dwDataBufferSize;
        m_dwNextToCompleteFrameIndex += dwFrame;
    }
    // Update the TD status.
    if (m_dwFirstError== USB_NO_ERROR) {
        m_dwFirstError = pCur->bfConditionCode;
    }
                    
    m_rPipe->AdvanceITDQueueHead();
    DWORD bReturn=FALSE;
    if (m_dwNextToCompleteFrameIndex>=m_Transfer.dwFrames) { // We are completed.
        bReturn=TRUE;
        DWORD dwOverallLength = 0 ;
        if (m_Transfer.adwIsochLengths!=NULL) {
            for (DWORD dwIndex = 0; dwIndex < m_Transfer.dwFrames; dwIndex ++ )
                dwOverallLength += m_Transfer.adwIsochLengths[dwIndex];
        }
        __try { // initializing transfer status parameters
            *m_Transfer.lpfComplete = TRUE;
            *m_Transfer.lpdwBytesTransferred = dwOverallLength ;
            *m_Transfer.lpdwError = m_dwFirstError;
            if (m_Transfer.lpfnCallback ) {
                ( *m_Transfer.lpfnCallback )( m_Transfer.lpvCallbackParameter );
                m_Transfer.lpfnCallback =NULL;
            }
            
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::DoneTD()(%s)::exception setting transfer status to complete\n"), m_rPipe->GetPipeType() ) );
        }
        
    }    
    return bReturn;
};

BOOL CITransfer::Canceled()
{
    __try { // initializing transfer status parameters
        m_dwNextToQueueFrameIndex=m_dwNextToCompleteFrameIndex=m_Transfer.dwFrames;
        *m_Transfer.lpfComplete = TRUE;
        *m_Transfer.lpdwError = USB_CANCELED_ERROR;
        if (m_Transfer.lpfnCallback ) {
            ( *m_Transfer.lpfnCallback )( m_Transfer.lpvCallbackParameter );
            m_Transfer.lpfnCallback =NULL;
        }
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        DEBUGMSG( ZONE_ERROR, (TEXT("CTransfer::Canceled()(%s)::exception setting transfer status to complete\n"), m_rPipe->GetPipeType() ) );
    }
        
    return TRUE;
}
// ******************************************************************               
// Scope: public
BOOL CITransfer::CheckFrame(WORD wMaxPacketSize)const
//
// Purpose: Check Each frame against buffer that used.
//
// return : TRUE: MATCH, FALSE: Error happens.
//
// Parameters: Packet Size.
//
{
    BOOL fValid = TRUE;
    __try {
        DWORD dwTotalData = 0;
        for ( DWORD frame = 0; frame < m_Transfer.dwFrames; frame++ ) {
            if ( m_Transfer.aLengths[ frame ] == 0 || 
                 m_Transfer.aLengths[ frame ] > wMaxPacketSize ) {
                fValid = FALSE;
                break;
            }
            dwTotalData += m_Transfer.aLengths[ frame ];
        }
        fValid = ( fValid &&
                   dwTotalData == m_Transfer.dwBufferSize );
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        fValid = FALSE;
    }
    return fValid;

}




ITDQueue::ITDQueue(IN CPhysMem* const pCPhysMem, DWORD, DWORD dwMaxPacketSize) 
: m_pCPhysMem( pCPhysMem)
, m_dwMaxPacketSize( dwMaxPacketSize)
{
    m_dwFramePerTD = 1;
    m_dwNumOfTD =  3; // double buffer to keep real time transfer
    m_dwTDBufferSize = min((dwMaxPacketSize * m_dwFramePerTD),MAX_TD_BUFFER_SIZE) ;
    m_pTDQueue = NULL;
    m_pBuffer = NULL;
    
};
ITDQueue::~ITDQueue()
{
    QueueDeInit();
}
void ITDQueue::QueueDeInit()
{
    if (m_pBuffer) {
        m_pCPhysMem->FreeMemory(m_pBuffer,m_pBufferPhysAddr,CPHYSMEM_FLAG_NOBLOCK);
        m_pBuffer=NULL;
        m_pBufferPhysAddr = 0;
    }
    if (m_pTDQueue) {
        m_pCPhysMem->FreeMemory((PBYTE)m_pTDQueue,m_pTDQueuePhysAddr,CPHYSMEM_FLAG_NOBLOCK);
        m_pTDQueue = NULL;
        m_pTDQueuePhysAddr = 0;
    }
}
BOOL ITDQueue::QueueInit()
{
    if (m_pTDQueue==NULL) {
        if (!m_pCPhysMem->AllocateMemory( DEBUG_PARAM( TEXT("IssueTransfer ITDs") )
                                m_dwNumOfTD *sizeof(ITD),
                                (PUCHAR *) &m_pTDQueue,
                                CPHYSMEM_FLAG_NOBLOCK))  {
            DEBUGMSG( ZONE_WARNING, (TEXT("TDQueue::TDQueue - no memory for ITD list\n") ) );
            m_pTDQueue= NULL;
        }
        else {
            m_pTDQueuePhysAddr = m_pCPhysMem->VaToPa( (PBYTE)m_pTDQueue );
            memset (  m_pTDQueue , 0, m_dwNumOfTD *sizeof(ITD));
        }
    }
    if (m_pBuffer==NULL) {
        if (!m_pCPhysMem->AllocateMemory( DEBUG_PARAM( TEXT("IssueTransfer ITDs") )
                                m_dwNumOfTD * m_dwTDBufferSize ,
                                (PUCHAR *) &m_pBuffer,
                                CPHYSMEM_FLAG_NOBLOCK))  {
            DEBUGMSG( ZONE_WARNING, (TEXT("TDQueue::TDQueue - no memory for TD list\n") ) );
            m_pBuffer= NULL;
        }
        else
            m_pBufferPhysAddr =  m_pCPhysMem->VaToPa(m_pBuffer);
    }
    m_dwHeadIndex = m_dwTailIndex = 0;
    if  ( m_pTDQueue!=NULL && m_pBuffer!=NULL ) {
        PREFAST_SUPPRESS(12009, "false positive, the dwNumOfTD is less than 0x1000");
        for (DWORD dwIndex=0;dwIndex< m_dwNumOfTD;dwIndex++) {
            (m_pTDQueue+dwIndex)->bfDiscard=1;
            (m_pTDQueue+dwIndex)->paNextTd.phys =  m_pTDQueuePhysAddr + sizeof(ITD) * IncIndex(dwIndex);
        }
        m_dwHeadIndex = m_dwTailIndex = 0;
        return TRUE;
    }
    return FALSE;
}
BOOL ITDQueue::QueueDisable()
{
    if  ( m_pTDQueue!=NULL && m_pBuffer!=NULL ) {
        for (DWORD dwIndex=0;dwIndex< m_dwNumOfTD;dwIndex++) {
            (m_pTDQueue+dwIndex)->bfDiscard=1;
        }
    }
    return TRUE;
}
BOOL ITDQueue::InitITDQueueTailITD( IN  CITransfer  *pTransfer,
                              IN CIsochronousPipe * pPipe,
                              IN const USHORT wStartFrame,
                              IN const USHORT wNumOfFrame,
                              IN LPCDWORD     lpFrameLength,
                              IN const UCHAR InterruptOnComplete,
                              IN const DWORD paBuffer,
                              IN const DWORD /*MaxLength*/)    
{
    if (IsITDQueueFull() || m_pTDQueue==NULL || m_pBuffer == NULL)
        return FALSE;
    ITD * pTD = m_pTDQueue + m_dwTailIndex;
    memset((PUCHAR)  pTD, 0, sizeof(ITD));
    // not really part of the ITD
    pTD->pTransfer = pTransfer;
    pTD->pNextTd =  m_pTDQueue + IncIndex(m_dwTailIndex);
    pTD->pPipe = pPipe;
    pTD->bfIsIsoch = 0;
    pTD->bfDiscard = 0;

    // the actual TD (null is legal for the last TD)
    pTD->paNextTd.phys = m_pTDQueuePhysAddr + sizeof(ITD) * IncIndex(m_dwTailIndex);
    
    pTD->bfStartFrame = (wStartFrame) & 0xFFFF;
    pTD->bfIsIsoch = 1;
    pTD->bfDiscard = 0;
    pTD->bfDelayInterrupt = InterruptOnComplete;
    pTD->bfConditionCode = USB_NOT_ACCESSED_ERROR;
    DWORD dwStartPhysAddr = (paBuffer!=0?paBuffer:(m_pBufferPhysAddr + m_dwTDBufferSize * m_dwTailIndex));
    pTD->paBufferPage0 = dwStartPhysAddr;
    WORD    wPageSelect =0;
    
    // must use buffer here, because all ITDFrame element is WORD(2 bytes) type!
    ITDFrame offsetPsw[gcITdNumOffsets];
    memset(offsetPsw, 0, sizeof(offsetPsw));
    for (DWORD dwIndex=0; dwIndex < wNumOfFrame && dwIndex< m_dwFramePerTD; dwIndex++) {
        offsetPsw[dwIndex].uHiBits = ~0; // init to NOT_ACCESSED
        offsetPsw[dwIndex].uOffset = ((dwStartPhysAddr) & (gcTdPageSize - 1)) | wPageSelect;
        dwStartPhysAddr += lpFrameLength[dwIndex];

        // Check to see if the next frame is on the same OHCI page as this one
        if ((pTD->paBufferPage0 ^ dwStartPhysAddr ) & ~(gcTdPageSize - 1))
            wPageSelect = gcTdPageSize;
    }
    ASSERT(sizeof(offsetPsw) == sizeof(pTD->offsetPsw));
    CopyToShareMemory(pTD->offsetPsw, offsetPsw, sizeof(offsetPsw));
    ASSERT(dwIndex!=0);
    
    pTD->paBufferEnd = dwStartPhysAddr  - 1;
    pTD->bfFrameCount = dwIndex - 1; // as described in table 4-3
    return TRUE;
};




