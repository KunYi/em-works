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

        bul_usbfn.h

Abstract:

        Bulverde USB Function Driver Header.

--*/
#ifndef __BUL_USBFN_H_
#define __BUL_USBFN_H_
#include <bulverde_usbd.h>
#include <bulverde_usbotg.h>
#include <bulverde_clkmgr.h>
#include <csync.h>
#include <cmthread.h>
#include <CRegEdit.h>
#include <CRefCon.h>
#include <usbfn.h>

#define ERRORCOUNTERINREGISTRY

#ifndef SHIP_BUILD
#define STR_MODULE _T("bulverde_usbfn!")
#define SETFNAME() LPCTSTR pszFname = STR_MODULE _T(__FUNCTION__) _T(":")
#else
#define SETFNAME()
#endif

#define MAX_ENDPOINT_NUMBER 0x18

class BulUsbDevice ;
class BulEndpoint ;

#define ENDPOINT_CONTROL_STATUS_REGISTER_OFFSET (0x100/sizeof(DWORD))
#define ENDPOINT_BYTECOUNT_REGISTER_OFFSET      (0x200/sizeof(DWORD))
#define ENDPOINT_DATA_REGISTER_OFFSET           (0x300/sizeof(DWORD))
#define ENDPOINT_CONFIGURATION_REGISTER_OFFSET  (0x400/sizeof(DWORD))

#define DEVICE_CONTROL_REGISTER (0/sizeof(DWORD))
#define DEVICE_INT_CR0_REGISTER (4/sizeof(DWORD))
#define DEVICE_INT_CR1_REGISTER (8/sizeof(DWORD))
#define DEVICE_INT_SR0_REGISTER (0xc/sizeof(DWORD))
#define DEVICE_INT_SR1_REGISTER (0x10/sizeof(DWORD))
#define DEVICE_Frame_NUMBER_REGISTER (0x14/sizeof(DWORD))
#define DEVICE_OTGICR_REGISTER (0x18/sizeof(DWORD))
#define DEVICE_OTGISR_REGISTER (0x1c/sizeof(DWORD))
// Registry Value.
#define BUL_USBFUNCTION_DOUBLEBUFFER_VALNAME TEXT("DoubleBuffer")
#define BUL_USBFUNCTION_DOUBLEBUFFER_VALTYPE REG_DWORD
#define BUL_USBFUNCTION_PRIORITY_VALNAME    TEXT("Priority256")
#define BUL_USBFUNCTION_PRIORITY_VALTYPE    REG_DWORD
// Debugging only registry.
#define BUL_USBFUNCTION_EP0_STALL_COUNTER_VALNAME TEXT("EP0StallCounter")
#define BUL_USBFUNCTION_EP0_STALL_COUNTER_VALTYPE REG_DWORD
#define BUL_USBFUNCTION_BAD_SETUP_COUNTER_VALNAME TEXT("BadSetupCounter")
#define BUL_USBFUNCTION_BAD_SETUP_COUNTER_VALTYPE REG_DWORD

#define BUL_USBFUNCTION_DEFAULT_PRIORITY    100
class CEndpointContainer : public CStaticContainer <BulEndpoint, MAX_ENDPOINT_NUMBER>
{
};
class BulOTGEventThread;
class BulUsbDevice :public CEndpointContainer, public CRegistryEdit, public CMiniThread {
public:
    BulUsbDevice(LPCTSTR lpActivePath);
    virtual ~BulUsbDevice();
    virtual DWORD Init(PVOID pvMddContext,
        PUFN_MDD_INTERFACE_INFO pMddInterfaceInfo, PUFN_PDD_INTERFACE_INFO pPddInterfaceInfo);
//  PDD interface.
    virtual BOOL DeleteAllEndpoint();
// Endpoint Function.
    virtual DWORD IsEndpointSupportable (DWORD dwEndpoint,UFN_BUS_SPEED Speed,PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc,
            BYTE bConfigurationValue=1, BYTE bInterfaceNumber=0, BYTE bAlternateSetting=0 );
    virtual DWORD InitEndpoint(DWORD dwEndpoint,UFN_BUS_SPEED Speed,PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc,
            BYTE bConfigurationValue=1, BYTE bInterfaceNumber=0, BYTE bAlternateSetting=0 );
    virtual DWORD DeinitEndpoint(DWORD dwEndpoint );
    virtual DWORD StallEndpoint(DWORD dwEndpoint );
    virtual DWORD ClearEndpointStall( DWORD dwEndpoint );
    virtual DWORD ResetEndpoint(DWORD dwEndpoint );
    virtual DWORD IsEndpointHalted( DWORD dwEndpoint, PBOOL pfHalted );
    virtual DWORD IssueTransfer(DWORD  dwEndpoint,PSTransfer pTransfer );
    virtual DWORD AbortTransfer(DWORD dwEndpoint, PSTransfer pTransfer);
    
//  Endpoint Zero Special
    virtual DWORD SendControlStatusHandshake(DWORD dwEndpoint);
//  Device Function.
    virtual DWORD Start();
    virtual DWORD Stop();
    virtual BOOL IsCableAttached() { return TRUE; };
    virtual DWORD  SetAddress( BYTE  bAddress );
    virtual void PowerDown();
    virtual void PowerUp() ;
    virtual void  SetPowerState( CEDEVICE_POWER_STATE cpsNew ) ;
    virtual DWORD IOControl( IOCTL_SOURCE source, DWORD dwCode, PBYTE  pbInBuf, DWORD cbInBuf, PBYTE pbOutBuf, DWORD cbOutBuf,PDWORD  pcbActualOutBuf );
    void    OTGSetupFeature() {
        m_fOTGSetupFeature = TRUE;
        ISTProcess();
    }
    
//  Register Access.
    void WriteUDCRegister(DWORD dwOffset, DWORD dwData) { 
        PREFAST_ASSERT(m_pUsbDevReg!=NULL);
        WRITE_REGISTER_ULONG(m_pUsbDevReg + dwOffset, dwData);
    }
    DWORD ReadUDCRegister(DWORD dwOffset) {
        PREFAST_ASSERT(m_pUsbDevReg!=NULL);
        return READ_REGISTER_ULONG(m_pUsbDevReg + dwOffset);
    }
    void WriteUDCRegisterByte(DWORD dwOffset, BYTE bData) {
        PREFAST_ASSERT(m_pUsbDevReg!=NULL);
        WRITE_REGISTER_UCHAR((PUCHAR)(m_pUsbDevReg + dwOffset),bData);
    }
    BYTE ReadUDCRegisterByte(DWORD dwOffset) {
        PREFAST_ASSERT(m_pUsbDevReg!=NULL);
        return READ_REGISTER_UCHAR ((PUCHAR)(m_pUsbDevReg + dwOffset));
    }

// Device Register Access.
    DWORD   ReadControlRegister() {  return ReadUDCRegister(DEVICE_CONTROL_REGISTER); }
    void    WriteControlRegister(DWORD dwData) { WriteUDCRegister(DEVICE_CONTROL_REGISTER, dwData);}
    DWORD   ReadIntrCtr0Register() { return ReadUDCRegister(DEVICE_INT_CR0_REGISTER); }
    DWORD   ReadIntrCtr1Register() { return ReadUDCRegister(DEVICE_INT_CR1_REGISTER); }
    DWORD   ReadIntrStatus0Register() { return ReadUDCRegister(DEVICE_INT_SR0_REGISTER); }
    DWORD   ReadIntrStatus1Register() { return ReadUDCRegister(DEVICE_INT_SR1_REGISTER); }
    void    WriteIntrCtr0Register(DWORD dwData) { WriteUDCRegister(DEVICE_INT_CR0_REGISTER, dwData); }
    void    WriteIntrCtr1Register(DWORD dwData) { WriteUDCRegister(DEVICE_INT_CR1_REGISTER, dwData); }
    void    WriteIntrStatus0Register(DWORD dwData) { WriteUDCRegister(DEVICE_INT_SR0_REGISTER,dwData); }
    void    WriteIntrStatus1Register(DWORD dwData) { WriteUDCRegister(DEVICE_INT_SR1_REGISTER,dwData); }

    DWORD   ReadOTGICR() { return ReadUDCRegister(DEVICE_OTGICR_REGISTER); };
    void    WriteOTGICR(DWORD dwData) { WriteUDCRegister(DEVICE_OTGICR_REGISTER,dwData); }
    DWORD   ReadOTGISR() { return ReadUDCRegister(DEVICE_OTGISR_REGISTER); };
    void    WriteOTGISR(DWORD dwData) { WriteUDCRegister(DEVICE_OTGISR_REGISTER,dwData); };
    // Interrupt
    BOOL    EnableEndpointInterrupt(DWORD dwEndpointIndex,BOOL bEnable);
    DWORD   GetEndpointIntrStatus(DWORD dwEndpointIndex);


    // 
    void MddTransferComplete(PSTransfer pTransfer) {
        SETFNAME();
        if (m_pvMddContext && pTransfer) {
            DEBUGMSG(ZONE_FUNCTION, (_T("%s MddTransferComplete pTransfer:0x%x"),pszFname,pTransfer));
            m_pfnNotify(m_pvMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD) pTransfer);
        }
    }
    BOOL DeviceNotification( DWORD dwMsg, DWORD dwParam ) {
        SETFNAME();
        if (m_pvMddContext) {
            DEBUGMSG(ZONE_FUNCTION, (_T("%s DeviceNotification dwMsg:0x%x,dwParam:0x%x"),pszFname,dwMsg,dwParam));
            return m_pfnNotify(m_pvMddContext, dwMsg, dwParam);
        }
        else {
            DebugBreak();
            return FALSE;
        }
    }
public:
    // Debugging function.
    void IncEp0StallCounter() {
#ifdef ERRORCOUNTERINREGISTRY
        DWORD dwEp0StallCounter = 0;
        if (!GetRegValue(BUL_USBFUNCTION_EP0_STALL_COUNTER_VALNAME,(LPBYTE) &dwEp0StallCounter,sizeof(dwEp0StallCounter))) {
            dwEp0StallCounter = 0;
        }
        dwEp0StallCounter++;
        RegSetValueEx( BUL_USBFUNCTION_EP0_STALL_COUNTER_VALNAME,BUL_USBFUNCTION_EP0_STALL_COUNTER_VALTYPE,
            (PBYTE)&dwEp0StallCounter,sizeof(dwEp0StallCounter));
#endif
    }
    void IncBadSetupCounter() {
#ifdef ERRORCOUNTERINREGISTRY
        DWORD dwBadSetupCounter = 0;
        if (!GetRegValue(BUL_USBFUNCTION_BAD_SETUP_COUNTER_VALNAME,(LPBYTE) &dwBadSetupCounter,sizeof(dwBadSetupCounter))) {
            dwBadSetupCounter = 0;
        }
        dwBadSetupCounter++;
        RegSetValueEx( BUL_USBFUNCTION_BAD_SETUP_COUNTER_VALNAME, BUL_USBFUNCTION_BAD_SETUP_COUNTER_VALTYPE,
            (PBYTE)&dwBadSetupCounter,sizeof(dwBadSetupCounter));
#endif
    }
protected: 
    BOOL  m_fResumeOccurred;

    //virtual BulEndpoint * GetEndpointBy(DWORD dwEndpointIndex) {        
    //    return  (dwEndpointIndex<MAX_ENDPOINT_NUMBER ? m_EndpointArray[dwEndpointIndex]:NULL);
    //}
    virtual void PowerMgr(BOOL bOff);
    
    volatile PULONG  m_pUsbDevReg;
    volatile PBULVERDE_CLKMGR_REG m_pDCCLKReg;
    BOOL    m_fIsCableAttached;
    //BulEndpoint *   m_EndpointArray[MAX_ENDPOINT_NUMBER];

    // IST
    DWORD       m_dwSysIntr;
    HANDLE      m_hISTEvent;
    DWORD       m_dwPriority;

    BOOL        m_fDoubleBuffer;
    PVOID       m_pvMddContext;

    // Protected Fucntion
    BOOL        HardwareInit();
    BOOL        ReInit(); // For Cable Detach & Attach , We have to re-init the Device Controller.
    // Device Info.
    DWORD       m_dwCurConfigure;
    DWORD       m_dwCurInterface;

    PFN_UFN_MDD_NOTIFY      m_pfnNotify;
    CEDEVICE_POWER_STATE    m_CurPowerState;
    HANDLE                  m_hParent;

    CLockObject m_ISTLock;
    void    ISTProcess();
    BOOL    m_fOTGSetupFeature;
private:
    DWORD   ThreadRun();
    UDCCR   m_prevUDCR;
    HANDLE  m_hOTGEvent;
    BulOTGEventThread * m_pOTGEventThread;
};

class BulOTGEventThread: public CMiniThread {
public :
    BulOTGEventThread(  BulUsbDevice& BulverUSBDevice, HANDLE& hOTGEvent,DWORD dwPrority);
    ~BulOTGEventThread();
    BOOL Init() {
        CeSetPriority(m_dwPriority);
        ThreadStart();
        return TRUE;
    }
private:
    DWORD   ThreadRun();
    BulUsbDevice&   m_BulverUSBDevice;
    HANDLE&         m_hOTGEvent;
    DWORD           m_dwPriority;
};
class BulEndpoint:public CRefObject , public CLockObject {
public :
    BulEndpoint(BulUsbDevice * const pUsbDevice, const DWORD dwPipeIndex,BOOL bDoubleBuffer = TRUE ) 
    :   m_pUsbDevice(pUsbDevice)
    ,   m_dwEndpointIndex(dwPipeIndex)
    ,   m_fDoubleBuffer(bDoubleBuffer)
    {
        m_pCurTransfer = NULL;
    }
    virtual ~BulEndpoint() {
        if (m_pUsbDevice)
            m_pUsbDevice->EnableEndpointInterrupt(m_dwEndpointIndex,FALSE);
    }
    virtual BOOL Init(PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc,
            BYTE bConfigurationValue, BYTE bInterfaceNumber, BYTE bAlternateSetting);
    virtual BOOL ReInit();
    // Read & Write UDCCSR
    DWORD   ReadControlStatusRegister() {
        PREFAST_ASSERT(m_pUsbDevice!=NULL);
        PREFAST_ASSERT(m_dwEndpointIndex<MAX_ENDPOINT_NUMBER);
        return m_pUsbDevice->ReadUDCRegister( ENDPOINT_CONTROL_STATUS_REGISTER_OFFSET + m_dwEndpointIndex);
    }; 
    void    WriteControlStatusRegister(DWORD dwData) {
        PREFAST_ASSERT(m_pUsbDevice!=NULL);
        PREFAST_ASSERT(m_dwEndpointIndex<MAX_ENDPOINT_NUMBER);
        m_pUsbDevice->WriteUDCRegister( ENDPOINT_CONTROL_STATUS_REGISTER_OFFSET + m_dwEndpointIndex,dwData);
    };
    // Read UDCBCR
    DWORD   ReadByteCountRegister(){
        PREFAST_ASSERT(m_pUsbDevice!=NULL);
        PREFAST_ASSERT(m_dwEndpointIndex<MAX_ENDPOINT_NUMBER);
        return ((m_pUsbDevice->ReadUDCRegister( ENDPOINT_BYTECOUNT_REGISTER_OFFSET + m_dwEndpointIndex)) & 0x3ff);
    }
    // Read or Write UDCDR.
    BYTE    ReadDataRegisterByte() {
        PREFAST_ASSERT(m_pUsbDevice!=NULL);
        PREFAST_ASSERT(m_dwEndpointIndex<MAX_ENDPOINT_NUMBER);
        return (m_pUsbDevice->ReadUDCRegisterByte( ENDPOINT_DATA_REGISTER_OFFSET + m_dwEndpointIndex));
    }
    // Read or Write UDCDR.
    DWORD    ReadDataRegister() {
        PREFAST_ASSERT(m_pUsbDevice!=NULL);
        PREFAST_ASSERT(m_dwEndpointIndex<MAX_ENDPOINT_NUMBER);
        return ( m_pUsbDevice->ReadUDCRegister( ENDPOINT_DATA_REGISTER_OFFSET + m_dwEndpointIndex));
    }
    void    WriteDataRegisterByte(BYTE bData) {
        PREFAST_ASSERT(m_pUsbDevice!=NULL);
        PREFAST_ASSERT(m_dwEndpointIndex<MAX_ENDPOINT_NUMBER);
        m_pUsbDevice->WriteUDCRegisterByte( ENDPOINT_DATA_REGISTER_OFFSET + m_dwEndpointIndex, bData);
    }
    void    WriteDataRegister(DWORD dwData) {
        PREFAST_ASSERT(m_pUsbDevice!=NULL);
        PREFAST_ASSERT(m_dwEndpointIndex<MAX_ENDPOINT_NUMBER);
        m_pUsbDevice->WriteUDCRegister( ENDPOINT_DATA_REGISTER_OFFSET + m_dwEndpointIndex, dwData);
    }
    // Read Or Write UDCCR.
    DWORD   ReadConfigurationRegister() {
        PREFAST_ASSERT(m_pUsbDevice!=NULL);
        PREFAST_ASSERT(m_dwEndpointIndex<MAX_ENDPOINT_NUMBER);
        if (m_pUsbDevice!=0) { // Endpoint ZERO is not supported
            return m_pUsbDevice->ReadUDCRegister( ENDPOINT_CONFIGURATION_REGISTER_OFFSET + m_dwEndpointIndex );
        }
        else {
            ASSERT(FALSE);
            return 0;
        }
    }
    void    WriteConfigurationRegister(DWORD dwData) {
        PREFAST_ASSERT(m_pUsbDevice!=NULL);
        PREFAST_ASSERT(m_dwEndpointIndex<MAX_ENDPOINT_NUMBER);
        if (m_pUsbDevice!=0) { // Endpoint ZERO is not supported
            m_pUsbDevice->WriteUDCRegister( ENDPOINT_CONFIGURATION_REGISTER_OFFSET + m_dwEndpointIndex, dwData );
        }
        else {
            ASSERT(FALSE);
        }
    }

    
    // Supported Function.
    virtual DWORD InitEndpoint(UFN_BUS_SPEED Speed,PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc) {
        // We should Reset the EndPoint and clear FIFO
        ResetEndpoint();
        return (m_pUsbDevice->EnableEndpointInterrupt(m_dwEndpointIndex,TRUE)?ERROR_SUCCESS:ERROR_GEN_FAILURE);
    }
    virtual DWORD DeinitEndpoint() {
        return (m_pUsbDevice->EnableEndpointInterrupt(m_dwEndpointIndex,FALSE)?ERROR_SUCCESS:ERROR_GEN_FAILURE);
    }
    virtual DWORD StallEndpoint();
    virtual DWORD ClearEndpointStall();
    virtual DWORD ResetEndpoint();
    virtual DWORD IsEndpointHalted(PBOOL pfHalted );
    virtual DWORD IssueTransfer(PSTransfer pTransfer ) ;
    virtual DWORD AbortTransfer(PSTransfer pTransfer );
    virtual DWORD SendControlStatusHandshake() { return ERROR_INVALID_PARAMETER; };
    // Attribute.
    USB_ENDPOINT_DESCRIPTOR GetEndpointDescriptor() { return m_epDesc; };
    // IST
    virtual DWORD   IST(DWORD dwIRBit) = 0 ;
protected:
    BulUsbDevice * const    m_pUsbDevice;
    const DWORD             m_dwEndpointIndex;
    USB_ENDPOINT_DESCRIPTOR m_epDesc;
    BYTE            m_bConfigurationValue,m_bInterfaceNumber,m_bAlternateSetting;
    BOOL            m_fDoubleBuffer;
    PSTransfer      m_pCurTransfer;
    BOOL            m_fZeroPacket;
    BOOL            m_fStalled;
//  Help Function.
    PSTransfer CompleteTransfer(DWORD dwError);
    void SendFakeFeature(BYTE bReuqest,WORD wFeatureSelector);
    DWORD XmitData(PBYTE pBuffer, DWORD dwLength);
    DWORD ReceiveData(PBYTE pBuffer, DWORD dwLength);    
};
#define EP0_MAX_PACKET_SIZE 0x10 
class BulEndpointZero : public BulEndpoint {
public:
    BulEndpointZero(BulUsbDevice * const pUsbDevice, BOOL bDoubleBuffer = TRUE ) 
        : BulEndpoint(pUsbDevice, 0 ,bDoubleBuffer) 
    {
        m_bNeedAck = FALSE;
        m_bSetupDirIn = FALSE;
        m_fInIST = FALSE;
    }
    BOOL Init(PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc,
            BYTE bConfigurationValue, BYTE bInterfaceNumber, BYTE bAlternateSetting);   
    virtual BOOL ReInit();
    DWORD ResetEndpoint();
    DWORD IssueTransfer(PSTransfer pTransfer );
    DWORD SendControlStatusHandshake() ;
    DWORD   IST(DWORD dwIRBit);
protected:
    BOOL  ContinueTransfer();
    BOOL  m_bNeedAck;
    BOOL  m_bSetupDirIn;
    BOOL  m_fInIST ;
private:
    USB_DEVICE_REQUEST m_backup_udr;
    USB_DEVICE_REQUEST m_cur_udr;
    BOOL               m_fBackedudr;
};

class BulEndpointIn : public BulEndpoint  {
public:
    BulEndpointIn(BulUsbDevice * const pUsbDevice,DWORD dwEndpointIndex, BOOL bDoubleBuffer = TRUE )
        : BulEndpoint(pUsbDevice, dwEndpointIndex ,bDoubleBuffer) 
    {;
    }
    DWORD   IST(DWORD dwIRBit) ;
};

class BulEndpointOut : public BulEndpoint  {
public:
    BulEndpointOut(BulUsbDevice * const pUsbDevice,DWORD dwEndpointIndex, BOOL bDoubleBuffer = TRUE )
        : BulEndpoint(pUsbDevice, dwEndpointIndex ,bDoubleBuffer) 
    {;
    }
    DWORD   IST(DWORD dwIRBit) ;
};


BulUsbDevice * CreateBulUsbDevice(LPCTSTR lpActivePath);

#endif



