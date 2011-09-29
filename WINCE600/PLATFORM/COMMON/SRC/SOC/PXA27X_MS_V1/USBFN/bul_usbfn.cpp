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
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        bul_usbfn.cpp

Abstract:

        Bulverde USB Function Driver.

--*/

#include <windows.h>
#include <ceddk.h>
#include <ddkreg.h>
#include <nkintr.h> // needed for SYSINTR_NOP
#include "bul_usbfn.h"
#include "bulverde_base_regs.h"
#include <pm.h>

BOOL BulEndpoint::Init (PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc,
            BYTE bConfigurationValue, BYTE bInterfaceNumber, BYTE bAlternateSetting)
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    BOOL bReturn = FALSE;
    Lock();
    m_bConfigurationValue= bConfigurationValue;
    m_bInterfaceNumber = bInterfaceNumber;
    m_bAlternateSetting =bAlternateSetting;
#ifndef BULVERDE_MULTI_INTEFACE_FIXED
    m_bInterfaceNumber = 0;
    m_bAlternateSetting =0;
#endif
    m_fZeroPacket = FALSE;
    m_fStalled = FALSE;
    // Change address according
    
    if ( pEndpointDesc && m_pUsbDevice!=NULL && m_dwEndpointIndex < MAX_ENDPOINT_NUMBER) {
        pEndpointDesc->bEndpointAddress = (UCHAR)((pEndpointDesc->bEndpointAddress & 0x80) | m_dwEndpointIndex);
        m_epDesc = *pEndpointDesc;
        bReturn = ReInit();
    }
    Unlock();
    FUNCTION_LEAVE_MSG();
    return bReturn;
}
BOOL   BulEndpoint::ReInit()
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    UDCCSR udccsr;
    udccsr.ulValue = 0;
    udccsr.epbit.PC = udccsr.epbit.TRN= udccsr.epbit.SST = udccsr.epbit.FEF =1;
    WriteControlStatusRegister(udccsr.ulValue);
    DEBUGMSG(ZONE_INIT, (_T("%s (%d)udccsr.ulValue= 0x%x\r\n"),pszFname,m_dwEndpointIndex,udccsr.ulValue));
    
    UDCCRAX udccrax;
    udccrax.ulValue = 0;
    udccrax.bit.CN = (m_bConfigurationValue & 3);
    udccrax.bit.ISN = (m_bInterfaceNumber & 0x7);
    udccrax.bit.AISN = m_bAlternateSetting & 0x7;
    udccrax.bit.EN = m_epDesc.bEndpointAddress & 0xf;
    udccrax.bit.ET = m_epDesc.bmAttributes & 0x3;
    udccrax.bit.ED = ((m_epDesc.bEndpointAddress & 0x80)!=0?1:0);
    udccrax.bit.MPS = m_epDesc.wMaxPacketSize & 0x3ff ;
    
    udccrax.bit.DE = 0;
    // Only In and Isochronouse transfer can use double buffer
    if (((m_epDesc.bEndpointAddress & 0x80) != 0 || (m_epDesc.bmAttributes & 0x3) == 1 ) && m_fDoubleBuffer)
        udccrax.bit.DE = 1 ;
        
    udccrax.bit.EE = 1;
    WriteConfigurationRegister(udccrax.ulValue);
    DEBUGMSG(ZONE_INIT, (_T("%s(%d) udccrax.ulValue= 0x%x\r\n"),pszFname,m_dwEndpointIndex,udccrax.ulValue));
    FUNCTION_LEAVE_MSG();
    return TRUE;
}
DWORD BulEndpoint::StallEndpoint()
{
    UDCCSR udccsr, udccsrWrite;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    Lock();
    udccsr.ulValue = ReadControlStatusRegister();
    udccsrWrite.ulValue = 0;
    udccsrWrite.epbit.DME = udccsr.epbit.DME;
    udccsrWrite.epbit.FST = 1; // Force Stall.
    udccsrWrite.epbit.SST = 1; // Clear Previous Stall Sent if there is any.
    WriteControlStatusRegister(udccsrWrite.ulValue);
    m_fStalled = TRUE;
    Unlock();
    FUNCTION_LEAVE_MSG();
    return ERROR_SUCCESS;
}
DWORD BulEndpoint::ClearEndpointStall()
{
    Lock();
    m_fStalled = FALSE;
    Unlock();
    return ERROR_SUCCESS;
}
DWORD BulEndpoint::ResetEndpoint()
{
    UDCCSR udccsr;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    DEBUGMSG(ZONE_WARNING, (_T("ResetEndpoint+ (%d) UDCCSR =0x%x UDCCRAX = 0x%x"),m_dwEndpointIndex, ReadControlStatusRegister(),ReadConfigurationRegister()));
    Lock();
    udccsr.ulValue = ReadControlStatusRegister();
    udccsr.epbit.PC = udccsr.epbit.TRN= udccsr.epbit.SST = udccsr.epbit.FEF =1;
    WriteControlStatusRegister(udccsr.ulValue);
    Unlock();
    DEBUGMSG(ZONE_WARNING, (_T("ResetEndpoint- (%d) UDCCSR =0x%x UDCCRAX = 0x%x"),m_dwEndpointIndex, ReadControlStatusRegister(),ReadConfigurationRegister()));
    FUNCTION_LEAVE_MSG();
    return ERROR_SUCCESS;
}
DWORD BulEndpoint::IsEndpointHalted(PBOOL pfHalted )
{
    Lock();
    if (m_fStalled)
        IST(0);
    if (pfHalted)
        *pfHalted = m_fStalled;
    Unlock();
    return ERROR_SUCCESS;
}
PSTransfer BulEndpoint::CompleteTransfer(DWORD dwError)
{
    if (m_pCurTransfer) {
        PSTransfer  pCurTransfer = m_pCurTransfer;
        m_pCurTransfer->dwUsbError= dwError;
        m_pCurTransfer = NULL;
        return pCurTransfer ;
    }
    return NULL;
}

DWORD BulEndpoint::IssueTransfer(PSTransfer pTransfer )
{
    DWORD dwReturn = ERROR_SUCCESS;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    Lock();
    DEBUGMSG(ZONE_FUNCTION, (_T("ReadConfigurationRegister (%d) UDCCSR =0x%x"),m_dwEndpointIndex,ReadConfigurationRegister()));
    
    if (pTransfer!=NULL && m_pCurTransfer == NULL && (pTransfer->cbBuffer==0 || pTransfer->pvBuffer!=NULL) ) { // If it is valid.        
        if (((pTransfer->dwFlags & USB_IN_TRANSFER)!= 0 )== ((m_epDesc.bEndpointAddress & 0x80)!=0)) {
            pTransfer ->pvPddData = (PVOID)m_dwEndpointIndex;
            m_pCurTransfer = pTransfer;
            m_pCurTransfer->cbTransferred = 0;
            m_pCurTransfer->pvPddData = (PVOID) m_dwEndpointIndex;
            m_fZeroPacket = (m_pCurTransfer->cbBuffer== 0 || m_pCurTransfer->pvBuffer== 0);
            m_pUsbDevice->EnableEndpointInterrupt(m_dwEndpointIndex,TRUE);
            // Trigger First IST.
            IST(EPINT_PACKET_COMPLETE);
        }
        else 
            dwReturn = ERROR_INVALID_DATA;
    }
    else 
        dwReturn = ERROR_NOT_READY;
    Unlock();
    FUNCTION_LEAVE_MSG();
    ASSERT(dwReturn == ERROR_SUCCESS);
    return dwReturn ;
}
DWORD BulEndpoint::AbortTransfer(PSTransfer pTransfer )
{
    DWORD dwReturn = ERROR_SUCCESS;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    Lock();
    PSTransfer pNotifyTransfer = NULL;
    if (pTransfer == m_pCurTransfer && m_pCurTransfer!=NULL) {
        ResetEndpoint();
        pNotifyTransfer = CompleteTransfer(UFN_CANCELED_ERROR);
    }
    else
        dwReturn = ERROR_INVALID_DATA;
    Unlock();
    if (pNotifyTransfer)
        m_pUsbDevice->MddTransferComplete(pNotifyTransfer);
    FUNCTION_LEAVE_MSG();
    return dwReturn;
}
DWORD BulEndpoint::XmitData(PBYTE pBuffer, DWORD dwLength)
{
    SETFNAME();
    DEBUGMSG(ZONE_FUNCTION, (_T("%s pBuffer=0x%x, dwLegnth= 0x%x\r\n"),pszFname,pBuffer,dwLength));
    if (pBuffer==NULL || dwLength==0 )
        return 0;
    // Spec 12.4.2
    union {
        BYTE  bData[4];
        DWORD dwData;
    } ;
    if (dwLength > m_epDesc.wMaxPacketSize)
        dwLength = m_epDesc.wMaxPacketSize;
    
    for (DWORD dwIndex = 0; dwIndex< dwLength; ) { 
        for ( DWORD dwCount = 0; dwCount < sizeof(DWORD) && dwIndex< dwLength ; dwCount ++, dwIndex++) {
            bData[dwCount] = *(pBuffer++);
        }
        if (dwCount >= sizeof(DWORD)) {
            WriteDataRegister(dwData);
        }
        else {
            for (DWORD dwCount2=0; dwCount2 < dwCount; dwCount2 ++)
                WriteDataRegisterByte(bData[dwCount2]);
        }
    };
    DEBUGMSG(ZONE_FUNCTION, (_T("%s Complete dwLength = 0x%x\r\n"),pszFname,dwLength));
    return dwLength;
}
DWORD BulEndpoint::ReceiveData(PBYTE pBuffer, DWORD dwLength)
{
    SETFNAME();
    DEBUGMSG(ZONE_FUNCTION, (_T("%s pBuffer=0x%x, dwLegnth= 0x%x\r\n"),pszFname,pBuffer,dwLength));
    PREFAST_ASSERT(pBuffer!=NULL);
    // Spec 12.4.2
    union {
        BYTE  bData[4];
        DWORD dwData;
    } ;
    DWORD dwAvailableDataSize = ReadByteCountRegister();
    DWORD dwDataRead;
    DWORD dwIndex;
    DWORD dwTotalRead = 0;
    while (dwAvailableDataSize) {
        dwData = ReadDataRegister();
        if (dwAvailableDataSize>=sizeof(DWORD)) {
            dwAvailableDataSize -= sizeof(DWORD);
            dwDataRead = sizeof(DWORD);
        }
        else {
            dwDataRead = dwAvailableDataSize;
            dwAvailableDataSize = 0;
        }
        for (dwIndex = 0 ; dwIndex < dwDataRead && dwIndex < dwLength; dwIndex ++) {
            *(pBuffer++) = bData[dwIndex];
            dwTotalRead ++;
        }
        dwLength -= dwIndex;
    }
    DEBUGMSG(ZONE_FUNCTION, (_T("%s Complete dwTotalRead = 0x%x\r\n"),pszFname,dwTotalRead));
    return dwTotalRead;
}
void BulEndpoint::SendFakeFeature(BYTE bRequest,WORD wFeatureSelector)
{
    USB_DEVICE_REQUEST udr;
    PREFAST_DEBUGCHK(m_pUsbDevice!=NULL);
    udr.bmRequestType = USB_REQUEST_FOR_ENDPOINT;
    udr.bRequest = bRequest;
    udr.wValue = wFeatureSelector ;
    udr.wIndex = m_epDesc.bEndpointAddress ;
    udr.wLength =0;
    m_pUsbDevice->DeviceNotification( UFN_MSG_SETUP_PACKET,(DWORD) &udr);
}
BOOL BulEndpointZero:: Init( PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc,
            BYTE bConfigurationValue, BYTE bInterfaceNumber, BYTE bAlternateSetting)
{
    Lock();
    m_fBackedudr = FALSE;
    BOOL bReturn = FALSE;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    m_bConfigurationValue= bConfigurationValue;
    m_bInterfaceNumber = bInterfaceNumber;
    m_bAlternateSetting =bAlternateSetting;
    
    if ( pEndpointDesc && m_pUsbDevice!=NULL && m_dwEndpointIndex < MAX_ENDPOINT_NUMBER) {
        m_epDesc = *pEndpointDesc;
        if ((m_epDesc.wMaxPacketSize & 0x3ff) >= EP0_MAX_PACKET_SIZE) {
            m_epDesc.wMaxPacketSize = pEndpointDesc->wMaxPacketSize = EP0_MAX_PACKET_SIZE;
        
            bReturn = ReInit();
        }
    }
    Unlock();
    FUNCTION_LEAVE_MSG();
    return bReturn;
}
BOOL BulEndpointZero::ReInit()
{
    m_bNeedAck = FALSE;
    m_bSetupDirIn = FALSE;
    UDCCSR udccsr;
    udccsr.ulValue = 0;        
    udccsr.ep0bit.SA = udccsr.ep0bit.SST = udccsr.ep0bit.FTF =udccsr.ep0bit.OPC=1;
    WriteControlStatusRegister(udccsr.ulValue);
    
    UDCCR udccr;
    udccr.ulValue = 0;
    udccr.bit.EMCE = 1;
    m_pUsbDevice->WriteControlRegister(udccr.ulValue);
    return TRUE;
}
DWORD BulEndpointZero::ResetEndpoint()
{
    UDCCSR udccsr;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    Lock();
    m_bNeedAck = FALSE;
    udccsr.ulValue = 0;        
    udccsr.ep0bit.SA = udccsr.ep0bit.SST = udccsr.ep0bit.FTF =udccsr.ep0bit.OPC=1;
    WriteControlStatusRegister(udccsr.ulValue);
    Unlock();
    FUNCTION_LEAVE_MSG();
    return ERROR_SUCCESS;
}
DWORD BulEndpointZero::IssueTransfer(PSTransfer pTransfer )
{   
    Lock();
    SETFNAME();
    FUNCTION_ENTER_MSG();
    DWORD dwReturn = ERROR_SUCCESS;
    if (pTransfer!=NULL && pTransfer->pvBuffer!=NULL && m_pCurTransfer == NULL) { // If it is valid.                
        m_pCurTransfer = pTransfer;
        m_pCurTransfer->cbTransferred = 0;
        m_pCurTransfer->pvPddData = (PVOID) m_dwEndpointIndex;
        
        UDCCSR udccsr;
        udccsr.ulValue = ReadControlStatusRegister();
        udccsr.ep0bit.FST = 0; //
        m_pCurTransfer->cbTransferred = 0;
        m_fZeroPacket = (m_pCurTransfer->cbBuffer== 0 || m_pCurTransfer->pvBuffer== 0);

        if (dwReturn == ERROR_SUCCESS) {
            m_pUsbDevice->EnableEndpointInterrupt(m_dwEndpointIndex,TRUE);
        }
        else {
            // We no long handle this because of return error.
            m_pCurTransfer = NULL;
        }
    }
    else 
        dwReturn = ERROR_INVALID_DATA;
    FUNCTION_LEAVE_MSG();
    Unlock();
    return dwReturn;
}
DWORD BulEndpointZero::SendControlStatusHandshake()
{
    Lock();
    SETFNAME();
    DWORD dwReturn = ERROR_SUCCESS ;
    if (m_pCurTransfer == NULL && m_bNeedAck) { // No Transfer.
        m_pUsbDevice->EnableEndpointInterrupt(m_dwEndpointIndex,TRUE);
        if (!m_bSetupDirIn) {
            UDCCSR udccsr;
            udccsr.ulValue = ReadControlStatusRegister();
            udccsr.ep0bit.SA = 0 ;
            udccsr.ep0bit.FST = 0; //
            udccsr.ep0bit.SST = 0;
            udccsr.ep0bit.IPR = 1; // Sent Zero Packet.
            udccsr.ep0bit.OPC = 0;
            WriteControlStatusRegister(udccsr.ulValue);
        }
        m_bNeedAck = FALSE;
        if (m_fBackedudr) {
            m_bNeedAck = TRUE;
            m_bSetupDirIn = ((m_backup_udr.bmRequestType & USB_REQUEST_DEVICE_TO_HOST)!=0?TRUE:FALSE);
            m_fZeroPacket = FALSE;
            m_fBackedudr = FALSE;
            m_cur_udr = m_backup_udr ;
            m_pUsbDevice->DeviceNotification(UFN_MSG_SETUP_PACKET, (DWORD) &m_backup_udr);
            if (!m_fInIST) {
                IST(EPINT_PACKET_COMPLETE);
            }
        }
        else if (m_bSetupDirIn && !m_fInIST )
            IST(EPINT_PACKET_COMPLETE);
        DEBUGMSG(ZONE_TRANSFER, (_T("%s Complete\r\n"),pszFname));
    }
    else
        DEBUGMSG(ZONE_TRANSFER, (_T("%s Skipped\r\n"),pszFname));
    Unlock();
    return ERROR_SUCCESS;
}
DWORD   BulEndpointZero::IST(DWORD dwIRBit)
{
    Lock();
    BOOL bContinue = TRUE;
    m_fInIST = TRUE;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    if ((dwIRBit & EPINT_FIFO_ERROR)!=0) { // FIFO Error. End
        DWORD dwUdccsr;
        dwUdccsr = ReadControlStatusRegister();
        DEBUGMSG(ZONE_TRANSFER, (_T("FIFO Error on Endpoint Zero UDCCSR=0x%x"),dwUdccsr));
    };
    while (bContinue && (dwIRBit & EPINT_PACKET_COMPLETE)!=0) {
        bContinue = FALSE;
        m_fStalled = FALSE; // Endpoint Zero is auto clear stall condition.
        UDCCSR udccsr;
        udccsr.ulValue = ReadControlStatusRegister();
        DEBUGMSG(ZONE_TRANSFER, (_T("Endpoint Zero ReadControlStatusRegister()=0x%x\n"),udccsr.ulValue));
        udccsr.ep0bit.FST = 0;
        if (udccsr.ep0bit.SA !=0 && udccsr.ep0bit.OPC==0) {
            m_pUsbDevice->IncBadSetupCounter();
            RETAILMSG(1, (_T("Endpoint Zero FAILED SETUP  udccsr=0x%x. Sent STALL \n"),udccsr.ulValue));
            udccsr.ep0bit.FST = 1;
        }
        if (udccsr.ep0bit.OPC) {
            if ( udccsr.ep0bit.SA ) { // This is setup Packet.
                if (m_pCurTransfer) { // Outstanding transfer.
                    //DebugBreak();
                    DEBUGMSG(ZONE_TRANSFER, (_T("Endpoint Zero Current Transfer Canceled\n")));
                    WriteControlStatusRegister( udccsr.ulValue);
                    PSTransfer pTransfer = CompleteTransfer( UFN_CANCELED_ERROR );
                    Unlock();
                    m_pUsbDevice->MddTransferComplete(pTransfer);
                    Lock();
                }
                ASSERT(ReadByteCountRegister() == sizeof (USB_DEVICE_REQUEST));
                union {
                    USB_DEVICE_REQUEST udr;
                    DWORD dw8Byte[2];
                };
                dw8Byte[0] = ReadDataRegister();
                dw8Byte[1] = ReadDataRegister();
                DEBUGMSG(ZONE_TRANSFER, (_T("Endpoint Zero Setup bmRequestType = 0xx%x, bRequest=0x%x, wValue=0x%x,wIndex=0x%x,wLength=0x%x\n"),
                    udr.bmRequestType,udr.bRequest,udr.wValue,udr.wIndex,udr.wLength));
                
                if (m_bNeedAck) { // Previous setup haven't ack yet. Disable Interrupt and wait for the Ack.
                    m_pUsbDevice->EnableEndpointInterrupt(m_dwEndpointIndex,FALSE);
                    DEBUGMSG(ZONE_TRANSFER, (_T("Endpoint Zero Disable Interrupt")));
                    m_fBackedudr = TRUE;
                    m_backup_udr = udr;
                    WriteControlStatusRegister( udccsr.ulValue);
                   continue;
                }
                else {
                    m_bNeedAck = TRUE;
                    m_bSetupDirIn = ((udr.bmRequestType & USB_REQUEST_DEVICE_TO_HOST)!=0?TRUE:FALSE);
                    bContinue = TRUE ;
                    m_fZeroPacket = FALSE;
                    m_cur_udr = udr ;
                    WriteControlStatusRegister( udccsr.ulValue);
                    m_pUsbDevice->DeviceNotification(UFN_MSG_SETUP_PACKET, (DWORD) &udr);
                }
                continue;
            }
            else if ( ReadByteCountRegister() == 0 ) { // ACK from Host.
                bContinue = TRUE; 
            }
            else if (m_pCurTransfer ) {
                if((m_pCurTransfer->dwFlags & USB_IN_TRANSFER)==0) { // Out Data.
                    BOOL bComplete =  (ReadByteCountRegister()<m_epDesc.wMaxPacketSize);
                    DWORD dwSize = (m_pCurTransfer->cbTransferred< m_pCurTransfer->cbBuffer? m_pCurTransfer->cbBuffer-m_pCurTransfer->cbTransferred: 0);
                    if (dwSize!=0) {
                        dwSize= ReceiveData((PBYTE)m_pCurTransfer->pvBuffer + m_pCurTransfer->cbTransferred , dwSize);
                        m_pCurTransfer->cbTransferred += dwSize;
                    }

                    bComplete = (bComplete || (m_pCurTransfer->cbTransferred>= m_pCurTransfer->cbBuffer));
                    // Check for the completeion.
                    if ( bComplete ) {
                        WriteControlStatusRegister( udccsr.ulValue);
                        PSTransfer pTransfer =CompleteTransfer(UFN_NO_ERROR);
                        bContinue = TRUE;
                        Unlock();
                        m_pUsbDevice->MddTransferComplete(pTransfer);
                        Lock();
                        continue;
                    }
                }
                else { // Error
                    udccsr.ep0bit.FST = 1;
                }
            }
            else { // Transfer Not queue yet. 
                //m_pUsbDevice->EnableEndpointInterrupt(m_dwEndpointIndex,FALSE);
                // Do not clean OPC
                udccsr.ep0bit.OPC = 0;
            }

        }
        
        if (udccsr.ep0bit.IPR==0 ) {
        // Because the Ep0 is half duplex. We assume the direction is correct.
            if (m_pCurTransfer && (m_pCurTransfer->dwFlags & USB_IN_TRANSFER)!=0 &&
                    m_pCurTransfer->cbTransferred <= m_pCurTransfer->cbBuffer) {
                DWORD dwTotalData = m_pCurTransfer->cbBuffer - m_pCurTransfer->cbTransferred ;
                dwTotalData = min (dwTotalData,m_epDesc.wMaxPacketSize) ;
                // Spec 12.5.5
                DWORD dwReturn = XmitData(((PBYTE)m_pCurTransfer->pvBuffer)+ m_pCurTransfer->cbTransferred, dwTotalData );
                ASSERT(dwReturn == dwTotalData);
                m_pCurTransfer->cbTransferred += dwReturn;
                if (dwTotalData< m_epDesc.wMaxPacketSize) {
                    udccsr.ep0bit.IPR = 1;
                    m_fZeroPacket = FALSE;
                }
                else 
                    m_fZeroPacket = (m_cur_udr.wLength > m_pCurTransfer->cbTransferred);
            }
        }
        else 
            udccsr.ep0bit.IPR = 0 ; // Do not set when there is packet.
            
        if (udccsr.ep0bit.SST) { // Stall happens.
            m_pUsbDevice->IncEp0StallCounter() ;
            DEBUGMSG(ZONE_TRANSFER, (_T("Stall Sent on Zero endpoint =0x%x"),m_dwEndpointIndex));
        }
        // Clean the status.
        WriteControlStatusRegister( udccsr.ulValue);
        if (m_pCurTransfer &&m_pCurTransfer->cbTransferred >= m_pCurTransfer->cbBuffer && !m_fZeroPacket ) {// Complete anyway
            PSTransfer pTransfer = CompleteTransfer(UFN_NO_ERROR);
            Unlock();
            m_pUsbDevice->MddTransferComplete(pTransfer);
            Lock();            
        }
        
    }
    m_fInIST = FALSE;
    Unlock();
    FUNCTION_LEAVE_MSG();
    return ERROR_SUCCESS;
}
DWORD   BulEndpointIn::IST(DWORD dwIRBit)
{
    Lock();
    BOOL bContinue = TRUE;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    if ((dwIRBit & EPINT_FIFO_ERROR)!=0) { // FIFO Error. End
        DWORD dwUdccsr;
        dwUdccsr = ReadControlStatusRegister();
        DEBUGMSG(ZONE_WARNING, (_T("FIFO Error on Endpoint IN(%d) UDCCSR=0x%x"),m_dwEndpointIndex,dwUdccsr));
    }

    while (bContinue) 
    { // Loop until all the event clear.
        bContinue = FALSE;
        UDCCSR udccsr;
        udccsr.ulValue = ReadControlStatusRegister();
        DEBUGMSG(ZONE_TRANSFER, (_T(" IN::IST(%d) UDCCSR=0x%x"),m_dwEndpointIndex,udccsr.ulValue));
        if (m_fStalled && (udccsr.epbit.PC!=0 || udccsr.epbit.TRN!=0) ) {
            // Stall has been clear silently. So we generate Faking Clear Feature for Endpoint Zero
            m_fStalled = FALSE;
            udccsr.epbit.PC = udccsr.epbit.TRN = udccsr.epbit.FEF = 0;
            WriteControlStatusRegister(udccsr.ulValue);
            bContinue = TRUE;
            SendFakeFeature(USB_REQUEST_CLEAR_FEATURE,USB_FEATURE_ENDPOINT_STALL);
            continue;
        }
        if ( udccsr.epbit.PC!=0 ) { // Packet Complete.
            if (udccsr.epbit.DPE!=0 ) { // Data Packet Error
                WriteControlStatusRegister(udccsr.ulValue);
                PSTransfer pTransfer = CompleteTransfer( UFN_NOT_COMPLETE_ERROR );
                Unlock();
                m_pUsbDevice->MddTransferComplete(pTransfer);
                Lock();                
                continue;
            }
            if ( m_pCurTransfer && !m_fZeroPacket &&  m_pCurTransfer->cbTransferred >= m_pCurTransfer->cbBuffer) {
                WriteControlStatusRegister(udccsr.ulValue);        
                PSTransfer pTransfer = CompleteTransfer(UFN_NO_ERROR);
                Unlock();
                m_pUsbDevice->MddTransferComplete(pTransfer);
                Lock();                
                continue;
            }
            bContinue = TRUE;
        }
        udccsr.epbit.SP = 0;
        if ( udccsr.epbit.FS!=0 && m_pCurTransfer!=NULL  && 
                (m_pCurTransfer->cbTransferred < m_pCurTransfer->cbBuffer || m_fZeroPacket) )  { // Include Eaqual for 0 packet.
            ASSERT((m_pCurTransfer->dwFlags & USB_IN_TRANSFER)!=NULL);
            DWORD dwXmitLength = (m_fZeroPacket?0:m_pCurTransfer->cbBuffer-m_pCurTransfer->cbTransferred);
            dwXmitLength = XmitData((PBYTE)m_pCurTransfer->pvBuffer + m_pCurTransfer->cbTransferred, dwXmitLength );
            m_pCurTransfer->cbTransferred += dwXmitLength;
            m_fZeroPacket = FALSE;
            if (dwXmitLength < m_epDesc.wMaxPacketSize) {
                udccsr.epbit.SP = 1;
                m_fZeroPacket = FALSE;
            }
            else
                m_fZeroPacket = FALSE;
            bContinue = TRUE;
        }
        if (udccsr.epbit.SST) {
            DEBUGMSG(ZONE_WARNING, (_T("Stall send on In Endpoint = 0x%x"),m_dwEndpointIndex));
            // We have to assume we clean stall here because this no more interrupt.
            udccsr.epbit.FST = 0 ;
            m_fStalled = FALSE;
            bContinue = TRUE;
            WriteControlStatusRegister(udccsr.ulValue);        
            SendFakeFeature(USB_REQUEST_CLEAR_FEATURE,USB_FEATURE_ENDPOINT_STALL);
            continue;
        }
        WriteControlStatusRegister(udccsr.ulValue);        
    }
    Unlock();
    FUNCTION_LEAVE_MSG();
    return ERROR_SUCCESS;
}
DWORD   BulEndpointOut::IST(DWORD dwIRBit)
{
    Lock();
    BOOL bContinue = TRUE;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    if ((dwIRBit & EPINT_FIFO_ERROR)!=0) { // FIFO Error. End
        DWORD dwUdccsr;
        dwUdccsr = ReadControlStatusRegister();
        DEBUGMSG(ZONE_WARNING, (_T("FIFO Error on Endpoint Out(%d) UDCCSR=0x%x"),m_dwEndpointIndex,dwUdccsr));
    }

    while (bContinue) {
        bContinue = FALSE;
        UDCCSR udccsr;
        udccsr.ulValue = ReadControlStatusRegister();
        DEBUGMSG(ZONE_TRANSFER, (_T(" Out::IST(%d) UDCCSR=0x%x"),m_dwEndpointIndex,udccsr.ulValue));
        if (m_fStalled && (udccsr.epbit.PC!=0 || udccsr.epbit.TRN!=0) ) {
            // Stall has been clear silently. So we generate Faking Clear Feature for Endpoint Zero
            m_fStalled = FALSE;
            udccsr.epbit.PC = udccsr.epbit.TRN = udccsr.epbit.FEF = 0;
            WriteControlStatusRegister(udccsr.ulValue);
            bContinue = TRUE;
            SendFakeFeature(USB_REQUEST_CLEAR_FEATURE,USB_FEATURE_ENDPOINT_STALL);
            continue;
        }
        // Unload in data if there is any
        if  ( udccsr.epbit.FS!=0 && m_pCurTransfer!=NULL  && 
                m_pCurTransfer->cbTransferred < m_pCurTransfer->cbBuffer)  { // Include Eaqual for 0 packet.
            DWORD dwReceiveLength = min (ReadByteCountRegister(),m_pCurTransfer->cbBuffer - m_pCurTransfer->cbTransferred);
            dwReceiveLength = ReceiveData((PBYTE)m_pCurTransfer->pvBuffer + m_pCurTransfer->cbTransferred,dwReceiveLength);
            m_pCurTransfer->cbTransferred += dwReceiveLength;
            if (m_pCurTransfer->cbTransferred >= m_pCurTransfer->cbBuffer) {
                WriteControlStatusRegister(udccsr.ulValue);
                PSTransfer pTransfer = CompleteTransfer(UFN_NO_ERROR);
                Unlock();
                m_pUsbDevice->MddTransferComplete(pTransfer);
                Lock();
                continue;
            }
            else
                bContinue = TRUE;
                
        }
        if ( udccsr.epbit.PC!=0 ) { // Packet Complete.
            if (udccsr.epbit.DPE!=0 ) { // Data Packet Error
                WriteControlStatusRegister(udccsr.ulValue);
                PSTransfer pTransfer = CompleteTransfer( UFN_NOT_COMPLETE_ERROR );
                Unlock();
                m_pUsbDevice->MddTransferComplete(pTransfer);
                Lock();                
                continue;
            }
            if ( m_pCurTransfer) {
                BOOL fShortPacket = FALSE;
                if (udccsr.epbit.BNE_BNF!=0 &&
                        m_pCurTransfer->cbTransferred < m_pCurTransfer->cbBuffer)  { // Last Try.
                    DWORD dwReceiveLength = min (ReadByteCountRegister(),m_pCurTransfer->cbBuffer - m_pCurTransfer->cbTransferred);
                    dwReceiveLength = ReceiveData((PBYTE)m_pCurTransfer->pvBuffer + m_pCurTransfer->cbTransferred,dwReceiveLength);
                    m_pCurTransfer->cbTransferred += dwReceiveLength;
                    fShortPacket = (dwReceiveLength!=0 && dwReceiveLength<m_epDesc.wMaxPacketSize) ;
                    
                }
                if (m_pCurTransfer->cbTransferred >= m_pCurTransfer->cbBuffer) {
                    WriteControlStatusRegister(udccsr.ulValue);
                    PSTransfer pTransfer = CompleteTransfer(UFN_NO_ERROR);
                    Unlock();
                    m_pUsbDevice->MddTransferComplete(pTransfer);
                    Lock();                
                    continue;
                }
                else if (udccsr.epbit.SP || fShortPacket ){
                    WriteControlStatusRegister(udccsr.ulValue);
                    PSTransfer pTransfer = CompleteTransfer(UFN_NO_ERROR);
                    Unlock();
                    m_pUsbDevice->MddTransferComplete(pTransfer);
                    Lock();                
                    continue;
                }
                bContinue = TRUE;
            }
            else { // Not ready yet.
                //m_pUsbDevice->EnableEndpointInterrupt(m_dwEndpointIndex,FALSE);
                // Do not clean anything
                udccsr.epbit.PC = 0;
                bContinue = FALSE;
            }
        }
        if (udccsr.epbit.SST) {
            DEBUGMSG(ZONE_WARNING, (_T("Stall Sent on Out endpoint =0x%x"),m_dwEndpointIndex));
            m_fStalled = TRUE;
            bContinue = TRUE;
        }
        if (udccsr.ulValue)
            WriteControlStatusRegister(udccsr.ulValue);
    }
    Unlock();
    FUNCTION_LEAVE_MSG();
    return ERROR_SUCCESS;
}
BulUsbDevice::BulUsbDevice(LPCTSTR lpActivePath )
:   CRegistryEdit(lpActivePath)
,   CMiniThread (0, TRUE)   
{
    m_pUsbDevReg = NULL;
    m_hISTEvent = NULL;
    m_pDCCLKReg = NULL;
    m_fDoubleBuffer = TRUE;
    m_pvMddContext = NULL;
    m_dwCurConfigure = MAXDWORD;
    m_dwCurInterface = MAXDWORD;
    m_pfnNotify = NULL;
    m_CurPowerState = PwrDeviceUnspecified ;
    m_hParent = CreateBusAccessHandle(lpActivePath);  
    m_pOTGEventThread = NULL;
    m_fOTGSetupFeature = FALSE;
}

BulUsbDevice::~BulUsbDevice()
{
    if (m_pOTGEventThread)
        delete m_pOTGEventThread;
    if (m_hISTEvent) {
        m_bTerminated=TRUE;
        ThreadStart();
        SetEvent(m_hISTEvent);
        ThreadTerminated(1000);
        InterruptDisable( m_dwSysIntr );         
        CloseHandle(m_hISTEvent);
    };
    for (DWORD dwIndex =0 ; dwIndex <MAX_ENDPOINT_NUMBER; dwIndex ++) {
        RemoveObjectBy( dwIndex );
    }
    
    if (m_pUsbDevReg!=NULL) {
        WriteIntrCtr0Register(0);
        WriteIntrCtr1Register(0);
        WriteControlRegister(0) ;
        MmUnmapIoSpace((PVOID)m_pUsbDevReg,0UL);
    }

    if (m_pDCCLKReg) {
        m_pDCCLKReg->cken &= ~XLLP_CLKEN_USBCLIENT ;
        MmUnmapIoSpace(m_pDCCLKReg,0);
    }

    if (m_hParent)
        CloseBusAccessHandle(m_hParent);
    
}
DWORD BulUsbDevice::Init(
    PVOID                       pvMddContext,
    PUFN_MDD_INTERFACE_INFO     pMddInterfaceInfo,
    PUFN_PDD_INTERFACE_INFO     pPddInterfaceInfo
    )
{
    m_pvMddContext = pvMddContext;
    m_pfnNotify = pMddInterfaceInfo->pfnNotify;
    pPddInterfaceInfo->pvPddContext = this;
    
    if ( !IsKeyOpened())
        return ERROR_INVALID_DATA; 
    
    DDKISRINFO dii;
    DDKWINDOWINFO dwi;
    DWORD dwRet = GetWindowInfo(&dwi);
    if(dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_INIT, (_T("BulUsbDevice:: DDKReg_GetWindowInfo() failed %d\r\n"), dwRet));
        return dwRet;
    } else if(dwi.dwNumMemWindows != 1) {
        return ERROR_INVALID_DATA;
    }  else if (dwi.memWindows[0].dwLen < sizeof(BULVERDE_USBD_REG)) {
        DEBUGMSG(ZONE_INIT, (_T("memLen of 0x%x is less than required 0x%x\r\n"),
            dwi.memWindows[0].dwLen, sizeof(BULVERDE_USBD_REG)));
        return ERROR_INVALID_DATA;
    }
    m_fIsCableAttached = FALSE;
    m_fResumeOccurred = FALSE;
    if (m_pDCCLKReg == NULL ) {
        PHYSICAL_ADDRESS ioPhysicalBase = {BULVERDE_BASE_REG_PA_CLKMGR, 0 };
        m_pDCCLKReg = (PBULVERDE_CLKMGR_REG)MmMapIoSpace(ioPhysicalBase, sizeof(BULVERDE_CLKMGR_REG),FALSE);
        if (m_pDCCLKReg != NULL)
            m_pDCCLKReg->cken |= XLLP_CLKEN_USBCLIENT ;
        else
            return ERROR_INVALID_DATA;            
    }

    // Map Windows.
    // Translate to System Address.
    if (m_pUsbDevReg==NULL ) {
        PHYSICAL_ADDRESS    ioPhysicalBase = { dwi.memWindows[0].dwBase, 0};
        m_pUsbDevReg = (PULONG) MmMapIoSpace(ioPhysicalBase, dwi.memWindows[0].dwLen,FALSE);
        if (m_pUsbDevReg==NULL )
            return ERROR_INVALID_DATA;
    }
    // get ISR configuration information
    dwRet = GetIsrInfo( &dii);
    if(dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_INIT, (_T("BulUsbDevice DDKReg_GetIsrInfo() failed %d\r\n"), dwRet));
        return dwRet;
    } else if(dii.dwSysintr == SYSINTR_NOP){
        DEBUGMSG(ZONE_INIT, (_T("BulUsbDevice  no SYSINTR value specified\r\n")));
        return ERROR_INVALID_DATA;
    };

    m_dwSysIntr = dii.dwSysintr;

    m_hISTEvent= CreateEvent(0,FALSE,FALSE,NULL);
    if (m_hISTEvent!=NULL)
        InterruptInitialize(m_dwSysIntr,m_hISTEvent,0,0);
    else
       return ERROR_INVALID_DATA;
        
    // Optional Registry.
    DWORD dwDataSize = sizeof(DWORD);
    DWORD dwDoubleBuffer = 1;
    if (!GetRegValue(BUL_USBFUNCTION_DOUBLEBUFFER_VALNAME, (LPBYTE) &dwDoubleBuffer,dwDataSize)){
        dwDoubleBuffer = 1;
    };
    m_fDoubleBuffer = (dwDoubleBuffer==1);
    // Read the IST priority
    dwDataSize = sizeof(m_dwPriority);
    if (!GetRegValue(BUL_USBFUNCTION_PRIORITY_VALNAME,(LPBYTE) &m_dwPriority, dwDataSize)) {
        m_dwPriority = BUL_USBFUNCTION_DEFAULT_PRIORITY;
    }
    CeSetPriority(m_dwPriority);
    if (HardwareInit()) {
        ThreadStart();
        if (BusChildIoControl(m_hParent,IOCTL_BUS_USBOTG_BULVERDE_GET_EVENT,&m_hOTGEvent,sizeof(m_hOTGEvent)) && m_hOTGEvent!=NULL){
            m_pOTGEventThread = new BulOTGEventThread(*this,m_hOTGEvent,m_dwPriority);
            if (m_pOTGEventThread && !m_pOTGEventThread->Init()){
                delete m_pOTGEventThread;
                m_pOTGEventThread = NULL;
            }
        }
        return ERROR_SUCCESS;
    }
    else
        return ERROR_INVALID_DATA;
}
BOOL BulUsbDevice::HardwareInit()
{
    BOOL bReturn = TRUE;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    Lock();
    // Disable Hardware
    UDCCR udccr;
    udccr.ulValue = ReadControlRegister();
    udccr.bit.UDE= udccr.bit.EMCE = 0;
    WriteControlRegister(udccr.ulValue);
    WriteIntrCtr0Register(0);
    WriteIntrCtr1Register(0); 
    // Ack all outstanding interrupt.
    WriteIntrStatus0Register(0xffffffff);
    WriteIntrStatus1Register(0xffffffff);
    Unlock();
    FUNCTION_LEAVE_MSG();
    return bReturn;
}
BOOL   BulUsbDevice::ReInit() // For Cable Detach & Attach , We have to re-init the Device Controller.
{
    Lock();
    HardwareInit();
    for (DWORD dwIndex=0; dwIndex<MAX_ENDPOINT_NUMBER; dwIndex++) {
        BulEndpoint *pEndpoint = ObjectIndex(dwIndex);
        if (pEndpoint!=NULL)  {
            pEndpoint->ReInit();
            pEndpoint->DeRef();
        }
    }
        
    //PVOID pvMddContext = m_pvMddContext;
    //m_pvMddContext = NULL;
    Start();
    
    for (dwIndex=0; dwIndex<MAX_ENDPOINT_NUMBER; dwIndex++) {
        if ( RawObjectIndex(dwIndex)!=NULL) {
            EnableEndpointInterrupt(dwIndex, TRUE);
        }
    }
    
    Unlock();
    return FALSE;
}
BOOL BulUsbDevice::DeleteAllEndpoint()
{
    for (DWORD dwIndex=0; dwIndex<MAX_ENDPOINT_NUMBER; dwIndex++)
        RemoveObjectBy( dwIndex );
    return TRUE;
}

//
#define UDCIR0_MAX 0x10
BOOL   BulUsbDevice::EnableEndpointInterrupt(DWORD dwEndpointIndex,BOOL bEnable)
{
    SETFNAME();
    DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Enter. dwEndpoint:0x%x,Enable:0x%x --\r\n"), pszFname,dwEndpointIndex,bEnable));
    
    if (dwEndpointIndex < MAX_ENDPOINT_NUMBER ) {
        Lock();
        if (dwEndpointIndex <UDCIR0_MAX) {
            DWORD dwIntrCtrBit = ((EPINT_PACKET_COMPLETE | EPINT_FIFO_ERROR) <<(dwEndpointIndex*2));
            DWORD dwIntrCtl = ReadIntrCtr0Register() ;
            if (bEnable)
                dwIntrCtl |= dwIntrCtrBit;
            else 
                dwIntrCtl &= ~dwIntrCtrBit;
            WriteIntrCtr0Register(dwIntrCtl);
        }
        else {
            dwEndpointIndex -= UDCIR0_MAX;
            DWORD dwIntrCtrBit = ((EPINT_PACKET_COMPLETE | EPINT_FIFO_ERROR) <<(dwEndpointIndex*2));
            DWORD dwIntrCtl = ReadIntrCtr1Register() ;
            if (bEnable)
                dwIntrCtl |= dwIntrCtl;
            else 
                dwIntrCtl &= ~dwIntrCtl;
            WriteIntrCtr1Register(dwIntrCtl);
        }
        Unlock();
        return TRUE;
    }
    return FALSE;
}
DWORD  BulUsbDevice::GetEndpointIntrStatus(DWORD dwEndpointIndex)
{
    DWORD dwReturnStatus = 0;
    if (dwEndpointIndex < MAX_ENDPOINT_NUMBER ) {
        if (dwEndpointIndex <UDCIR0_MAX) {
            dwReturnStatus = (ReadIntrStatus0Register()>>(dwEndpointIndex*2)) & (EPINT_PACKET_COMPLETE | EPINT_FIFO_ERROR);
        }
        else {
            dwEndpointIndex -= MAX_ENDPOINT_NUMBER;
            dwReturnStatus = (ReadIntrStatus1Register()>>(dwEndpointIndex*2)) & (EPINT_PACKET_COMPLETE | EPINT_FIFO_ERROR);
        }
    }
    return dwReturnStatus;
}


// Interface Function.
DWORD BulUsbDevice::IsEndpointSupportable (DWORD dwEndpoint,UFN_BUS_SPEED Speed,PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc,
            BYTE bConfigurationValue, BYTE bInterfaceNumber, BYTE bAlternateSetting )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    DEBUGMSG(ZONE_FUNCTION, (_T("%s Enter. dwEndpoint:0x%x,Speed:0x%x --\r\n"), pszFname,dwEndpoint,Speed));
    Lock();
    if ((Speed & BS_FULL_SPEED)==BS_FULL_SPEED &&  pEndpointDesc  &&
            dwEndpoint < MAX_ENDPOINT_NUMBER && RawObjectIndex(dwEndpoint)==0 ) {
        BulEndpoint *pEndpoint = NULL;
        if (dwEndpoint == 0 ) // Endpoint Zero.
             pEndpoint = new BulEndpointZero(this,m_fDoubleBuffer);
        else {
            if ( (pEndpointDesc->bEndpointAddress & 0x80)!=0) 
                pEndpoint = new BulEndpointIn(this,dwEndpoint,m_fDoubleBuffer);
            else 
                pEndpoint = new BulEndpointOut(this,dwEndpoint,m_fDoubleBuffer);
        }
        if (pEndpoint &&  pEndpoint->Init(pEndpointDesc,bConfigurationValue,bInterfaceNumber,bAlternateSetting)) {
            dwError = ERROR_SUCCESS;
            InsertObject(dwEndpoint, pEndpoint) ;
        }
        else {
            if (pEndpoint) {
                delete pEndpoint;
            }
        }
    }
    Unlock();
    ASSERT(dwError == ERROR_SUCCESS);
    DEBUGMSG(ZONE_FUNCTION, (_T("%s return errorcode = %d --\r\n"), pszFname,dwError));
    return dwError;
}
DWORD BulUsbDevice::InitEndpoint(DWORD dwEndpoint,UFN_BUS_SPEED Speed,PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc,
            BYTE bConfigurationValue, BYTE bInterfaceNumber, BYTE bAlternateSetting )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    BulEndpoint *pEndpoint = ObjectIndex(dwEndpoint);
    if (pEndpoint) {
        dwError = pEndpoint->InitEndpoint(Speed, pEndpointDesc);
        pEndpoint->DeRef();
    }
    FUNCTION_LEAVE_MSG();
    return dwError;
};

DWORD BulUsbDevice::DeinitEndpoint(DWORD dwEndpoint )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    BulEndpoint *pEndpoint = ObjectIndex(dwEndpoint);
    if (pEndpoint) {
        dwError = pEndpoint->DeinitEndpoint();
        pEndpoint->DeRef();
    }
    FUNCTION_LEAVE_MSG();
    return dwError;
    
}
DWORD BulUsbDevice::StallEndpoint(DWORD dwEndpoint )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    BulEndpoint *pEndpoint = ObjectIndex(dwEndpoint);
    if (pEndpoint) {
        dwError = pEndpoint->StallEndpoint();
        pEndpoint->DeRef();
    }
    FUNCTION_LEAVE_MSG();
    return dwError;
}
DWORD BulUsbDevice::ClearEndpointStall( DWORD dwEndpoint )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    BulEndpoint *pEndpoint = ObjectIndex(dwEndpoint);
    if (pEndpoint) {
        dwError = pEndpoint->ClearEndpointStall();
        pEndpoint->DeRef();
    }
    FUNCTION_LEAVE_MSG();
    return dwError;    
}
DWORD BulUsbDevice::ResetEndpoint(DWORD dwEndpoint )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    BulEndpoint *pEndpoint =  ObjectIndex(dwEndpoint);
    if (pEndpoint) {
        dwError = pEndpoint->ResetEndpoint();
        pEndpoint->DeRef();
    }
    FUNCTION_LEAVE_MSG();
    return dwError;    
    
}
DWORD BulUsbDevice::IsEndpointHalted( DWORD dwEndpoint, PBOOL pfHalted )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    BulEndpoint *pEndpoint =  ObjectIndex(dwEndpoint);
    if (pEndpoint) {
        dwError = pEndpoint->IsEndpointHalted(pfHalted);
        pEndpoint->DeRef();
    }
    FUNCTION_LEAVE_MSG();
    return dwError;    
}
DWORD BulUsbDevice::IssueTransfer(DWORD  dwEndpoint,PSTransfer pTransfer )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    BulEndpoint *pEndpoint =  ObjectIndex(dwEndpoint);
    if (pEndpoint) {
        dwError = pEndpoint->IssueTransfer(pTransfer);
        pEndpoint->DeRef();
    }
    FUNCTION_LEAVE_MSG();
    return dwError;    
}
DWORD BulUsbDevice::AbortTransfer(DWORD dwEndpoint, PSTransfer pTransfer)
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    if (pTransfer) {
        BulEndpoint *pEndpoint =  ObjectIndex(dwEndpoint);
        if (pEndpoint) {
            dwError = pEndpoint->AbortTransfer(pTransfer);
            pEndpoint->DeRef();
        }
    }
    FUNCTION_LEAVE_MSG();
    return dwError;    
}
    
//  Endpoint Zero Special
DWORD BulUsbDevice::SendControlStatusHandshake(DWORD dwEndpoint)
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    SETFNAME();
    FUNCTION_ENTER_MSG();
    BulEndpoint *pEndpoint =  ObjectIndex(0);
    if (pEndpoint) {
        dwError = pEndpoint->SendControlStatusHandshake();
        pEndpoint->DeRef();
    }
    FUNCTION_LEAVE_MSG();
    return dwError;    
}
//  Device Function.
DWORD BulUsbDevice::Start()
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    DWORD dwReturn = ERROR_SUCCESS;
    Lock();
    //ASSERT(m_pvMddContext==NULL);
    m_dwCurConfigure = MAXDWORD;
    m_dwCurInterface = MAXDWORD;
    // Enable Device Interrupt.
    WriteIntrCtr1Register( ReadIntrCtr1Register() | 
        (UDCISR1_IRCC | UDCISR1_IRRU | UDCISR1_IRSU |UDCISR1_IRRS ));
    // Enable UDC.
    UDCCR udccr;
    udccr.ulValue = ReadControlRegister();
    udccr.bit.OEN = 1;
    udccr.bit.UDE = 1;
    udccr.bit.EMCE = 0 ;
    WriteControlRegister(udccr.ulValue);
    udccr.ulValue = ReadControlRegister();
    ASSERT(udccr.bit.EMCE==0);
    if (udccr.bit.EMCE==1)
        dwReturn = ERROR_INVALID_PARAMETER ;
    Unlock();
    FUNCTION_LEAVE_MSG();
    return dwReturn;
}
DWORD BulUsbDevice::Stop()
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    Lock();
    //ASSERT(m_pvMddContext!=NULL);
    //m_pvMddContext = NULL;
    // Enable Device Interrupt.
    WriteIntrCtr0Register(0);
    WriteIntrCtr1Register(0);    
    Unlock();
    FUNCTION_LEAVE_MSG();
    return ERROR_SUCCESS;
    
}
DWORD  BulUsbDevice::SetAddress( BYTE  bAddress )
{
    // Do we need handle this? NO?
    return ERROR_SUCCESS;
}
void BulUsbDevice::PowerMgr(BOOL bOff)
{
    Lock();
    if (bOff) {
        // Disable Device to simulate remove from bus.
        UDCCR udccr;
        udccr.ulValue = 0;
        udccr.bit.UDE  = 0 ;
        WriteControlRegister(udccr.ulValue);
    }
    else {
        UDCCR udccr;
        udccr.ulValue = 0;
        udccr.bit.UDE = 1 ;
        udccr.bit.OEN = 1;
        WriteControlRegister(udccr.ulValue);
        SetInterruptEvent(m_dwSysIntr);
    }
    Unlock();
}

void BulUsbDevice::PowerDown() 
{
    if (m_CurPowerState == PwrDeviceUnspecified) {
        PowerMgr(TRUE) ;
    }
}
void BulUsbDevice::PowerUp()
{
    m_fResumeOccurred = TRUE;
    if (m_CurPowerState == PwrDeviceUnspecified) {
        PowerMgr(FALSE) ;
    }
}
void BulUsbDevice::SetPowerState( CEDEVICE_POWER_STATE cpsNew )
{
    SETFNAME();
    
    // Adjust cpsNew.
    if (cpsNew != m_CurPowerState ) {
        if (cpsNew == D1 || cpsNew == D2) {
            // D1 and D2 are not supported.
            cpsNew = D0;
        }
        else if (m_CurPowerState== D4) {
            // D4 can only go to D0.
            cpsNew = D0;
        }
    }

    if (cpsNew != m_CurPowerState ) {
        BOOL bBusSucceed = FALSE;
        DEBUGMSG(ZONE_FUNCTION, (_T("%s Going from D%u to D%u\r\n"), pszFname, m_CurPowerState , cpsNew));
        if ( (cpsNew < m_CurPowerState) && m_hParent) {
            bBusSucceed = SetDevicePowerState(m_hParent, cpsNew, NULL);
            if (bBusSucceed) {
                PowerMgr(FALSE);
            }
        }

        switch (cpsNew) {
        case D0:
            KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &m_dwSysIntr,sizeof(m_dwSysIntr), NULL, 0, NULL);
            break;
        case D3:
            KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &m_dwSysIntr,  sizeof(m_dwSysIntr), NULL, 0, NULL);
            break;
        case D4:
            KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &m_dwSysIntr,  sizeof(m_dwSysIntr), NULL, 0, NULL);
            break;
        }

        if ( (cpsNew > m_CurPowerState ) && m_hParent  ) {
            bBusSucceed = SetDevicePowerState(m_hParent, cpsNew, NULL);
            if (bBusSucceed && cpsNew == D4) {
                PowerMgr(TRUE);
            }
        }
        m_CurPowerState = cpsNew;
    }
}
DWORD BulUsbDevice::IOControl( IOCTL_SOURCE source, DWORD dwCode, PBYTE  pbIn, DWORD cbIn, PBYTE pbOut, DWORD cbOut,PDWORD  pcbActualOut )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet = ERROR_INVALID_PARAMETER;

    switch (dwCode) {
    case IOCTL_UFN_GET_PDD_INFO:
        if ( source != BUS_IOCTL || pbOut == NULL || cbOut != sizeof(UFN_PDD_INFO) ) {
            break;
        }

        // Not currently supported.
        break;

    case IOCTL_BUS_GET_POWER_STATE:
        if (source == MDD_IOCTL) {
            PREFAST_DEBUGCHK(pbIn);
            DEBUGCHK(cbIn == sizeof(CE_BUS_POWER_STATE));

            PCE_BUS_POWER_STATE pCePowerState = (PCE_BUS_POWER_STATE) pbIn;
            PREFAST_DEBUGCHK(pCePowerState->lpceDevicePowerState);

            DEBUGMSG(ZONE_FUNCTION, (_T("%s IOCTL_BUS_GET_POWER_STATE\r\n"), pszFname));

            *pCePowerState->lpceDevicePowerState = m_CurPowerState;

            dwRet = ERROR_SUCCESS;
        }
        break;

    case IOCTL_BUS_SET_POWER_STATE:
        if (source == MDD_IOCTL) {
            PREFAST_DEBUGCHK(pbIn);
            DEBUGCHK(cbIn == sizeof(CE_BUS_POWER_STATE));

            PCE_BUS_POWER_STATE pCePowerState = (PCE_BUS_POWER_STATE) pbIn;

            PREFAST_DEBUGCHK(pCePowerState->lpceDevicePowerState);
            DEBUGCHK(VALID_DX(*pCePowerState->lpceDevicePowerState));

            DEBUGMSG(ZONE_FUNCTION, (_T("%s IOCTL_BUS_GET_POWER_STATE(D%u)\r\n"), pszFname, *pCePowerState->lpceDevicePowerState));
            SetPowerState( *pCePowerState->lpceDevicePowerState );
            dwRet = ERROR_SUCCESS;
        }
        break;
    }
    FUNCTION_LEAVE_MSG();
    return dwRet;
}
void    BulUsbDevice::ISTProcess()
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    m_ISTLock.Lock();
    DWORD dwIntr0Status = ReadIntrStatus0Register() ;
    DWORD dwIntr1Status = ReadIntrStatus1Register() ; 
    

    WriteIntrStatus0Register(dwIntr0Status) ;
    WriteIntrStatus1Register(dwIntr1Status) ; 
    DEBUGMSG(ZONE_FUNCTION, (_T("%s : BulUsbDevice dwIntr0Status 0x%x, dwIntr1Status 0x%x, dwIntr0Ctrl=0x%x, dwIntr11Ctrl=0x%x, OTGICR=0x%x \r\n "), pszFname, dwIntr0Status,dwIntr1Status,ReadIntrCtr0Register(), ReadIntrCtr1Register()));


    if (m_fIsCableAttached != IsCableAttached() || m_fResumeOccurred ) {
        m_fIsCableAttached = IsCableAttached();

        UDCCR udccr;
        udccr.ulValue = ReadControlRegister();
        udccr.bit.EMCE = 0 ;
        if (m_fIsCableAttached || m_fResumeOccurred ) {
            DEBUGMSG(ZONE_FUNCTION, (_T("%s : Attach Detected\r\n"),pszFname));
            m_fResumeOccurred = FALSE ;
            ReInit();
        }
        else {
            DEBUGMSG(ZONE_FUNCTION, (_T("%s : Detach Detected\r\n"),pszFname));
            DeviceNotification( UFN_MSG_BUS_EVENTS, UFN_DETACH);
        }
    }
    // Processing Device Interrupt 
    if ( (dwIntr1Status & UDCISR1_IRCC)!=0 || m_fOTGSetupFeature) { // Setup Config or Setup Interface has been received.
        USB_DEVICE_REQUEST udr;
        UDCCR udccr;
        udccr.ulValue = ReadControlRegister();
        DEBUGMSG(ZONE_FUNCTION, (_T("%s : Setup Configuration and Interface received UDCCR=0x%x.\r\n"),pszFname,udccr.ulValue));
        udccr.bit.SMAC = 1;
        udccr.bit.EMCE = 0 ;
        WriteControlRegister(udccr.ulValue);
        m_fOTGSetupFeature = FALSE;
        if (udccr.bit.ACN != m_dwCurConfigure) { // COnfiguration has changed.
            udr.bmRequestType = USB_REQUEST_FOR_DEVICE;
            udr.bRequest = USB_REQUEST_SET_CONFIGURATION;
            udr.wValue = (USHORT) udccr.bit.ACN;
            udr.wIndex =  0;
            udr.wLength =0;
            m_dwCurConfigure = udccr.bit.ACN ;
            DeviceNotification( UFN_MSG_SETUP_PACKET,(DWORD) &udr);
        }
        if (udccr.bit.AIN*0x100 + udccr.bit.AAISN != m_dwCurInterface) {
            udr.bmRequestType = USB_REQUEST_FOR_DEVICE;
            udr.bRequest = USB_REQUEST_SET_INTERFACE;
            udr.wValue = (USHORT) udccr.bit.AAISN;
            udr.wIndex = (USHORT) udccr.bit.AIN;
            udr.wLength =  0;
            m_dwCurInterface = udccr.bit.AIN*0x100 + udccr.bit.AAISN ;
            // We can not do this because MDD will stall the endpoint.
            //DeviceNotification( UFN_MSG_SETUP_PACKET,(DWORD) &udr);
        }
        // For OTG.
        if (udccr.bit.BHNP !=0 &&  udccr.bit.BHNP!= m_prevUDCR.bit.BHNP) { // Host Set HNP Feature.
            DEBUGMSG(ZONE_FUNCTION, (_T("%s : BHNP Detected\r\n"),pszFname));
            udr.bmRequestType = USB_REQUEST_FOR_DEVICE;
            udr.bRequest = USB_REQUEST_SET_FEATURE ;
            udr.wValue =  USB_FEATURE_B_HNP_ENABLE;
            udr.wIndex =  0 ;
            udr.wLength =0;
            m_prevUDCR.bit.BHNP =  udccr.bit.BHNP ;
            DeviceNotification( UFN_MSG_SETUP_PACKET,(DWORD) &udr);
        }
        if (udccr.bit.AHNP !=0  && udccr.bit.AHNP != m_prevUDCR.bit.AHNP) {// Host Set HNP Support.
            DEBUGMSG(ZONE_FUNCTION, (_T("%s : AHNP Detected\r\n"),pszFname));
            udr.bmRequestType = USB_REQUEST_FOR_DEVICE;
            udr.bRequest = USB_REQUEST_SET_FEATURE ;
            udr.wValue =  USB_FEATURE_A_HNP_SUPPORT;
            udr.wIndex =  0 ;
            udr.wLength =0;
            m_prevUDCR.bit.AHNP = udccr.bit.AHNP;
            DeviceNotification( UFN_MSG_SETUP_PACKET,(DWORD) &udr);
        }
        if (udccr.bit.AALTHNP!=0 && udccr.bit.AALTHNP != m_prevUDCR.bit.AALTHNP) { // Host Set Alter HNP Support.
            DEBUGMSG(ZONE_FUNCTION, (_T("%s : AALTHNP Detected\r\n"),pszFname));
            udr.bmRequestType = USB_REQUEST_FOR_DEVICE;
            udr.bRequest = USB_REQUEST_SET_FEATURE ;
            udr.wValue =  USB_FEATURE_A_ALT_HNP_SUPPORT;
            udr.wIndex =  0 ;
            udr.wLength =0;
            m_prevUDCR.bit.AALTHNP = udccr.bit.AALTHNP;
            DeviceNotification(UFN_MSG_SETUP_PACKET,(DWORD) &udr);
        }
        
    }
    if ( dwIntr1Status & UDCISR1_IRRU) { // Resume Detected
        DEBUGMSG(ZONE_FUNCTION, (_T("%s : Resume Detected\r\n"),pszFname));
        DeviceNotification( UFN_MSG_BUS_EVENTS, UFN_RESUME);
    }
    if (dwIntr1Status & UDCISR1_IRSU ) { // Suspend Detected.
        DEBUGMSG(ZONE_FUNCTION, (_T("%s : Suspend Detected\r\n"),pszFname));
        DeviceNotification( UFN_MSG_BUS_EVENTS, UFN_SUSPEND);
    }
    if (dwIntr1Status & UDCISR1_IRRS) { // Reset Detected.
        DEBUGMSG(ZONE_FUNCTION, (_T("%s : Reset Detected\r\n"),pszFname));
        // Set DETACH First
        DeviceNotification( UFN_MSG_BUS_EVENTS, UFN_DETACH);
        // Set ATTACH.
        DeviceNotification( UFN_MSG_BUS_EVENTS, UFN_ATTACH);
        // Set Reset
        DeviceNotification( UFN_MSG_BUS_EVENTS, UFN_RESET);
        // This device can only support FULL Speed.
        DeviceNotification( UFN_MSG_BUS_SPEED, BS_FULL_SPEED);
        // The HW Filters the Set Address ... Fake it here
        DeviceNotification( UFN_MSG_SET_ADDRESS, 0xFF);
        UDCCR udccr;
        USB_DEVICE_REQUEST udr;
        udccr.ulValue = ReadControlRegister();
        if (udccr.bit.ACN == m_dwCurConfigure) { // COnfiguration has changed.
            udr.bmRequestType = USB_REQUEST_FOR_DEVICE;
            udr.bRequest = USB_REQUEST_SET_CONFIGURATION;
            udr.wValue = (USHORT) udccr.bit.ACN;
            udr.wIndex =  0;
            udr.wLength =0;
            DeviceNotification( UFN_MSG_SETUP_PACKET,(DWORD) &udr);
        }
        else { // THis device is not configured
            m_dwCurConfigure = MAXDWORD ;
        }
        if (udccr.bit.AIN*0x100 + udccr.bit.AAISN == m_dwCurInterface) {
            udr.bmRequestType = USB_REQUEST_FOR_DEVICE;
            udr.bRequest = USB_REQUEST_SET_INTERFACE;
            udr.wValue = (USHORT) udccr.bit.AAISN;
            udr.wIndex = (USHORT) udccr.bit.AIN;
            udr.wLength =  0;
            // We can not do this because MDD will stall the endpoint.
            //DeviceNotification( UFN_MSG_SETUP_PACKET,(DWORD) &udr);
        }
        else { // THis device is not configured
            m_dwCurInterface = MAXDWORD ;
        }
        m_prevUDCR.ulValue = 0 ; // Reset Should clear all previous state;
    }
    BulEndpoint *pEndpoint;
    DWORD dwIntrStatusCopy = dwIntr0Status;
    for (DWORD dwIndex =0 ; dwIndex < UDCIR0_MAX ; dwIndex ++ ) {
        if  ((dwIntrStatusCopy & ( EPINT_PACKET_COMPLETE | EPINT_FIFO_ERROR ))!=0 ) {
            if ((pEndpoint = ObjectIndex(dwIndex)) != NULL ) {
                pEndpoint->IST(dwIntrStatusCopy & ( EPINT_PACKET_COMPLETE | EPINT_FIFO_ERROR ));
                pEndpoint->DeRef();
            }
        }
        dwIntrStatusCopy >>=2;
    }
    dwIntrStatusCopy = dwIntr1Status;
    for (DWORD dwIndex =UDCIR0_MAX ; dwIndex < MAX_ENDPOINT_NUMBER ; dwIndex ++ ) {
        if  ((dwIntrStatusCopy & ( EPINT_PACKET_COMPLETE | EPINT_FIFO_ERROR ))!=0 ) {
            if ((pEndpoint = ObjectIndex(dwIndex)) != NULL ) {
                pEndpoint->IST(dwIntrStatusCopy & ( EPINT_PACKET_COMPLETE | EPINT_FIFO_ERROR ));
                pEndpoint->DeRef();
            }
        }
        dwIntrStatusCopy >>=2;
    }
    m_ISTLock.Unlock();
    FUNCTION_LEAVE_MSG();
}

#ifdef DEBUG
const DWORD cISTTimeOut = 2000;
#else
const DWORD cISTTimeOut = INFINITE ;
#endif
DWORD BulUsbDevice::ThreadRun()
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    while (!m_bTerminated && m_hISTEvent!=NULL) {
        if ( WaitForSingleObject(m_hISTEvent,cISTTimeOut) != WAIT_OBJECT_0) {
            DEBUGMSG(ZONE_FUNCTION, (_T("%s : Interrupt Time out dwIntr0Status=0x%x dwIntr1Status = 0x%x. dwIntr0Ctrl=0x%x, dwIntr11Ctrl=0x%x\r\n"),pszFname,
                ReadIntrStatus0Register(), ReadIntrStatus1Register(),ReadIntrCtr0Register(), ReadIntrCtr1Register()));
            DEBUGMSG(ZONE_FUNCTION, (_T("%s : ControlRegister() =0x%x ControlStatusRegister(0)=0x%x\r\n"),pszFname,
                 ReadControlRegister(), ReadUDCRegister( ENDPOINT_CONTROL_STATUS_REGISTER_OFFSET + 0 ) ));
            continue;
        }
        ISTProcess();
        InterruptDone(m_dwSysIntr);
    }
    FUNCTION_LEAVE_MSG();
    return 1;
}

BulOTGEventThread::BulOTGEventThread(  BulUsbDevice& BulverUSBDevice, HANDLE& hOTGEvent,DWORD dwPrority)
:   m_BulverUSBDevice(BulverUSBDevice)
,   m_hOTGEvent (hOTGEvent)
,   m_dwPriority(dwPrority)
,   CMiniThread (0, TRUE) 
{
}
BulOTGEventThread::~BulOTGEventThread()
{
    m_bTerminated=TRUE;
    ThreadStart();
    SetEvent(m_hOTGEvent);
    ThreadTerminated(1000);
}

DWORD   BulOTGEventThread::ThreadRun()
{
    while (!m_bTerminated && m_hOTGEvent!=NULL) {
        if ( WaitForSingleObject(m_hOTGEvent,INFINITE)== WAIT_OBJECT_0) {
            m_BulverUSBDevice.OTGSetupFeature();
        }
        else
            break;
    }
    return 0;
}
DWORD
WINAPI
UfnPdd_Deinit(
    PVOID pvPddContext
    )
{
    if (pvPddContext) {
        delete ((BulUsbDevice *)pvPddContext);
        return ERROR_SUCCESS ;
    }
    else
        return ERROR_INVALID_PARAMETER;;
}


DWORD 
WINAPI 
UfnPdd_IsConfigurationSupportable(
    PVOID                       pvPddContext,
    UFN_BUS_SPEED               Speed,
    PUFN_CONFIGURATION          pConfiguration
    )
{
    // TODO: Need to do anything?
    return ERROR_SUCCESS;
}


DWORD
WINAPI
UfnPdd_IsEndpointSupportable(
    PVOID                       pvPddContext,
    DWORD                       dwEndpoint,
    UFN_BUS_SPEED               Speed,
    PUSB_ENDPOINT_DESCRIPTOR    pEndpointDesc,
    BYTE                        bConfigurationValue,
    BYTE                        bInterfaceNumber,
    BYTE                        bAlternateSetting
    )
{
    // TODO: Use extra parameters
    
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->IsEndpointSupportable( dwEndpoint, Speed, pEndpointDesc ,bConfigurationValue,bInterfaceNumber,bAlternateSetting );
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_InitEndpoint(
    PVOID                       pvPddContext,
    DWORD                       dwEndpoint,
    UFN_BUS_SPEED               Speed,
    PUSB_ENDPOINT_DESCRIPTOR    pEndpointDesc,
    PVOID                       pvReserved,
    BYTE                        bConfigurationValue,
    BYTE                        bInterfaceNumber,
    BYTE                        bAlternateSetting
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->InitEndpoint( dwEndpoint, Speed, pEndpointDesc, bConfigurationValue,bInterfaceNumber,bAlternateSetting );
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_RegisterDevice(
    PVOID                           pvPddContext,
    PCUSB_DEVICE_DESCRIPTOR         pHighSpeedDeviceDesc,
    PCUFN_CONFIGURATION             pHighSpeedConfig,
    PCUSB_CONFIGURATION_DESCRIPTOR  pHighSpeedConfigDesc,
    PCUSB_DEVICE_DESCRIPTOR         pFullSpeedDeviceDesc,
    PCUFN_CONFIGURATION             pFullSpeedConfig,
    PCUSB_CONFIGURATION_DESCRIPTOR  pFullSpeedConfigDesc,
    PCUFN_STRING_SET                pStringSets,
    DWORD                           cStringSets
    )
{
    return ERROR_SUCCESS;
}


DWORD
WINAPI
UfnPdd_DeregisterDevice(
    PVOID   pvPddContext
    )
{
    if (pvPddContext) {
        ((BulUsbDevice *)pvPddContext)->DeleteAllEndpoint();
    };
    return ERROR_SUCCESS;
}


DWORD
WINAPI
UfnPdd_Start(
    PVOID        pvPddContext
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->Start();
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_Stop(
    PVOID        pvPddContext
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->Stop();
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_IssueTransfer(
    PVOID  pvPddContext,
    DWORD  dwEndpoint,
    PSTransfer pTransfer
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->IssueTransfer(dwEndpoint,pTransfer);
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_AbortTransfer(
    PVOID           pvPddContext,
    DWORD           dwEndpoint,
    PSTransfer      pTransfer
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->AbortTransfer(dwEndpoint, pTransfer);
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_DeinitEndpoint(
    PVOID           pvPddContext,
    DWORD           dwEndpoint
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->DeinitEndpoint(dwEndpoint);
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_StallEndpoint(
    PVOID           pvPddContext,
    DWORD           dwEndpoint
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->StallEndpoint(dwEndpoint);
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_ClearEndpointStall(
    PVOID           pvPddContext,
    DWORD           dwEndpoint
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->ClearEndpointStall(dwEndpoint);
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_SendControlStatusHandshake(
    PVOID           pvPddContext,
    DWORD           dwEndpoint
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->SendControlStatusHandshake(dwEndpoint);
    };
    return dwError;
}

DWORD
WINAPI
UfnPdd_SetAddress(
    PVOID pvPddContext,
    BYTE  bAddress
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->SetAddress(bAddress);
    };
    return dwError;

}

DWORD
WINAPI
UfnPdd_IsEndpointHalted(
    PVOID pvPddContext,
    DWORD dwEndpoint,
    PBOOL pfHalted
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->IsEndpointHalted(dwEndpoint,pfHalted);
    };
    return dwError;
}


DWORD
WINAPI
UfnPdd_InitiateRemoteWakeup(
    PVOID pvPddContext
    )
{
    // TODO: Fill in

    return ERROR_SUCCESS;
}


void
WINAPI
UfnPdd_PowerDown(
    PVOID pvPddContext
    )
{
    if (pvPddContext) {
        ((BulUsbDevice *)pvPddContext)->PowerDown();
    };
}

void
WINAPI
UfnPdd_PowerUp(
    PVOID pvPddContext
    )
{
    if (pvPddContext) {
        ((BulUsbDevice *)pvPddContext)->PowerUp();
    };
}


// IOCTLs with a Function value between 0x200 and 0x2FF are reserved 
// for the OEM.
DWORD
WINAPI
UfnPdd_IOControl(
    PVOID           pvPddContext,
    IOCTL_SOURCE    source,
    DWORD           dwCode,
    PBYTE           pbInBuf,
    DWORD           cbInBuf,
    PBYTE           pbOutBuf,
    DWORD           cbOutBuf,
    PDWORD          pcbActualOutBuf
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    if (pvPddContext) {
        dwError =((BulUsbDevice *)pvPddContext)->IOControl(source, dwCode, pbInBuf, cbInBuf, pbOutBuf,  cbOutBuf, pcbActualOutBuf);
    };
    return dwError;
}


// C- Converter.
extern "C"
DWORD
WINAPI
UfnPdd_Init(
    LPCTSTR                     pszActiveKey,
    PVOID                       pvMddContext,
    PUFN_MDD_INTERFACE_INFO     pMddInterfaceInfo,
    PUFN_PDD_INTERFACE_INFO     pPddInterfaceInfo
    )
{
    static const UFN_PDD_INTERFACE_INFO sc_PddInterfaceInfo = {
        UFN_PDD_INTERFACE_VERSION,
        UFN_PDD_CAPS_SUPPORTS_FULL_SPEED,
        MAX_ENDPOINT_NUMBER,
        NULL,
        
        &UfnPdd_Deinit,
        &UfnPdd_IsConfigurationSupportable,
        &UfnPdd_IsEndpointSupportable,
        &UfnPdd_InitEndpoint,
        &UfnPdd_RegisterDevice,
        &UfnPdd_DeregisterDevice,
        &UfnPdd_Start,
        &UfnPdd_Stop,
        &UfnPdd_IssueTransfer,
        &UfnPdd_AbortTransfer,
        &UfnPdd_DeinitEndpoint,
        &UfnPdd_StallEndpoint,
        &UfnPdd_ClearEndpointStall,
        &UfnPdd_SendControlStatusHandshake,
        &UfnPdd_SetAddress,
        &UfnPdd_IsEndpointHalted,
        &UfnPdd_InitiateRemoteWakeup,
        &UfnPdd_PowerDown,
        &UfnPdd_PowerUp,
        &UfnPdd_IOControl,
    };

    memcpy(pPddInterfaceInfo, &sc_PddInterfaceInfo, sizeof(sc_PddInterfaceInfo));
    
    BulUsbDevice * pDevice = CreateBulUsbDevice(pszActiveKey );
    if (pDevice && pDevice->Init( pvMddContext, pMddInterfaceInfo, pPddInterfaceInfo)==ERROR_SUCCESS) {
        return ERROR_SUCCESS;
    }
    
    if (pDevice!=NULL)
        delete pDevice;

    return ERROR_INVALID_PARAMETER;
}


extern "C"
BOOL
UfnPdd_DllEntry(
    HANDLE hDllHandle,
    DWORD  dwReason, 
    LPVOID lpReserved
    )
{
    return TRUE; // Nothing to do.
}


#ifdef DEBUG
UFN_GENERATE_DPCURSETTINGS(UFN_DEFAULT_DPCURSETTINGS_NAME, 
    _T(""), _T(""), _T(""), _T(""),
    DBG_ERROR | DBG_WARNING | DBG_INIT);
#endif

