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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  CSerpdd.cpp

Abstract:

    Serial PDD Common Code.

Notes: 
--*/

#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <notify.h>
#include <ddkreg.h>
#include <serhw.h>
#include <ser16550.h>
#include <hw16550.h>
#include <ddkreg.h>
#include <pm.h>
#include <Serdbg.h>
#include "CSerPdd.h"

#include <pnotify.h>

CSerialPDDPowerUpCallback::CSerialPDDPowerUpCallback(CSerialPDD * pSerialObj)
:   m_pSerialObj(pSerialObj)
,   CMiniThread (0, TRUE)
{
    DEBUGCHK(m_pSerialObj!=NULL);
    m_hEvent =CreateEvent(NULL,FALSE,FALSE,NULL);
    ThreadStart();
}
CSerialPDDPowerUpCallback::~CSerialPDDPowerUpCallback()
{
    m_bTerminated = TRUE;
    SignalCallback();
    ThreadTerminated(1000);
    if (m_hEvent!=NULL)
        CloseHandle(m_hEvent);
}
DWORD CSerialPDDPowerUpCallback::ThreadRun()
{
    while (!IsTerminated() && m_hEvent!=NULL && m_pSerialObj!=NULL) {
        if (WaitForSingleObject(m_hEvent,INFINITE)==WAIT_OBJECT_0) {
            if (!IsTerminated()) {
                m_pSerialObj->NotifyPDDInterrupt(INTR_NONE);
            }
        }
        else 
            ASSERT(FALSE);
        
    }
    return 0;
}

CSerialPDD::CSerialPDD(LPTSTR lpActivePath, PVOID pMdd,  PHWOBJ pHwObj  )
:   CRegistryEdit(lpActivePath)
,   m_pMdd(pMdd)
,   m_pHwObj(pHwObj)
{

    m_hParent = CreateBusAccessHandle(lpActivePath);
    m_PowerHelperHandle = INVALID_HANDLE_VALUE;
    m_hPowerLock = NULL;
    // Initial Open Count.
    m_lOpenCount = 0;
    m_ulCommErrors = 0;
    m_PowerCallbackThread = NULL;
    m_ulRxBufferSize = 0;
    memset(&m_PowerCapabilities,0,sizeof(m_PowerCapabilities));
    if (!GetRegValue(PC_REG_SERIALPRIORITY_VAL_NAME,(LPBYTE)&m_dwPriority256,sizeof(DWORD))) {
        m_dwPriority256 = DEFAULT_CE_THREAD_PRIORITY+55;
    }
}
CSerialPDD::~CSerialPDD()
{   
    InitialEnableInterrupt(FALSE); 
    if (m_PowerHelperHandle != INVALID_HANDLE_VALUE )
        DDKPwr_Deinitialize(m_PowerHelperHandle);
    if (m_hParent)
        CloseBusAccessHandle(m_hParent);
    if (m_PowerCallbackThread)
        delete m_PowerCallbackThread;
    InitialPower(FALSE);
}

BOOL CSerialPDD::Init()
{
    InitialPower(TRUE);
    if (m_PowerCallbackThread  == NULL) {
        m_PowerCallbackThread = new CSerialPDDPowerUpCallback(this);
        if ( m_PowerCallbackThread  && !m_PowerCallbackThread->Init()){
            delete m_PowerCallbackThread;
            m_PowerCallbackThread = NULL;
        }
    }
    m_PowerHelperHandle = DDKPwr_Initialize(SetPowerStateStatic, (DWORD)this , TRUE, 1000 );
    if (!GetRegValue( SERIAL_RX_BUFFER_SIZE, (LPBYTE)&m_ulRxBufferSize,sizeof(m_ulRxBufferSize))) {
        m_ulRxBufferSize = 0 ;
    }
    return (IsKeyOpened() && m_PowerCallbackThread!=NULL && m_PowerHelperHandle!=INVALID_HANDLE_VALUE );
}
void CSerialPDD::PostInit() 
{
    InitialEnableInterrupt(TRUE); 
    InitModem(TRUE);
}

BOOL CSerialPDD::Open()
{
    if (InterlockedExchange(&m_lOpenCount,1) !=0)
        return FALSE;
    
    PREFAST_ASSERT(m_PowerHelperHandle!=INVALID_HANDLE_VALUE);
    ASSERT(m_hPowerLock==NULL);
    m_hPowerLock= DDKPwr_RequestLevel( m_PowerHelperHandle, D0 );  
    ASSERT(m_hPowerLock!=NULL);
    
    SetDefaultConfiguration(); 
    InitLine(TRUE);
    InitReceive(TRUE);
    InitXmit(TRUE);
    return TRUE;
}
BOOL CSerialPDD::Close()
{
    if (InterlockedExchange(&m_lOpenCount,0) !=1)
        return FALSE;
    InitXmit(FALSE);
    InitReceive(FALSE);
    InitLine(FALSE);
    
    PREFAST_ASSERT(m_PowerHelperHandle!=INVALID_HANDLE_VALUE);
    ASSERT(m_hPowerLock!=NULL);
    DDKPwr_ReleaseLevel(m_PowerHelperHandle, m_hPowerLock);  
    m_hPowerLock=NULL;
    
    return TRUE;
}
BOOL CSerialPDD::Ioctl(DWORD dwCode,PBYTE pBufIn,DWORD dwLenIn,PBYTE pBufOut,DWORD dwLenOut,PDWORD pdwActualOut)
{
    BOOL RetVal=FALSE;
    switch ( dwCode ) {
    case IOCTL_POWER_CAPABILITIES: 
        if (!pBufOut || dwLenOut < sizeof(POWER_CAPABILITIES) || !pdwActualOut) {
            SetLastError(ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
        } else {
            m_IsThisPowerManaged= TRUE;
            PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pBufOut;        
            *ppc= GetPowerCapabilities();
            if (pdwActualOut)
                *pdwActualOut = sizeof(POWER_CAPABILITIES);
            RetVal = TRUE;
        }
        break;
    case IOCTL_POWER_SET:
        if (!pBufOut || dwLenOut < sizeof(CEDEVICE_POWER_STATE) || !pdwActualOut || m_PowerHelperHandle == INVALID_HANDLE_VALUE ) {
            SetLastError(ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
        } else {
            m_IsThisPowerManaged= TRUE;
            CEDEVICE_POWER_STATE newDx = *(PCEDEVICE_POWER_STATE) pBufOut;
            DEBUGMSG(1, (TEXT("COM: IOCTL_POWER_SET: D%d\r\n"), newDx));
            RetVal = DDKPwr_SetDeviceLevel( m_PowerHelperHandle, newDx, NULL );
            // did we set the device power?
            if(RetVal == TRUE) {
                *(PCEDEVICE_POWER_STATE)pBufOut = DDKPwr_GetDeviceLevel(m_PowerHelperHandle );
                if (pdwActualOut) {
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                }
            }
            else
                SetLastError(ERROR_INVALID_PARAMETER);
        }
        break;
    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        RetVal = FALSE;
        break;
    }
    return RetVal;
}
//
//Power Managment Operation
BOOL CSerialPDD::InitialPower(BOOL bInit)
{
    m_PowerState=D0;
    m_IsThisPowerManaged=FALSE;
    m_PowerCapabilities.DeviceDx=DX_MASK(D0)|DX_MASK(D3)|DX_MASK(D4);
    return TRUE;
}
CEDEVICE_POWER_STATE CSerialPDD::SetPowerStateStatic(DWORD dwContext, CEDEVICE_POWER_STATE powerState)
{
    CEDEVICE_POWER_STATE rState = powerState;
    if (dwContext) {
        ((CSerialPDD *)dwContext)->SetDevicePowerState(powerState);
        rState = ((CSerialPDD *)dwContext)->GetDevicePowerState();
    }
    else
        ASSERT(FALSE);
    return rState;
}
BOOL CSerialPDD::PowerOff()
{
    if (!m_IsThisPowerManaged ) {
        SerialRegisterBackup();        
        if (m_hParent ) {
            ::SetDevicePowerState( m_hParent, D4, NULL);
        }
    }
    return TRUE;
}
BOOL CSerialPDD::PowerOn()
{
    if (!m_IsThisPowerManaged ) {
        if ( m_hParent ) {
            ::SetDevicePowerState( m_hParent, D0, NULL);
        }
        SerialRegisterRestore();
    }
    m_IsResumed = 1;
    if (m_PowerCallbackThread)
        m_PowerCallbackThread->SignalCallback();
    return TRUE;
}
BOOL  CSerialPDD::SetDevicePowerState(CEDEVICE_POWER_STATE pwrState)
{
    if (m_PowerState!=pwrState && pwrState != PwrDeviceUnspecified) {
        BOOL bReturn = TRUE;
        if ((m_PowerState != D3 && m_PowerState !=D4) && (pwrState== D3 || pwrState==D4)) { // Power Off.
            SerialRegisterBackup();
        }
        if (m_hParent && ::SetDevicePowerState( m_hParent, pwrState, NULL)) {
            m_PowerState = pwrState;
            bReturn =  TRUE;
        }
        else
            bReturn =  FALSE;
        if ((m_PowerState == D3 || m_PowerState ==D4) && (pwrState!= D3 && pwrState!=D4)) { // Power On.
            SerialRegisterRestore();
            m_IsResumed = 1;
        }
        return bReturn;
    }
    return TRUE;
}
//
// Interrupt Handle
BOOL CSerialPDD::InitialEnableInterrupt(BOOL /*bEnable*/ ) 
{
    m_InterruptLock.Lock();
    m_dwInterruptFlag = INTR_NONE;
    m_InterruptLock.Unlock();
    return TRUE;
}
extern "C" VOID SerialEventHandler(PVOID pMddHead);

BOOL CSerialPDD::NotifyPDDInterrupt(INTERRUPT_TYPE interruptType)
{
    m_InterruptLock.Lock();
    // The interrupt is define as Bit event.
    m_dwInterruptFlag |= (DWORD)interruptType;
    m_InterruptLock.Unlock();
    if (IsPowerResumed ( )) {
        if (m_lOpenCount) { // If application is opened.
            EventCallback( EV_POWER );
        }
        else {
            if (GetModemStatus() & MS_RLSD_ON)
                CeEventHasOccurred (NOTIFICATION_EVENT_RS232_DETECTED, NULL);
        }
    }
    m_InterruptLock.Lock();
    SerialEventHandler(m_pMdd);
    m_InterruptLock.Unlock();
    return TRUE;
}
INTERRUPT_TYPE CSerialPDD::GetInterruptType()
{
    m_InterruptLock.Lock();
    // The interrupt is define as Bit event.
    INTERRUPT_TYPE lIntrFlagRet= (INTERRUPT_TYPE )m_dwInterruptFlag;
    m_dwInterruptFlag = INTR_NONE;
    m_InterruptLock.Unlock();
    return lIntrFlagRet;
}
BOOL CSerialPDD::DataReplaced(PBYTE puData,BOOL isBadData)
{
    BOOL bReturn = FALSE;
    if (puData) {
        UCHAR inputData= *puData;
        if (m_DCB.fDsrSensitivity && IsDSROff() ) {
        }
        else 
        if (inputData==NULL &&  m_DCB.fNull) {
        }
        else {
            bReturn = TRUE;
            if (m_DCB.fErrorChar && isBadData) {
                inputData = m_DCB.ErrorChar;
            }
            else 
            if (inputData == m_DCB.EvtChar) {
                EventCallback(EV_RXFLAG );
            }
            *puData = inputData;
        }
    }
    return bReturn;
}
BOOL CSerialPDD::EventCallback(ULONG fdwEventMask,ULONG fdwModemStatus)
{
    EvaluateEventFlag(m_pMdd,fdwEventMask);
    if (!IsOpen() && ((fdwEventMask & EV_RLSD)!=0 && (fdwModemStatus & MS_RLSD_ON)!=0)) {
        DEBUGMSG (ZONE_EVENTS,
                  (TEXT(" CSerialPDD::EventCallback (eventMask = 0x%x, modemstatus = 0x%x) - device was closed\r\n"),
                  fdwEventMask,fdwModemStatus));
        CeEventHasOccurred (NOTIFICATION_EVENT_RS232_DETECTED, NULL);
    }
    return TRUE;
}

// For PurgeComm. The PDD only handle The clean up the hardware.
BOOL CSerialPDD::PurgeComm( DWORD fdwAction)
{
    if (fdwAction & PURGE_RXCLEAR ) {
        CancelReceive();
    }
    if (fdwAction & PURGE_TXCLEAR ) {
        CancelXmit();
    }
    return TRUE;
}
void CSerialPDD::SetReceiveError(ULONG ulNewErrors)
{
    if ( ulNewErrors!=0) {
        m_HardwareLock.Lock();
        m_ulCommErrors |= ulNewErrors;
        m_HardwareLock.Unlock();
        EventCallback(EV_ERR);
    }
}
ULONG CSerialPDD::GetReceivedError()
{
    m_HardwareLock.Lock();
    ULONG ulReturn =  m_ulCommErrors ;
    m_ulCommErrors  = 0;
    m_HardwareLock.Unlock();
    return ulReturn;
}

void CSerialPDD::SetDefaultConfiguration()
{
    // Default Value. Can be altered.
    m_CommPorp.wPacketLength       = 0xffff;
    m_CommPorp.wPacketVersion      = 0xffff;
    m_CommPorp.dwServiceMask       = SP_SERIALCOMM;
    m_CommPorp.dwReserved1         = 0;
    m_CommPorp.dwMaxTxQueue        = 16;
    m_CommPorp.dwMaxRxQueue        = 16;
    m_CommPorp.dwMaxBaud           = BAUD_115200;
    m_CommPorp.dwProvSubType       = PST_RS232;
    m_CommPorp.dwProvCapabilities  =
        PCF_DTRDSR | PCF_RLSD | PCF_RTSCTS |
        PCF_SETXCHAR |
        PCF_INTTIMEOUTS |
        PCF_PARITY_CHECK |
        PCF_SPECIALCHARS |
        PCF_TOTALTIMEOUTS |
        PCF_XONXOFF;
    m_CommPorp.dwSettableBaud      =
        BAUD_075 | BAUD_110 | BAUD_150 | BAUD_300 | BAUD_600 |
        BAUD_1200 | BAUD_1800 | BAUD_2400 | BAUD_4800 |
        BAUD_7200 | BAUD_9600 | BAUD_14400 |
        BAUD_19200 | BAUD_38400 | BAUD_56K | BAUD_128K |
        BAUD_115200 | BAUD_57600 | BAUD_USER;
    m_CommPorp.dwSettableParams    =
        SP_BAUD | SP_DATABITS | SP_HANDSHAKING | SP_PARITY |
        SP_PARITY_CHECK | SP_RLSD | SP_STOPBITS;
    m_CommPorp.wSettableData       =
        DATABITS_5 | DATABITS_6 | DATABITS_7 | DATABITS_8;
    m_CommPorp.wSettableStopParity =
        STOPBITS_10 | STOPBITS_20 |
        PARITY_NONE | PARITY_ODD | PARITY_EVEN | PARITY_SPACE |
        PARITY_MARK;
    // Setup m_DCB.
    
    // Set Detault Parameter.
    SetOutputMode(FALSE, TRUE );    // No IR.
    // For DCB. The PDD only need to take care BaudRate, ByteSize Parity & StopBit
    m_DCB.DCBlength  = sizeof(DCB);
    SetBaudRate(m_DCB.BaudRate   = 9600,FALSE);
    SetByteSize(m_DCB.ByteSize   = 8);
    SetParity(m_DCB.Parity     = NOPARITY);
    SetStopBits(m_DCB.StopBits   = ONESTOPBIT);
    
}
BOOL CSerialPDD::SetDCB(LPDCB lpDCB)
{
    BOOL bReturn = TRUE;
    if (IsOpen() && lpDCB && lpDCB->DCBlength == sizeof(DCB)) {
        
        m_HardwareLock.Lock();
        DCB LocalDCB = *lpDCB;
        if (lpDCB->BaudRate != m_DCB.BaudRate) {
            if (lpDCB->BaudRate == 0 || !SetBaudRate(lpDCB->BaudRate, m_fIREnable)) {
                LocalDCB.BaudRate = m_DCB.BaudRate ;
                bReturn = FALSE;
            }
        }
        if (lpDCB->ByteSize != m_DCB.ByteSize ) {
            if (!SetByteSize(lpDCB->ByteSize))  {
                LocalDCB.ByteSize = m_DCB.ByteSize;
                bReturn = FALSE;
            }
        }
        if (lpDCB->Parity != m_DCB.Parity) {
            if (!SetParity(lpDCB->Parity)) {
                LocalDCB.Parity = m_DCB.Parity ;
                bReturn=FALSE;
            }
        }
        if (lpDCB->StopBits != m_DCB.StopBits) {
            if (!SetStopBits(lpDCB->StopBits)) {
                LocalDCB.StopBits = m_DCB.StopBits;
                bReturn = FALSE;
            }
        }
        m_DCB = LocalDCB;
        m_HardwareLock.Unlock();        
    }
    else
        bReturn = FALSE;
    return bReturn;
};

// Default Rate to Divisor Converter.
BOOL CSerialPDD::GetDivisorOfRate(ULONG BaudRate,PULONG pulDivisor)
{
static const
PAIRS    s_LS_BaudPairs[] =    {
    {50,        2307},
    {75,        1538},
    {110,        1049},
    {135,        858},
    {150,        769},
    {300,        384},
    {600,        192},
    {1200,        96},
    {1800,        64},
    {2000,        58},
    {2400,        48},
    {3600,        32},
    {4800,        24},
    {7200,        16},
    {9600,        12},
    {12800,        9},
    {14400,        8},
    {19200,     6},
    {23040,     5},
    {28800,     4},
    {38400,     3},
    {57600,     2},
    {115200,    1}
};
    for (DWORD dwIndex =0 ; dwIndex <dim(s_LS_BaudPairs) && s_LS_BaudPairs[dwIndex].Key<=BaudRate; dwIndex ++) {
        if (s_LS_BaudPairs[dwIndex].Key== BaudRate){
            if (pulDivisor)
                *pulDivisor = s_LS_BaudPairs[dwIndex].AssociatedValue;
            return TRUE;
        }
    }
    return FALSE;
    
}
//--------------------------------------------------------------------------------------------------------
//
// Converting C Style to C++ Serial Object.
/*
 @doc OEM 
 @func PVOID | SerInit | Initializes device identified by argument.
 *  This routine sets information controlled by the user
 *  such as Line control and baud rate. It can also initialize events and
 *  interrupts, thereby indirectly managing initializing hardware buffers.
 *  Exported only to driver, called only once per process.
 *
 @rdesc The return value is a PVOID to be passed back into the HW
 dependent layer when HW functions are called.
 */
PVOID
SerInit(
       ULONG   Identifier, // @parm Device identifier.
       PVOID   pMddHead,   // @parm First argument to mdd callbacks.
       PHWOBJ  pHWObj      // @parm Pointer to our own HW OBJ for this device
       )
{
    DEBUGMSG (ZONE_CLOSE,(TEXT("+SerInit, 0x%X\r\n"), Identifier));
    CSerialPDD * pSerialPDD = NULL;
    if (pHWObj) {
        DWORD dwIndex= pHWObj->dwIntID;
        pHWObj->dwIntID = 0;
        pSerialPDD = CreateSerialObject((LPTSTR)Identifier,pMddHead, pHWObj,dwIndex); 
    }
    if (pSerialPDD==NULL) {
        ASSERT(FALSE);
        LocalFree(pHWObj);
    }
    DEBUGMSG (ZONE_CLOSE,(TEXT("-SerInit, 0x%X\r\n"), pSerialPDD));
    return pSerialPDD;
}
ULONG
SerDeinit(
         PVOID   pHead   // @parm PVOID returned by SerInit.
         )
{
    CSerialPDD * pSerialPDD  = (CSerialPDD *) pHead;
    if (pSerialPDD ) {
        PHWOBJ pHwObj = pSerialPDD->GetHwObject();
        DeleteSerialObject(pSerialPDD);
        if (pHwObj)
            LocalFree(pHwObj);
        return (TRUE);
    }
    else
        return FALSE;

}
//
BOOL
SerPostInit(
           PVOID   pHead // @parm PVOID returned by HWinit.
           )
{
    CSerialPDD * pSerialPDD    = (CSerialPDD *)pHead;

    DEBUGMSG (ZONE_INIT,(TEXT("+SL_PostInit, 0x%X\r\n"),  pSerialPDD ));
    if ( pSerialPDD ) {
         pSerialPDD->PostInit();
    }
    DEBUGMSG (ZONE_INIT,(TEXT("-SL_PostInit, 0x%X\r\n"),  pSerialPDD ));
    return(TRUE);
}
BOOL
SerOpen(
       PVOID   pHead /*@parm PVOID returned by Serinit. */
       )
{
    DEBUGMSG (ZONE_OPEN, (TEXT("SerOpen (%X)\r\n"),pHead));
    CSerialPDD * pSerialPDD    = (CSerialPDD *)pHead;
    BOOL bReturn = FALSE;
    if ( pSerialPDD) {
        bReturn = pSerialPDD->Open();
    }
    DEBUGMSG (ZONE_OPEN, (TEXT("-SerOpen(%X) return %d \r\n"),pSerialPDD,bReturn));
    return bReturn;
}
ULONG
SerClose(
        PVOID   pHead   // @parm PVOID returned by SerInit.
        )
{
    DEBUGMSG (ZONE_CLOSE, (TEXT("SerClose (%X)\r\n"),pHead));
    CSerialPDD * pSerialPDD    = (CSerialPDD *)pHead;
    BOOL bReturn = FALSE;
    if ( pSerialPDD) {
        bReturn = pSerialPDD->Close();
    }
    DEBUGMSG (ZONE_CLOSE, (TEXT("-SerClose(%X) return %d \r\n"),pSerialPDD,bReturn));
    return (ULONG)bReturn;
}
INTERRUPT_TYPE
SerGetInterruptType(
                   PVOID pHead      // Pointer to hardware head
                   )
{
    CSerialPDD * pSerialPDD  = ( CSerialPDD * )pHead;
    INTERRUPT_TYPE interrupts=INTR_NONE;
    DEBUGMSG (ZONE_EVENTS,(TEXT("+SerGetInterruptType 0x%X\r\n"), pHead));
    if (pSerialPDD)
        interrupts= pSerialPDD->GetInterruptType();
    DEBUGMSG (ZONE_EVENTS,(TEXT("-SerGetInterruptType (0x%X) return %X\r\n"), pHead, interrupts));
    return interrupts;
}
ULONG
SerRxIntr(
         PVOID pHead,                // @parm Pointer to hardware head
         PUCHAR pRxBuffer,           // @parm Pointer to receive buffer
         ULONG *pBufflen             // @parm In = max bytes to read, out = bytes read
         )
{
    DEBUGMSG (ZONE_READ|ZONE_EVENTS,(TEXT("+SerRxIntr( 0x%X,0x%X,0x%X)\r\n"), 
            pHead,pRxBuffer,(pBufflen!=NULL?*pBufflen:0)));
    DWORD dwReturn=0;
    CSerialPDD * pSerialPDD  = ( CSerialPDD * )pHead;
    if (pSerialPDD)
        dwReturn = pSerialPDD->ReceiveInterruptHandler(pRxBuffer,pBufflen);
    DEBUGMSG (ZONE_READ|ZONE_EVENTS,(TEXT("-SerRxIntr( 0x%X,0x%X,0x%X) return 0x%X\r\n"), 
            pHead,pRxBuffer,(pBufflen!=NULL?*pBufflen:0),dwReturn));
    return dwReturn;
}
VOID
SerTxIntrEx(
           PVOID pHead,                // Hardware Head
           PUCHAR pTxBuffer,          // @parm Pointer to receive buffer
           ULONG *pBufflen            // @parm In = max bytes to transmit, out = bytes transmitted
           )
{
    DEBUGMSG (ZONE_WRITE|ZONE_EVENTS,(TEXT("+SerTxIntrEx( 0x%X,0x%X,0x%X)\r\n"), 
            pHead,pTxBuffer,(pBufflen!=NULL?*pBufflen:0)));
    CSerialPDD * pSerialPDD  = ( CSerialPDD * )pHead;
    if (pSerialPDD) 
        pSerialPDD->XmitInterruptHandler(pTxBuffer,pBufflen);
    DEBUGMSG (ZONE_WRITE|ZONE_EVENTS,(TEXT("-SerTxIntrEx( 0x%X,0x%X,0x%X)\r\n"), 
            pHead,pTxBuffer,(pBufflen!=NULL?*pBufflen:0)));
}
VOID
SerModemIntr(
            PVOID pHead                // Hardware Head
            )
{
    DEBUGMSG (ZONE_EVENTS,(TEXT("+SerModemIntr 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->ModemInterruptHandler();
    DEBUGMSG (ZONE_EVENTS,(TEXT("-SerModemIntr 0x%X\r\n"), pHead));
}
VOID
SerLineIntr(
           PVOID pHead                // Hardware Head
           )
{
    DEBUGMSG (ZONE_EVENTS,(TEXT("+SerLineIntr 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->LineInterruptHandler();
    DEBUGMSG (ZONE_EVENTS,(TEXT("-SerLineIntr 0x%X\r\n"), pHead));
}
ULONG
SerGetRxBufferSize(
                  PVOID pHead
                  )
{
    DEBUGMSG (ZONE_EVENTS,(TEXT("+SerGetRxBufferSize 0x%X\r\n"), pHead));
    ULONG ulRet = 0;
    if (pHead)
        ulRet = ((CSerialPDD *)pHead)->GetRxBufferSize();
    DEBUGMSG (ZONE_EVENTS,(TEXT("-SerGetRxBufferSize 0x%X\r\n"), pHead));
    return ulRet;
}
BOOL
SerPowerOff(
           PVOID   pHead       // @parm PVOID returned by SerInit.
           )
{
    DEBUGMSG (ZONE_INIT,(TEXT("+SerPowerOff 0x%X\r\n"), pHead));
    BOOL bReturn = FALSE;
    if (pHead)
        bReturn = ((CSerialPDD *)pHead)->PowerOff();
    DEBUGMSG (ZONE_INIT,(TEXT("-SerPowerOff 0x%X return %d\r\n"), pHead,bReturn));
    return bReturn;
}
BOOL
SerPowerOn(
          PVOID   pHead       // @parm  PVOID returned by SerInit.
          )
{
    DEBUGMSG (ZONE_INIT,(TEXT("+SerPowerOn 0x%X\r\n"), pHead));
    BOOL bReturn = FALSE;
    if (pHead)
        bReturn = ((CSerialPDD *)pHead)->PowerOn();
    DEBUGMSG (ZONE_INIT,(TEXT("-SerPowerOn 0x%X return %d\r\n"), pHead,bReturn));
    return bReturn;
}
VOID
SerClearDTR(
           PVOID   pHead // @parm PVOID returned by HWinit.
           )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SerClearDTR 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->SetDTR(FALSE);
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SerClearDTR 0x%X\r\n"), pHead));
}
VOID
SerSetDTR(
           PVOID   pHead // @parm PVOID returned by HWinit.
           )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SerSetDTR 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->SetDTR(TRUE);
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SerSetDTR 0x%X\r\n"), pHead));
}
VOID
SerClearRTS(
           PVOID   pHead // @parm PVOID returned by HWinit.
           )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SerClearRTS 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->SetRTS(FALSE);
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SerClearRTS 0x%X\r\n"), pHead));
}
VOID
SerSetRTS(
           PVOID   pHead // @parm PVOID returned by HWinit.
           )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SerSetRTS 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->SetRTS(TRUE);
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SerSetRTS0x%X\r\n"), pHead));
}
BOOL
SerEnableIR(
           PVOID   pHead, // @parm PVOID returned by Serinit.
           ULONG   BaudRate  // @parm PVOID returned by HWinit.
           )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SerEnableIR 0x%X\r\n"), pHead));
    CSerialPDD * pSerialPDD  = ( CSerialPDD * )pHead;
    if (pSerialPDD)  {
        pSerialPDD->SetOutputMode(TRUE, FALSE);
        pSerialPDD->SetBaudRate(BaudRate,TRUE);
    }
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SerEnableIR 0x%X\r\n"), pHead));
    return (TRUE);
}
BOOL
SerDisableIR(
            PVOID   pHead /*@parm PVOID returned by Serinit. */
            )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SerDisableIR 0x%X\r\n"), pHead));
    CSerialPDD * pSerialPDD  = ( CSerialPDD * )pHead;
    if (pSerialPDD)  {
        pSerialPDD->SetOutputMode(FALSE, TRUE);
    }
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SerDisableIR 0x%X \r\n"), pHead));
    return (TRUE);
}
VOID
SerClearBreak(
             PVOID   pHead // @parm PVOID returned by HWinit.
             )
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_ClearBreak, 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->SetBreak(FALSE);
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_ClearBreak, 0x%X\r\n"), pHead));
}
VOID
SerSetBreak(
           PVOID   pHead // @parm PVOID returned by HWinit.
           )
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SL_SetBreak, 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->SetBreak(TRUE);
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetBreak, 0x%X\r\n"), pHead));
}

BOOL
SerXmitComChar(
              PVOID   pHead,    // @parm PVOID returned by HWInit.
              UCHAR   ComChar   // @parm Character to transmit. 
              )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SerXmitComChar 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->XmitComChar(ComChar);    
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SerXmitComChar 0x%X return %d \r\n"), pHead));
    return TRUE;
}
ULONG
SerGetStatus(
            PVOID    pHead,    // @parm PVOID returned by HWInit.
            LPCOMSTAT    lpStat    // Pointer to LPCOMMSTAT to hold status.
            )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SerGetStatus 0x%X\r\n"), pHead));
    ULONG ulReturn = (ULONG)-1;
    if (pHead && lpStat) {
        CSerialPDD * pSerialPDD  = ( CSerialPDD * )pHead;
        lpStat->fCtsHold = (((pSerialPDD->GetDCB()).fOutxCtsFlow && pSerialPDD->IsCTSOff())?1:0);
        lpStat->fDsrHold = (((pSerialPDD->GetDCB()).fOutxDsrFlow && pSerialPDD->IsDSROff())?1:0);
        // NOTE - I think what they really want to know here is
        // the amount of data in the MDD buffer, not the amount
        // in the UART itself.  Just set to 0 for now since the
        // MDD doesn't take care of this.
        lpStat->cbInQue = 0;
        lpStat->cbOutQue = 0;
        ulReturn = pSerialPDD->GetReceivedError();
    }        
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SerGetStatus 0x%X reutnr 0x%X\r\n"), pHead,ulReturn));
    return ulReturn;
}
VOID
PreDeinit(
        PVOID   pHead    // @parm PVOID returned by HWInit.
        )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SL_Reset 0x%X\r\n"), pHead));
    if (pHead)
        ((CSerialPDD *)pHead)->PreDeinit();    
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SL_Reset 0x%X\r\n"), pHead));
}
VOID
SerGetModemStatus(
                 PVOID   pHead,        // @parm PVOID returned by HWInit.
                 PULONG  pModemStatus    // @parm PULONG passed in by user.
                 )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SL_GetModemStatus 0x%X\r\n"), pHead));
    if (pHead && pModemStatus ) {
        *pModemStatus = ((CSerialPDD *)pHead)->GetModemStatus();
    }
    DEBUGMSG (ZONE_FUNCTION | ZONE_EVENTS,
              (TEXT("-SL_GetModemStatus 0x%X (stat x%X) \r\n"), pHead, (pModemStatus!=NULL?*pModemStatus:0)));
    return;
}
VOID
SerGetCommProperties(
                    PVOID   pHead,      // @parm PVOID returned by SerInit. 
                    LPCOMMPROP  pCommProp   // @parm Pointer to receive COMMPROP structure. 
                    )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SerGetCommProperties 0x%X\r\n"), pHead));
    if (pHead && pCommProp ) {
        *pCommProp = ((CSerialPDD *)pHead)->GetCommProperties();
    }
    DEBUGMSG (ZONE_FUNCTION | ZONE_EVENTS,
              (TEXT("-SerGetCommProperties 0x%X \r\n"), pHead));
    return;
}
VOID
SerPurgeComm(
            PVOID   pHead,        // @parm PVOID returned by HWInit.
            DWORD   fdwAction        // @parm Action to take. 
            )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SL_PurgeComm 0x%X\r\n"), pHead));
    if (pHead ) {
        ((CSerialPDD *)pHead)->PurgeComm( fdwAction);
    }
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SL_PurgeComm 0x%X\r\n"), pHead));
    return;
}
BOOL
SerSetDCB(
         PVOID   pHead,        // @parm    PVOID returned by HWInit.
         LPDCB   lpDCB       // @parm    Pointer to DCB structure
         )
{
    BOOL bRet = FALSE;
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SL_SetDCB 0x%X\r\n"), pHead));
    if (pHead  &&  lpDCB ) {
        bRet=((CSerialPDD *)pHead)->SetDCB(lpDCB);
    }
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SL_SetDCB 0x%X return %d \r\n"), pHead,bRet));
    return(bRet);
}
BOOL
SerSetCommTimeouts(
                  PVOID   pHead,        // @parm    PVOID returned by HWInit.
                  LPCOMMTIMEOUTS   lpCommTimeouts // @parm Pointer to CommTimeout structure
                  )
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("+SL_SetCommTimeout 0x%X\r\n"), pHead));
    // I do not think PDD need CommTimeouts parameter for operation.
    // All timeouts are taken care of by the MDD.
    DEBUGMSG (ZONE_FUNCTION,(TEXT("-SL_SetCommTimeout 0x%X\r\n"), pHead));
    return(TRUE);
}
BOOL
SerIoctl(PVOID pHead, DWORD dwCode,PBYTE pBufIn,DWORD dwLenIn,
         PBYTE pBufOut,DWORD dwLenOut,PDWORD pdwActualOut)
{
    BOOL RetVal = FALSE;
    DEBUGMSG (ZONE_FUNCTION, (TEXT("+SerIoctl 0x%X\r\n"), pHead));
    if (pHead)
        RetVal= ((CSerialPDD *)pHead)->Ioctl( dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
    DEBUGMSG (ZONE_FUNCTION, (TEXT("-SerIoctl( 0x%X\r\n"), pHead));
    return(RetVal);
}
const
HW_VTBL IoVTbl = {			
    SerInit,
    SerPostInit,
    SerDeinit,
    SerOpen,
    SerClose,
    SerGetInterruptType,
    SerRxIntr,
    SerTxIntrEx,
    SerModemIntr,
    SerLineIntr,
    SerGetRxBufferSize,
    SerPowerOff,
    SerPowerOn,
    SerClearDTR,
    SerSetDTR,
    SerClearRTS,
    SerSetRTS,
    SerEnableIR,
    SerDisableIR,
    SerClearBreak,
    SerSetBreak,
    SerXmitComChar,
    SerGetStatus,
    PreDeinit,
    SerGetModemStatus,
    SerGetCommProperties,
    SerPurgeComm,
    SerSetDCB,
    SerSetCommTimeouts,
    SerIoctl
    };


// GetSerialObj : The purpose of this function is to allow multiple PDDs to be
// linked with a single MDD creating a multiport driver.  In such a driver, the
// MDD must be able to determine the correct vtbl and associated parameters for
// each PDD.  Immediately prior to calling HWInit, the MDD calls GetSerialObject
// to get the correct function pointers and parameters.
//
extern "C" PHWOBJ
GetSerialObject(
               DWORD DeviceArrayIndex
               )
{
    PHWOBJ pSerObj;

    // Unlike many other serial samples, we do not have a statically allocated
    // array of HWObjs.  Instead, we allocate a new HWObj for each instance
    // of the driver.  The MDD will always call GetSerialObj/HWInit/HWDeinit in
    // that order, so we can do the alloc here and do any subsequent free in
    // HWDeInit.
    // Allocate space for the HWOBJ.
    pSerObj=(PHWOBJ)LocalAlloc( LPTR ,sizeof(HWOBJ) );
    if ( !pSerObj )
        return (NULL);

    // Fill in the HWObj structure that we just allocated.

    pSerObj->BindFlags = THREAD_IN_PDD;     // PDD create thread when device is first attached.
    pSerObj->dwIntID = DeviceArrayIndex;   // Only it is useful when set set THREAD_AT_MDD. We use this to transfer DeviceArrayIndex
    pSerObj->pFuncTbl = (HW_VTBL *) &IoVTbl; // Return pointer to appropriate functions

    // Now return this structure to the MDD.
    return (pSerObj);
}


