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
//     Trans.cpp
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
#include <Cphysmem.hpp>
#pragma warning(pop)

#include "trans.h"
#include "cpipe.h"
#include "chw.h"
#include "cehcd.h"

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

#define RETAIL_LOG 0

#if 0 // Remove-W4: Warning C4505 workaround
//***********************************************************************************
static void 
memdump(unsigned char * data,   unsigned short num_bytes, unsigned short offset    )
//
// Purpose: Dump the memory content to the console
//
// Parameters: data - pointer to the data to be dumped
//             num_bytes - size of data buffer
//             offset - the index where it starts with
//
// Returns: Nothing
//
// Notes:
//
// ******************************************************************
{       
    unsigned short i,j,l;
    unsigned char tmp_str[100]; 
    unsigned char tmp_str1[10];    
    for (i = 0; i < num_bytes; i += 16)    {        
        unsigned short n ;      
        tmp_str[0]='\0';        
        n = i+offset ;        
        for (j=0; j<4; j++) {
            l=n%16;
            if (l>=10)              
                tmp_str[3-j]=(unsigned char)('A'+l-10);         
            else                
                tmp_str[3-j]=(unsigned char)(l+'0');            
            n >>= 4 ;       
        }       
        tmp_str[4]='\0';        
        strcat ( (char *)tmp_str, ": ");        
        /*          Output the hex bytes        */        
        for (j = i; j < (i+16); j++) {          
            int m ;            
            if (j < num_bytes)  {  
                m=((unsigned int)((unsigned char)*(data+j)))/16 ;               
                if (m>=10)                  
                    tmp_str1[0]='A'+(unsigned char)m-10;                
                else                    
                    tmp_str1[0]=(unsigned char)m+'0';               
                m=((unsigned int)((unsigned char)*(data+j)))%16 ;               
                if (m>=10)                  
                    tmp_str1[1]='A'+(unsigned char)m-10;                
                else                    
                    tmp_str1[1]=(unsigned char)m+'0';               
                tmp_str1[2]='\0';               
                strcat ((char *)tmp_str, (char *)tmp_str1);             
                strcat ((char *)tmp_str, " ");            
            }            
            else {                
                strcat((char *)tmp_str,"   ");            
            }        
        }        
        strcat((char *)tmp_str, "  ");      
        l=(unsigned short)strlen((char *)tmp_str);        

        /*         * Output the ASCII bytes        */        
        for (j = i; j < (i+16); j++){            
            if (j < num_bytes){
                char c = *(data+j);                
                if (c < ' ' || c > 'z') 
                    c = '.';                
                tmp_str[l++]=c;            
            }            
            else            
                tmp_str[l++]=' ';
        }       
        tmp_str[l++]='\r';        tmp_str[l++]='\n';        tmp_str[l++]='\0';     
    }
}
#endif

//********************************************************************************************
DWORD CTransfer::m_dwGlobalTransferID=0;
CTransfer::CTransfer(IN CPipe * const pCPipe, IN CPhysMem * const pCPhysMem,STransfer *pSTransfer) 
    : m_pCPipe(pCPipe)
    , m_pCPhysMem(pCPhysMem)
//
// Purpose: Constructor of CTransfer class
//
// Parameters: pCPipe - Pointer to CPipe object to be associated with
//             pCPhysMem - Pointer to CPhysMem to be used
//             pSTransfer - Pointer to STransfer containing all the information for USB transfer
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    m_pNextTransfer=NULL;
    m_paControlHeader=0;
    m_pAllocatedForControl=NULL;
    m_pAllocatedForClient=NULL;
    memcpy(&m_sTransfer, pSTransfer,sizeof(STransfer));
    m_DataTransferred =0 ;
    m_fDoneTransferCalled = FALSE;
    m_dwTransferID = m_dwGlobalTransferID++;
#ifdef IRAM_PATCH
    m_pVirtualStart = NULL;
#endif
}

//********************************************************************
CTransfer::~CTransfer()
//
// Purpose: Destructor of CTransfer class
//
// Parameters: None
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    if (m_pAllocatedForControl!=NULL) 
        m_pCPhysMem->FreeMemory( PUCHAR(m_pAllocatedForControl),m_paControlHeader,  CPHYSMEM_FLAG_NOBLOCK );
    if (m_pAllocatedForClient!=NULL)
        m_pCPhysMem->FreeMemory( PUCHAR(m_pAllocatedForClient), m_sTransfer.paBuffer,  CPHYSMEM_FLAG_NOBLOCK );
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE , (TEXT("CTransfer::~CTransfer() (this=0x%x,m_pAllocatedForControl=0x%x,m_pAllocatedForClient=0x%x)\r\n"),
        this,m_pAllocatedForControl,m_pAllocatedForClient));

}

//*******************************************************************
BOOL CTransfer::Init(void)
//
// Purpose: Initialize routine for the CTransfer class
//          This includes allocation of memory and setup the CTransfer class
//          necessary to do the transfer.  Finally, it calls AddTransfer() to do
//          the transfer
//
// Parameters: None
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CTransfer::Init (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    // We must allocate the control header memory here so that cleanup works later.
    if (m_sTransfer.lpvControlHeader != NULL &&  m_pAllocatedForControl == NULL ) {
        // This must be a control transfer. It is asserted elsewhere,
        // but the worst case is we needlessly allocate some physmem.
        if ( !m_pCPhysMem->AllocateMemory(
                                   DEBUG_PARAM( TEXT("IssueTransfer SETUP Buffer") )
                                   sizeof(USB_DEVICE_REQUEST),
                                   &m_pAllocatedForControl,
                                   CPHYSMEM_FLAG_NOBLOCK ) ) {
            DEBUGMSG( ZONE_WARNING, (TEXT("CPipe(%s)::IssueTransfer - no memory for SETUP buffer\n"), m_pCPipe->GetPipeType() ) );
            m_pAllocatedForControl=NULL;
            return FALSE;
        }
        m_paControlHeader = m_pCPhysMem->VaToPa( m_pAllocatedForControl );
        DEBUGCHK( m_pAllocatedForControl != NULL && m_paControlHeader != 0 );

        __try {
            memcpy(m_pAllocatedForControl,m_sTransfer.lpvControlHeader,sizeof(USB_DEVICE_REQUEST));
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            // bad lpvControlHeader
            return FALSE;
        }
    }
#ifdef DEBUG
    if ( m_sTransfer.dwFlags & USB_IN_TRANSFER ) {
        // I am leaving this in for two reasons:
        //  1. The memset ought to work even on zero bytes to NULL.
        //  2. Why would anyone really want to do a zero length IN?
        DEBUGCHK( m_sTransfer.dwBufferSize > 0 &&
                  m_sTransfer.lpvBuffer != NULL );
        __try { // IN buffer, trash it
            memset( PUCHAR( m_sTransfer.lpvBuffer ), GARBAGE, m_sTransfer.dwBufferSize );
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            DEBUGMSG(ZONE_ERROR,(TEXT("CTransfer: Bad buffer to memset function")));
        }
    }
#endif // DEBUG

    if ( m_sTransfer.dwBufferSize > 0 && m_sTransfer.paBuffer == 0 ) { 

        // ok, there's data on this transfer and the client
        // did not specify a physical address for the
        // buffer. So, we need to allocate our own.

        if ( !m_pCPhysMem->AllocateMemory(
                                   DEBUG_PARAM( TEXT("IssueTransfer Buffer") )
                                   m_sTransfer.dwBufferSize,
                                   &m_pAllocatedForClient, 
                                   CPHYSMEM_FLAG_NOBLOCK ) ) {
            DEBUGMSG( ZONE_WARNING, (TEXT("CPipe(%s)::IssueTransfer - no memory for TD buffer\n"), m_pCPipe->GetPipeType() ) );
            m_pAllocatedForClient = NULL;
            return FALSE;
        }
        m_sTransfer.paBuffer = m_pCPhysMem->VaToPa( m_pAllocatedForClient );
#ifdef IRAM_PATCH
        m_pVirtualStart = m_pAllocatedForClient;
#endif
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"%d pa %x\r\n", __LINE__, m_sTransfer.paBuffer));
#endif
        PREFAST_DEBUGCHK( m_pAllocatedForClient != NULL);
        PREFAST_DEBUGCHK( m_sTransfer.lpvBuffer!=NULL);
        DEBUGCHK(m_sTransfer.paBuffer != 0 );

        if ( !(m_sTransfer.dwFlags & USB_IN_TRANSFER) ) {
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"cpy c b\r\n"));
#endif
            __try { // copying client buffer for OUT transfer
                memcpy( m_pAllocatedForClient, m_sTransfer.lpvBuffer, m_sTransfer.dwBufferSize );               
                //if ((m_sTransfer.dwBufferSize == 512) || (m_sTransfer.dwBufferSize == 31))
                //DEBUGMSG(1, (TEXT("Out Transaction size %d\r\n"), m_sTransfer.dwBufferSize));
                //memdump(m_pAllocatedForClient, (USHORT)m_sTransfer.dwBufferSize, 0);
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvClientBuffer
                   return FALSE;
            }
        }
    }
#ifdef IRAM_PATCH
    else
    {
        m_pVirtualStart = (PBYTE)m_sTransfer.lpvBuffer;
    }
#endif
    
    DEBUGMSG(  ZONE_TRANSFER && ZONE_VERBOSE , (TEXT("CQTransfer::Init (this=0x%x,id=0x%x),m_pAllocatedForControl=0x%x,m_pAllocatedForClient=0x%x\r\n"),
        this,m_dwTransferID,m_pAllocatedForControl,m_pAllocatedForClient));
#ifdef IRAM_PATCH
    RETAILMSG(RETAIL_LOG, (L"Co%x, Cl%x, v_S%x, s%d\r\n", m_pAllocatedForControl, m_pAllocatedForClient, m_pVirtualStart, m_sTransfer.dwBufferSize));
#endif
    return AddTransfer();
}

//********************************************************************************
CQTransfer::~CQTransfer()
//
// Purpose: Destructor of CQTransfer
//
// Parameters: None
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    CQTD * pCurTD = m_pCQTDList;
    while (pCurTD!=NULL) {
         CQTD * pNextTD = pCurTD->GetNextTD();
         pCurTD->~CQTD();
         m_pCPhysMem->FreeMemory((PBYTE)pCurTD,m_pCPhysMem->VaToPa((PBYTE)pCurTD), CPHYSMEM_FLAG_HIGHPRIORITY |CPHYSMEM_FLAG_NOBLOCK );
         pCurTD = pNextTD;
    }
}

//********************************************************************
BOOL CQTransfer::AddTransfer() 
//
// Purpose: Setup all the queue transfer descriptor and do the actual transfer
//          of the data through IssueTransfer
//
// Parameters: None
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::AddTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    if (m_pCQTDList) { // Has been created. Somthing wrong.
        ASSERT(FALSE);
        return FALSE;
    }
    BOOL bDataToggle1= FALSE;
    CQTD * pStatusTD = NULL;
    if (m_paControlHeader!=NULL && m_sTransfer.lpvControlHeader!=NULL) { 
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"AT, se p\r\n"));
#endif
        // This is setup packet.
        if (m_pCQTDList = new(m_pCPhysMem) CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead())) {            
            PhysBufferArray bufferArray;
#ifdef IRAM_PATCH
            bufferArray.bufVirtStart = m_pAllocatedForControl;
#endif
            bufferArray.dwNumOfBlock=1;
            bufferArray.dwStartOffset=m_paControlHeader & EHCI_PAGE_OFFSET_MASK;
            bufferArray.dwBlockSize=min(bufferArray.dwStartOffset + sizeof(USB_DEVICE_REQUEST),EHCI_PAGE_SIZE );
            bufferArray.dwArrayBlockAddr[0]=(m_paControlHeader & EHCI_PAGE_ADDR_MASK);
            bufferArray.dwArrayBlockAddr[1]=((m_paControlHeader+sizeof(USB_DEVICE_REQUEST)) & EHCI_PAGE_ADDR_MASK ); // Terminate
            m_pCQTDList->IssueTransfer( TD_SETUP_PID, bDataToggle1, sizeof(USB_DEVICE_REQUEST),&bufferArray,TRUE);
            bDataToggle1 = !bDataToggle1;
        }
        else 
            return FALSE;
        // Status Packet
        pStatusTD = new(m_pCPhysMem) CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead());
        if (pStatusTD) {            
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"AT, st p\r\n"));
#endif
            PhysBufferArray bufferArray;
            bufferArray.dwNumOfBlock=0;
            bufferArray.dwBlockSize=0;
            bufferArray.dwStartOffset=0;
            bufferArray.dwArrayBlockAddr[0]=0;
            bufferArray.dwArrayBlockAddr[1]=0;// Terminate
            pStatusTD->IssueTransfer( (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?TD_OUT_PID:TD_IN_PID,
                TRUE, 0 ,&bufferArray,TRUE);
        }
        else 
            return FALSE;
    };
    CQTD * pPrevTD=m_pCQTDList;
#if 1 /*Eric merge public code for zero length transfer*/
    if ( m_pCQTDList==NULL  && m_sTransfer.dwBufferSize == 0 ) { // No Control but Zero Length
        ASSERT((m_sTransfer.dwFlags & USB_IN_TRANSFER)==0);// No meaning for IN Zero length packet.
        CQTD * pCurTD = new( m_pCPhysMem) CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead());
        if (pCurTD) {            
            PhysBufferArray bufferArray;
#ifdef IRAM_PATCH
            bufferArray.bufVirtStart = m_pVirtualStart;
#endif
            bufferArray.dwNumOfBlock=0;
            bufferArray.dwBlockSize=0;
            bufferArray.dwStartOffset=0;
            bufferArray.dwArrayBlockAddr[0]=0;
            bufferArray.dwArrayBlockAddr[1]=0;// Terminate
            pCurTD->IssueTransfer( (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0? TD_IN_PID: TD_OUT_PID,
                bDataToggle1, 0 ,&bufferArray,TRUE);
            if (pPrevTD) {
                pPrevTD->QueueNextTD(pCurTD);
                pPrevTD=pCurTD;
            }
            else { // THis is First. So update m_pQTDList
                pPrevTD= m_pCQTDList = pCurTD;
            }
        }
        else {
            if ( pStatusTD) {
                pStatusTD->~CQTD();
                m_pCPhysMem->FreeMemory((PBYTE)pStatusTD,m_pCPhysMem->VaToPa((PBYTE)pStatusTD), CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK );
            }
            return FALSE;                
        }
    }
    else
#endif
    if (m_sTransfer.lpvBuffer &&  m_sTransfer.paBuffer && m_sTransfer.dwBufferSize) {
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"%d t len %d\r\n", __LINE__, m_sTransfer.dwBufferSize));
        DWORD TDCount = 0;
#endif
        DWORD dwCurPos=0;
        while ( dwCurPos< m_sTransfer.dwBufferSize) {
#ifdef EHCI_PROBE
            TDCount++;
            RETAILMSG(RETAIL_LOG, (L"No.%d TD\r\n", TDCount));
#endif
            CQTD * pCurTD = new( m_pCPhysMem) CQTD(this, ((CQueuedPipe * const)m_pCPipe)->GetQHead());
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"Data TD allocated\r\n"));
#endif
            ASSERT(pCurTD!=NULL);
            if (pCurTD==NULL) {
                // delete  pStatusTD;
                if ( pStatusTD) {
                    pStatusTD->~CQTD();
                    m_pCPhysMem->FreeMemory((PBYTE)pStatusTD,m_pCPhysMem->VaToPa((PBYTE)pStatusTD), CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK );
                }
                return FALSE;                
            }                
            DWORD dwCurPhysAddr=  m_sTransfer.paBuffer + dwCurPos;
            // We only can queue maximun 4 page for one TD and  Align with Packet Size.
            // Note: Even though we can limit to 16K, but if start offset != 0, it would require 5 buffer pointers, instead of 4.
            DWORD dwPacketSize= (m_pCPipe->GetEndptDescriptor()).wMaxPacketSize & 0x7ff;
            ASSERT(dwPacketSize!=0);
#ifdef IRAM_PATCH
            DWORD dwCurLength = IRAM_MAX_TD_TRANSFER_LENGTH;
#else
            DWORD dwCurLength = ((EHCI_PAGE_SIZE * MAX_QTD_PAGE_SIZE)/dwPacketSize)*dwPacketSize;            
#endif
            dwCurLength = min(m_sTransfer.dwBufferSize - dwCurPos, dwCurLength);
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"%d dwCurLength is %d\r\n", __LINE__, dwCurLength));
#endif
            PhysBufferArray bufferArray;
#ifdef IRAM_PATCH
            bufferArray.bufVirtStart = m_pVirtualStart + dwCurPos;
#endif
            bufferArray.dwStartOffset= dwCurPhysAddr & EHCI_PAGE_OFFSET_MASK;
            bufferArray.dwNumOfBlock=  (bufferArray.dwStartOffset + dwCurLength + EHCI_PAGE_SIZE -1 ) /EHCI_PAGE_SIZE;          
            ASSERT(bufferArray.dwNumOfBlock<=(MAX_QTD_PAGE_SIZE+1) && bufferArray.dwNumOfBlock!=0);            
            bufferArray.dwBlockSize = min (bufferArray.dwStartOffset + m_sTransfer.dwBufferSize- dwCurPos,EHCI_PAGE_SIZE);
            
#ifdef IRAM_PATCH
            /*
             * Since IRAM patch only uses 2K buffer size, so only the first block is needed
             */
            bufferArray.dwArrayBlockAddr[0] = (dwCurPhysAddr & EHCI_PAGE_OFFSET_MASK);
#else
            for (DWORD dwIndex=0;dwIndex<bufferArray.dwNumOfBlock+1 && dwIndex<5;dwIndex++) {
                bufferArray.dwArrayBlockAddr[dwIndex] = (dwCurPhysAddr & EHCI_PAGE_ADDR_MASK);
                dwCurPhysAddr += EHCI_PAGE_SIZE;
            }
#endif
            DWORD dwReturnLength = pCurTD->IssueTransfer( (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?TD_IN_PID:TD_OUT_PID,
                bDataToggle1, dwCurLength,&bufferArray,TRUE);
            ASSERT(dwReturnLength == dwCurLength);

            // Setup for Short Packet
            if ( pStatusTD) {
#ifdef EHCI_PROBE
                //RETAILMSG(1, (L"SetAltNextQTDPointer\r\n"));
#endif
#ifdef IRAM_PATCH
                pCurTD->QueueNextTD(pStatusTD);
#else
                pCurTD->SetAltNextQTDPointer(pStatusTD->GetPhysAddr());
#endif
            }
            
            DWORD dwNumOfPacket = (dwReturnLength + dwPacketSize-1)/dwPacketSize;
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"total %d Ps\r\n", dwNumOfPacket));
#endif
            
            dwCurPos += dwReturnLength;
            if (pPrevTD) {
#ifdef EHCI_PROBE
                //RETAILMSG(1, (L"next TD\r\n"));
#endif
                pPrevTD->QueueNextTD(pCurTD);
                pPrevTD=pCurTD;
            }
            else { // THis is First. So update m_pQTDList
#ifdef EHCI_PROBE
                RETAILMSG(RETAIL_LOG, (L"first TD\r\n"));
#endif
                pPrevTD= m_pCQTDList = pCurTD;
            }
            if ((dwNumOfPacket & 1)!=0) // if it is odd packet number. Toggle the data toggle.
                bDataToggle1 = !bDataToggle1;
        }
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"total %d TDs\r\n", TDCount));
#endif
    }
    // We have to append Status TD here.
    if (pStatusTD) { // This is setup packet.
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"%d\r\n", __LINE__));
#endif
        if (pPrevTD) {
            pPrevTD->QueueNextTD(pStatusTD);
            pPrevTD=pStatusTD;
        }
        else { // Something Wrong.
            ASSERT(FALSE);
            //delete pCurTD;
            pStatusTD->~CQTD();
            m_pCPhysMem->FreeMemory((PBYTE)pStatusTD,m_pCPhysMem->VaToPa((PBYTE)pStatusTD), CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK  );
            return FALSE;
        }
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"\r\n"));
#endif
    };
    return TRUE;    
}

//***********************************************************************************************
BOOL CQTransfer::DoneTransfer()
//
// Purpose: Check if transfer is done and success, acknowledge back into MDD
//
// Parameters: None
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::DoneTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    BOOL bIsTransDone = IsTransferDone();
    ASSERT(bIsTransDone == TRUE);
    if (bIsTransDone && !m_fDoneTransferCalled) {
        DWORD dwUsbError = USB_NO_ERROR;
        DWORD dwDataNotTransferred = 0;
        m_fDoneTransferCalled = TRUE;
        
        CQTD * pCurTD = m_pCQTDList;
        while ( pCurTD!=NULL) {
            pCurTD->CheckStructure();
            if (  pCurTD->qTD_Token.qTD_TContext.PID != 2) // Do not count Setup TD
                dwDataNotTransferred +=  pCurTD ->qTD_Token.qTD_TContext.BytesToTransfer ;
            if (pCurTD->qTD_Token.qTD_TContext.Halted==1) { // This Transfer Has been halted due to error.
                // This is error. We do not have error code for EHCI so generically we set STALL error.
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_STALL_ERROR;
            }
            else
            if (pCurTD->qTD_Token.qTD_TContext.Active ==1) {
                if (dwUsbError == USB_NO_ERROR) 
                    dwUsbError = USB_NOT_COMPLETE_ERROR;
                break;
            }
            pCurTD = pCurTD->GetNextTD();
        }
        ASSERT(dwDataNotTransferred <= m_sTransfer.dwBufferSize);
        if (dwDataNotTransferred >= m_sTransfer.dwBufferSize)
            dwDataNotTransferred = m_sTransfer.dwBufferSize;
        m_DataTransferred = m_sTransfer.dwBufferSize -  dwDataNotTransferred ;
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"tot %d\r\n", m_DataTransferred));
#endif

        // We have to update the buffer when this is IN Transfer.
        if ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=NULL && m_pAllocatedForClient!=NULL && m_sTransfer.dwBufferSize!=0) {
            __try { // copying client buffer for OUT transfer
#ifdef EHCI_PROBE
                RETAILMSG(RETAIL_LOG, (L"mcpy IN %d b\r\n",__LINE__, m_sTransfer.dwBufferSize));
#endif
                memcpy( m_sTransfer.lpvBuffer, m_pAllocatedForClient, m_sTransfer.dwBufferSize );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvBuffer.
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_CLIENT_BUFFER_ERROR;
            }
        }

        __try { // setting transfer status and executing callback function
            if (m_sTransfer.lpfComplete !=NULL)
                *m_sTransfer.lpfComplete = TRUE;
            if (m_sTransfer.lpdwError!=NULL)
                *m_sTransfer.lpdwError = dwUsbError;
            if (m_sTransfer.lpdwBytesTransferred)
                *m_sTransfer.lpdwBytesTransferred =  m_DataTransferred;
            if ( m_sTransfer.lpStartAddress ) {
#ifdef EHCI_PROBE
                RETAILMSG(RETAIL_LOG, (L"cback\r\n"));
#endif
#ifdef EHCI_PROBE
                RETAILMSG(RETAIL_LOG, (L"cb %x %x\r\n", m_sTransfer.lpStartAddress, m_sTransfer.lpvNotifyParameter));
#endif
                ( *m_sTransfer.lpStartAddress )(m_sTransfer.lpvNotifyParameter );
                m_sTransfer.lpStartAddress = NULL ; // Make sure only do once.
            }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
              DEBUGMSG( ZONE_ERROR, (TEXT("CQueuedPipe(%s)::CheckForDoneTransfers - exception setting transfer status to complete\n"), m_pCPipe->GetPipeType() ) );
        }
        return (dwUsbError==USB_NO_ERROR);
    }
    else
        return TRUE;
}

//**********************************************************************************
BOOL CQTransfer::AbortTransfer()
//
// Purpose: Abort the current transfer
//
// Parameters: None
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::AbortTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    CQTD * pCurTD = m_pCQTDList;
    while ( pCurTD!=NULL) {
        pCurTD->DeActiveTD();
        pCurTD = pCurTD->GetNextTD();
    }
    Sleep(2);// Make sure the shcdule has advanced. and current Transfer has completeded.
    return TRUE;
}

//*********************************************************************
BOOL CQTransfer::IsTransferDone()
//
// Purpose: This is to read the current transfer descriptor and see if there is 
//          any error or if the transfer is still active.
//
// Parameters: None
//
// Returns: TRUE - completed, FALSE - not completed
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::IsTransferDone (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    if (m_pCQTDList==NULL) { // Has been created. Somthing wrong.
        return TRUE;
    }
    CQTD * pCurTD = m_pCQTDList;
    BOOL bReturn=TRUE;
    while ( pCurTD!=NULL) {
        ASSERT(pCurTD->m_pTrans == this);
        if (pCurTD->qTD_Token.qTD_TContext.XactErr == 1) 
 
        if (pCurTD->qTD_Token.qTD_TContext.DataBufferError == 1)
        {
            if ((m_sTransfer.dwFlags & USB_IN_TRANSFER) != 0x00)
                DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("TD Token: DataBufferError in IN\r\n")));
            else
                DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("TD Token: DataBufferError in OUT\r\n")));
        }

        if (pCurTD->qTD_Token.qTD_TContext.Halted==1) { // This Transfer Has been halted due to error.
            DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer: Halted\r\n")));
            break;
        }
        if (pCurTD->qTD_Token.qTD_TContext.Active == 1) { 
            bReturn = FALSE;
            break;
        }


        if (pCurTD ->GetLinkValid() == FALSE) { // No like connected. This Transfer is  aborted.
            break;
        }

        pCurTD = pCurTD ->GetNextTD();
    }
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CQTransfer::IsTransferDone (this=0x%x) return %d \r\n"),this,bReturn));
    return bReturn;
}

//*************************************************************************************************
CIsochTransfer::CIsochTransfer(IN CIsochronousPipe * const pCPipe,IN CEhcd * const pCEhcd,STransfer *pSTransfer) 
    : CTransfer(pCPipe,pCEhcd->GetPhysMem(),pSTransfer)
    ,m_pCEhcd(pCEhcd)
//
// Purpose: Constructor of CIsochTransfer 
//
// Parameters: 
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    m_dwFrameIndexStart = m_sTransfer.dwStartingFrame;
    m_dwNumOfTD =0;
    m_dwSchedTDIndex=0;

    m_dwDequeuedTDIndex=0;
    
    m_dwArmedTDIndex =0;
    m_dwArmedBufferIndex=0;
    m_dwFirstError =  USB_NO_ERROR;
    m_dwLastFrameIndex = 0;

};

inline DWORD  CIsochTransfer::GetMaxTransferPerItd()
{   
    return (GetPipe())->GetMaxTransferPerItd(); 
};

// ******************************************************************
CITransfer::CITransfer(IN CIsochronousPipe * const pCPipe, IN CEhcd * const pCEhcd,STransfer *pSTransfer)
    :CIsochTransfer (pCPipe, pCEhcd,pSTransfer)
//
// Purpose: Constructor of High Speed Isochronous Transfer
//
// Parameters: 
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    m_pCITDList =0;
    ASSERT(GetMaxTransferPerItd()!=0);
    ASSERT(GetMaxTransferPerItd() <= MAX_TRNAS_PER_ITD);
}

//*******************************************************************
CITransfer::~CITransfer()
//
// Purpose: Destructor of High Speed Isochronous Transfer
//
// Parameters: 
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    ASSERT(m_dwSchedTDIndex==m_dwNumOfTD || m_dwLastFrameIndex<m_dwNumOfTD);
    ASSERT(m_dwDequeuedTDIndex ==  m_dwNumOfTD||m_dwLastFrameIndex<m_dwNumOfTD );
    if (m_pCITDList && m_dwNumOfTD) {
        AbortTransfer();
        for (DWORD dwIndex=0;dwIndex<m_dwNumOfTD;dwIndex++)
            if (*(m_pCITDList + dwIndex)) {
                ASSERT((*(m_pCITDList + dwIndex) )->GetLinkValid() != TRUE); // Invalid Next Link
                //Free *(m_pCITDList + dwIndex);
                GetPipe()->FreeCITD(*(m_pCITDList + dwIndex));
                *(m_pCITDList + dwIndex) = NULL;
            }
        delete m_pCITDList;
    }
}

//*********************************************************************
BOOL CITransfer::ArmTD()
//
// Purpose: Arm/schedule the transfer descriptor - this is to setup the transfer descriptor
//          and perform the IssueTransfer on the iTD to do the actual transfer. Called by
//          AddTransfer()
//
// Parameters: 
//
// Returns: TRUE - something has to be scheduled/armed
//          FALSE - nothing
//
// Notes:
// ******************************************************************
{
    BOOL bAnyArmed = FALSE;
    if (m_pCITDList && m_dwArmedTDIndex<m_dwNumOfTD ) { // Something TD wait for Arm.
        DWORD dwCurDataPhysAddr = m_sTransfer.paBuffer + m_dwArmedBufferIndex;
        DWORD dwFrameIndex = m_dwArmedTDIndex * GetMaxTransferPerItd() ;
        while (dwFrameIndex< m_sTransfer.dwFrames && m_dwArmedTDIndex < m_dwNumOfTD) {
            *(m_pCITDList + m_dwArmedTDIndex) = GetPipe()->AllocateCITD( this);
            if (*(m_pCITDList + m_dwArmedTDIndex) == NULL)             
                break;           
            DWORD dwTransLenArray[MAX_TRNAS_PER_ITD+1];  // Reserved one for cross boundary condition
            DWORD dwFrameAddrArray[MAX_TRNAS_PER_ITD+1]; // Reserved one for cross boundary condition
            for (DWORD i = 0; i < MAX_TRNAS_PER_ITD+1; i++)
                dwTransLenArray[i] = dwFrameAddrArray[i] = 0;
            for (DWORD dwTransIndex=0; dwTransIndex <  GetMaxTransferPerItd() && dwFrameIndex < m_sTransfer.dwFrames ; dwTransIndex ++) {
                dwTransLenArray[dwTransIndex]=  *(m_sTransfer.aLengths + dwFrameIndex);
                dwFrameAddrArray[dwTransIndex] = dwCurDataPhysAddr;
                 dwCurDataPhysAddr +=dwTransLenArray[dwTransIndex];
                m_dwArmedBufferIndex += dwTransLenArray[dwTransIndex];
                dwFrameIndex ++;
            }

            // Not at page 6, we want to avoid the cross page boundary
            // According to EHCI Specification 4.7.1, we should have one extra buffer pointer
            // in case of cross-boundary.
            if ((dwTransIndex > 0) && (dwTransIndex < MAX_TRNAS_PER_ITD+1 ))
            {
                 if (((dwFrameAddrArray[dwTransIndex-1]+dwTransLenArray[dwTransIndex-1]) & EHCI_PAGE_ADDR_MASK)!=
                    (dwFrameAddrArray[dwTransIndex-1] & EHCI_PAGE_ADDR_MASK))
                {
                    dwFrameAddrArray[MAX_TRNAS_PER_ITD] = ((dwFrameAddrArray[dwTransIndex-1] +0x1000) & EHCI_PAGE_ADDR_MASK);
                 }
            }            


            if (dwTransIndex !=0) {
                (*(m_pCITDList + m_dwArmedTDIndex))->IssueTransfer( 
                    dwTransIndex,dwTransLenArray, dwFrameAddrArray,
                    dwFrameIndex>= m_sTransfer.dwFrames ,
                    (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0);
                (*(m_pCITDList+ m_dwArmedTDIndex ))->SetIOC(TRUE); // Interrupt On every TD.
            }
            else {
                ASSERT(FALSE);
                break;
            }
            m_dwArmedTDIndex ++;
            bAnyArmed=TRUE;
        }
    }
    return bAnyArmed;
}

//***********************************************************************************************
BOOL CITransfer::AddTransfer () 
//
// Purpose: Setup all the queue transfer descriptor list and called up ArmTD to do the transfer
//
// Parameters: None
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::AddTransfer (this=0x%x,id=0x%x)\r\n"),this,m_dwTransferID));
    if (m_dwNumOfTD!=0 || m_pCITDList != NULL) {
        ASSERT(FALSE);
        return FALSE;
    }
    m_dwNumOfTD = (m_sTransfer.dwFrames + GetMaxTransferPerItd()-1)/GetMaxTransferPerItd();
    //DEBUGMSG(1, (TEXT("CITransfer:AddTransfer: NumberOfTD(%d), dwFrames(%d)\r\n"),
    //    m_dwNumOfTD, m_sTransfer.dwFrames));
    m_dwSchedTDIndex=0;
    m_dwArmedBufferIndex=0;
    m_dwArmedTDIndex = 0 ;
    m_dwFirstError =  USB_NO_ERROR;
    if (m_sTransfer.lpvBuffer &&  m_sTransfer.paBuffer && m_sTransfer.dwBufferSize) {
        // Allocate space for CITD List
        m_pCITDList =(CITD **) new PVOID[m_dwNumOfTD];
        if (m_pCITDList!=NULL) {
            memset(m_pCITDList,0,sizeof(CITD *)*m_dwNumOfTD);
            ArmTD();
            return TRUE;
        }
    }
    ASSERT(FALSE);
    return FALSE;
}
//****************************************************************************************************
BOOL CITransfer::ScheduleTD(DWORD dwCurFrameIndex,DWORD /*dwCurMicroFrameIndex*/)
//
// Purpose: Schedule a USB Transfer on this pipe, this is used by AbortTransfer only.
//
// Parameters: dwCurrFrameIndex - current frame index
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes: 
//******************************************************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::ScheduleTD (this=0x%x,id=0x%x,curFrameIndex=0x%x)\r\n"),this,m_dwTransferID, dwCurFrameIndex));
    BOOL bReturn = FALSE;
    ArmTD();
    if (m_dwSchedTDIndex < m_dwNumOfTD && m_pCITDList !=0 && m_pCEhcd ) {
        if ((long)(dwCurFrameIndex - m_dwFrameIndexStart) > (long)m_dwSchedTDIndex) {
            m_dwSchedTDIndex = dwCurFrameIndex - m_dwFrameIndexStart;
        }
        m_dwSchedTDIndex = min( m_dwSchedTDIndex , m_dwNumOfTD);
        
        DWORD EndShedTDIndex = dwCurFrameIndex + (m_pCEhcd->GetPeriodicMgr()).GetFrameSize()-1;
        DWORD dwNumTDCanSched;
        if ( (long)(EndShedTDIndex - m_dwFrameIndexStart ) >= 0)
            dwNumTDCanSched =EndShedTDIndex - m_dwFrameIndexStart ;
        else  // Too Early.
            dwNumTDCanSched = 0;
        dwNumTDCanSched= min(m_dwNumOfTD ,dwNumTDCanSched);
        dwNumTDCanSched = min (m_dwArmedTDIndex, dwNumTDCanSched);
        if (m_dwSchedTDIndex < dwNumTDCanSched) { // Do scudule those index.            
            for (DWORD dwIndex = m_dwSchedTDIndex ; dwIndex<dwNumTDCanSched; dwIndex++) {
                m_pCEhcd->PeriodQueueITD(*(m_pCITDList+dwIndex),m_dwFrameIndexStart + dwIndex );
                (*(m_pCITDList+dwIndex))->CheckStructure ();
                ASSERT((*(m_pCITDList+dwIndex))->m_pTrans == this);
            }
            m_dwSchedTDIndex = dwNumTDCanSched;
            bReturn = TRUE;
        }
    }
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::ScheduleTD (this=0x%x) return %d\r\n"),this,bReturn));
    return bReturn;
}

//***************************************************************************************
BOOL CITransfer::IsTransferDone(DWORD dwCurFrameIndex,DWORD /*dwCurMicroFrameIndex*/)
//
// Purpose: This is to check if the transfer has been completed for Isochronous High Speed Transfer
//
// Parameters: dwCurFrameIndex - minimal frame index to which the transfer to check against if completed.
//
// Returns: TRUE - successfully tranferred, FALSE - failure
//
// Notes: 
//****************************************************************************************
{
    // Dequeue those TD has Transfered.
    m_dwLastFrameIndex = dwCurFrameIndex - m_dwFrameIndexStart;
    if ((long)(dwCurFrameIndex - m_dwFrameIndexStart)>= 1 ) {
        DWORD dwTransfered = min(dwCurFrameIndex - m_dwFrameIndexStart , m_dwSchedTDIndex);
        for (DWORD dwIndex=m_dwDequeuedTDIndex;dwIndex<dwTransfered && dwIndex<m_dwNumOfTD ; dwIndex++) {
            if ( *(m_pCITDList+dwIndex) != NULL ) {
                ASSERT((*(m_pCITDList+dwIndex))->m_pTrans == this);
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
                BOOL bResult =m_pCEhcd->PeriodDeQueueTD((*(m_pCITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
#else
                m_pCEhcd->PeriodDeQueueTD((*(m_pCITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
#endif
                ASSERT(bResult == TRUE);
                (*(m_pCITDList + dwIndex) )->SetLinkValid(FALSE);
                (*(m_pCITDList+ dwIndex))->CheckStructure ();
                DWORD dwFrameIndex = dwIndex * GetMaxTransferPerItd();
                
                for (DWORD dwTrans=0; dwTrans<GetMaxTransferPerItd() && dwFrameIndex< m_sTransfer.dwFrames; dwTrans++) {
                    DWORD dwTDError= USB_NO_ERROR;
                    if ((*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.Active!=0) 
                        dwTDError = USB_NOT_COMPLETE_ERROR;
                    else
                    if ((*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.XactErr!=0) 
                        dwTDError = USB_ISOCH_ERROR;
                    else
                    if ((*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.BabbleDetected!=0) 
                        dwTDError = USB_STALL_ERROR;
                    else
                    if ((*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.DataBufferError!=0) 
                        dwTDError = ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?USB_DATA_OVERRUN_ERROR:USB_DATA_UNDERRUN_ERROR);
                        
                    if (m_dwFirstError == USB_NO_ERROR   ) { // only update first time
                        m_dwFirstError = dwTDError;
                    }
                    
                    DWORD dwTransLength =(*(m_pCITDList + dwIndex))->iTD_StatusControl[dwTrans].iTD_SCContext.TransactionLength;
                    if (dwFrameIndex< m_sTransfer.dwFrames) {
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
                        __try { // setting isoch OUT status parameters
                            m_sTransfer.adwIsochErrors[ dwFrameIndex ] = dwTDError;
                            // Document said length is only update for Isoch IN 3.3.2
                            m_sTransfer.adwIsochLengths[ dwFrameIndex ] = ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?
                                    dwTransLength :  *(m_sTransfer.aLengths + dwFrameIndex ));
                            m_DataTransferred += m_sTransfer.adwIsochLengths[ dwFrameIndex ];
                        } __except( EXCEPTION_EXECUTE_HANDLER ) {
                        }
#pragma prefast(pop)
                    }
                    dwFrameIndex ++;

                }
                GetPipe()->FreeCITD(*(m_pCITDList + dwIndex));
                *(m_pCITDList + dwIndex) = NULL;
            }
        }
        m_dwDequeuedTDIndex = dwIndex;
    }
    BOOL bReturn = ((long)(dwCurFrameIndex-m_dwFrameIndexStart)>= (long)m_dwNumOfTD);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer:::IsTransferDone (this=0x%x,curFrameIndex=0x%x) return %d \r\n"),this, dwCurFrameIndex,bReturn));
    return bReturn;
}

//**********************************************************************
BOOL CITransfer::AbortTransfer()
//
// Purpose: Abort current high speed isochronous transfer
//
// Parameters: None (all parameters are in m_transfer)
//
// Returns: TRUE - success, FALSE - failure
//
// Notes: 
//**********************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::AbortTransfer (this=0x%x,id=0x%x) \r\n"),this,m_dwTransferID));
    for (DWORD dwIndex=m_dwDequeuedTDIndex;dwIndex < m_dwNumOfTD && dwIndex< m_dwSchedTDIndex; dwIndex++) {
        if ( *(m_pCITDList+dwIndex) != NULL) {
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
            BOOL bResult=m_pCEhcd->PeriodDeQueueTD((*(m_pCITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
#else
            m_pCEhcd->PeriodDeQueueTD((*(m_pCITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
#endif
            ASSERT(bResult == TRUE);
            (*(m_pCITDList + dwIndex) )->SetLinkValid(FALSE);
            
            DWORD dwFrameIndex = dwIndex * GetMaxTransferPerItd();            
            for (DWORD dwTrans=0; dwTrans<GetMaxTransferPerItd() && dwFrameIndex< m_sTransfer.dwFrames; dwTrans++) {
                if (dwFrameIndex< m_sTransfer.dwFrames) {
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
                    __try { // setting isoch OUT status parameters
                        m_sTransfer.adwIsochErrors[ dwFrameIndex ] = USB_NOT_COMPLETE_ERROR;;
                        m_sTransfer.adwIsochLengths[ dwFrameIndex ] = 0;
                    } __except( EXCEPTION_EXECUTE_HANDLER ) {
                    }
#pragma prefast(pop)
                }
                dwFrameIndex ++;
            }
            if (m_dwFirstError == USB_NO_ERROR   ) { // only update first time
                m_dwFirstError = USB_NOT_COMPLETE_ERROR;
            }
        }
    }
    m_dwDequeuedTDIndex = m_dwSchedTDIndex = m_dwNumOfTD;
    Sleep(2); // Make Sure EHCI nolong reference to those TD;
    return DoneTransfer(m_dwFrameIndexStart+m_dwNumOfTD, 0);
};

//**************************************************************************************
BOOL CITransfer::DoneTransfer(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex)
//
// Purpose: This is to check if high speed isochronous transfer is done through IsTransferDone, copying
//          the data if IN transfer and then performing the acknowledgement back into MDD
//
// Parameters: 
//
// Returns: TRUE - success, FALSE - failure
//
// Notes: 
//***************************************************************************************
{
    BOOL bIsTransDone = IsTransferDone(dwCurFrameIndex,dwCurMicroFrameIndex);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::DoneTransfer (this=0x%x,id=0x%x,curFrameIndex=0x%x, bIsTransDone=%d \r\n")
        ,this, m_dwTransferID, dwCurFrameIndex,bIsTransDone));
    ASSERT(bIsTransDone == TRUE);
    if (bIsTransDone && !m_fDoneTransferCalled ) {
        DWORD dwUsbError = USB_NO_ERROR;
        m_fDoneTransferCalled = TRUE;
        // We have to update the buffer when this is IN Transfer.
        if ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=NULL && m_pAllocatedForClient!=NULL) {
            __try { // copying client buffer for OUT transfer
                memcpy( m_sTransfer.lpvBuffer, m_pAllocatedForClient, m_sTransfer.dwBufferSize );
                //DEBUGMSG(1, (TEXT("IN Transfer of size 0x%x\r\n"), m_sTransfer.dwBufferSize));
                //memdump((unsigned char *)m_sTransfer.lpvBuffer, (USHORT)m_sTransfer.dwBufferSize, 0);
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvBuffer.
                if (dwUsbError == USB_NO_ERROR)
                    dwUsbError = USB_CLIENT_BUFFER_ERROR;
            }
        }
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
        __try { // setting transfer status and executing callback function
            if (m_sTransfer.lpfComplete !=NULL)
                *m_sTransfer.lpfComplete = TRUE;
            if (m_sTransfer.lpdwError!=NULL)
                *m_sTransfer.lpdwError = dwUsbError;
            if (m_sTransfer.lpdwBytesTransferred)
                *m_sTransfer.lpdwBytesTransferred =  m_DataTransferred;
            if ( m_sTransfer.lpStartAddress ) {
                ( *m_sTransfer.lpStartAddress )(m_sTransfer.lpvNotifyParameter );
                m_sTransfer.lpStartAddress = NULL ; // Make sure only do once.
            }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
        }
#pragma prefast(pop)
        return (dwUsbError==USB_NO_ERROR);
    }
    else
        return TRUE;

}

//************************************************************************************************
CSITransfer::CSITransfer (IN  CIsochronousPipe * const pCPipe,IN CEhcd * const pCEhcd,STransfer *pSTransfer)
    :CIsochTransfer(pCPipe, pCEhcd,pSTransfer)
//
// Purpose: Constructor for CSITransfer (Class for Full/Low Speed isochronous transfer)
//
// Parameters: 
//
// Returns: Nothing
//
// Notes: 
//**************************************************************************************************
{
    m_pCSITDList =0;
}

//***********************************************************************************************
CSITransfer::~CSITransfer()
//
// Purpose: De-constructor for CSITransfer (Class for Full/Low Speed isochronous transfer)
//
// Parameters: 
//
// Returns: Nothing
//
// Notes: 
//**************************************************************************************************
{
    ASSERT(m_dwSchedTDIndex==m_dwNumOfTD);
    ASSERT(m_dwDequeuedTDIndex ==  m_dwNumOfTD);
    if (m_pCSITDList && m_dwNumOfTD) {
        AbortTransfer();
        for (DWORD dwIndex=0;dwIndex<m_dwNumOfTD;dwIndex++)
            if (*(m_pCSITDList + dwIndex)) {
                ASSERT((*(m_pCSITDList + dwIndex) )->GetLinkValid() != TRUE); // Invalid Next Link
                GetPipe()->FreeCSITD(*(m_pCSITDList + dwIndex));
                *(m_pCSITDList + dwIndex) = NULL;
            }
        delete m_pCSITDList;
    }
}

//****************************************************************************************
BOOL CSITransfer::ArmTD()
//
// Purpose: Arm/schedule the transfer descriptor - this is to setup the transfer descriptor
//          and perform the IssueTransfer on the iTD to do the actual transfer. Called by
//          AddTransfer()
//
// Parameters: 
//
// Returns: TRUE - something has to be scheduled/armed
//          FALSE - nothing
//
// Notes:
// ******************************************************************
{
    BOOL bAnyArmed = FALSE;
    if (m_pCSITDList && m_dwArmedTDIndex<m_dwNumOfTD ) { // Something TD wait for Arm.
        DWORD dwCurDataPhysAddr =   m_sTransfer.paBuffer + m_dwArmedBufferIndex ;
        CSITD * pPrevCSITD= (m_dwArmedTDIndex==0?NULL:*(m_pCSITDList + m_dwArmedTDIndex-1));
        while( m_dwArmedTDIndex < m_dwNumOfTD) {
            DWORD dwLength=  *(m_sTransfer.aLengths + m_dwArmedTDIndex);
            *(m_pCSITDList + m_dwArmedTDIndex) = GetPipe()->AllocateCSITD( this,pPrevCSITD);
            if (*(m_pCSITDList + m_dwArmedTDIndex) == NULL) {
                break;
            }
            else {
                pPrevCSITD = *(m_pCSITDList + m_dwArmedTDIndex);
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
                BOOL bReturn=(*(m_pCSITDList + m_dwArmedTDIndex))->IssueTransfer(dwCurDataPhysAddr,dwCurDataPhysAddr+ dwLength -1, dwLength,
                    TRUE,// Interrupt On Completion
                    (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0);
#else
                (*(m_pCSITDList + m_dwArmedTDIndex))->IssueTransfer(dwCurDataPhysAddr,dwCurDataPhysAddr+ dwLength -1, dwLength,
                    TRUE,// Interrupt On Completion
                    (m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0);
#endif
                ASSERT(bReturn == TRUE);
                // Interrupt on any CITD completion
                (*(m_pCSITDList+ m_dwArmedTDIndex))->SetIOC(TRUE);
                dwCurDataPhysAddr += dwLength;
                m_dwArmedBufferIndex +=dwLength;
                m_dwArmedTDIndex ++ ;
                bAnyArmed = TRUE;
            }
        }
        
    }
    return bAnyArmed;
}

//****************************************************************************************
BOOL CSITransfer::AddTransfer() 
//
// Purpose: Setup all the queue transfer descriptor list and called up ArmTD to do the transfer
//
// Parameters: None
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CSITransfer::AddTransfer (this=0x%x,id=0x%xm_dwFrameIndexStart=%x)\r\n"),this,m_dwTransferID,m_dwFrameIndexStart));
    if (m_dwNumOfTD!=0 || m_pCSITDList != NULL) {
        ASSERT(FALSE);
        return FALSE;
    }
    m_dwNumOfTD = m_sTransfer.dwFrames ;
    m_dwSchedTDIndex=0;
    m_dwArmedTDIndex =0;
    m_dwArmedBufferIndex=0;
    m_dwFirstError =  USB_NO_ERROR;
    if (m_sTransfer.lpvBuffer &&  m_sTransfer.paBuffer && m_sTransfer.dwBufferSize) {
        // Allocate space for CITD List
        m_pCSITDList = (CSITD **)new PVOID[m_dwNumOfTD];
        if (m_pCSITDList!=NULL) {
            memset(m_pCSITDList,0,sizeof(CSITD *)*m_dwNumOfTD);
            ArmTD();
            return TRUE;
        }
    }
    ASSERT(FALSE);
    return FALSE;
}

//******************************************************************************************************
BOOL CSITransfer::ScheduleTD(DWORD dwCurFrameIndex,DWORD /*dwCurMicroFrameIndex*/)
//
// Purpose: Schedule a USB Transfer on this pipe, this is used by AbortTransfer only.
//
// Parameters: dwCurrFrameIndex - current frame index
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes: 
//******************************************************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CSITransfer::ScheduleTD (this=0x%x,id=0x%x,curFrameIndex=0x%x,m_dwFrameIndexStart=0x%x)\r\n"),this,m_dwTransferID, dwCurFrameIndex,m_dwFrameIndexStart));
    BOOL bReturn = FALSE;
    ArmTD();
    if (m_dwSchedTDIndex < m_dwNumOfTD && m_pCSITDList !=0 && m_pCEhcd) {
        if ((long)(dwCurFrameIndex - m_dwFrameIndexStart) > (long)m_dwSchedTDIndex) {
            m_dwSchedTDIndex = dwCurFrameIndex - m_dwFrameIndexStart;
        }
        m_dwSchedTDIndex = min( m_dwSchedTDIndex , m_dwNumOfTD);
        
        DWORD EndShedTDIndex = dwCurFrameIndex +  (m_pCEhcd->GetPeriodicMgr()).GetFrameSize()-1;
        DWORD dwNumTDCanSched;
        if ( (long)(EndShedTDIndex - m_dwFrameIndexStart ) >= 0)
            dwNumTDCanSched =EndShedTDIndex - m_dwFrameIndexStart ;
        else  // Too Early.
            dwNumTDCanSched = 0;
        dwNumTDCanSched= min(m_dwNumOfTD ,dwNumTDCanSched);        
        dwNumTDCanSched= min(m_dwArmedTDIndex ,dwNumTDCanSched);
        
        if (m_dwSchedTDIndex < dwNumTDCanSched) { // Do scudule those index.
            for (DWORD dwIndex = m_dwSchedTDIndex ; dwIndex<dwNumTDCanSched; dwIndex++) {
                m_pCEhcd->PeriodQueueSITD(*(m_pCSITDList+dwIndex),m_dwFrameIndexStart + dwIndex );
                (*(m_pCSITDList+dwIndex))->CheckStructure ();
                ASSERT((*(m_pCSITDList+dwIndex))->m_pTrans == this);
            }
            m_dwSchedTDIndex = dwNumTDCanSched;
            bReturn = TRUE;
        }
    }
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer::ScheduleTD (this=0x%x) return %d\r\n"),this,bReturn));
    return bReturn;
}
//***************************************************************************************
BOOL CSITransfer::IsTransferDone(DWORD dwCurFrameIndex,DWORD /*dwCurMicroFrameIndex*/)
//
// Purpose: This is to check if the transfer has been completed for Isochronous High Speed Transfer
//
// Parameters: dwCurFrameIndex - minimal frame index to which the transfer to check against if completed.
//
// Returns: TRUE - successfully tranferred, FALSE - failure
//
// Notes: 
//****************************************************************************************

{
    DWORD dwCount = 0;
    // Dequeue those TD has Transfered.
    m_dwLastFrameIndex = dwCurFrameIndex - m_dwFrameIndexStart;
    if ((long)(dwCurFrameIndex - m_dwFrameIndexStart)>= 1 ) {
        DWORD dwTransfered = min(dwCurFrameIndex - m_dwFrameIndexStart , m_dwSchedTDIndex);
        ASSERT(m_dwSchedTDIndex<=m_dwArmedTDIndex);
        for (DWORD dwIndex=m_dwDequeuedTDIndex;dwIndex<dwTransfered && dwIndex < m_dwNumOfTD ; dwIndex++) {
            dwCount = 0;
DO_RECHECK:
            if ( *(m_pCSITDList+dwIndex) != NULL && (*(m_pCSITDList + dwIndex) )->GetLinkValid()) {
                (*(m_pCSITDList+dwIndex))->CheckStructure ();
                ASSERT((*(m_pCSITDList+dwIndex))->m_pTrans == this);
                m_pCEhcd->PeriodDeQueueTD((*(m_pCSITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
                (*(m_pCSITDList + dwIndex) )->SetLinkValid(FALSE);
                
            }
            if (*(m_pCSITDList + dwIndex)!=NULL) {
                DWORD dwTDError = USB_NO_ERROR;
                (*(m_pCSITDList+dwIndex))->CheckStructure ();
                if ((*(m_pCSITDList +dwIndex))->sITD_TransferState.sITD_TSContext.Active!=0) 
                {
                    dwTDError = USB_NOT_COMPLETE_ERROR;
                    // Do a 50 msec check
                    if (dwCount++ < 25)
                    {
                        Sleep(2);
                        goto DO_RECHECK;                    
                    }
                }
                else
                if ((*(m_pCSITDList + dwIndex))->sITD_TransferState.sITD_TSContext.XactErr!=0) 
                    dwTDError = USB_ISOCH_ERROR;
                else
                if ((*(m_pCSITDList + dwIndex))->sITD_TransferState.sITD_TSContext.BabbleDetected!=0) 
                    dwTDError = USB_STALL_ERROR;
                else
                if ((*(m_pCSITDList + dwIndex))->sITD_TransferState.sITD_TSContext.DataBufferError!=0) 
                    dwTDError = ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=0?USB_DATA_OVERRUN_ERROR:USB_DATA_UNDERRUN_ERROR);
                else 
                if (((*(m_pCSITDList + dwIndex))->sITD_TransferState.dwSITD_TransferState & 0xff)!=0 )
                    dwTDError = USB_BIT_STUFFING_ERROR;
                
                if (m_dwFirstError == USB_NO_ERROR   ) { // only update first time
                    m_dwFirstError = dwTDError;
                }
                if (dwTDError!= USB_NO_ERROR) {
                    DEBUGMSG( ZONE_TRANSFER , (TEXT("CITransfer::DoneTransfer (this=0x%x, dwFrameIndex=%d) Error(%d) \r\n"),
                        this,dwIndex, dwTDError));
                }
                if (dwIndex< m_sTransfer.dwFrames) {
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
                    __try { // setting isoch OUT status parameters
                        m_sTransfer.adwIsochErrors[ dwIndex] = dwTDError;
                        m_sTransfer.adwIsochLengths[ dwIndex ] = *(m_sTransfer.aLengths + dwIndex) - (*(m_pCSITDList + dwIndex))->sITD_TransferState.sITD_TSContext.BytesToTransfer;
                        m_DataTransferred += m_sTransfer.adwIsochLengths[ dwIndex ];
                    } __except( EXCEPTION_EXECUTE_HANDLER ) {
                    }
#pragma prefast(pop)
                }

                GetPipe()->FreeCSITD(*(m_pCSITDList + dwIndex));
                *(m_pCSITDList + dwIndex) = NULL;
            }
        }
        m_dwDequeuedTDIndex = dwIndex;
    }
    BOOL bReturn= ((long)(dwCurFrameIndex-m_dwFrameIndexStart)>=(long)m_dwNumOfTD);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("CITransfer:::IsTransferDone (this=0x%x,curFrameIndex=0x%x) return %d \r\n"),this, dwCurFrameIndex,bReturn));
    return bReturn;
}

//*******************************************************************
BOOL CSITransfer::AbortTransfer()
//
// Purpose: Abort the current transfer
//
// Parameters: None
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
// ******************************************************************
{
    for (DWORD dwIndex=m_dwDequeuedTDIndex;dwIndex < m_dwNumOfTD && dwIndex<m_dwSchedTDIndex ; dwIndex++) {
        if ( *(m_pCSITDList+dwIndex) != NULL ) {
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
            BOOL bResult =m_pCEhcd->PeriodDeQueueTD((*(m_pCSITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
#else
            m_pCEhcd->PeriodDeQueueTD((*(m_pCSITDList+dwIndex))->GetPhysAddr(),dwIndex + m_dwFrameIndexStart);
#endif
            ASSERT(bResult == TRUE);
            (*(m_pCSITDList + dwIndex) )->SetLinkValid(FALSE);
            
            if (dwIndex< m_sTransfer.dwFrames) {
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
                __try { // setting isoch OUT status parameters
                    m_sTransfer.adwIsochErrors[ dwIndex] = USB_NOT_COMPLETE_ERROR;
                    m_sTransfer.adwIsochLengths[ dwIndex ] = 0;
                } __except( EXCEPTION_EXECUTE_HANDLER ) {
                }
#pragma prefast(pop)
            }
            if (m_dwFirstError == USB_NO_ERROR   ) { // only update first time
                m_dwFirstError = USB_NOT_COMPLETE_ERROR;
            }
            GetPipe()->FreeCSITD(*(m_pCSITDList + dwIndex));
            *(m_pCSITDList + dwIndex) = NULL;
        }
    }
    m_dwArmedTDIndex = m_dwDequeuedTDIndex = m_dwSchedTDIndex = m_dwNumOfTD;
    Sleep(2); // Make Sure EHCI nolong reference to those TD;
    return DoneTransfer(m_dwFrameIndexStart+m_dwNumOfTD, 0);
};

//******************************************************************************
BOOL CSITransfer::DoneTransfer(DWORD dwCurFrameIndex,DWORD dwCurMicroFrameIndex)
//
// Purpose: Check if transfer is done and success, acknowledge back into MDD
//
// Parameters: 
//
// Returns: TRUE - success, FALSE - failure
//
// Notes:
// ******************************************************************
{
    BOOL bIsTransDone = IsTransferDone(dwCurFrameIndex, dwCurMicroFrameIndex);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, 
            (TEXT("CITransfer::DoneTransfer (this=0x%x,id=0x%x,curFrameIndex=0x%x, bIsTransDone=%d \r\n")
        ,this,m_dwTransferID, dwCurFrameIndex,bIsTransDone));
    ASSERT(bIsTransDone == TRUE);
    if (bIsTransDone && !m_fDoneTransferCalled) {
        m_fDoneTransferCalled = TRUE;
        // We have to update the buffer when this is IN Transfer.
        if ((m_sTransfer.dwFlags & USB_IN_TRANSFER)!=NULL && m_pAllocatedForClient!=NULL) {
            __try { // copying client buffer for OUT transfer
                memcpy( m_sTransfer.lpvBuffer, m_pAllocatedForClient, m_sTransfer.dwBufferSize );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  // bad lpvBuffer.
                if (m_dwFirstError == USB_NO_ERROR)
                    m_dwFirstError = USB_CLIENT_BUFFER_ERROR;
            }
        }
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
        __try { // setting transfer status and executing callback function
            if (m_sTransfer.lpfComplete !=NULL)
                *m_sTransfer.lpfComplete = TRUE;
            if (m_sTransfer.lpdwError!=NULL)
                *m_sTransfer.lpdwError = m_dwFirstError;
            if (m_sTransfer.lpdwBytesTransferred)
                *m_sTransfer.lpdwBytesTransferred =  m_DataTransferred;            
            if ( m_sTransfer.lpStartAddress ) {
                ( *m_sTransfer.lpStartAddress )(m_sTransfer.lpvNotifyParameter );
                m_sTransfer.lpStartAddress = NULL ; // Make sure only do once.
            }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
        }
#pragma prefast(pop)
        return (m_dwFirstError==USB_NO_ERROR);
    }
    else
        return TRUE;

}



