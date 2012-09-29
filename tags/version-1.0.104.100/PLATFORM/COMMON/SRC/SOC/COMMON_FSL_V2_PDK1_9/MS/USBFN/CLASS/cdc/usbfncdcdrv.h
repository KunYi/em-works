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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

Abstract:

    Platform dependent Serial definitions for usb function
    controller.

Notes: 
--*/
#ifndef __USBFNDRV_H_
#define __USBFNDRV_H_
#include <usbfntypes.h>
#include <CMthread.h>
#include <CRegEdit.h>
#include <CSync.h>
#include <cserpdd.h>

#define REG_USB_DEVICE_CLASS_NAME   TEXT("USBFNDeviceClass")
#define REG_USB_DEVICE_CLASS_VAL    REG_DWORD
#define UMS_REG_INTERFACE_SUBCLASS_VAL      (_T("InterfaceSubClass"))
#define UMS_REG_INTERFACE_PROTOCOL_VAL      (_T("InterfaceProtocol"))
#define UMS_REG_MAX_PACKET_SIZE_0_VAL       (_T("MaxPacketSize0"))
#define UMS_REG_VENDOR_VAL                  (_T("Vendor"))
#define UMS_REG_PRODUCT_VAL                 (_T("Product"))
#define UMS_REG_TIMEOUT_VAL                 (_T("Timeout"))

#define CIC_INTERFACE_CODE                  0x02
#define DIC_INTERFACE_CODE                  0x0A
#define CIC_SUBCLASS_CODE                   0x02 //ACM 0x02
#define CIC_PROTOCOL_CODE                   0x00
#define DIC_SUBCLASS_CODE                   0x00
#define DIC_PROTOCOL_CODE                   0x00

#define DESCRIPTOR_TYPE_CS_INTERFACE        0x24
#define DESCRIPTOR_TYPE_CS_ENDPOINT         0x25

/* Descriptor SubType in Communications Class Functional Descriptors */
#define HEADER_FUNC_DESC              (0x00)
#define CALL_MANAGEMENT_FUNC_DESC     (0x01)
#define ABSTRACT_CONTROL_FUNC_DESC    (0x02)
#define DIRECT_LINE_FUNC_DESC         (0x03)
#define TELEPHONE_RINGER_FUNC_DESC    (0x04)
#define TELEPHONE_REPORT_FUNC_DESC    (0x05)
#define UNION_FUNC_DESC               (0x06)
#define COUNTRY_SELECT_FUNC_DESC      (0x07)
#define TELEPHONE_MODES_FUNC_DESC     (0x08)
#define USB_TERMINAL_FUNC_DESC        (0x09)
#define NETWORK_CHANNEL_FUNC_DESC     (0x0A)
#define PROTOCOL_UNIT_FUNC_DESC       (0x0B)
#define EXTENSION_UNIT_FUNC_DESC      (0x0C)
#define MULTI_CHANNEL_FUNC_DESC       (0x0D)
#define CAPI_CONTROL_FUNC_DESC        (0x0E)
#define ETHERNET_NETWORKING_FUNC_DESC (0x0F)
#define ATM_NETWORKING_FUNC_DESC      (0x10)
#define WIRELESS_CONTROL_FUNC_DESC    (0x11)
#define MOBILE_DIRECT_LINE_FUNC_DESC  (0x12)
#define MDLM_DETAIL_FUNC_DESC         (0x13)
#define DEVICE_MANAGEMENT_FUNC_DESC   (0x14)
#define OBEX_FUNC_DESC                (0x15)
#define COMMAND_SET_FUNC_DESC         (0x16)
#define COMMAND_SET_DETAIL_FUNC_DESC  (0x17)
#define TELEPHONE_CONTROL_FUNC_DESC   (0x18)
#define OBEX_SERVICE_ID_FUNC_DESC     (0x19)

#define MAX_CDC_FUNCTION_DESCRIPTOR_SIZE 0x100


typedef struct _USB_HEADER_FUNC_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bDescriptorSubType;
    UCHAR bcdCDC1;
    UCHAR bcdCDC2;
} USB_HEADER_FUNC_DESCRIPTOR, *PUSB_HEAD_FUNC_DESCRIPTOR;

typedef struct _USB_UNION_FUNC_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bDescriptorSubType;
    UCHAR bControllInterfaces;
    UCHAR bSubordinateInterfaces;
} USB_UNION_FUNC_DESCRIPTOR, *PUSB_UNION_FUNC_DESCRIPTOR;

typedef struct _USB_CALL_MANAGERMENT_FUNC_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bDescriptorSubType;
    UCHAR bmCapabilities;
    UCHAR bDataInterfaces;
} USB_CALL_MANAGERMENT_FUNC_DESCRIPTOR, *PUSB_CALL_MANAGERMENT_FUNC_DESCRIPTOR;

typedef struct _USB_ACM_FUNC_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bDescriptorSubType;
    UCHAR bmCapabilities;
} USB_ACM_FUNC_DESCRIPTOR, *PUSB_ACM_FUNC_DESCRIPTOR;

class CUsbFuncPipe;
class CUsbFn;
class USBSerialFn;
class CUsbFuncTransfer {
public:
    CUsbFuncTransfer(CUsbFuncPipe *hPipe, DWORD dwFlags,DWORD cbBuffer, PVOID pvBuffer, DWORD dwBufferPhysicalAddress);
    ~CUsbFuncTransfer();
    BOOL    Init();
    BOOL    IssueTransfer(DWORD dwLength = MAXDWORD);
    BOOL    IsTransferClosed() { return (m_ufnTransfer==NULL);};
    BOOL    IsTransferComplete() { return (IsTransferClosed() || WaitForTransferComplete(0));};
    BOOL    WaitForTransferComplete(DWORD dwTicks);
    HANDLE  GetCompleteEventHandle() { return m_hCompleteEvent; };
    BOOL   GetTransferStatus(PDWORD pdwBytesTranfered, PDWORD pdwError);
    BOOL   CloseTransfer();
    BOOL   AbortTransfer();
    PVOID   GetBufferPtr() { return m_pvBuffer; };
    DWORD   GetBufferSize() { return m_dwBufferSize; };
private:
    static DWORD WINAPI CompleteNotificationStub(PVOID pvNotifyParameter);
protected:
    DWORD WINAPI  CompleteNotification();
private:
    HANDLE  m_hCompleteEvent;
    const DWORD   m_dwFlags;
    CUsbFuncPipe * const m_pPipe;
    UFN_TRANSFER  m_ufnTransfer;
    const DWORD   m_dwBufferSize;
    const PVOID   m_pvBuffer;
    const DWORD   m_dwPhysAddr;
    
};
#define MAX_TRANSFER 4
class CUsbFuncPipe : public CMiniThread, public CLockObject {
public:
    CUsbFuncPipe(USBSerialFn *pSerialFn,UFN_HANDLE hDevice,PCUFN_FUNCTIONS pUfnFuncs,UCHAR bEndpointAddr,BOOL fRead,DWORD dwMaxPacketSize,  DWORD dwMaxTransferSize, DWORD dwMaxNumOfTransfer);
    ~CUsbFuncPipe();
    virtual BOOL Init();
    BOOL IsPipeOpened() { return m_hPipe!=NULL; };
    BOOL IsAnySpaceAvailable();
    BOOL OpenPipe();
    void ClosePipe();
private:
    DWORD IncIndex(DWORD dwIndex) { return ((dwIndex+1<m_dwNumOfTransfer)?dwIndex+1:0); } ;
public:
    DWORD ReadData(PUCHAR pRxBuffer,ULONG *pBufflen);  
    void WriteData(PUCHAR pRxBuffer,ULONG *pBufflen);  
    BOOL CancelTransfer();
    BOOL  TransferComplete( CUsbFuncTransfer * pTransfer) {  
#if DEBUG
        if (pTransfer) {
            for (DWORD dwIndex =0 ; dwIndex < m_dwNumOfTransfer; dwIndex++)
                if (m_pTransferArray[dwIndex] == pTransfer)
                    return TRUE;
        }
        ASSERT(FALSE);
#endif
        return TRUE;
    }
    PCUFN_FUNCTIONS GetFunctionPtr() { return m_pUfnFuncs; };
    HANDLE GetDeviceHandle() { return m_hDevice; };
    HANDLE GetPipeHandle() { return m_hPipe; };
private:
    UFN_PIPE  m_hPipe;
    const HANDLE  m_hDevice;
    const BYTE    m_bEndpointAddr;
    const BOOL    m_fRead;
    const PCUFN_FUNCTIONS m_pUfnFuncs;  
    USBSerialFn * const m_pSerialFn;
    PBYTE   m_pbBuffer;
    DWORD   m_dwBufferSize;
    DWORD   m_dwBufferPhysAddr;

    HANDLE      m_TerminateEvent;
    const DWORD m_dwNumOfTransfer;
    const DWORD m_dwTranferSize;
    const DWORD m_dwMaxPacketSize;
    DWORD       m_dwCompleteIndex;
    DWORD       m_dwWriteIndex;
    DWORD       m_dwCurPosition;
    BOOL        m_fZeroLengthNeeded;
    CUsbFuncTransfer *m_pTransferArray[MAX_TRANSFER];
private:
    virtual DWORD ThreadRun();   // IST
    void WriteDataOnce(PUCHAR pTxBuffer, ULONG *pBuffLen) ;
    
};

#define SET_CONTROL_LINE_STATE  0x22

enum CONTROL_RESPONSE {
    CR_SUCCESS = 0,
    CR_SUCCESS_SEND_CONTROL_HANDSHAKE, // Use if no data stage
    CR_STALL_DEFAULT_PIPE,
    CR_UNHANDLED_REQUEST,
};
#define ENDPOINT_NUM_OF_CIC       1
#define ENDPOINT_NUM_OF_DIC       2
#define INTERFACE_NUM_OF_ENDPOINT 3
#define STRING_NUM_OF_STRING 3
class CUsbFn  {
public:
    CUsbFn(LPCTSTR lpActivePath);
    virtual ~CUsbFn();
    BOOL    Init();
// Function related to USB Class Specific function.
    virtual void CableDetached() = 0;
    virtual void CableAttached() = 0;
    virtual DWORD ModemSignal(DWORD dwNewModemStatus) { return 0;};
// USB function
    virtual CONTROL_RESPONSE HandleClearFeature(USB_DEVICE_REQUEST udr) { return CR_SUCCESS;};
    virtual CONTROL_RESPONSE HandleClassRequest(USB_DEVICE_REQUEST udr);
// Control Line State - sent to device on default control pipe
#define USB_COMM_DTR    0x0001
#define USB_COMM_RTS    0x0002    
    void HandleRequest( DWORD dwMsg, USB_DEVICE_REQUEST udr );
protected:
    UFN_HANDLE  m_hDevice;
    UFN_FUNCTIONS   m_UfnFuncs;
    PCUFN_FUNCTIONS m_pUfnFuncs;
    PVOID       m_pvInterface;
    LPTSTR      m_lpActivePath;
    BOOL    m_fInterrupt ;        
    // Default Endpoint Handling
protected:
    virtual DWORD   StartUSBFunction();
// Info
    USB_DEVICE_DESCRIPTOR m_HighSpeedDeviceDesc;
    USB_DEVICE_DESCRIPTOR m_FullSpeedDeviceDesc;
    UFN_ENDPOINT    m_HighSpeedEndpoints[INTERFACE_NUM_OF_ENDPOINT];
    UFN_ENDPOINT    m_FullSpeedEndpoints[INTERFACE_NUM_OF_ENDPOINT];
    UFN_INTERFACE   m_HighSpeedInterface1;
    UFN_INTERFACE   m_HighSpeedInterface2;
    UFN_INTERFACE   m_FullSpeedInterface1;
    UFN_INTERFACE   m_FullSpeedInterface2;
    UFN_CONFIGURATION m_HighSpeedConfig1;
    UFN_CONFIGURATION m_HighSpeedConfig2;
    UFN_CONFIGURATION m_FullSpeedConfig1;
    UFN_CONFIGURATION m_FullSpeedConfig2;
    UFN_CLIENT_REG_INFO m_RegInfo ;
    LPCWSTR m_rgpszStrings0409[STRING_NUM_OF_STRING];
    UFN_STRING_SET m_rgStringSets ;
protected:
    UFN_PIPE m_hDefaultPipe;
    UFN_BUS_SPEED m_CurrentSpeed;
    static BOOL WINAPI DeviceNotifyStub(PVOID   pvNotifyParameter, DWORD   dwMsg, DWORD   dwParam);
    BOOL DeviceNotify(DWORD dwMsg, DWORD dwParam);
};

class USBSerialFn: public CSerialPDD ,public CUsbFn {
public:
    USBSerialFn(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj );
    ~USBSerialFn();
    virtual BOOL Init();
    DWORD   GetPriority256() { return m_dwPriority256; };
private:
    BOOL OpenPipe(CUsbFuncPipe **ppPipe,UFN_HANDLE hDevice,PCUFN_FUNCTIONS pUfnFuncs,UCHAR bEndpointAddr,BOOL fRead,DWORD dwMaxPacketSize, DWORD dwMaxTransferSize, DWORD dwMaxNumOfTransfer) {
        PREFAST_ASSERT(ppPipe!=NULL);        
        m_HardwareLock.Lock();
        if ( *ppPipe == NULL) {
            *ppPipe = new CUsbFuncPipe ( this,hDevice,pUfnFuncs,bEndpointAddr,fRead,dwMaxPacketSize,dwMaxTransferSize,dwMaxNumOfTransfer);
            if (*ppPipe && !(*ppPipe)->Init()) {
                delete *ppPipe;
                *ppPipe = NULL;
            }
            if (*ppPipe) {
                (*ppPipe)->OpenPipe();
            }
        }
        m_HardwareLock.Unlock();        
        return (*ppPipe!=NULL);
    };
    BOOL ClosePipe (CUsbFuncPipe **ppPipe) {
        PREFAST_ASSERT(ppPipe!=NULL);
        m_HardwareLock.Lock();
        CUsbFuncPipe * pPipe = *ppPipe;
        *ppPipe = NULL;
        m_HardwareLock.Unlock();
        if (pPipe)
            delete pPipe ;
        return TRUE;
    }
public:
    BOOL OpenBulkIn(UFN_HANDLE hDevice,PCUFN_FUNCTIONS pUfnFuncs,UCHAR bEndpointAddr,BOOL fRead,DWORD dwMaxPacketSize, DWORD dwMaxTransferSize, DWORD dwMaxNumOfTransfer) {
        return OpenPipe(&m_pBulkIn,hDevice,pUfnFuncs,bEndpointAddr,fRead,dwMaxPacketSize,dwMaxTransferSize,dwMaxNumOfTransfer);
    };
    BOOL OpenBulkOut(UFN_HANDLE hDevice,PCUFN_FUNCTIONS pUfnFuncs,UCHAR bEndpointAddr,BOOL fRead,DWORD dwMaxPacketSize, DWORD dwMaxTransferSize, DWORD dwMaxNumOfTransfer) {
        return OpenPipe(&m_pBulkOut,hDevice,pUfnFuncs,bEndpointAddr,fRead,dwMaxPacketSize,dwMaxTransferSize,dwMaxNumOfTransfer);
    }
    BOOL OpenInterruptIn(UFN_HANDLE hDevice,PCUFN_FUNCTIONS pUfnFuncs,UCHAR bEndpointAddr,BOOL fRead,DWORD dwMaxPacketSize, DWORD dwMaxTransferSize, DWORD dwMaxNumOfTransfer) {
        return OpenPipe(& m_pInterruptIn, hDevice,pUfnFuncs,bEndpointAddr,fRead,dwMaxPacketSize,dwMaxTransferSize,dwMaxNumOfTransfer);
    }
    BOOL CloseBulkIn() { return ClosePipe(&m_pBulkIn);};
    BOOL CloseBulkOut() { return ClosePipe(&m_pBulkOut); };
    BOOL CloseInterruptIn() { return ClosePipe(&m_pInterruptIn); };
protected:
    CUsbFuncPipe * m_pBulkIn;
    CUsbFuncPipe * m_pBulkOut;
    CUsbFuncPipe * m_pInterruptIn;
    DWORD       m_curHostModemStatus;
public:
    BOOL    EndpointNotification(CUsbFuncPipe * pPipe);
public: // USB Class Specific.
    virtual void CableDetached();
    virtual void CableAttached();
    virtual DWORD   ModemSignal(DWORD dwNewModemStatus) {
        m_curHostModemStatus = dwNewModemStatus;
        NotifyPDDInterrupt(INTR_MODEM);
        return m_curHostModemStatus;
    }
//
//Power Managment Operation
    virtual void    SerialRegisterBackup() {;};
    virtual void    SerialRegisterRestore() {;};
// 
//  Tx Function.
public:
    virtual BOOL    InitXmit(BOOL ) { return TRUE;};
    virtual void    XmitInterruptHandler(PUCHAR pTxBuffer, ULONG *pBuffLen);
    virtual void    XmitComChar(UCHAR ComChar) ;
    virtual BOOL    EnableXmitInterrupt(BOOL /*bEnable*/) { return TRUE; };
    virtual BOOL    CancelXmit() ;
//
//  Rx Function.
    virtual BOOL    InitReceive(BOOL ) { return TRUE; };
    virtual ULONG   ReceiveInterruptHandler(PUCHAR pRxBuffer,ULONG *pBufflen);
    virtual ULONG   CancelReceive() ;
// 
//  Line Function is meaningless. fake it.
    virtual BOOL    InitLine(BOOL ) { return TRUE; };
    virtual void    LineInterruptHandler() {;};
    virtual void    SetBreak(BOOL ) { ;     };
    virtual BOOL    SetBaudRate(ULONG ,BOOL ) { return TRUE; };
    virtual BOOL    SetByteSize(ULONG ) { return TRUE; };
    virtual BOOL    SetParity(ULONG ) { return TRUE; };
    virtual BOOL    SetStopBits(ULONG StopBits) { return TRUE; };
//
//  Modem
private:
//D2      DSR state  (1=Active, 0=Inactive)
//D1      CTS state  (1=Active, 0=Inactive)
//D0      Data Available  - (1=Host should read IN endpoint, 0=No data currently available)
#define USBFN_SERIAL_DSR_SET 0x4
#define USBFN_SERIAL_CTS_SET 0x2
#define USBFN_SERIAL_DATA_AVAILABLE 0x1
    void SetModemSignal(BOOL bSet, BYTE bBitSet) ;
public:
    virtual BOOL    InitModem(BOOL ) { return TRUE; };
    virtual void    ModemInterruptHandler() {GetModemStatus();} // This is Used to Indicate Modem Signal Changes.
    virtual ULONG   GetModemStatus();
    virtual void    SetDTR(BOOL bSet) { SetModemSignal(bSet, USBFN_SERIAL_DSR_SET); };
    virtual void    SetRTS(BOOL bSet) { SetModemSignal(bSet, USBFN_SERIAL_CTS_SET); };
private:
    BYTE    m_bModemSetState[2];
    BYTE    m_bOldModemState;
    DWORD   m_bOldHostModemStatus;

};

// Abstract Control Model defines
#define USB_COMM_SEND_ENCAPSULATED_COMMAND      0x0000
#define USB_COMM_GET_ENCAPSULATED_RESPONSE      0x0001
#define USB_COMM_SET_COMM_FEATURE               0x0002
#define USB_COMM_GET_COMM_FEATURE               0x0003
#define USB_COMM_CLEAR_COMM_FEATURE             0x0004
#define USB_COMM_SET_LINE_CODING                0x0020
#define USB_COMM_GET_LINE_CODING                0x0021
#define USB_COMM_SET_CONTROL_LINE_STATE         0x0022
#define USB_COMM_SEND_BREAK                     0x0023

// Line Coding Stop Bits
#define USB_COMM_STOPBITS_10                    0x0000
#define USB_COMM_STOPBITS_15                    0x0001
#define USB_COMM_STOPBITS_20                    0x0002

// Line Coding Parity Type
#define USB_COMM_PARITY_NONE                    0x0000
#define USB_COMM_PARITY_ODD                     0x0001
#define USB_COMM_PARITY_EVEN                    0x0002
#define USB_COMM_PARITY_MARK                    0x0003
#define USB_COMM_PARITY_SPACE                   0x0004

typedef struct _USB_COMM_LINE_CODING
{
    ULONG       DTERate;
    UCHAR       CharFormat;
    UCHAR       ParityType;
    UCHAR       DataBits;
} USB_COMM_LINE_CODING, *PUSB_COMM_LINE_CODING;

// Abstract Control Model Notification defines
#define USB_COMM_NETWORK_CONNECTION             0x0000
#define USB_COMM_RESPONSE_AVAILABLE             0x0001
#define USB_COMM_SERIAL_STATE                   0x0020

// Serial State Notification bits
#define USB_COMM_DCD                            0x0001
#define USB_COMM_DSR                            0x0002
#define USB_COMM_BREAK                          0x0004
#define USB_COMM_RING                           0x0008
#define USB_COMM_FRAMING_ERROR                  0x0010
#define USB_COMM_PARITY_ERROR                   0x0020
#define USB_COMM_OVERRUN                        0x0040

typedef struct _USB_COMM_SERIAL_STATUS
{
    UCHAR       RequestType;
    UCHAR       Notification;
    USHORT      Value;
    USHORT      Index;
    USHORT      Length;
    USHORT      SerialState;
} USB_COMM_SERIAL_STATUS, *PUSB_COMM_SERIAL_STATUS;


class USBSerSerialFn: public  USBSerialFn {
public:
    USBSerSerialFn(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj );
    ~USBSerSerialFn();
    virtual CONTROL_RESPONSE HandleClassRequest(USB_DEVICE_REQUEST udr);
    void SetModemSignal(BOOL bSet, BYTE bBitSet) ;
private:
    WORD m_wModemSetState;
    USB_COMM_SERIAL_STATUS m_CommSerialStatus;
    UFN_INTERFACE   m_HighSpeedInterfaceArray[2];
    UFN_INTERFACE   m_FullSpeedInterfaceArray[2];
    UFN_ENDPOINT    m_CICHighSpeedEndpoints[ENDPOINT_NUM_OF_CIC];
    UFN_ENDPOINT    m_DICHighSpeedEndpoints[ENDPOINT_NUM_OF_DIC];
    UFN_ENDPOINT    m_CICFullSpeedEndpoints[ENDPOINT_NUM_OF_CIC];
    UFN_ENDPOINT    m_DICFullSpeedEndpoints[ENDPOINT_NUM_OF_DIC];
    USB_HEADER_FUNC_DESCRIPTOR               m_headerFuncDesc;
    USB_CALL_MANAGERMENT_FUNC_DESCRIPTOR     m_cmFuncDesc;
    USB_ACM_FUNC_DESCRIPTOR                  m_acmFuncDesc;
    USB_UNION_FUNC_DESCRIPTOR                m_unionFuncDesc;
    UCHAR           m_ucFuncDesc[MAX_CDC_FUNCTION_DESCRIPTOR_SIZE];
    USHORT          m_wFuncDescSize;
public:
    virtual void    SetDTR(BOOL bSet) { SetModemSignal(bSet, USB_COMM_DSR); };
    virtual void    SetRTS(BOOL bSet) {;};
    virtual void    CableAttached();

protected:
    virtual DWORD   StartUSBFunction();
    USB_COMM_LINE_CODING m_CommLineCoding;
private:
    static DWORD WINAPI CompleteNotificationStub(PVOID pvNotifyParameter) {
        return ((USBSerSerialFn *)pvNotifyParameter)->CompleteNotification();
    }
    void ConstructConfigurationDescriptor() ;
protected:
    virtual     BOOL  IssueTransfer(PBYTE pBuffer, DWORD dwLength, BOOL fIn);
    virtual     DWORD WINAPI  CompleteNotification();
    HANDLE      m_hTransferHandle;
};
#endif


