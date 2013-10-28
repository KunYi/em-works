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
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
// 
// Module Name:  
//     CTd.cpp
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//

/*---------------------------------------------------------------------------
* Copyright (C) 2005-2007, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/


#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "ctd.h"
#include "trans.h"
#include "cpipe.h"
#ifdef FREESCALE_HANG_DEBUG
#undef FREESCALE_HANG_DEBUG
#endif

#define RETAIL_LOG 0
#define CRITICAL_LOG 0
#define IRAM_MEM_LOG 0
//#define FREESCALE_HANG_DEBUG  1
//******************************************************************************
void * CNextLinkPointer::operator new(size_t stSize, CPhysMem * const pCPhysMem)
//
// Purpose: Operator overloading for "new" in CNextLinkPointer
//
// Parameters: 
//
// Returns: Virtual address of the allocated memory
//
// ******************************************************************
{
    PVOID pVirtAddr=0;
    if (stSize<sizeof(PVOID))
        stSize = sizeof(PVOID);
    if (pCPhysMem && stSize ) {
        while (pVirtAddr==NULL) {
            if (!pCPhysMem->AllocateMemory( DEBUG_PARAM( TEXT("CNextLinkPointer")) stSize, (PUCHAR *)&pVirtAddr,CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK)) {
                pVirtAddr=NULL;
                break;
            }
            else {
                // Structure can not span 4k Page bound. refer EHCI 3. Note.
                DWORD dwPhysAddr =  pCPhysMem->VaToPa((PUCHAR)pVirtAddr);
                if ((dwPhysAddr & 0xfffff000)!= ((dwPhysAddr + stSize-1) & 0xfffff000)) {
                    // Cross Page bound. trash it.
                    pVirtAddr= NULL;
                };
            }
        }
    }
    return pVirtAddr;
}

//******************************************************************
void CNextLinkPointer::operator delete(void * /*pointer*/)
//
// Purpose: Operator overloading for "delete" in CNextLinkPointer
//
// Parameters: 
//
// Returns: Nothing
//
// ******************************************************************
{
    ASSERT(FALSE); // Can not use this operator.
}

CITD::CITD(CITransfer * pIsochTransfer)
: m_pTrans(pIsochTransfer)
, m_CheckFlag(CITD_CHECK_FLAG_VALUE)
{
    ASSERT( (&(nextLinkPointer.dwLinkPointer))+ 15 == (&(iTD_BufferPagePointer[6].dwITD_BufferPagePointer)));// Check for Data Intergraty.
//    m_pNext=NULL;
    
    for (DWORD dwCount=0;dwCount<MAX_PHYSICAL_BLOCK;dwCount++) 
        iTD_BufferPagePointer[dwCount].dwITD_BufferPagePointer = 0;
    
    for (dwCount=0; dwCount<MAX_TRNAS_PER_ITD; dwCount++) 
        iTD_StatusControl[dwCount].dwITD_StatusControl=0;
    
    m_dwPhys = (m_pTrans?(m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this):0);
    
}

//******************************************************************
void CITD::ReInit(CITransfer * pIsochTransfer)
//
// Purpose: Re-initalize the high speed isoch transfer descriptor
//
// Parameters: pIsochTransfer - pointer to CITransfer
//
// Returns: Nothing
//
// ******************************************************************
{
    nextLinkPointer.dwLinkPointer=1;
    m_pTrans =pIsochTransfer ;    
    for (DWORD dwCount=0;dwCount<MAX_PHYSICAL_BLOCK;dwCount++) 
        iTD_BufferPagePointer[dwCount].dwITD_BufferPagePointer = 0;
    
    for (dwCount=0; dwCount<MAX_TRNAS_PER_ITD; dwCount++) 
        iTD_StatusControl[dwCount].dwITD_StatusControl=0;
    
    m_dwPhys = (m_pTrans?(m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this):0);
}

//********************************************************************
void CITD::SetIOC(BOOL bSet)
//
// Purpose: Set the Interrupt On Completion (IOC) bit on the Isoch Transfer descriptor
//
// Parameters: bSet - TRUE : Set IOC, FALSE: Clear IOC
//
// Returns: Nothing
//
// ******************************************************************
{
    CheckStructure ();
    if (bSet) {
        for (int iCount = MAX_TRNAS_PER_ITD-1;iCount>0;iCount--)
            if (iTD_StatusControl[iCount].iTD_SCContext.TransactionLength!=0) {
                iTD_StatusControl[iCount].iTD_SCContext.InterruptOnComplete=1;
                break;
            }
    }
    else {
        for (int iCount =0;iCount< MAX_TRNAS_PER_ITD;iCount++)
            if (iTD_StatusControl[iCount].iTD_SCContext.TransactionLength!=0) 
                iTD_StatusControl[iCount].iTD_SCContext.InterruptOnComplete=0;
    }
}

//****************************************************************************************************************
DWORD CITD::IssueTransfer(DWORD dwNumOfTrans,PDWORD pdwTransLenArray, PDWORD pdwFrameAddrArray,BOOL bIoc,BOOL bIn)
//
// Purpose: Perform actual transfer of the data on the high speed isochronous transfer
//
// Parameters: dwNumOfTrans - total number of transactions on the TD
//             pdwTransLenArray - array containing the len of each transaction
//             pdwFrameAddrArray - array containing address [buffer pointer] of each transaction
//             bIoc - if Interrupt On Completion (IOC) bit should be set
//             bIn - IN or OUT transcation
//
// Returns: Number of transcations successfully setup
//
// ******************************************************************
{
    CheckStructure ();
    if (dwNumOfTrans ==NULL || dwNumOfTrans>MAX_TRNAS_PER_ITD ||pdwTransLenArray==NULL || pdwFrameAddrArray==NULL) {
        ASSERT(FALSE);
        return 0;
    }
    // Initial Buffer Pointer.
    for (DWORD dwCount=0;dwCount<MAX_PHYSICAL_BLOCK;dwCount++) 
        iTD_BufferPagePointer[dwCount].dwITD_BufferPagePointer = 0;
    
    DWORD dwCurBufferPtr=0;
    DWORD dwCurValidPage = ((DWORD)-1) &  EHCI_PAGE_ADDR_MASK;
    for (dwCount=0;dwCount<MAX_TRNAS_PER_ITD +1  && dwCount < dwNumOfTrans +1 && dwCurBufferPtr < MAX_PHYSICAL_BLOCK ;dwCount++) {
        if (dwCurValidPage !=  ((*(pdwFrameAddrArray+dwCount))&EHCI_PAGE_ADDR_MASK) ) {
            dwCurValidPage = iTD_BufferPagePointer[dwCurBufferPtr].dwITD_BufferPagePointer 
                    = (*(pdwFrameAddrArray+dwCount) ) & EHCI_PAGE_ADDR_MASK;
            if (*(pdwTransLenArray +dwCount)==0) { // Endof Transfer
                break;
            }
            dwCurBufferPtr ++;
        }
    }

    // Add the Cross Boundary page pointer if any
    if (*(pdwFrameAddrArray+MAX_TRNAS_PER_ITD) != 0)
    {
        dwCurValidPage = iTD_BufferPagePointer[dwCurBufferPtr].dwITD_BufferPagePointer 
                  = (*(pdwFrameAddrArray+MAX_TRNAS_PER_ITD) ) & EHCI_PAGE_ADDR_MASK;        
        dwCurBufferPtr++;
    }
        

    USB_ENDPOINT_DESCRIPTOR endptDesc = m_pTrans->m_pCPipe->GetEndptDescriptor();
    
    iTD_BufferPagePointer[0].iTD_BPPContext1.DeviceAddress= m_pTrans->m_pCPipe->GetDeviceAddress();
    iTD_BufferPagePointer[0].iTD_BPPContext1.EndPointNumber=endptDesc.bEndpointAddress;
    iTD_BufferPagePointer[1].iTD_BPPContext2.MaxPacketSize=endptDesc.wMaxPacketSize & 0x7ff;
    iTD_BufferPagePointer[1].iTD_BPPContext2.Direction=(bIn?1:0);
    iTD_BufferPagePointer[2].iTD_BPPContext3.Multi=((endptDesc.wMaxPacketSize>>11) & 3)+1;
    ASSERT(((endptDesc.wMaxPacketSize>>11)&3)!=3);
    
    // Initial Transaction 
    dwCurValidPage = (*pdwFrameAddrArray) &  EHCI_PAGE_ADDR_MASK;
    dwCurBufferPtr=0;    
    for (dwCount=0; dwCount<MAX_TRNAS_PER_ITD; dwCount++) {
        iTD_StatusControl[dwCount].dwITD_StatusControl=0;
        if (dwCount < dwNumOfTrans) {
            if (dwCurValidPage != (*(pdwFrameAddrArray+dwCount)&EHCI_PAGE_ADDR_MASK)) {
                dwCurValidPage = *(pdwFrameAddrArray+dwCount)&EHCI_PAGE_ADDR_MASK;
                if (*(pdwTransLenArray+dwCount) != 0)
                    dwCurBufferPtr ++;                
            }
            iTD_StatusControl[dwCount].iTD_SCContext.TransactionLength = *(pdwTransLenArray+dwCount);
            iTD_StatusControl[dwCount].iTD_SCContext.TransationOffset  = (*(pdwFrameAddrArray+dwCount) & EHCI_PAGE_OFFSET_MASK);
            iTD_StatusControl[dwCount].iTD_SCContext.PageSelect = dwCurBufferPtr;
            iTD_StatusControl[dwCount].iTD_SCContext.Active = 1;
            if (dwCount == dwNumOfTrans -1 && bIoc )  { // if thiere is last one and interrupt on completion. do it.
                iTD_StatusControl[dwCount].iTD_SCContext.InterruptOnComplete=1;
            }
        }
        
 
    }
    
    return (dwNumOfTrans<MAX_TRNAS_PER_ITD?dwNumOfTrans:MAX_TRNAS_PER_ITD);
    
};

//**********************************************************************************
CSITD::CSITD(CSITransfer * pTransfer,CSITD * pPrev)
: m_pTrans(pTransfer)
, m_CheckFlag(CSITD_CHECK_FLAG_VALUE)
// 
// Purpose: Constructor for Spilt Isochronous Transfer
//
// Parameters: pIsochTransfer - pointer to CITransfer
//
// Returns: Nothing
//
// ******************************************************************

{
    ASSERT((&(nextLinkPointer.dwLinkPointer))+ 6 == &(backPointer.dwLinkPointer)); // Check for Data Intergraty.
    sITD_CapChar.dwSITD_CapChar=0;
    microFrameSchCtrl.dwMicroFrameSchCtrl=0;
    sITD_TransferState.dwSITD_TransferState=0;
    sITD_BPPage[0].dwSITD_BPPage=0;
    sITD_BPPage[1].dwSITD_BPPage=0;
    backPointer.dwLinkPointer=1; // Invalid Back Link

    UCHAR S_Mask = (m_pTrans?m_pTrans->m_pCPipe->GetSMask():1);
    ASSERT(S_Mask!=0);
    if (S_Mask==0) // Start Mask has to be present
        S_Mask=1;
    microFrameSchCtrl.sITD_MFSCContext.SplitStartMask=S_Mask;
    microFrameSchCtrl.sITD_MFSCContext.SplitCompletionMask=(m_pTrans?m_pTrans->m_pCPipe->GetCMask():0);
    m_pPrev=pPrev;
    m_dwPhys =(m_pTrans?(m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this):0);
}

//***********************************************************************************
void CSITD::ReInit(CSITransfer * pTransfer,CSITD * pPrev)
//
// Purpose: Re-initalize the low/full speed spilt isoch transfer descriptor
//
// Parameters: pTransfer - pointer to CSITransfer
//             pPrev - Previous pointer to CSITD
//
// Returns: Nothing
//
// ******************************************************************
{
    ASSERT( pTransfer!=NULL);
    nextLinkPointer.dwLinkPointer=1;
    m_pTrans=pTransfer;
    sITD_CapChar.dwSITD_CapChar=0;
    microFrameSchCtrl.dwMicroFrameSchCtrl=0;
    sITD_TransferState.dwSITD_TransferState=0;
    sITD_BPPage[0].dwSITD_BPPage=0;
    sITD_BPPage[1].dwSITD_BPPage=0;
    backPointer.dwLinkPointer=1; // Invalid Back Link

    UCHAR S_Mask = (m_pTrans?m_pTrans->m_pCPipe->GetSMask():1);
    ASSERT(S_Mask!=0);
    if (S_Mask==0) // Start Mask has to be present
        S_Mask=1;
    microFrameSchCtrl.sITD_MFSCContext.SplitStartMask=S_Mask;
    microFrameSchCtrl.sITD_MFSCContext.SplitCompletionMask=(m_pTrans?m_pTrans->m_pCPipe->GetCMask():0);
    m_pPrev=pPrev;
    m_dwPhys = (m_pTrans?(m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this):0);

}

//********************************************************************
#define MAX_SPLIT_TRANSFER_LENGTH 188
DWORD CSITD::IssueTransfer(DWORD dwPhysAddr, DWORD dwEndPhysAddr, DWORD dwLen,BOOL bIoc,BOOL bIn)
//
// Purpose: Perform actual transfer of the data on the high speed isochronous transfer
//
// Parameters: dwPhysAddr - physical address first buffer pointer
//             dwEndPhysAddr - physical address of second buffer pointer
//             dwLen - total length to transfer
//             bIoc - if Interrupt On Completion (IOC) bit should be set
//             bIn - IN or OUT transcation
//
// Returns: 1 - success, 0 - failure
//
// ******************************************************************
{
    CheckStructure ();
    if (dwPhysAddr==0 || dwLen==0||  dwLen > 1024) {
        ASSERT(FALSE);
        return 0;
    }
    sITD_BPPage[0].sITD_BPPage0.BufferPointer = ((dwPhysAddr) >> EHCI_PAGE_ADDR_SHIFT);
    sITD_BPPage[0].sITD_BPPage0.CurrentOffset = ((dwPhysAddr) & EHCI_PAGE_OFFSET_MASK);
    sITD_BPPage[1].sITD_BPPage1.BufferPointer = (dwEndPhysAddr >> EHCI_PAGE_ADDR_SHIFT);
    
    USB_ENDPOINT_DESCRIPTOR endptDesc = m_pTrans->m_pCPipe->GetEndptDescriptor();
    sITD_CapChar.sITD_CCContext.DeviceAddress = m_pTrans->m_pCPipe->GetDeviceAddress();
    sITD_CapChar.sITD_CCContext.Endpt = endptDesc.bEndpointAddress;
    sITD_CapChar.sITD_CCContext.HubAddress = m_pTrans->m_pCPipe->m_bHubAddress;
    sITD_CapChar.sITD_CCContext.PortNumber= m_pTrans->m_pCPipe->m_bHubPort;
    sITD_CapChar.sITD_CCContext.Direction =(bIn?1:0);

     // S-Mask and C-Mask has been initialized
     // Status will be inactive and DoStartSlit.

    sITD_TransferState.sITD_TSContext.BytesToTransfer = dwLen;
    sITD_TransferState.sITD_TSContext.PageSelect=0; // Always use first page first.
    sITD_TransferState.sITD_TSContext.IOC= (bIoc?1:0);
     //
    sITD_BPPage[1].sITD_BPPage1.TP=(dwLen>MAX_SPLIT_TRANSFER_LENGTH?1:0);
    DWORD dwSlitCount=( dwLen+MAX_SPLIT_TRANSFER_LENGTH-1) /MAX_SPLIT_TRANSFER_LENGTH;
    if (dwSlitCount>6) {
        ASSERT(FALSE);
        dwSlitCount=6;
     }
    if(bIn)
        dwSlitCount=1;
    sITD_BPPage[1].sITD_BPPage1.T_Count = dwSlitCount;
    sITD_TransferState.sITD_TSContext.Active=1;

    // Setup the back pointer if there is.
    if (m_pPrev==NULL || m_pPrev->GetPhysAddr()== 0)
        backPointer.dwLinkPointer=1;
    else {
        CNextLinkPointer backP;
        backP.SetNextPointer(m_pPrev->GetPhysAddr(), TYPE_SELECT_SITD, TRUE);
        backPointer.dwLinkPointer = backP.GetDWORD();        
    }
    return 1;
     
};

//***************************************************************************************
CQTD::CQTD( CQTransfer * pTransfer, CQH * pQh)
: m_pTrans(pTransfer)
, m_pQh(pQh)
, m_CheckFlag(CQTD_CHECK_FLAG_VALUE)
//
// Purpose: Constructor for CQTD (Asychronous Queue Transfer descriptor)
//
// Parameters: pTransfer - pointer to CQTransfer object
//             pQh - pointer to CQH object
//
// Returns: Nothing
//
// ******************************************************************
{
    ASSERT((&(nextLinkPointer.dwLinkPointer))+ 3 == &(qTD_BufferPointer[0].dwQTD_BufferPointer)); // Check for Data Intergraty.    
    m_pNext=NULL;
    altNextQTDPointer.dwLinkPointer=1;
    qTD_Token.dwQTD_Token=0;
    for (DWORD dwIndex=0;dwIndex<5; dwIndex++)
        qTD_BufferPointer[dwIndex].dwQTD_BufferPointer=0;
    nextLinkPointer.dwLinkPointer=1;
    altNextQTDPointer.dwLinkPointer=1;
    qTD_Token.dwQTD_Token=0;
    for (dwIndex=0;dwIndex<5;dwIndex++)
        qTD_BufferPointer[dwIndex].dwQTD_BufferPointer=0;
    m_dwPhys = (m_pTrans->m_pCPipe->GetCPhysMem())-> VaToPa((PBYTE)this);
#ifdef IRAM_PATCH
    m_pOrigStart = NULL;
#endif
}

//*********************************************************************************************************************
DWORD CQTD::IssueTransfer(DWORD dwPID, BOOL bToggle1, DWORD dwTransLength, PPhysBufferArray pPhysBufferArray,BOOL bIoc)
//
// Purpose: Perform actual transfer of the data on asynchronous transfer
//
// Parameters: dwPID - Packet ID (Setup, IN or OUT)
//             bToggle1 - Data toggle bit should be enabled or not.
//             dwTransLenth - total length to transfer
//             pPhysBufferArray - pointer to physical address of data buffer
//             bIoc - if Interrupt On Completion (IOC) bit should be set             
//
// Returns: Total size of data successfully transferred
//
// ******************************************************************

{
    CheckStructure ();
    if ( pPhysBufferArray ==NULL ) {
        ASSERT(FALSE);
        return 0;
    }
    ASSERT((pPhysBufferArray->dwBlockSize== dwTransLength + (pPhysBufferArray ->dwStartOffset & EHCI_PAGE_OFFSET_MASK)) ||
        pPhysBufferArray->dwBlockSize == EHCI_PAGE_SIZE);
    DWORD dwMaxPacketSize = (m_pTrans->m_pCPipe->GetEndptDescriptor()).wMaxPacketSize & 0x7ff;
    DWORD dwMaxPacketNumber=(EHCI_PAGE_SIZE* MAX_QTD_PAGE_SIZE)/dwMaxPacketSize;
    DWORD dwTotalTransfer=min(dwTransLength,dwMaxPacketNumber*dwMaxPacketSize);
    DWORD dwTotalPage = (pPhysBufferArray ->dwStartOffset + dwTransLength + EHCI_PAGE_SIZE -1)/EHCI_PAGE_SIZE;
    //DEBUGMSG(1, (TEXT("QTD Start offset(%d), transfer length(%d)\r\n"), pPhysBufferArray->dwStartOffset, dwTransLength));
    ASSERT(dwTotalPage<= (MAX_QTD_PAGE_SIZE+1));
    for (DWORD dwIndex=0;dwIndex<dwTotalPage && dwIndex<MAX_QTD_PAGE_SIZE+1;dwIndex++) {
        qTD_BufferPointer[dwIndex].dwQTD_BufferPointer = (pPhysBufferArray ->dwArrayBlockAddr[dwIndex] &  EHCI_PAGE_ADDR_MASK);
    }
    qTD_BufferPointer[0].qTD_BPContext.CurrentOffset = pPhysBufferArray ->dwStartOffset & EHCI_PAGE_OFFSET_MASK;
#ifdef IRAM_PATCH
    qTD_Token.qTD_TContext.PID = (dwPID== TD_OUT_PID ?TD_OUT_TOKEN:(dwPID==TD_SETUP_PID?TD_SETUP_TOKEN:TD_IN_TOKEN));  //0 - OUT, 1 - IN, 2 - SETUP
#else
    qTD_Token.qTD_TContext.PID = (dwPID== TD_OUT_PID ?0:(dwPID==TD_SETUP_PID?2:1));  //0 - OUT, 1 - IN, 2 - SETUP
#endif
    qTD_Token.qTD_TContext.CEER = 3;
    qTD_Token.qTD_TContext.C_Page= 0;
    qTD_Token.qTD_TContext.IOC=((dwTotalTransfer < dwTransLength)?0:(bIoc?1:0));  //need to examine..
    qTD_Token.qTD_TContext.BytesToTransfer=dwTotalTransfer;
    qTD_Token.qTD_TContext.DataToggle=(bToggle1?1:0);
    qTD_Token.qTD_TContext.Active = 1; 
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"PID %d, IOC %d, BTT %d, DT %d\r\n", qTD_Token.qTD_TContext.PID, qTD_Token.qTD_TContext.IOC, dwTotalTransfer, bToggle1));
#endif

#ifdef IRAM_PATCH
    m_pOrigStart = pPhysBufferArray->bufVirtStart;
    m_dwQtdBufLength = dwTransLength;
    RETAILMSG(RETAIL_LOG, (L"CQTD %x : IT b%x, l%d, %x...\r\n", this, m_pOrigStart, m_dwQtdBufLength, *m_pOrigStart));

    if (dwTransLength == 0)
    {
        RETAILMSG(RETAIL_LOG, (L"Dump zero length QTD\r\n"));
        {
            RETAILMSG(RETAIL_LOG, (L"IOC %d\r\n", qTD_Token.qTD_TContext.IOC));
            RETAILMSG(RETAIL_LOG, (L"BTT %d\r\n", qTD_Token.qTD_TContext.BytesToTransfer));
        }
    }
#endif

    return dwTotalTransfer;

}

//********************************************************************************
CQTD * CQTD::QueueNextTD(CQTD * pNextTD)
//
// Purpose: Set the next TD on the transfer descriptor list.
//
// Parameters: pNextTD - pointer to TD to be queued
//
// Returns: Pointer to next TD object
//
// ******************************************************************
{
    CheckStructure ();
    CQTD * pReturn = m_pNext;
    m_pNext = pNextTD;
    SetNextPointer(pNextTD-> GetPhysAddr(),TYPE_SELECT_ITD,TRUE);
    return pReturn;
}

#ifdef IRAM_PATCH
/*
 * When cloning the IRAM QTD, we make sure that its next TD is 0
 */ 
void CQTD::FlushNextTD()
{
    CheckStructure();
    nextLinkPointer.dwLinkPointer=1;
    altNextQTDPointer.dwLinkPointer=1;
}


/*
 *
 */
void CQTD::ResetIRamBP(ULONG iRamStartAddr)
{
    CheckStructure();
    qTD_BufferPointer[0].dwQTD_BufferPointer = iRamStartAddr; //This also make offset as 0
}

void CQTD::Overlay(CQTD* clonedTD)
{
    CheckStructure();
    qTD_Token.dwQTD_Token = clonedTD->GetDwQTD_Token();
}

DWORD CQTD::GetDwQTD_Token()
{
    CheckStructure();
    return qTD_Token.dwQTD_Token;
}
#endif
//***********************************************************************************
#ifdef IRAM_PATCH
#ifdef IRAM_PATCH_EXTEND
CQH::CQH(CPipe *pPipe, CPhysMem* pDRamMem,  CPhysMem* pIRamEleMem, CPhysMem* pIRamDataMem, BOOL bInIRAM)
    : m_pPipe(pPipe)
    , m_CheckFlag(CQH_CHECK_FLAG_VALUE)
    , m_pDRamMem(pDRamMem)
    , m_pIRamDataMem(pIRamDataMem)
    , m_pIRamEleMem(pIRamEleMem)
#else
CQH::CQH(CPipe *pPipe, CPhysMem* pIRamEleMem, CPhysMem* pIRamDataMem, BOOL bInIRAM)
    : m_pPipe(pPipe)
    , m_CheckFlag(CQH_CHECK_FLAG_VALUE)
    , m_pIRamDataMem(pIRamDataMem)
    , m_pIRamEleMem(pIRamEleMem)
#endif
#else
CQH::CQH(CPipe *pPipe)
    : m_pPipe(pPipe)
    , m_CheckFlag(CQH_CHECK_FLAG_VALUE)
#endif
//
// Purpose: Constructor for class CQH
//
// Parameters: pPipe - Pointer to pipe object to be used
//
// Returns: Nothing
//
// ******************************************************************
{
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"CQH::CQH\r\n"));
#endif
    ASSERT((&(nextLinkPointer.dwLinkPointer))+ 1 == &(qH_StaticEndptState.qH_StaticEndptState[0])); // Check for Data Integrity.
    ASSERT((&(qH_StaticEndptState.qH_StaticEndptState[0]))+ 4 == &(qTD_Overlay.altNextQTDPointer.dwLinkPointer)); // Check for Data Integrity.
    PREFAST_DEBUGCHK( pPipe );
    CheckStructure ();
    m_pNextQHead = NULL;
    USB_ENDPOINT_DESCRIPTOR endptDesc =pPipe->GetEndptDescriptor();
    qH_StaticEndptState.qH_StaticEndptState[0]=0;
    qH_StaticEndptState.qH_StaticEndptState[1]=0;
    qH_StaticEndptState.qH_SESContext.DeviceAddress= pPipe->GetDeviceAddress();
    qH_StaticEndptState.qH_SESContext.I=0;
    qH_StaticEndptState.qH_SESContext.Endpt =endptDesc.bEndpointAddress;
    qH_StaticEndptState.qH_SESContext.ESP = (pPipe->IsHighSpeed()?2:(pPipe->IsLowSpeed()?1:0));
    qH_StaticEndptState.qH_SESContext.DTC = 1; // Enable Data Taggle.
    //qH_StaticEndptState.qH_SESContext.H=1;
    qH_StaticEndptState.qH_SESContext.H=0;
    qH_StaticEndptState.qH_SESContext.MaxPacketLength =endptDesc.wMaxPacketSize & 0x7ff;
    qH_StaticEndptState.qH_SESContext.C  = 
        ((endptDesc.bmAttributes &  USB_ENDPOINT_TYPE_MASK)==USB_ENDPOINT_TYPE_CONTROL && pPipe->IsHighSpeed()!=TRUE ?1:0);
    qH_StaticEndptState.qH_SESContext.RL=0x0; // TODO Check this is value is correct or not.
// DWORD 2
    qH_StaticEndptState.qH_SESContext.UFrameSMask = pPipe->GetSMask();
    qH_StaticEndptState.qH_SESContext.UFrameCMask = pPipe->GetCMask();

    qH_StaticEndptState.qH_SESContext.HubAddr = pPipe->m_bHubAddress;
    qH_StaticEndptState.qH_SESContext.PortNumber = pPipe->m_bHubPort;

    //qH_StaticEndptState.qH_SESContext.Mult = ((endptDesc.wMaxPacketSize>>11) & 3)+1;
    qH_StaticEndptState.qH_SESContext.Mult = 1;

    currentQTDPointer.dwLinkPointer = 0; 
    nextQTDPointer.dwLinkPointer = 1; // Terminate;
    memset((void *)&qTD_Overlay, 0 , sizeof(QTD));
    qTD_Overlay.altNextQTDPointer.dwLinkPointer = 1; // This is For short transfer 
    qTD_Overlay.qTD_Token.qTD_TContext.Halted = 1;
#ifdef IRAM_PATCH
    if (bInIRAM)
    {
        RETAILMSG(RETAIL_LOG, (L"get IRAM %x\r\n", pPipe->GetIRamElePhysMem()));
        m_dwPhys = (pPipe->GetIRamElePhysMem())->VaToPa((PBYTE)this);
        RETAILMSG(RETAIL_LOG, (L"This QH has phy %x\r\n", m_dwPhys));
    }
    else
    {
        m_dwPhys = (pPipe->GetCPhysMem())-> VaToPa((PBYTE)this);
    }
#else
    m_dwPhys = (pPipe->GetCPhysMem())-> VaToPa((PBYTE)this);
#endif
#ifdef IRAM_PATCH
    m_pIRamStart = NULL;
    m_pIRamTd = NULL;
#endif
//    m_pStaticQHead = NULL;
}

//********************************************************************************
BOOL CQH::QueueTD(CQTD * pCurTD)
//
// Purpose: Set up the currentQTD Pointer onto the current QH
//
// Parameters: pCurTD - pointer to TD to be queued
//
// Returns: TRUE - success, FALSE - failure
//
// ******************************************************************
{
#ifdef IRAM_PATCH
    /*
     * The Original Algorithm simply attach the TD-List to QH
     *
     * In IRAM patch, we should first attach the TD-List to a virtural pointer, 
     * then every time move a single TD (which has total length not beyond 2K, 
     * we ensure this when creating the TD-List in AddTransfer)
     *
     * In order to implement this algorithm, we need to add two pointer to QH
     *
     *      m_pVirtualTdList - point to the TD list corresponding to STransfer
     *      m_pCurrentTd - used to traverse the TD list
     *
     */
    CheckStructure();
    if (pCurTD && nextQTDPointer.lpContext.Terminate!=0 && qTD_Overlay.qTD_Token.qTD_TContext.Active == 0)
    {
        currentQTDPointer.dwLinkPointer = 0;
        ASSERT((pCurTD->GetPhysAddr() & 0x1f) == 0);
        //nextQTDPointer.dwLinkPointer = pCurTD->GetPhysAddr();
        m_pVirtualTdList = pCurTD;
        m_pCurrentTd = m_pVirtualTdList;

        //RETAILMSG(1, (L"ini m_pCurrentTd as %x\r\n"));
        //RETAILMSG(1, (L"need we set halt? currnt is %d\r\n", qTD_Overlay.qTD_Token.qTD_TContext.Halted));
        //qTD_Overlay.qTD_Token.qTD_TContext.Halted = 0; // need we do this?
        return TRUE;
    }
    else
    {
        ASSERT(FALSE);
        return FALSE;
    }
#else
    CheckStructure ();
    if (pCurTD  && nextQTDPointer.lpContext.Terminate!=0 && qTD_Overlay.qTD_Token.qTD_TContext.Active == 0 ) { // Queue If they queue is not active.
        //memcpy((void *)&qTD_Overlay, pCurTD->GetQTDData(),sizeof(QTD));
        currentQTDPointer.dwLinkPointer =  0;
        ASSERT((pCurTD->GetPhysAddr() & 0x1f) == 0);
        nextQTDPointer.dwLinkPointer =pCurTD->GetPhysAddr(); // This will activate the TD trasnfer.
        qTD_Overlay.qTD_Token.qTD_TContext.Halted = 0;
        return TRUE;
    }
    else {
        ASSERT(FALSE);
        return FALSE;
    };
#endif
}

#ifdef IRAM_PATCH
void CQH::FreeIRamSpace() 
{
    RETAILMSG(RETAIL_LOG, (L"Free Iram space @ %x and %x\r\n", m_pIRamEleMem, m_pIRamDataMem));
    if (m_pIRamTd != NULL)
    {
        memset(m_pIRamTd, 0, sizeof(QTD));
#ifdef IRAM_PATCH_EXTEND
        if (m_bEleUseExtendMem)
        {
            m_pDRamMem->FreeMemory((PBYTE)m_pIRamTd, 
                                      m_pDRamMem->VaToPa((PBYTE)m_pIRamTd),
                                      CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
        }
        else
        {
            m_pIRamEleMem->FreeMemory((PBYTE)m_pIRamTd, 
                                      m_pIRamEleMem->VaToPa((PBYTE)m_pIRamTd),
                                      CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
        }
#else
        m_pIRamEleMem->FreeMemory((PBYTE)m_pIRamTd, 
                                  m_pIRamEleMem->VaToPa((PBYTE)m_pIRamTd),
                                  CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
#endif
        m_pIRamTd = NULL;
        RETAILMSG(RETAIL_LOG, (L"TD free\r\n"));
    }
    if (m_pIRamStart != NULL)
    {
        memset(m_pIRamStart, 0, m_pCurrentTd->BufLength());
#ifdef IRAM_PATCH_EXTEND
        if (m_bDatUseExtendMem)
        {
            m_pDRamMem->FreeMemory((PBYTE)m_pIRamStart,
                                        m_pDRamMem->VaToPa((PBYTE)m_pIRamStart),
                                        CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
        }
        else
        {
            m_pIRamDataMem->FreeMemory((PBYTE)m_pIRamStart,
                                        m_pIRamDataMem->VaToPa((PBYTE)m_pIRamStart),
                                        CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
        }
#else
        m_pIRamDataMem->FreeMemory((PBYTE)m_pIRamStart,
                                    m_pIRamDataMem->VaToPa((PBYTE)m_pIRamStart),
                                    CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
#endif
        m_pIRamStart = NULL;
        RETAILMSG(RETAIL_LOG, (L"DB free\r\n"));
    }
}



//********************************************************************************
BOOL CQH::QueueNextSubTD()
//
// Purpose: Set up the currentQTD Pointer onto the current QH
//
// Parameters: pCurTD - pointer to TD to be queued
//
// Returns: TRUE - success, FALSE - failure
//
// ******************************************************************
{
    /*
     * BookMarks : QueueNextSubTD()
     */
    /*
     * This function is new for IRAM-PATCH
     * 
     * TD list is attached to m_pVirtualTdList, m_pCurrentTd is 
     * pointing to current ovelayed one, m_pIRamTd is the cloned one
     * in iRAM
     *
     * 1. check m_pIRamTd is NULL
     * 2. allocate in IRAM a new QTD stucture
     * 3. memcopy (DRAM, IRAM)
     * 4. m_pCurrentTd++
     * 5. modification to SRAM one
     *    5.1 next NULL
     *    5.2 Allocate IRAM data buffer
     *    5.3 BP point to IRAM one
     * 6. memcopy for OUT transfers
     * 7. Link IRAM TD to QH
     */
    ULONG iramDataPtPA = 0; 

    // 1.
    ASSERT(m_pIRamTd == NULL);
    if (m_pIRamTd != NULL)
    {
        RETAILMSG(RETAIL_LOG, (L"something happen need to check\r\n"));
    }
    // 2.
#ifdef IRAM_PATCH_EXTEND
    m_bEleUseExtendMem = FALSE;
#endif
    while (m_pIRamTd == NULL)
    {
        RETAILMSG(RETAIL_LOG, (L"new IRamTd @ %x\r\n", m_pIRamEleMem));
        if (!m_pIRamEleMem->AllocateMemory(DEBUG_PARAM( TEXT("IRAM_TD")) sizeof(CQTD), (PUCHAR*)&m_pIRamTd, CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK))
        {
#ifdef IRAM_PATCH_EXTEND
            RETAILMSG(1, (L"td allocate failure in iram, try allocate in dram\r\n"));
            m_bEleUseExtendMem = TRUE;
            if (!m_pDRamMem->AllocateMemory(DEBUG_PARAM( TEXT("IRAM_TD")) sizeof(CQTD), (PUCHAR*)&m_pIRamTd, CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK))
            {
                RETAILMSG(1, (L"also allocated failure in dram\r\n"));
                m_bEleUseExtendMem = FALSE;
            }
#else
            RETAILMSG(1, (L"td allocate failure in iram\r\n"));
#endif
        }
    }
    RETAILMSG(IRAM_MEM_LOG, (L"new iRamTD Ele RAM\r\n"));
#if (IRAM_MEM_LOG == 1)
    m_pIRamEleMem->ShowMemInfo();
#endif
    // 3.
#ifdef EHCI_PROBE
    //RETAILMSG(RETAIL_LOG, (L"before cloning, curtd is %x, start %x, len %d, clone to %x\r\n", m_pCurrentTd, m_pCurrentTd->StartOfBuffer(), m_pCurrentTd->BufLength(), m_pIRamTd));
#endif
    //RETAILMSG(1, (L"%d %x(%x) %x\r\n", sizeof(CQTD), m_pIRamTd, m_pIRamEleMem->VaToPa((PUCHAR)m_pIRamTd), m_pCurrentTd));
    memcpy(m_pIRamTd, m_pCurrentTd,sizeof(CQTD));
    // 4. Need to do back overlay, so delay the traverse
    //m_pCurrentTd = m_pCurrentTd->GetNextTD();

    //5.1
    m_pIRamTd->FlushNextTD();
#ifdef IRAM_PATCH_EXTEND
    if (m_bEleUseExtendMem)
    {
        m_pIRamTd->SetPhysAddr(m_pDRamMem->VaToPa((PUCHAR)m_pIRamTd));
    }
    else
    {
        m_pIRamTd->SetPhysAddr(m_pIRamEleMem->VaToPa((PUCHAR)m_pIRamTd));
    }
#else
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"TD phy %x\r\n", m_pIRamEleMem->VaToPa((PUCHAR)m_pIRamTd)));
#endif
    m_pIRamTd->SetPhysAddr(m_pIRamEleMem->VaToPa((PUCHAR)m_pIRamTd));
#endif

    //5.2
#ifdef IRAM_PATCH_EXTEND
    m_bDatUseExtendMem = FALSE;
#endif
    while (m_pIRamStart == NULL)
    {
        RETAILMSG(RETAIL_LOG, (L"new IRamDB @ %x\r\n", m_pIRamDataMem));
        if (!m_pIRamDataMem->AllocateMemory(DEBUG_PARAM(TEXT("IRAM_DATA")) m_pCurrentTd->BufLength(), (PUCHAR*)&m_pIRamStart, CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK))
        {
#ifdef IRAM_PATCH_EXTEND
            RETAILMSG(1, (L"iram data allocate failure, try allocate in dram\r\n"));
            m_bDatUseExtendMem = TRUE;
            if (!m_pDRamMem->AllocateMemory(DEBUG_PARAM(TEXT("IRAM_DATA")) m_pCurrentTd->BufLength(), (PUCHAR*)&m_pIRamStart, CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK))
            {
                RETAILMSG(1, (L"also allocate failure in dram\r\n"));
                m_bDatUseExtendMem = FALSE;
            }
#else
            RETAILMSG(1, (L"iram data allocate failure\r\n"));
#endif
        }
    }
#ifdef IRAM_PATCH_EXTEND
    if (m_bDatUseExtendMem)
    {
        iramDataPtPA = m_pDRamMem->VaToPa(m_pIRamStart);
    }
    else
    {
        iramDataPtPA = m_pIRamDataMem->VaToPa(m_pIRamStart);
    }
#else
    iramDataPtPA = m_pIRamDataMem->VaToPa(m_pIRamStart);
#endif
    RETAILMSG(IRAM_MEM_LOG, (L"iram Data Data RAM\r\n"));
#if (IRAM_MEM_LOG == 1)
    m_pIRamDataMem->ShowMemInfo();
#endif

    //5.3
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"BP phy %x\r\n", iramDataPtPA));
#endif
    m_pIRamTd->ResetIRamBP(iramDataPtPA);

    //RETAILMSG(1, (L"IsOutTransfer will return %d \r\n", m_pCurrentTd->IsOutTransfer()));
    //6
    //RETAILMSG(1, (L"memcopy for OUT & SETUP packet\r\n"));
    if (m_pCurrentTd->IsOutTransfer())
    {
        RETAILMSG(CRITICAL_LOG, (L"mcpy %d f %x to %x\r\n", m_pCurrentTd->BufLength(), m_pCurrentTd->StartOfBuffer(), m_pIRamStart));
        //{
        //    PUCHAR pOrigData = m_pCurrentTd->StartOfBuffer();
        //    DWORD i;
        //    for (i=0; i!=min(31, m_pCurrentTd->BufLength());)
        //    {
        //        RETAILMSG(RETAIL_LOG, (L"%2x ", *pOrigData));
        //        pOrigData++;
        //        i++;
        //        if (i % 16 == 0) RETAILMSG(1, (L"\r\n"));
        //    }
        //    if (i % 16 !=0) RETAILMSG(1, (L"\r\n"));
        //}
        memcpy(m_pIRamStart, m_pCurrentTd->StartOfBuffer(), m_pCurrentTd->BufLength());
        RETAILMSG(IRAM_MEM_LOG, (L"mc %d to %x(%x)\r\n", m_pCurrentTd->BufLength(), m_pIRamStart, iramDataPtPA));
    }

    //7
    RETAILMSG(RETAIL_LOG, (L"a Itd\r\n"));
    //RETAILMSG(1, (L"currentQTDPointer.dwLinkPointer is %x\r\n", currentQTDPointer.dwLinkPointer));
    //RETAILMSG(1, (L"qTD_Overlay.qTD_Token.qTD_TContext.Halted is %d\r\n", qTD_Overlay.qTD_Token.qTD_TContext.Halted));
    //this->Dump();
    RETAILMSG(IRAM_MEM_LOG, (L"QH is\r\n"));
#if (IRAM_MEM_LOG == 1)
    this->Dump();
#endif
    RETAILMSG(IRAM_MEM_LOG, (L"TD link is\r\n"));
#if (IRAM_MEM_LOG == 1)
    m_pIRamTd->Dump();
#endif
    nextQTDPointer.dwLinkPointer = m_pIRamTd->GetPhysAddr(); // This will activate the TD trasnfer.
    RETAILMSG(CRITICAL_LOG, (L"A! %x\r\n", nextQTDPointer.dwLinkPointer));
    qTD_Overlay.qTD_Token.qTD_TContext.Halted = 0;

    return TRUE;
}
#endif
