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
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-20010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  usbfncdcdrv.cpp

Abstract:

    Serial PDD Common Code.

Notes: 
--*/
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <notify.h>
#include <serhw.h>
#include <devload.h>
#include <windev.h>
#include <serdbg.h>


#include "usbfncdcdrv.h"


CUsbFuncTransfer::CUsbFuncTransfer(CUsbFuncPipe *hPipe, DWORD dwFlags,DWORD cbBuffer, PVOID pvBuffer, DWORD dwBufferPhysicalAddress)
:   m_pPipe(hPipe)
,   m_dwFlags(dwFlags)
,   m_dwBufferSize (cbBuffer)
,   m_pvBuffer (pvBuffer)
,   m_dwPhysAddr (dwBufferPhysicalAddress)
{
    ASSERT(hPipe!=NULL);
    ASSERT(cbBuffer!=NULL);
    ASSERT(pvBuffer!=NULL);
    m_hCompleteEvent=CreateEvent(NULL, TRUE, FALSE, NULL); // Manual Reset and Initial FALSE
    m_ufnTransfer = NULL;
    

}
CUsbFuncTransfer::~CUsbFuncTransfer()
{
    if (m_pPipe) {
        if ( m_ufnTransfer) {
            if (!IsTransferComplete()) { // We need Cancel it.
                AbortTransfer();
            }
            CloseTransfer();
        }
    }
    if (m_hCompleteEvent) {
        CloseHandle(m_hCompleteEvent);
    }
}
BOOL CUsbFuncTransfer::Init()
{
    return (m_pPipe && m_pPipe->GetFunctionPtr() && m_hCompleteEvent);
}
BOOL CUsbFuncTransfer::IssueTransfer(DWORD dwLength)
{
    PREFAST_ASSERT(m_pPipe);
    PREFAST_ASSERT(m_pPipe->GetFunctionPtr());
    if (dwLength>GetBufferSize())
        dwLength = GetBufferSize();
    
    if (IsTransferClosed()) {
        ResetEvent(m_hCompleteEvent);
        DWORD dwRet =m_pPipe->GetFunctionPtr()->lpIssueTransfer(
                m_pPipe->GetDeviceHandle(),m_pPipe->GetPipeHandle(),CompleteNotificationStub,this,m_dwFlags,
                dwLength,m_pvBuffer,m_dwPhysAddr, NULL, &m_ufnTransfer);
        RETAILMSG (0,(TEXT("CUsbFuncTransfer::IssueTransfer(Handle=0x%x,flags =0x%x size=0x%x)\r\n"),
            m_ufnTransfer,m_dwFlags, dwLength));
//        ASSERT(dwRet == ERROR_SUCCESS);
        return(dwRet == ERROR_SUCCESS);     
    }
    else
        return FALSE;
}

BOOL CUsbFuncTransfer::WaitForTransferComplete(DWORD dwTicks)
{
    PREFAST_ASSERT(m_hCompleteEvent!=NULL);
    if (!IsTransferClosed()) {
       return ( WaitForSingleObject(m_hCompleteEvent , dwTicks) == WAIT_OBJECT_0);
    }
    else
        return TRUE;
}
BOOL  CUsbFuncTransfer::GetTransferStatus(PDWORD pdwBytesTranfered, PDWORD pdwError)
{
    PREFAST_ASSERT(m_pPipe);
    PREFAST_ASSERT(m_pPipe->GetFunctionPtr());
    if (!IsTransferClosed()) {
        DWORD dwRet = m_pPipe->GetFunctionPtr()->lpGetTransferStatus(m_pPipe->GetDeviceHandle(),m_ufnTransfer,pdwBytesTranfered,pdwError);
        ASSERT(dwRet == ERROR_SUCCESS);
        return(dwRet == ERROR_SUCCESS);     
    }
    else {
        if (pdwError)
            *pdwError = UFN_NOT_COMPLETE_ERROR;
        if (pdwBytesTranfered)
            *pdwBytesTranfered = 0;
        return FALSE;
    }
}
BOOL CUsbFuncTransfer::CloseTransfer()
{
    PREFAST_ASSERT(m_pPipe);
    PREFAST_ASSERT(m_pPipe->GetFunctionPtr());
    if (!IsTransferClosed()) {
        DWORD dwRet = m_pPipe->GetFunctionPtr()->lpCloseTransfer(m_pPipe->GetDeviceHandle(),m_ufnTransfer);
        ASSERT(dwRet == ERROR_SUCCESS);
        m_ufnTransfer = NULL;
        return(dwRet == ERROR_SUCCESS);     
    }
    else
        return TRUE;
    
}
BOOL CUsbFuncTransfer::AbortTransfer()
{
    PREFAST_ASSERT(m_pPipe);
    PREFAST_ASSERT(m_pPipe->GetFunctionPtr());
    if (!IsTransferComplete()) {
        DWORD dwRet = m_pPipe->GetFunctionPtr()->lpAbortTransfer(m_pPipe->GetDeviceHandle(),m_ufnTransfer);
        DEBUGMSG (ZONE_EVENTS,(TEXT("CUsbFuncTransfer::AbortTransfer(Handle=0x%x,flags =0x%x )\r\n"),
            m_ufnTransfer,m_dwFlags));
        ASSERT(dwRet == ERROR_SUCCESS);
        return(dwRet == ERROR_SUCCESS);             
    }
    else
        return TRUE;
}
DWORD WINAPI CUsbFuncTransfer::CompleteNotificationStub(PVOID pvNotifyParameter)
{
    DEBUGMSG (ZONE_EVENTS,(TEXT("CUsbFuncTransfer::CompleteNotificationStub(pv=0x%x)\r\n"),pvNotifyParameter));
    ASSERT(pvNotifyParameter);
    if (pvNotifyParameter) {
        return ((CUsbFuncTransfer *)pvNotifyParameter) -> CompleteNotification();
    }
    else {
        ASSERT(FALSE);
        return ERROR_INVALID_DATA;
    }
}

DWORD  WINAPI  CUsbFuncTransfer::CompleteNotification()
{
    PREFAST_ASSERT(m_hCompleteEvent!=NULL);
    PREFAST_ASSERT(m_pPipe);
    if (!IsTransferClosed()) {
        SetEvent(m_hCompleteEvent);
        m_pPipe->TransferComplete(this);
        return ERROR_SUCCESS;
    }
    else {
        ASSERT(FALSE);
        return ERROR_INVALID_DATA;
    }
}

//------------------------------------------------Pipe ----------------------------------------------
CUsbFuncPipe::CUsbFuncPipe(USBSerialFn * pSerialFn, UFN_HANDLE hDevice,PCUFN_FUNCTIONS pUfnFuncs,UCHAR bEndpointAddr,BOOL fRead,DWORD dwMaxPacketSize, DWORD dwMaxTransferSize, DWORD dwMaxNumOfTransfer)
:   CMiniThread (0, TRUE) 
,   m_pSerialFn (pSerialFn)
,   m_hDevice ( hDevice)
,   m_pUfnFuncs(pUfnFuncs)
,   m_bEndpointAddr(bEndpointAddr)
,   m_fRead ( fRead )
,   m_dwMaxPacketSize ( dwMaxPacketSize )
,   m_dwTranferSize(dwMaxTransferSize)
,   m_dwNumOfTransfer(min(MAX_TRANSFER,dwMaxNumOfTransfer))
{
    // Calculate buffer needed.
    m_dwBufferSize = m_dwNumOfTransfer*m_dwTranferSize;
    m_pbBuffer = new BYTE[m_dwBufferSize ];
    m_dwBufferPhysAddr = 0; 
    for (DWORD dwIndex = 0; dwIndex<  MAX_TRANSFER; dwIndex ++)
        m_pTransferArray[dwIndex] = NULL;
    m_dwWriteIndex = m_dwCompleteIndex = 0;
    m_dwCurPosition = 0;
    m_fZeroLengthNeeded = FALSE;
    m_TerminateEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // Manual Reset and Initial FALSE
    m_hPipe = NULL;
}
BOOL CUsbFuncPipe::Init()
{
    if (m_TerminateEvent && m_pbBuffer ) {
        Lock();
        // We assume the read should has higher priority than write.
        if (m_fRead)
            CeSetPriority(m_pSerialFn->GetPriority256()-1);
        else 
            CeSetPriority(m_pSerialFn->GetPriority256()+1);
        ThreadStart();
        Unlock();
        return TRUE;
    }
    return FALSE;
}
CUsbFuncPipe::~CUsbFuncPipe()
{
    Lock();
    m_bTerminated = TRUE;
    ThreadStart();
    if (m_TerminateEvent) {
        SetEvent(m_TerminateEvent);
    }
    Unlock();
    BOOL fRet=WaitThreadComplete(5000);
    ASSERT(fRet == TRUE);
    Lock();
    ClosePipe();
    if (m_pbBuffer!=NULL) {
        delete m_pbBuffer;
    }
    if ( m_TerminateEvent)
        CloseHandle(m_TerminateEvent);
    Unlock();
}
BOOL CUsbFuncPipe::OpenPipe()
{
    Lock();
    RETAILMSG(0,(TEXT("OpenPipe address 0x%x numberoftransfer 0x%x m_fRead %d\r\n"),m_bEndpointAddr,m_dwNumOfTransfer,m_fRead));
    if (m_hPipe==NULL) {
        DWORD dwRet = m_pUfnFuncs->lpOpenPipe(m_hDevice,  m_bEndpointAddr, &m_hPipe);
        if (dwRet != ERROR_SUCCESS) {
            m_hPipe = NULL;
            ASSERT(FALSE);
        }
        if (m_hPipe != NULL) {
            m_dwWriteIndex = m_dwCompleteIndex = 0;
            BOOL fReturn = TRUE;
            for (DWORD dwIndex = 0; dwIndex<m_dwNumOfTransfer; dwIndex ++) {
                m_pTransferArray[dwIndex] = new CUsbFuncTransfer(this,
                        m_fRead? USB_OUT_TRANSFER: USB_IN_TRANSFER ,
                        m_dwTranferSize,
                        m_pbBuffer + (dwIndex*m_dwTranferSize),
                        m_dwBufferPhysAddr!=0?(m_dwBufferPhysAddr+(dwIndex*m_dwTranferSize)): 0 ) ;
                if (m_pTransferArray[dwIndex]!= NULL && m_pTransferArray[dwIndex]->Init()== TRUE ) { // Successful
                    
                    if (m_fRead) { // Lauch Issue Transfer first.
                        RETAILMSG(0,(TEXT("ready to issue transfer\r\n")));
                        if (!m_pTransferArray[dwIndex]->IssueTransfer()) {
                            ASSERT(FALSE);
                            fReturn = FALSE;
                            break;
                        }
                    }                
                }
                else {
                    ASSERT(FALSE);
                    fReturn = FALSE;
                    break;
                }
            }
            if (!fReturn) { // Fail on Open Pipe or on Transfer.
                ClosePipe();
            }
        }
    }
    if (m_hPipe)
        SetEvent(m_TerminateEvent);
    Unlock();
    return (m_hPipe!=NULL);
}
void CUsbFuncPipe::ClosePipe()
{
    Lock();
    if (m_hPipe!=NULL) {
        // Delete Transfer In order is required.
        DWORD dwIndex = m_dwCompleteIndex;
        do {
            if (m_pTransferArray[dwIndex]!=NULL) {
                delete m_pTransferArray[dwIndex];
                m_pTransferArray[dwIndex] = NULL;
            }
            dwIndex= (dwIndex!=0?dwIndex-1:m_dwNumOfTransfer-1);
        }
        while (dwIndex !=m_dwCompleteIndex ) ;
        m_pUfnFuncs->lpClosePipe(m_hDevice, m_hPipe);
    }
    m_hPipe = NULL;
    Unlock();
}


DWORD CUsbFuncPipe::ReadData(PUCHAR pRxBuffer,ULONG *pBufflen)
{
    DWORD dwBytesDropped = 0;
    Lock();
    if (m_hPipe != NULL && m_fRead && pRxBuffer && pBufflen && *pBufflen) {
        DEBUGMSG (ZONE_READ,(TEXT("+CUsbFuncPipe::ReadData (0x%x,0x%x)\r\n"),pRxBuffer, pBufflen?*pBufflen:0));
        DWORD dwRoomLeft = *pBufflen;
        DWORD dwBytesStored = 0;
        PREFAST_ASSERT(m_pTransferArray[m_dwCompleteIndex] != NULL);
        DWORD dwBytes=0;
        DWORD dwError=UFN_NO_ERROR;
        if ( !m_pTransferArray[m_dwCompleteIndex]->IsTransferClosed() &&
                m_pTransferArray[m_dwCompleteIndex]->IsTransferComplete() &&
                m_pTransferArray[m_dwCompleteIndex]->GetTransferStatus(&dwBytes, &dwError) &&
                dwError == UFN_NO_ERROR ) {
            PBYTE pBuffer = (PBYTE)m_pTransferArray[m_dwCompleteIndex]->GetBufferPtr();
            while (dwRoomLeft && m_dwCurPosition < dwBytes ) { // Then we have some work to do.
                BYTE bData = pBuffer[m_dwCurPosition++];
                if (m_pSerialFn == NULL || (m_pSerialFn->DataReplaced(&bData,FALSE))) {
                     *pRxBuffer++ = bData;
                     dwRoomLeft --;
                     dwBytesStored ++;
                }
            }
            if (m_dwCurPosition >= dwBytes) { // This transfer has been completely unloaded.
                m_pTransferArray[m_dwCompleteIndex]->CloseTransfer();
                m_pTransferArray[m_dwCompleteIndex]->IssueTransfer();
                // Advance to next one.
                m_dwCompleteIndex = IncIndex(m_dwCompleteIndex);
                m_dwCurPosition = 0;
            }
        }
        else {
//            ASSERT(FALSE);
            if (!m_pTransferArray[m_dwCompleteIndex]->IsTransferClosed()) {
                m_pTransferArray[m_dwCompleteIndex]->AbortTransfer();
                m_pTransferArray[m_dwCompleteIndex]->CloseTransfer();
                m_pTransferArray[m_dwCompleteIndex]->IssueTransfer();
            }
            dwBytesDropped = m_pTransferArray[m_dwCompleteIndex]->GetBufferSize();
            m_dwCompleteIndex = IncIndex(m_dwCompleteIndex);
            m_dwCurPosition = 0;
        }
        *pBufflen = dwBytesStored;
        DEBUGMSG (ZONE_READ,(TEXT("-CUsbFuncPipe::ReadData (0x%x,0x%x) return drop=%x\r\n"),pRxBuffer, pBufflen?*pBufflen:0,dwBytesDropped));
    }
    else
        ASSERT(FALSE);
    Unlock();
    return dwBytesDropped ;
}
// 
//WriteDataOnce:
//   It generates one Transfer if transfer that indexed by m_dwWriteIndex is not used.
//
void CUsbFuncPipe::WriteDataOnce(PUCHAR pTxBuffer, ULONG *pBuffLen) 
{
    Lock();
    DEBUGMSG (ZONE_WRITE,(TEXT("+CUsbFuncPipe::WriteDataOnce (0x%x,0x%x) , m_fZeroLengthNeeded =%d \r\n"),
        pTxBuffer,pBuffLen?*pBuffLen:0,m_fZeroLengthNeeded));
    PREFAST_ASSERT(pBuffLen!=NULL);
    if (m_pTransferArray[m_dwCompleteIndex]->IsTransferComplete()) {
        m_pTransferArray[m_dwCompleteIndex]->CloseTransfer();
        m_dwCompleteIndex = IncIndex(m_dwCompleteIndex);
    }
    // Next Xmit Transfer
    if (m_pTransferArray[m_dwWriteIndex]->IsTransferClosed()) {
        if (pTxBuffer && pBuffLen && *pBuffLen) {
            DWORD dwInputLength = min(*pBuffLen,m_pTransferArray[m_dwWriteIndex]->GetBufferSize());
            memcpy(m_pTransferArray[m_dwWriteIndex]->GetBufferPtr(),pTxBuffer, dwInputLength);
            m_pTransferArray[m_dwWriteIndex]->IssueTransfer(dwInputLength);
            m_fZeroLengthNeeded = ((dwInputLength  & (m_dwMaxPacketSize-1)) == 0 );
            DEBUGMSG (ZONE_WRITE,(TEXT("CUsbFuncPipe::WriteDataOnce: MaxPacketSize=%d,dwInputLength=%d , m_fZeroLengthNeeded =%d \r\n"),
                m_dwMaxPacketSize,dwInputLength, m_fZeroLengthNeeded));
            m_dwWriteIndex = IncIndex(m_dwWriteIndex);
            SetEvent(m_TerminateEvent);
            *pBuffLen = dwInputLength;
        }
        else {
            if (m_fZeroLengthNeeded) { // Send 0 Length Packet.
                m_pTransferArray[m_dwWriteIndex]->IssueTransfer(0);
                m_dwWriteIndex = IncIndex(m_dwWriteIndex);
                SetEvent(m_TerminateEvent);
            }
            m_fZeroLengthNeeded = FALSE;
            *pBuffLen = 0;
        }
    }
    else
        *pBuffLen = 0;
    DEBUGMSG (ZONE_WRITE,(TEXT("-CUsbFuncPipe::WriteDataOnce (0x%x,0x%x) , m_fZeroLengthNeeded =%d \r\n"),
        pTxBuffer,pBuffLen?*pBuffLen:0,m_fZeroLengthNeeded));
    Unlock();
}
//
// WriteData:
//   It generate multiple Transfer when free transfer and data available.
//
void CUsbFuncPipe::WriteData(PUCHAR pTxBuffer, ULONG *pBuffLen) 
{
    Lock();
    RETAILMSG (1,(TEXT("+CUsbFuncPipe::WriteData (0x%x,0x%x)\r\n"), pTxBuffer,pBuffLen?*pBuffLen:0));
    DWORD dwLeft =(pBuffLen?*pBuffLen:0);
    if (!m_fRead) {
        DWORD dwBytes ;
        do {
            dwBytes =  dwLeft;
            WriteDataOnce(pTxBuffer,&dwBytes);
            pTxBuffer += dwBytes;
            if (dwLeft >= dwBytes)
                dwLeft -= dwBytes;
            else 
                dwLeft = 0;
        } while (dwBytes!=0);
    }
    else
        ASSERT(FALSE);
    if (pBuffLen)
        *pBuffLen -= dwLeft;
    Unlock();
    DEBUGMSG (ZONE_WRITE,(TEXT("-CUsbFuncPipe::WriteData (0x%x,0x%x)\r\n"), pTxBuffer,pBuffLen?*pBuffLen:0));

}
BOOL  CUsbFuncPipe::CancelTransfer()
{
    Lock();
    if (m_fRead) {
        while (m_pTransferArray[m_dwCompleteIndex] && !m_pTransferArray[m_dwCompleteIndex]->IsTransferClosed() &&
                m_pTransferArray[m_dwCompleteIndex]->IsTransferComplete()) {
            m_pTransferArray[m_dwCompleteIndex]->CloseTransfer();
            m_pTransferArray[m_dwCompleteIndex]->IssueTransfer();
            // Advance to next one.
            m_dwCompleteIndex = IncIndex(m_dwCompleteIndex);
            m_dwCurPosition = 0;
        }
    }
    else {
        while (m_pTransferArray[m_dwCompleteIndex] && !m_pTransferArray[m_dwCompleteIndex]->IsTransferClosed()) {
            if (!m_pTransferArray[m_dwCompleteIndex]->IsTransferComplete())
                m_pTransferArray[m_dwCompleteIndex]->AbortTransfer();
            m_pTransferArray[m_dwCompleteIndex]->CloseTransfer();
            m_dwCompleteIndex = IncIndex(m_dwCompleteIndex);
        }
    }
    SetEvent(m_TerminateEvent);
    Unlock();
    return TRUE;
}

BOOL CUsbFuncPipe::IsAnySpaceAvailable()
{
    BOOL fReturn = FALSE;
    if (!m_fRead) {
        Lock();
        fReturn =(m_pTransferArray[m_dwWriteIndex]!=NULL 
                    && m_pTransferArray[m_dwWriteIndex]->IsTransferComplete());
        Unlock();
    }
    return fReturn;
}

DWORD CUsbFuncPipe::ThreadRun()
{
    while (m_TerminateEvent && ! IsTerminated()) {
        Lock();
        if (m_pTransferArray[m_dwCompleteIndex] && !m_pTransferArray[m_dwCompleteIndex]->IsTransferClosed()) {
            HANDLE rgh[2];
            rgh[0] = m_pTransferArray[m_dwCompleteIndex]->GetCompleteEventHandle();
            rgh[1] = m_TerminateEvent;
            Unlock();
            WaitForMultipleObjects(dim(rgh),rgh,FALSE,INFINITE);
            Lock();
        }
        else {
            Unlock();
            WaitForSingleObject(m_TerminateEvent,INFINITE);
            Lock();
        }
        if (m_pTransferArray[m_dwCompleteIndex] && !IsTerminated()) {
            DWORD dwCurIndex = m_dwCompleteIndex;
            if (!m_pTransferArray[dwCurIndex]->IsTransferClosed() && 
                    m_pTransferArray[dwCurIndex]->IsTransferComplete()) {
                Unlock();
                if (m_pSerialFn)
                    m_pSerialFn->EndpointNotification(this);                    
                Lock();
            }            
        }
        Unlock();
    }
    return (1);
}
//--------------------------USB FN Serial Driver -------------------------------------------
USBSerialFn::USBSerialFn(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj )
:   CSerialPDD(lpActivePath, pMdd,  pHwObj  )
,   CUsbFn (lpActivePath)
{
    PREFAST_ASSERT(lpActivePath!=NULL);
    m_pBulkIn = NULL;
    m_pBulkOut = NULL;
    m_pInterruptIn = NULL;
    m_bOldHostModemStatus = m_curHostModemStatus = 0;
    m_bModemSetState[0]=m_bModemSetState[1]=0;
    
}
USBSerialFn::~USBSerialFn()
{
    if ( m_pBulkIn) {
//        ASSERT(FALSE);
        delete m_pBulkIn;
    }
    if ( m_pBulkOut ) {
//        ASSERT(FALSE);
        delete m_pBulkOut;
    }
    if (m_pInterruptIn ) {
//        ASSERT(FALSE);
        delete m_pInterruptIn;
            
    }
}
BOOL USBSerialFn::Init()
{
    BOOL fReturn = TRUE;
    fReturn = (CUsbFn::Init() && CSerialPDD::Init() && m_hDefaultPipe!=NULL);
    ASSERT(fReturn);
    return fReturn;
};
BOOL USBSerialFn::EndpointNotification(CUsbFuncPipe * pPipe)
{
    if (pPipe!=NULL) {
        DWORD interruptType =  INTR_NONE;
        m_HardwareLock.Lock();
        if ( pPipe == m_pBulkIn) { // Device to Host.
            interruptType |= INTR_TX;
        }
        else 
        if ( pPipe == m_pBulkOut){ // Host to Device.
            interruptType |= INTR_RX;
        }
        if (pPipe == m_pInterruptIn) {
            RETAILMSG(1,(TEXT("INTERRUPT EP\r\n")));
            if ( m_pInterruptIn->IsAnySpaceAvailable()) {
                if (m_bOldModemState !=m_bModemSetState[0]) {
                    DWORD dwLength = sizeof(m_bModemSetState);
                    m_bModemSetState [0] |= USBFN_SERIAL_DATA_AVAILABLE; // Always set data available.
                    m_pInterruptIn->WriteData(m_bModemSetState,&dwLength);
                    m_bOldModemState =  m_bModemSetState[0];
                }
                else
                    m_pInterruptIn->WriteData(NULL,NULL);
            }
            else
                ASSERT(FALSE);
            
        }
        m_HardwareLock.Unlock(); 
        return NotifyPDDInterrupt((INTERRUPT_TYPE)interruptType);
    }
    return FALSE;
}
/*BOOL USBSerialFn::InitXmit(BOOL bInit) 
{
    m_HardwareLock.Lock();
    BOOL bReturn = FALSE;
    if (m_pBulkIn) {
        if (bInit)
            bReturn = m_pBulkIn->OpenPipe();
        else {
            m_pBulkIn->ClosePipe();
            bReturn = TRUE;
        }
        ASSERT(bReturn);
    }
    m_HardwareLock.Unlock();
    return bReturn;
};
*/
void    USBSerialFn::XmitInterruptHandler(PUCHAR pTxBuffer, ULONG *pBuffLen)
{
    m_HardwareLock.Lock();
    if (m_pBulkIn) {
        if ((m_DCB.fOutxCtsFlow && IsCTSOff()) ||(m_DCB.fOutxDsrFlow && IsDSROff())) { // We are in flow off
            DEBUGMSG(ZONE_THREAD|ZONE_WRITE,(TEXT("USBSerialFn::XmitInterruptHandler! Flow Off, Data Discard.\r\n")));
            if (pBuffLen)
                *pBuffLen= 0;
        }
        else
            m_pBulkIn->WriteData(pTxBuffer,pBuffLen);
    }
    m_HardwareLock.Unlock();
};
void    USBSerialFn::XmitComChar(UCHAR ComChar) 
{
    DWORD dwLength = 1; // We can not do special.
    BOOL fWait = TRUE;
    DWORD dwCount = 100;
    while (fWait && dwCount) {
        m_HardwareLock.Lock();
        if (m_pBulkIn) {
            if (m_pBulkIn->IsAnySpaceAvailable()) {
                m_pBulkIn->WriteData(&ComChar,&dwLength);
                fWait = FALSE;
            }
        }
        else 
            fWait = FALSE;
        m_HardwareLock.Unlock();
        if (fWait)
            Sleep(10);
        dwCount -- ;
    }    
}
BOOL USBSerialFn::CancelXmit()
{
    m_HardwareLock.Lock();
     if (m_pBulkIn) {
        m_pBulkIn->CancelTransfer();
    }
    m_HardwareLock.Unlock();
    return TRUE;
}

/*BOOL    USBSerialFn::InitReceive(BOOL bInit)
{
    m_HardwareLock.Lock();
    BOOL bReturn = FALSE;
    if (m_pBulkOut) {
        if (bInit)
            bReturn = m_pBulkOut->OpenPipe();
        else {
            m_pBulkOut->ClosePipe();
            bReturn = TRUE;
        }
        ASSERT(bReturn);
    }
    m_HardwareLock.Unlock();
    return bReturn;
}
*/
ULONG   USBSerialFn::ReceiveInterruptHandler(PUCHAR pRxBuffer,ULONG *pBufflen)
{
    DWORD dwReturn = 0;
    m_HardwareLock.Lock();
    if (m_pBulkOut) {
        dwReturn = m_pBulkOut->ReadData(pRxBuffer,pBufflen);
    }
    m_HardwareLock.Unlock();
    return dwReturn;
}
ULONG   USBSerialFn::CancelReceive()
{
    m_HardwareLock.Lock();
     if (m_pBulkIn) {
        m_pBulkIn->CancelTransfer();
    }
    m_HardwareLock.Unlock();
    return TRUE;
}

/*BOOL    USBSerialFn::InitModem(BOOL bInit)
{
    m_HardwareLock.Lock();
    BOOL bReturn = FALSE;
    if (m_pInterruptIn) {
        if (bInit)
            bReturn = m_pInterruptIn->OpenPipe();
        else{
            m_pInterruptIn->ClosePipe();
            bReturn = TRUE;
        }
    }
    m_bOldModemState = m_bModemSetState[0]= m_bModemSetState[1] = USBFN_SERIAL_DATA_AVAILABLE;
    m_HardwareLock.Unlock();
    return bReturn;
}
*/
void USBSerialFn::SetModemSignal(BOOL bSet, BYTE bBitSet)
{
    m_HardwareLock.Lock();
    if (bSet)
        m_bModemSetState[0] |= bBitSet;
    else
        m_bModemSetState[0] &= ~bBitSet;
    
    if (m_pInterruptIn && m_bOldModemState !=m_bModemSetState[0] && m_pInterruptIn->IsAnySpaceAvailable()) {
        DWORD dwLength = sizeof(m_bModemSetState);
        m_bModemSetState [0] |= USBFN_SERIAL_DATA_AVAILABLE;
        m_pInterruptIn->WriteData(m_bModemSetState,&dwLength);
        m_bOldModemState =  m_bModemSetState[0];
    }
    m_HardwareLock.Unlock();

}
ULONG   USBSerialFn::GetModemStatus() 
{ 
    m_HardwareLock.Lock();
    ULONG Events = 0;
    m_bOldHostModemStatus ^= m_curHostModemStatus; 
    if ( m_bOldHostModemStatus & MS_DSR_ON)
        Events |= EV_DSR;
    if ( m_bOldHostModemStatus & MS_RLSD_ON)
        Events |= EV_RLSD;
    if ( m_bOldHostModemStatus & MS_CTS_ON)
        Events |= EV_CTS;
    m_bOldHostModemStatus = m_curHostModemStatus;
    m_HardwareLock.Unlock();
    if (Events)
        EventCallback(Events,m_curHostModemStatus);        
    return m_curHostModemStatus;
};

//
// D A T A / M A C R O S
//
#define MAX_INTERRUPT_ENDPOINT_PACKET_SIZE 8
#define CB_CONFIG_DESCRIPTOR  (sizeof(USB_CONFIGURATION_DESCRIPTOR)+28+7+23)
//  definitions
#define COMMAND_PASSED                      0x00
#define COMMAND_FAILED                      0x01
#define PHASE_ERROR                         0x02

#define EP0_PACKET_SIZE                     USB_FULL_HIGH_SPEED_CONTROL_MAX_PACKET_SIZE
#define HIGH_SPEED_BULK_PACKET_SIZES        USB_HIGH_SPEED_BULK_MAX_PACKET_SIZE
#define FULL_SPEED_BULK_PACKET_SIZES        USB_FULL_SPEED_BULK_MAX_PACKET_SIZE 
#define USB_VERSION                         0x200

#define PID_MICROSOFT_SERIAL_PROTOTYPE      0x00ce

    
#define BULK_IN_ENDPOINT_ADDRESS  0x83
#define BULK_OUT_ENDPOINT_ADDRESS 0x02

#define INTERRUPT_IN_ENDPOINT_ADDRESS 0x81  // Optional

#define BULK_IN_DESCRIPTOR_INDEX    1
#define BULK_OUT_DESCRIPTOR_INDEX   0
#define INTERRUPT_IN_DESCRIPTOR_INDEX 2

CUsbFn::CUsbFn (LPCTSTR lpActivePath)
:   m_pUfnFuncs(&m_UfnFuncs)
{
    DWORD dwLen = _tcslen(lpActivePath);
    m_lpActivePath = new TCHAR[dwLen+1];
    if (m_lpActivePath) {
        _tcscpy(m_lpActivePath,lpActivePath);
        m_lpActivePath[dwLen]= 0;
    }
    m_hDevice = NULL;
    m_hDefaultPipe = NULL;
    m_pvInterface = NULL;
    m_CurrentSpeed = BS_HIGH_SPEED;


    const USB_DEVICE_DESCRIPTOR HighSpeedDeviceDesc = {
        sizeof(USB_DEVICE_DESCRIPTOR),          // bLength
        USB_DEVICE_DESCRIPTOR_TYPE,             // bDescriptorType
        USB_VERSION,                            // bcdUSB
        0x00,                                   // bDeviceClass
        0x00,                                   // bDeviceSubClass
        0x00,                                   // bDeviceProtocol
        EP0_PACKET_SIZE,                        // bMaxPacketSize0
        0,                                      // idVendor
        0,                                      // idProduct
        0x0000,                                 // bcdDevice
        0x01,                                   // iManufacturer
        0x02,                                   // iProduct
        0x03,                                   // iSerialNumber
        0x01                                    // bNumConfigurations
    };
    m_HighSpeedDeviceDesc = HighSpeedDeviceDesc;

    const USB_DEVICE_DESCRIPTOR FullSpeedDeviceDesc = {
        sizeof(USB_DEVICE_DESCRIPTOR),          // bLength
        USB_DEVICE_DESCRIPTOR_TYPE,             // bDescriptorType
        USB_VERSION,                            // bcdUSB
        0x00,                                   // bDeviceClass
        0x00,                                   // bDeviceSubClass
        0x00,                                   // bDeviceProtocol
        EP0_PACKET_SIZE,                        // bMaxPacketSize0
        0,                                      // idVendor
        0,                                      // idProduct
        0x0000,                                 // bcdDevice
        0x01,                                   // iManufacturer
        0x02,                                   // iProduct
        0x03,                                   // iSerialNumber
        0x01                                    // bNumConfigurations
    };
    m_FullSpeedDeviceDesc = FullSpeedDeviceDesc;

    const UFN_ENDPOINT    HighSpeedEndpoints[INTERFACE_NUM_OF_ENDPOINT] = {
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                BULK_IN_ENDPOINT_ADDRESS,       // bEndpointAddress (endpoint 1, in)
                USB_ENDPOINT_TYPE_BULK,         // bmAttributes
                HIGH_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
                0x00                            // bInterval (interrupt only)
            },
            NULL
        },
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                BULK_OUT_ENDPOINT_ADDRESS,      // bEndpointAddress (endpoint 2, out)
                USB_ENDPOINT_TYPE_BULK,         // bmAttributes
                HIGH_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
                0x00                            // bInterval (interrupt only)
            },
            NULL
        },
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                INTERRUPT_IN_ENDPOINT_ADDRESS,  // bEndpointAddress (endpoint 2, out)
                USB_ENDPOINT_TYPE_INTERRUPT,    // bmAttributes
                MAX_INTERRUPT_ENDPOINT_PACKET_SIZE,   // wMaxPacketSize
                0xc                            // bInterval (interrupt only)
            },
            NULL
        }
    };
    memcpy(m_HighSpeedEndpoints,HighSpeedEndpoints,sizeof(m_HighSpeedEndpoints));
    
    const UFN_ENDPOINT    FullSpeedEndpoints[INTERFACE_NUM_OF_ENDPOINT] = {
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                BULK_IN_ENDPOINT_ADDRESS,       // bEndpointAddress (endpoint 1, in)
                USB_ENDPOINT_TYPE_BULK,         // bmAttributes
                FULL_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
                0x00                            // bInterval (interrupt only)
            },
            NULL
        },
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                BULK_OUT_ENDPOINT_ADDRESS,      // bEndpointAddress (endpoint 2, out)
                USB_ENDPOINT_TYPE_BULK,         // bmAttributes
                FULL_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
                0x00                            // bInterval (interrupt only)
            },
            NULL
        },
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                INTERRUPT_IN_ENDPOINT_ADDRESS,  // bEndpointAddress (endpoint 2, out)
                USB_ENDPOINT_TYPE_INTERRUPT,    // bmAttributes
                MAX_INTERRUPT_ENDPOINT_PACKET_SIZE,   // wMaxPacketSize
                0x20                            // bInterval (interrupt only)
            },
            NULL
        }
    };
    memcpy(m_FullSpeedEndpoints,FullSpeedEndpoints,sizeof(m_FullSpeedEndpoints));

    const UFN_INTERFACE HighSpeedInterface1 = {
        sizeof(UFN_INTERFACE),
        {
            sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
            USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
            0x00,                               // bInterfaceNumber    
            0x00,                               // bAlternateSetting

            dim(m_HighSpeedEndpoints),          // bNumEndpoints
            0xff,       // bInterfaceClass
            0xff,// bInterfaceSubClass
            0xff,             // bInterfaceProtocol
            0x00                                // iInterface    
        },
        NULL,                                   // extended
        0,
        m_HighSpeedEndpoints                    // endpoint array
    };
    m_HighSpeedInterface1 = HighSpeedInterface1;
    
    const UFN_INTERFACE HighSpeedInterface2 = {
        sizeof(UFN_INTERFACE),
        {
            sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
            USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
            0x00,                               // bInterfaceNumber    
            0x00,                               // bAlternateSetting

            dim(m_HighSpeedEndpoints)-1,        // bNumEndpoints
            0xff,       // bInterfaceClass
            0xff,// bInterfaceSubClass
            0xff,             // bInterfaceProtocol
            0x00                                // iInterface    
        },
        NULL,                                   // extended
        0,
        m_HighSpeedEndpoints                    // endpoint array
    };
    m_HighSpeedInterface2 = HighSpeedInterface2;
        
    const UFN_INTERFACE FullSpeedInterface1 = {
        sizeof(UFN_INTERFACE),
        {
            sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
            USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
            0x00,                               // bInterfaceNumber    
            0x00,                               // bAlternateSetting

            dim(m_FullSpeedEndpoints),          // bNumEndpoints
            0xff,       // bInterfaceClass
            0xff,// bInterfaceSubClass
            0xff,             // bInterfaceProtocol
            0x00                                // iInterface    
        },
        NULL,                                   // extended
        0,
        m_FullSpeedEndpoints                    // endpoint array
    };
    m_FullSpeedInterface1 = FullSpeedInterface1;
    
    const UFN_INTERFACE  FullSpeedInterface2 = {
        sizeof(UFN_INTERFACE),
        {
            sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
            USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
            0x00,                               // bInterfaceNumber    
            0x00,                               // bAlternateSetting

            dim(m_FullSpeedEndpoints)-1,          // bNumEndpoints
            0xff,       // bInterfaceClass
            0xff,// bInterfaceSubClass
            0xff,             // bInterfaceProtocol
            0x00                                // iInterface    
        },
        NULL,                                   // extended
        0,
        m_FullSpeedEndpoints                    // endpoint array
    };
    m_FullSpeedInterface2 = FullSpeedInterface2;
    
    const UFN_CONFIGURATION HighSpeedConfig1 = {
        sizeof(UFN_CONFIGURATION),
        {
            sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
            USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType 
            CB_CONFIG_DESCRIPTOR,               // wTotalLength
            0x01,                               // bNumInterfaces
            0x01,                               // bConfigurationValue
            0x00,                               // iConfiguration
            USB_CONFIG_RESERVED_ATTRIBUTE | USB_CONFIG_SELF_POWERED,            // bmAttributes
            0x00                                // MaxPower
        },
        NULL,
        0x00,                                   
        &m_HighSpeedInterface1                  // interface array
    };
    m_HighSpeedConfig1 = HighSpeedConfig1 ;
    
    const UFN_CONFIGURATION HighSpeedConfig2 = {
        sizeof(UFN_CONFIGURATION),
        {
            sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
            USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType
            
            CB_CONFIG_DESCRIPTOR- sizeof(USB_ENDPOINT_DESCRIPTOR), // wTotalLength
            0x01,                               // bNumInterfaces
            0x01,                               // bConfigurationValue
            0x00,                               // iConfiguration
            USB_CONFIG_RESERVED_ATTRIBUTE | USB_CONFIG_SELF_POWERED,            // bmAttributes
            0x00                                // MaxPower
        },
        NULL,
        0x00,                                   
        &m_HighSpeedInterface2                  // interface array
    };
    m_HighSpeedConfig2 = HighSpeedConfig2;

    const UFN_CONFIGURATION FullSpeedConfig1 = {
        sizeof(UFN_CONFIGURATION),
        {
            sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
            USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType
            
            CB_CONFIG_DESCRIPTOR,               // wTotalLength
            0x01,                               // bNumInterfaces
            0x01,                               // bConfigurationValue
            0x00,                               // iConfiguration
            USB_CONFIG_RESERVED_ATTRIBUTE | USB_CONFIG_SELF_POWERED,            // bmAttributes
            0x00                                // MaxPower
        },
        NULL,
        0x00,                                   
        &m_FullSpeedInterface1                   // interface array
    };
    m_FullSpeedConfig1 = FullSpeedConfig1 ;

    const UFN_CONFIGURATION FullSpeedConfig2 = {
        sizeof(UFN_CONFIGURATION),
        {
            sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
            USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType
            
            CB_CONFIG_DESCRIPTOR - sizeof(USB_ENDPOINT_DESCRIPTOR), // wTotalLength
            0x01,                               // bNumInterfaces
            0x01,                               // bConfigurationValue
            0x00,                               // iConfiguration
            USB_CONFIG_RESERVED_ATTRIBUTE | USB_CONFIG_SELF_POWERED,            // bmAttributes
            0x00                                // MaxPower
        },
        NULL,
        0x00,
        &m_FullSpeedInterface2                   // interface array
    };
    m_FullSpeedConfig2 = FullSpeedConfig2 ;
    
    m_RegInfo.dwSize = sizeof(m_RegInfo);
    m_rgpszStrings0409[0]=  m_RegInfo.szVendor;
    m_rgpszStrings0409[1]=  m_RegInfo.szProduct;
    m_rgpszStrings0409[2]=  m_RegInfo.szSerialNumber;
    
    const UFN_STRING_SET rgStringSets = {
        0x0409, m_rgpszStrings0409, dim(m_rgpszStrings0409)
    };
    m_rgStringSets = rgStringSets ;
}
#define COMLAYER_DRIVER_PATH TEXT("ComLayer")
BOOL CUsbFn::Init()
{
    if (m_lpActivePath &&
            UfnInitializeInterface(m_lpActivePath, &m_hDevice, &m_UfnFuncs, &m_pvInterface)== ERROR_SUCCESS) {
        PREFAST_ASSERT(m_hDevice!=NULL);
        PREFAST_ASSERT(m_pUfnFuncs!=NULL);
        CRegistryEdit driverKey(m_lpActivePath);
        if (driverKey.IsKeyOpened()) {
            DWORD dwRet = UfnGetRegistryInfo(m_lpActivePath, &m_RegInfo);
            if (dwRet == ERROR_SUCCESS) {
                m_FullSpeedDeviceDesc.idVendor = m_HighSpeedDeviceDesc.idVendor = (USHORT) m_RegInfo.idVendor;
                m_FullSpeedDeviceDesc.idProduct = m_HighSpeedDeviceDesc.idProduct = (USHORT) m_RegInfo.idProduct;
                m_FullSpeedDeviceDesc.bcdDevice = m_HighSpeedDeviceDesc.bcdDevice = (USHORT) m_RegInfo.bcdDevice;

                DWORD cStrings = dim(m_rgpszStrings0409);
                DWORD iSerialNumber = 3;
                if (m_RegInfo.szSerialNumber[0] == 0) {
                    DWORD dwSuccessSerialNumber = UfnGetSystemSerialNumber(
                        m_RegInfo.szSerialNumber, dim(m_RegInfo.szSerialNumber));

                    RETAILMSG(1,(TEXT("CUsbFn::Init UfnGetSystemSerialNumber %s\r\n"),m_RegInfo.szSerialNumber));
                    
                    if (dwSuccessSerialNumber != ERROR_SUCCESS) {
                        // No serial number
                        cStrings = dim(m_rgpszStrings0409) - 1;
                        iSerialNumber = 0;
                    }
                }

                m_rgStringSets.cStrings = cStrings;
                m_HighSpeedDeviceDesc.iSerialNumber = (UCHAR) iSerialNumber;
                m_FullSpeedDeviceDesc.iSerialNumber = (UCHAR) iSerialNumber;

                UfnCheckPID(&m_HighSpeedDeviceDesc, &m_FullSpeedDeviceDesc, 
                    PID_MICROSOFT_SERIAL_PROTOTYPE);
                
                // Register the descriptor tree with device controller
                m_fInterrupt = TRUE;
                dwRet = StartUSBFunction();
            }
            return (dwRet == ERROR_SUCCESS );
        }
    }
    return FALSE;
}
DWORD   CUsbFn::StartUSBFunction()
{
    DWORD dwRet = ERROR_GEN_FAILURE;
    
#ifdef USE_INTERRUPT_ENDPOINT
    dwRet = m_pUfnFuncs->lpRegisterDevice(m_hDevice,&m_HighSpeedDeviceDesc, &m_HighSpeedConfig1, 
        &m_FullSpeedDeviceDesc, &m_FullSpeedConfig1,&m_rgStringSets, 1 );
#endif            
    if (dwRet != ERROR_SUCCESS) { // Try Second
        m_fInterrupt = FALSE;
        dwRet = m_pUfnFuncs->lpRegisterDevice(m_hDevice,&m_HighSpeedDeviceDesc, &m_HighSpeedConfig2, 
            &m_FullSpeedDeviceDesc, &m_FullSpeedConfig2,&m_rgStringSets, 1 );
    }
    if (dwRet == ERROR_SUCCESS ) { // Do I need Initial Pipe Here?
        dwRet = m_pUfnFuncs->lpStart(m_hDevice, DeviceNotifyStub, this,  &m_hDefaultPipe);

    }
    return dwRet;
}

CUsbFn::~CUsbFn()
{
    if (m_lpActivePath)
        delete m_lpActivePath;

    if (m_pvInterface)
        UfnDeinitializeInterface(m_pvInterface);
}

BOOL WINAPI CUsbFn::DeviceNotifyStub(PVOID   pvNotifyParameter, DWORD   dwMsg, DWORD   dwParam)
{
    PREFAST_ASSERT(pvNotifyParameter!=NULL);
    return ((CUsbFn *)pvNotifyParameter)->DeviceNotify(dwMsg,dwParam);
}
BOOL CUsbFn::DeviceNotify(DWORD dwMsg, DWORD dwParam)
{
    DEBUGMSG (ZONE_WRITE|ZONE_EVENTS,(TEXT("DeviceNotify (0x%x,0x%x)\r\n"),dwMsg, dwParam));
    switch(dwMsg) {
        case UFN_MSG_BUS_EVENTS: {
            // Ensure device is in running state
            DEBUGCHK(m_hDefaultPipe);

           switch(dwParam) {                
                case UFN_DETACH:
                    CableDetached();
                    break;
                case UFN_ATTACH:
                    break;
                case UFN_RESET: {
                    CableDetached();
                    break;
                }
             } 

           break;
        }
        
        case UFN_MSG_BUS_SPEED:
             m_CurrentSpeed = (UFN_BUS_SPEED) dwParam;
        break;

        case UFN_MSG_SETUP_PACKET:
        case UFN_MSG_PREPROCESSED_SETUP_PACKET: {
            HandleRequest(dwMsg,*(PUSB_DEVICE_REQUEST)dwParam);
            break;
        }

        case UFN_MSG_CONFIGURED: {
            if (dwParam == 0) {
                CableDetached();
            }
            else {
                CableAttached();
            }
            break;
        }
        
    }
    return TRUE;
}
void CUsbFn::HandleRequest(
    DWORD dwMsg,
    USB_DEVICE_REQUEST udr
    )
{
    CONTROL_RESPONSE response;

    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        response = CR_SUCCESS; // Don't respond since it was already handled.
        
        if ( udr.bmRequestType ==
            (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_ENDPOINT) ) {
            switch (udr.bRequest) {
                case USB_REQUEST_CLEAR_FEATURE:
                    // This may be needed in the future to terminate a transfer 
                    // in progress
                    HandleClearFeature(udr);
                    break;
            }
        }
    }
    else {
        DEBUGCHK(dwMsg == UFN_MSG_SETUP_PACKET);
        response = CR_STALL_DEFAULT_PIPE;

        if (udr.bmRequestType & USB_REQUEST_CLASS) {
            response = HandleClassRequest(udr);
        }
    }

    if (response == CR_STALL_DEFAULT_PIPE) {
        m_pUfnFuncs->lpStallPipe(m_hDevice, m_hDefaultPipe);
        m_pUfnFuncs->lpSendControlStatusHandshake(m_hDevice);
    }
    else if (response == CR_SUCCESS_SEND_CONTROL_HANDSHAKE) {
        m_pUfnFuncs->lpSendControlStatusHandshake(m_hDevice);
    }
}
CONTROL_RESPONSE CUsbFn::HandleClassRequest(
    USB_DEVICE_REQUEST udr
    )
{

    CONTROL_RESPONSE response = CR_STALL_DEFAULT_PIPE;
           
    if (udr.bmRequestType == 
            (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_HOST_TO_DEVICE) ) { 
        if (udr.bRequest == SET_CONTROL_LINE_STATE) {
            /* Host is notifying us of control line state.
             * wValue contains bitmask
             * 0 - DTR
             * 1 - RTS
             */
            DEBUGMSG( ZONE_FUNCTION, (TEXT("SET_CONTROL_LINE_STATE %X\r\n"),
                 udr.wValue));
            DWORD dwModemStatus = 0;
            if (udr.wValue & USB_COMM_DTR)
              dwModemStatus |= (MS_DSR_ON|MS_RLSD_ON); // DTR active, set DSR/RLSD
            if (udr.wValue & USB_COMM_RTS) 
              dwModemStatus |= MS_CTS_ON;   // RTS active, set CTS
            ModemSignal(dwModemStatus);
            response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE;
        }
    }
    else {
        RETAILMSG(1, (_T("Unrecognized Serial class bRequest -> 0x%x\r\n"), udr.bmRequestType));
        ASSERT(FALSE);
    }
    ASSERT(response == CR_SUCCESS_SEND_CONTROL_HANDSHAKE);
    return response;
}


void USBSerialFn::CableDetached()
{
    // This is last time to call this structure.
    CloseBulkIn();
    CloseBulkOut();
    CloseInterruptIn();
    ModemSignal(0);
}
void USBSerialFn::CableAttached() 
{
   m_HardwareLock.Lock();
    DWORD dwBulkSize = (m_CurrentSpeed == BS_HIGH_SPEED?
        m_HighSpeedEndpoints[BULK_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize:
        m_FullSpeedEndpoints[BULK_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize);
    BYTE uEdptAddr  = (m_CurrentSpeed == BS_HIGH_SPEED?
        m_HighSpeedEndpoints[BULK_IN_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress:
        m_FullSpeedEndpoints[BULK_IN_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress);
    OpenBulkIn(m_hDevice, m_pUfnFuncs,uEdptAddr,FALSE, dwBulkSize, 0x1000, 2);
    
    dwBulkSize = (m_CurrentSpeed == BS_HIGH_SPEED?
        m_HighSpeedEndpoints[BULK_OUT_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize:
        m_FullSpeedEndpoints[BULK_OUT_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize);
    uEdptAddr  = (m_CurrentSpeed == BS_HIGH_SPEED?
        m_HighSpeedEndpoints[BULK_OUT_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress:
        m_FullSpeedEndpoints[BULK_OUT_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress);
    OpenBulkOut(m_hDevice, m_pUfnFuncs,uEdptAddr,TRUE,dwBulkSize,dwBulkSize, 4); // WCEUSBSH Bug. Only can usb MaxPacketSize
    
    if (m_fInterrupt) {
        DWORD dwInterruptSize = (m_CurrentSpeed == BS_HIGH_SPEED?
            m_HighSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize:
            m_FullSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize);
        uEdptAddr  = (m_CurrentSpeed == BS_HIGH_SPEED?
            m_HighSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress:
            m_FullSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress);
        OpenInterruptIn(m_hDevice, m_pUfnFuncs,uEdptAddr,FALSE,dwInterruptSize,dwInterruptSize, 1);
    }
    m_HardwareLock.Unlock();
}

void USBSerSerialFn::CableAttached() 
{
    m_HardwareLock.Lock();
    DWORD dwBulkSize = (m_CurrentSpeed == BS_HIGH_SPEED?
        m_DICHighSpeedEndpoints[BULK_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize:
        m_DICFullSpeedEndpoints[BULK_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize);
    BYTE uEdptAddr  = (m_CurrentSpeed == BS_HIGH_SPEED?
        m_DICHighSpeedEndpoints[BULK_IN_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress:
        m_DICFullSpeedEndpoints[BULK_IN_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress);
    RETAILMSG(0,(TEXT("CableAttached open bulk in 0x%x\r\n"),uEdptAddr));
    OpenBulkIn(m_hDevice, m_pUfnFuncs,uEdptAddr,FALSE, dwBulkSize, 0x1000, 2);
    
    dwBulkSize = (m_CurrentSpeed == BS_HIGH_SPEED?
        m_DICHighSpeedEndpoints[BULK_OUT_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize:
        m_DICFullSpeedEndpoints[BULK_OUT_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize);
    uEdptAddr  = (m_CurrentSpeed == BS_HIGH_SPEED?
        m_DICHighSpeedEndpoints[BULK_OUT_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress:
        m_DICFullSpeedEndpoints[BULK_OUT_DESCRIPTOR_INDEX].Descriptor.bEndpointAddress);
    RETAILMSG(0,(TEXT("CableAttached open bulk out 0x%x\r\n"),uEdptAddr));
    OpenBulkOut(m_hDevice, m_pUfnFuncs,uEdptAddr,TRUE,dwBulkSize,dwBulkSize, 4); // WCEUSBSH Bug. Only can usb MaxPacketSize
    
    if (m_fInterrupt) {
        DWORD dwInterruptSize = (m_CurrentSpeed == BS_HIGH_SPEED?
            m_CICHighSpeedEndpoints[0].Descriptor.wMaxPacketSize:
            m_CICFullSpeedEndpoints[0].Descriptor.wMaxPacketSize);
        uEdptAddr  = (m_CurrentSpeed == BS_HIGH_SPEED?
            m_CICHighSpeedEndpoints[0].Descriptor.bEndpointAddress:
            m_CICFullSpeedEndpoints[0].Descriptor.bEndpointAddress);
        RETAILMSG(1,(TEXT("CableAttached open interrupt in 0x%x\r\n"),uEdptAddr));
        OpenInterruptIn(m_hDevice, m_pUfnFuncs,uEdptAddr,FALSE,dwInterruptSize,dwInterruptSize, 1);
    }
    m_HardwareLock.Unlock();
}


void USBSerSerialFn::ConstructConfigurationDescriptor() 
{
    // device descriptors
    const USB_DEVICE_DESCRIPTOR HighSpeedDeviceDesc = {
            sizeof(USB_DEVICE_DESCRIPTOR),          // bLength
            USB_DEVICE_DESCRIPTOR_TYPE,             // bDescriptorType
            USB_VERSION,                            // bcdUSB
            0x02,                                   // bDeviceClass
            0x00,                                   // bDeviceSubClass
            0x00,                                   // bDeviceProtocol
            EP0_PACKET_SIZE,                        // bMaxPacketSize0
            0,                                      // idVendor
            0,                                      // idProduct
            0x0000,                                 // bcdDevice
            0x01,                                   // iManufacturer
            0x02,                                   // iProduct
            0x03,                                   // iSerialNumber
            0x01                                    // bNumConfigurations
    };
    m_HighSpeedDeviceDesc = HighSpeedDeviceDesc;
    
    const USB_DEVICE_DESCRIPTOR FullSpeedDeviceDesc = {
            sizeof(USB_DEVICE_DESCRIPTOR),          // bLength
            USB_DEVICE_DESCRIPTOR_TYPE,             // bDescriptorType
            USB_VERSION,                            // bcdUSB
            0x02,                                   // bDeviceClass
            0x00,                                   // bDeviceSubClass
            0x00,                                   // bDeviceProtocol
            EP0_PACKET_SIZE,                        // bMaxPacketSize0
            0,                                      // idVendor
            0,                                      // idProduct
            0x0000,                                 // bcdDevice
            0x01,                                   // iManufacturer
            0x02,                                   // iProduct
            0x03,                                   // iSerialNumber
            0x01                                    // bNumConfigurations
    };
    m_FullSpeedDeviceDesc = FullSpeedDeviceDesc;
    
    //Functional description class-specific descriptor
    UCHAR *pFuncDesc = NULL;
    const USB_HEADER_FUNC_DESCRIPTOR headerFunctionDesc = {
        sizeof(USB_HEADER_FUNC_DESCRIPTOR),
        DESCRIPTOR_TYPE_CS_INTERFACE,
        HEADER_FUNC_DESC,
        0x10,
        0x01
    };

    const USB_CALL_MANAGERMENT_FUNC_DESCRIPTOR cmFunctionDesc = {
        sizeof(USB_CALL_MANAGERMENT_FUNC_DESCRIPTOR),
        DESCRIPTOR_TYPE_CS_INTERFACE,
        CALL_MANAGEMENT_FUNC_DESC,
        0x01,
        0x01     
    };

    const USB_ACM_FUNC_DESCRIPTOR acmFunctionDesc = {
        sizeof(USB_ACM_FUNC_DESCRIPTOR),
        DESCRIPTOR_TYPE_CS_INTERFACE,
        ABSTRACT_CONTROL_FUNC_DESC,
        0x06    
    };

    const USB_UNION_FUNC_DESCRIPTOR unionFunctionDesc = {
        sizeof(USB_UNION_FUNC_DESCRIPTOR),
        DESCRIPTOR_TYPE_CS_INTERFACE,
        UNION_FUNC_DESC,
        0x00,
        0x01
    };

    m_headerFuncDesc = headerFunctionDesc;
    m_cmFuncDesc     = cmFunctionDesc;
    m_acmFuncDesc    = acmFunctionDesc;
    m_unionFuncDesc  = unionFunctionDesc;
    

    pFuncDesc = m_ucFuncDesc;
    m_wFuncDescSize = 0;
    
    memcpy(pFuncDesc, (PVOID)&m_headerFuncDesc, sizeof(USB_HEADER_FUNC_DESCRIPTOR));
    m_wFuncDescSize += sizeof(USB_HEADER_FUNC_DESCRIPTOR);
    pFuncDesc += sizeof(USB_HEADER_FUNC_DESCRIPTOR);

    if(m_wFuncDescSize >= MAX_CDC_FUNCTION_DESCRIPTOR_SIZE)
    {
        RETAILMSG(1,(TEXT("Function Descriptor is too long\r\n")));
    }
    else
    {
        RETAILMSG(0,(TEXT("Function Descriptor length %d\r\n"),m_wFuncDescSize));
    }

    memcpy(pFuncDesc, (PVOID)&m_cmFuncDesc, sizeof(USB_CALL_MANAGERMENT_FUNC_DESCRIPTOR));
    m_wFuncDescSize += sizeof(USB_CALL_MANAGERMENT_FUNC_DESCRIPTOR);
    pFuncDesc += sizeof(USB_CALL_MANAGERMENT_FUNC_DESCRIPTOR);

    if(m_wFuncDescSize >= MAX_CDC_FUNCTION_DESCRIPTOR_SIZE)
    {
        RETAILMSG(1,(TEXT("Function Descriptor is too long\r\n")));
    }
    else
    {
        RETAILMSG(0,(TEXT("Function Descriptor length %d\r\n"),m_wFuncDescSize));
    }

    memcpy(pFuncDesc, (PVOID)&m_acmFuncDesc, sizeof(USB_ACM_FUNC_DESCRIPTOR));
    m_wFuncDescSize += sizeof(USB_ACM_FUNC_DESCRIPTOR);
    pFuncDesc += sizeof(USB_ACM_FUNC_DESCRIPTOR);

    if(m_wFuncDescSize >= MAX_CDC_FUNCTION_DESCRIPTOR_SIZE)
    {
        RETAILMSG(1,(TEXT("Function Descriptor is too long\r\n")));
    }
    else
    {
        RETAILMSG(0,(TEXT("Function Descriptor length %d\r\n"),m_wFuncDescSize));
    }

    memcpy(pFuncDesc, (PVOID)&m_unionFuncDesc, sizeof(USB_UNION_FUNC_DESCRIPTOR));
    m_wFuncDescSize += sizeof(USB_UNION_FUNC_DESCRIPTOR);
    pFuncDesc += sizeof(USB_UNION_FUNC_DESCRIPTOR);

    if(m_wFuncDescSize >= MAX_CDC_FUNCTION_DESCRIPTOR_SIZE)
    {
        RETAILMSG(1,(TEXT("Function Descriptor is too long\r\n")));
    }
    else
    {
        RETAILMSG(0,(TEXT("Function Descriptor length %d\r\n"),m_wFuncDescSize));
    }

    RETAILMSG(0,(TEXT("CDC functional descriptor:\r\n")));
    for(int index = 0; index < m_wFuncDescSize; index++)
    {
        RETAILMSG(0,(TEXT("0x%02x "),m_ucFuncDesc[index]));
    }
    RETAILMSG(0,(TEXT("\r\n")));
    //endpoint description class-specific descriptor
    
    const UFN_ENDPOINT    CICHighSpeedEndpoints[ENDPOINT_NUM_OF_CIC] = {
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                INTERRUPT_IN_ENDPOINT_ADDRESS,  // bEndpointAddress (endpoint 2, out)
                USB_ENDPOINT_TYPE_INTERRUPT,    // bmAttributes
                MAX_INTERRUPT_ENDPOINT_PACKET_SIZE,   // wMaxPacketSize
                0xc                            // bInterval (interrupt only)
            },
            NULL,
            0
        }
    };
    memcpy(m_CICHighSpeedEndpoints,CICHighSpeedEndpoints,sizeof(m_CICHighSpeedEndpoints));
    RETAILMSG(0,(TEXT("CDC functional descriptor222:\r\n")));
    for(int index = 0; index < m_wFuncDescSize; index++)
    {
        RETAILMSG(0,(TEXT("0x%02x "),m_ucFuncDesc[index]));
    }
    RETAILMSG(0,(TEXT("\r\n")));
    
    const UFN_ENDPOINT    CICFullSpeedEndpoints[ENDPOINT_NUM_OF_CIC] = {
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                INTERRUPT_IN_ENDPOINT_ADDRESS,  // bEndpointAddress (endpoint 2, out)
                USB_ENDPOINT_TYPE_INTERRUPT,    // bmAttributes
                MAX_INTERRUPT_ENDPOINT_PACKET_SIZE,   // wMaxPacketSize
                0x20                            // bInterval (interrupt only)
            },
            NULL
        }
    };
    memcpy(m_CICFullSpeedEndpoints,CICFullSpeedEndpoints,sizeof(m_CICFullSpeedEndpoints));
    RETAILMSG(0,(TEXT("CDC functional descriptor333:\r\n")));
    for(int index = 0; index < m_wFuncDescSize; index++)
    {
        RETAILMSG(0,(TEXT("0x%02x "),m_ucFuncDesc[index]));
    }
    RETAILMSG(0,(TEXT("\r\n")));

    const UFN_ENDPOINT    DICHighSpeedEndpoints[ENDPOINT_NUM_OF_DIC] = {
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                BULK_OUT_ENDPOINT_ADDRESS,      // bEndpointAddress (endpoint 2, out)
                USB_ENDPOINT_TYPE_BULK,         // bmAttributes
                HIGH_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
                0x00                            // bInterval (interrupt only)
            },
            NULL
        },
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                BULK_IN_ENDPOINT_ADDRESS,       // bEndpointAddress (endpoint 1, in)
                USB_ENDPOINT_TYPE_BULK,         // bmAttributes
                HIGH_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
                0x00                            // bInterval (interrupt only)
            },
            NULL
        }
    };
    memcpy(m_DICHighSpeedEndpoints,DICHighSpeedEndpoints,sizeof(m_DICHighSpeedEndpoints));
    RETAILMSG(0,(TEXT("CDC functional descriptor444:\r\n")));
    for(int index = 0; index < m_wFuncDescSize; index++)
    {
        RETAILMSG(0,(TEXT("0x%02x "),m_ucFuncDesc[index]));
    }
    RETAILMSG(0,(TEXT("\r\n")));
    
    const UFN_ENDPOINT    DICFullSpeedEndpoints[ENDPOINT_NUM_OF_DIC] = {
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                BULK_OUT_ENDPOINT_ADDRESS,      // bEndpointAddress (endpoint 2, out)
                USB_ENDPOINT_TYPE_BULK,         // bmAttributes
                FULL_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
                0x00                            // bInterval (interrupt only)
            },
            NULL
        },
        {
            sizeof(UFN_ENDPOINT),
            {
                sizeof(USB_ENDPOINT_DESCRIPTOR),// bLength
                USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType
                BULK_IN_ENDPOINT_ADDRESS,       // bEndpointAddress (endpoint 1, in)
                USB_ENDPOINT_TYPE_BULK,         // bmAttributes
                FULL_SPEED_BULK_PACKET_SIZES,   // wMaxPacketSize
                0x00                            // bInterval (interrupt only)
            },
            NULL
        }
    };
    memcpy(m_DICFullSpeedEndpoints,DICFullSpeedEndpoints,sizeof(m_DICFullSpeedEndpoints));
    RETAILMSG(0,(TEXT("CDC functional descriptor555 %d:\r\n"),sizeof(m_DICFullSpeedEndpoints)));
    for(int index = 0; index < m_wFuncDescSize; index++)
    {
        RETAILMSG(0,(TEXT("0x%02x "),m_ucFuncDesc[index]));
    }
    RETAILMSG(0,(TEXT("\r\n")));

    //interface descriptor

    const UFN_INTERFACE CICHighSpeedInterface = {
        sizeof(UFN_INTERFACE),
        {
            sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
            USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
            0x00,                               // bInterfaceNumber    
            0x00,                               // bAlternateSetting

            dim(m_CICHighSpeedEndpoints),          // bNumEndpoints
            CIC_INTERFACE_CODE,                    // bInterfaceClass
            CIC_SUBCLASS_CODE,                     // bInterfaceSubClass ACM
            CIC_PROTOCOL_CODE,                     // bInterfaceProtocol
            0x00                                   // iInterface    
        },
        (PVOID)m_ucFuncDesc,                       // extended
        (DWORD)m_wFuncDescSize,
        m_CICHighSpeedEndpoints                    // endpoint array
    };
    m_HighSpeedInterface1 = CICHighSpeedInterface;

    RETAILMSG(0,(TEXT("CDC functional descriptor222:\r\n")));
    for(int index = 0; index < m_wFuncDescSize; index++)
    {
        RETAILMSG(0,(TEXT("0x%02x "),m_ucFuncDesc[index]));
    }
    RETAILMSG(0,(TEXT("\r\n")));

    RETAILMSG(0,(TEXT("1111111111 extend descriptor 0x%x\r\n"),m_HighSpeedInterface1.pvExtended));//   m_pUfnInterface->pvExtended));
    PBYTE tmp = (PBYTE)m_HighSpeedInterface1.pvExtended;
    for(DWORD index = 0; index < m_HighSpeedInterface1.cbExtended; index++)
    {
        RETAILMSG(0,(TEXT("0x%02x "),*(tmp+index)));
    }
    RETAILMSG(0,(TEXT("\r\n")));
    
    const UFN_INTERFACE DICHighSpeedInterface = {
        sizeof(UFN_INTERFACE),
        {
            sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
            USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
            0x01,                               // bInterfaceNumber    
            0x00,                               // bAlternateSetting

            dim(m_DICHighSpeedEndpoints),        // bNumEndpoints
            DIC_INTERFACE_CODE,                  // bInterfaceClass
            DIC_SUBCLASS_CODE,                   // bInterfaceSubClass
            DIC_PROTOCOL_CODE,                   // bInterfaceProtocol
            0x00                                // iInterface    
        },
        NULL,                                   // extended
        0,
        m_DICHighSpeedEndpoints                    // endpoint array
    };
    m_HighSpeedInterface2 = DICHighSpeedInterface;
        
    const UFN_INTERFACE CICFullSpeedInterface = {
        sizeof(UFN_INTERFACE),
        {
            sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
            USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
            0x00,                               // bInterfaceNumber    
            0x00,                               // bAlternateSetting

            dim(m_CICFullSpeedEndpoints),          // bNumEndpoints
            CIC_INTERFACE_CODE,                    // bInterfaceClass
            CIC_SUBCLASS_CODE,                     // bInterfaceSubClass ACM
            CIC_PROTOCOL_CODE,                     // bInterfaceProtocol
            0x00                                   // iInterface    
        },
        (PVOID)m_ucFuncDesc,                       // extended
        (DWORD)m_wFuncDescSize,
        m_CICFullSpeedEndpoints                    // endpoint array
    };
    m_FullSpeedInterface1 = CICFullSpeedInterface;
    
    const UFN_INTERFACE  DICFullSpeedInterface = {
        sizeof(UFN_INTERFACE),
        {
            sizeof(USB_INTERFACE_DESCRIPTOR),   // bLength
            USB_INTERFACE_DESCRIPTOR_TYPE,      // bDescriptorType
            0x01,                               // bInterfaceNumber    
            0x00,                               // bAlternateSetting

            dim(m_DICFullSpeedEndpoints),          // bNumEndpoints
            DIC_INTERFACE_CODE,                  // bInterfaceClass
            DIC_SUBCLASS_CODE,                   // bInterfaceSubClass
            DIC_PROTOCOL_CODE,                   // bInterfaceProtocol
            0x00                                // iInterface    
        },
        NULL,                                   // extended
        0,
        m_DICFullSpeedEndpoints                    // endpoint array
    };
    m_FullSpeedInterface2 = DICFullSpeedInterface;

    m_HighSpeedInterfaceArray[0] = m_HighSpeedInterface1;
    m_HighSpeedInterfaceArray[1] = m_HighSpeedInterface2;

    m_FullSpeedInterfaceArray[0] = m_FullSpeedInterface1;
    m_FullSpeedInterfaceArray[1] = m_FullSpeedInterface2;

    //configuration descpritor
    
    const UFN_CONFIGURATION HighSpeedConfig1 = {
        sizeof(UFN_CONFIGURATION),
        {
            sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
            USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType
            
            CB_CONFIG_DESCRIPTOR,               // wTotalLength
            0x02,                               // bNumInterfaces
            0x01,                               // bConfigurationValue
            0x00,                               // iConfiguration
            USB_CONFIG_RESERVED_ATTRIBUTE | USB_CONFIG_SELF_POWERED,            // bmAttributes
            0x32                                // MaxPower
        },
        NULL,
        0x00,                                   
        m_HighSpeedInterfaceArray               // interface array
    };
    m_HighSpeedConfig1 = HighSpeedConfig1 ;
    
    m_HighSpeedConfig2 = m_HighSpeedConfig1;

    RETAILMSG(0,(TEXT("USBSerSerialFn::ConstructConfigurationDescriptor highspeed config interface 0 extend descriptor 0x%x\r\n"),m_HighSpeedConfig1.pInterfaces[0].pvExtended));//   m_pUfnInterface->pvExtended));
    tmp = (PBYTE)m_HighSpeedConfig1.pInterfaces[0].pvExtended;
    for(DWORD index = 0; index < m_HighSpeedConfig1.pInterfaces[0].cbExtended; index++)
    {
        RETAILMSG(0,(TEXT("0x%02x "),*(tmp+index)));
    }
    RETAILMSG(0,(TEXT("\r\n")));

    const UFN_CONFIGURATION FullSpeedConfig1 = {
        sizeof(UFN_CONFIGURATION),
        {
            sizeof(USB_CONFIGURATION_DESCRIPTOR),// bLength
            USB_CONFIGURATION_DESCRIPTOR_TYPE,  // bDescriptorType
            
            CB_CONFIG_DESCRIPTOR,               // wTotalLength
            0x02,                               // bNumInterfaces
            0x01,                               // bConfigurationValue
            0x00,                               // iConfiguration
            USB_CONFIG_RESERVED_ATTRIBUTE | USB_CONFIG_SELF_POWERED,            // bmAttributes
            0x32                                // MaxPower
        },
        NULL,
        0x00,                                   
        m_FullSpeedInterfaceArray                   // interface array
    };
    m_FullSpeedConfig1 = FullSpeedConfig1 ;

    m_FullSpeedConfig2 = m_FullSpeedConfig1 ;
    
    m_RegInfo.dwSize = sizeof(m_RegInfo);
    m_rgpszStrings0409[0]=  m_RegInfo.szVendor;
    m_rgpszStrings0409[1]=  m_RegInfo.szProduct;
    m_rgpszStrings0409[2]=  m_RegInfo.szSerialNumber;
    
    const UFN_STRING_SET rgStringSets = {
        0x0409, m_rgpszStrings0409, dim(m_rgpszStrings0409)
    };
    m_rgStringSets = rgStringSets ;

}

USBSerSerialFn::USBSerSerialFn(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj )
:   USBSerialFn(lpActivePath,pMdd,pHwObj )
{
    // Setup Discriptor default value for USBSer driver.
    m_FullSpeedDeviceDesc.bDeviceClass =    m_HighSpeedDeviceDesc.bDeviceClass = 0x20;
    m_FullSpeedDeviceDesc.bDeviceSubClass = m_HighSpeedDeviceDesc.bDeviceSubClass = 0;
    m_FullSpeedDeviceDesc.bDeviceProtocol = m_HighSpeedDeviceDesc.bDeviceProtocol = 0;
    
    m_FullSpeedDeviceDesc.idProduct = m_HighSpeedDeviceDesc.idProduct =   0x2504 ; // refer to fsl cdc code implementation
    m_FullSpeedDeviceDesc.idVendor = m_HighSpeedDeviceDesc.idVendor =    0x0300 ;
    m_FullSpeedDeviceDesc.bcdDevice = m_HighSpeedDeviceDesc.bcdDevice =   0x0002;

    ConstructConfigurationDescriptor();

    //m_FullSpeedInterface1.Descriptor.bInterfaceClass  = m_HighSpeedInterface1.Descriptor.bInterfaceClass = 0x02;
    //m_FullSpeedInterface1.Descriptor.bInterfaceSubClass  = m_HighSpeedInterface1.Descriptor.bInterfaceSubClass = 0xff;
    //m_FullSpeedInterface1.Descriptor.bInterfaceProtocol  = m_HighSpeedInterface1.Descriptor.bInterfaceProtocol = 0xff;

    const USB_COMM_LINE_CODING CommLineCoding = {
        9600, USB_COMM_STOPBITS_10, USB_COMM_PARITY_NONE,8             
    };
    m_CommLineCoding = CommLineCoding;
    m_hTransferHandle = NULL;
    m_wModemSetState = USB_COMM_DCD  ;
    const USB_COMM_SERIAL_STATUS CommSerialStatus= {
        0, USB_COMM_SERIAL_STATE, 0, 0, 0, USB_COMM_DCD
    };
    m_CommSerialStatus = CommSerialStatus;

    // USBSER requires interrupt endpoint maxpacket size bigger than 8. So we try 64 to find anything available.
    m_HighSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize = 0x40;
    m_FullSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize = 0x40;

    
}
USBSerSerialFn::~USBSerSerialFn()
{
    if (m_pUfnFuncs && m_hDevice && m_hDefaultPipe && m_hTransferHandle) {
        DWORD dwRet = m_pUfnFuncs->lpAbortTransfer(m_hDevice,m_hTransferHandle);
        DEBUGMSG (ZONE_EVENTS,(TEXT("USBSerSerialFn::~USBSerSerialFn(Handle=0x%x)\r\n"), m_pUfnFuncs));
    }
}
DWORD   USBSerSerialFn::StartUSBFunction()
{
    DWORD dwRet = ERROR_GEN_FAILURE;
    RETAILMSG(0,(TEXT("USBSerSerialFn::StartUSBFunction interface 0 extend descriptor 0x%x\r\n"),m_HighSpeedConfig1.pInterfaces->pvExtended));
    PBYTE tmp = (PBYTE)m_HighSpeedConfig1.pInterfaces[0].pvExtended;
    for(DWORD index = 0; index < m_HighSpeedConfig1.pInterfaces[0].cbExtended; index++)
    {
        RETAILMSG(0,(TEXT("0x%02x "),*(tmp+index)));
    }
    RETAILMSG(0,(TEXT("\r\n")));
    dwRet = m_pUfnFuncs->lpRegisterDevice(m_hDevice, &m_HighSpeedDeviceDesc, &m_HighSpeedConfig1, 
        &m_FullSpeedDeviceDesc, &m_FullSpeedConfig1, &m_rgStringSets, 1 );
    if (m_HighSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize <=8 ||
            m_FullSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize <=8) {
        RETAILMSG (0,(TEXT("USBSerSerialFn::StartUSBFunction: Interrupt Endpoint point MaxPacket (%d, %d) is too small \r\n"),
            m_HighSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize,
            m_FullSpeedEndpoints[INTERRUPT_IN_DESCRIPTOR_INDEX].Descriptor.wMaxPacketSize));            
        return ERROR_GEN_FAILURE;
    }
    if (dwRet == ERROR_SUCCESS ) { // Do I need Initial Pipe Here?
        m_fInterrupt = TRUE;
        dwRet = m_pUfnFuncs->lpStart(m_hDevice, DeviceNotifyStub, (CUsbFn *)this,  &m_hDefaultPipe);
    }
    else
    {
        RETAILMSG(1,(TEXT("USBSerSerialFn::StartUSBFunction lpRegisterDevice failed\r\n")));
    }
    return dwRet;
}
BOOL  USBSerSerialFn::IssueTransfer(PBYTE pBuffer, DWORD dwLength, BOOL fIn)
{
    DWORD dwFlags = (fIn? USB_IN_TRANSFER: USB_OUT_TRANSFER);
    BOOL bReturn = FALSE;
    m_HardwareLock.Lock();
    if (pBuffer && m_hTransferHandle == NULL) {
        DWORD dwRet = m_pUfnFuncs->lpIssueTransfer(m_hDevice,m_hDefaultPipe,CompleteNotificationStub,this,dwFlags,
            dwLength,pBuffer,NULL,NULL,&m_hTransferHandle);
        if (dwRet != ERROR_SUCCESS)
            m_hTransferHandle = NULL;
        bReturn = (dwRet == ERROR_SUCCESS);     
    }
    m_HardwareLock.Unlock();
    ASSERT(bReturn == TRUE);
    return bReturn;
}
DWORD WINAPI  USBSerSerialFn::CompleteNotification()
{
    DWORD dwRet = ERROR_GEN_FAILURE ;
    RETAILMSG(0,(TEXT("receive CompleteNotification\r\n")));
    m_HardwareLock.Lock();
    if (m_hTransferHandle) {
        m_pUfnFuncs->lpCloseTransfer(m_hDevice,m_hTransferHandle);
        m_pUfnFuncs->lpSendControlStatusHandshake(m_hDevice);  
        m_hTransferHandle = NULL;
        dwRet = ERROR_SUCCESS;
    }
    m_HardwareLock.Unlock();
    ASSERT(dwRet == ERROR_SUCCESS) ;
    return (dwRet);
}

CONTROL_RESPONSE USBSerSerialFn::HandleClassRequest(USB_DEVICE_REQUEST udr)
{
    CONTROL_RESPONSE response = CR_STALL_DEFAULT_PIPE;
    if (udr.bmRequestType ==  (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_DEVICE_TO_HOST) ) { 
        switch (udr.bRequest) {
          case USB_COMM_GET_LINE_CODING:
            RETAILMSG(0,(TEXT("USB_COMM_GET_LINE_CODING %d %d\r\n"),sizeof(m_CommLineCoding),udr.wLength));
            if (IssueTransfer((PBYTE)&m_CommLineCoding,min( sizeof(m_CommLineCoding), udr.wLength), TRUE )) {
                response = CR_SUCCESS ;
            }
            break;
        }
    }
    else if (udr.bmRequestType ==  (USB_REQUEST_CLASS | USB_REQUEST_FOR_INTERFACE | USB_REQUEST_HOST_TO_DEVICE) ) { 
        switch (udr.bRequest) {
          case USB_COMM_SET_LINE_CODING:
            RETAILMSG(0,(TEXT("recevie set line coding command ready to send %d byte\r\n"),sizeof(m_CommLineCoding)));
            if (IssueTransfer((PBYTE)&m_CommLineCoding, 7, FALSE )) {
                RETAILMSG(0,(TEXT("receive line coding\r\n")));
                response = CR_SUCCESS ;
            }
            break;
          case USB_COMM_SET_CONTROL_LINE_STATE: {
            RETAILMSG(0, (TEXT("SET_CONTROL_LINE_STATE %X\r\n"), udr.wValue));
            DWORD dwModemStatus = 0;
            if (udr.wValue & USB_COMM_DTR)
                dwModemStatus |= (MS_DSR_ON|MS_RLSD_ON); // DTR active, set DSR/RLSD
            //if (udr.wValue & USB_COMM_RTS) 
            //  dwModemStatus |= MS_CTS_ON;   // RTS active, set CTS
            // Desktop Driver has problem to set RTS. We always set it on.
            dwModemStatus |= MS_CTS_ON ;
            ModemSignal(dwModemStatus);
            response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE ;
            break;
          }
          case USB_COMM_SEND_BREAK: {
            EventCallback(EV_BREAK);
            // Do I need Sleep Here? Sleep(udr.wValue)
            response = CR_SUCCESS_SEND_CONTROL_HANDSHAKE ;
            break;
          }
          default:
            ASSERT(FALSE);
            break;
        }
    }
    return response;
    
}
void USBSerSerialFn::SetModemSignal(BOOL bSet, BYTE bBitSet)
{
    m_HardwareLock.Lock();
    if (bSet)
        m_wModemSetState |= bBitSet;
    else
        m_wModemSetState &= ~bBitSet;
    
    if (m_pInterruptIn && m_CommSerialStatus.SerialState != m_wModemSetState && m_pInterruptIn->IsAnySpaceAvailable()) {
        DWORD dwLength = sizeof(m_CommSerialStatus);
        m_CommSerialStatus.SerialState = m_wModemSetState;
        m_pInterruptIn->WriteData((PBYTE)&m_CommSerialStatus,&dwLength);
    }
    m_HardwareLock.Unlock();
}

CSerialPDD * CreateSerialObject(LPTSTR lpActivePath, PVOID pMdd,PHWOBJ pHwObj, DWORD dwDeviceArrayIndex )
{
    CSerialPDD * pSerialPDD = NULL;
    switch (dwDeviceArrayIndex) {
      case 0:
        pSerialPDD = new USBSerialFn(lpActivePath,pMdd, pHwObj);
        break;
      case 1:
        pSerialPDD = new USBSerSerialFn(lpActivePath,pMdd, pHwObj);
        break;
      default:
        ASSERT(FALSE);
    }
        
    if (pSerialPDD && !pSerialPDD->Init()) {
        delete pSerialPDD;
        pSerialPDD = NULL;
    }
    return pSerialPDD;
}
void DeleteSerialObject(CSerialPDD * pSerialPDD)
{
    if (pSerialPDD)
        delete pSerialPDD;
}

