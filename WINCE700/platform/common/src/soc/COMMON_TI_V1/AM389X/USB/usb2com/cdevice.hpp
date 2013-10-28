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
//     cdevice.hpp
//
// Abstract: Implements classes for managing USB devices
//
//                  CDevice (ADT)
//                /               \
//            CFunction        CHub (ADT)
//                            /          \
//                        CRootHub   CExternalHub
//
// Notes:
//

#ifndef __CDEVICE_HPP__
#define __CDEVICE_HPP__

#include <globals.hpp>
#include <sync.hpp>
#include <pipeabs.hpp>

class CHcd;
class CDevice;
class CFunction;
class CHub;
class CRootHub;
class CExternalHub;


class CHcd;

class CDeviceGlobal {
public:
    CDeviceGlobal();
    virtual ~CDeviceGlobal();
    BOOL Initialize(IN PVOID pHcd );
    void DeInitialize( void );
    BOOL ReserveAddress( OUT UCHAR& rAddress );
    void  FreeAddress( IN const UCHAR address );
    // Address 0 function
    CritSec_Status Addr0LockEntry(ULONG ulTimeout) { return m_csAddress0Lock.EnterCritSec_Ex(ulTimeout); };
    void Addr0LockPrepareDelete() { m_csAddress0Lock.PrepareDeleteCritSec_Ex ();};
    void Addr0LockLeave() { m_csAddress0Lock.LeaveCritSec_Ex ();};
    //Object Countdown
    BOOL ObjCountdownInc () {return  m_objCountdown.IncrCountdown (); };
    void ObjCountdownDec () { m_objCountdown.DecrCountdown ();};
    virtual PVOID   AddedTt( UCHAR uHubAddress,UCHAR uPort)=0;
    virtual BOOL    DeleteTt( UCHAR uHubAddress,UCHAR uPort, PVOID ttContext)=0;
    virtual LPCTSTR GetControllerName( void ) const = 0;

public:
    LPUSBD_SELECT_CONFIGURATION_PROC GetpUSBDSelectConfigurationProc() { return m_pUSBDSelectConfigurationProc; };
    LPUSBD_SUSPEND_RESUME_PROC GetpUSBDSuspendedResumed() { return m_pUSBDSuspendResumed; };
    LPUSBD_ATTACH_PROC GetpUSBDAttachProc() { return m_pUSBDAttachProc; };
    LPUSBD_DETACH_PROC GetpUSBDDetachProc() { return m_pUSBDDetachProc; };
    LPVOID             GetpHcdContext() { return m_pvHcdContext ;};
private:
    PVOID m_pHcd;
    CRITICAL_SECTION     m_csFreeAddressArrayLock;
    DWORD                m_dwFreeAddressArray[4];
    CritSec_Ex           m_csAddress0Lock; // critical section when using address 0
    Countdown            m_objCountdown;   // counts stray detach threads
    // USBD.dll related variables
    HINSTANCE            m_hUSBDInstance;
    LPUSBD_SELECT_CONFIGURATION_PROC m_pUSBDSelectConfigurationProc;
    // this procedure is called when new USB devices (functions) are attached
    LPUSBD_ATTACH_PROC   m_pUSBDAttachProc;
    // this procedure is called when USB devices (functions) are detached
    LPUSBD_DETACH_PROC   m_pUSBDDetachProc;
    LPUSBD_SUSPEND_RESUME_PROC m_pUSBDSuspendResumed;
    // OUT param for lpHcdAttachProc call
    LPVOID               m_pvHcdContext;

#ifdef DEBUG
    BOOL g_fAlreadyCalled;
#endif // DEBUG

};

// abstract base class for devices
class CDevice
{
    friend class CHub;
public:
    // ****************************************************
    // Public Functions for CDevice
    // ****************************************************

    CDevice( IN const UCHAR address,
             IN const USB_DEVICE_INFO& rDeviceInfo,
             IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
             IN const UCHAR tierNumber,
             IN CDeviceGlobal * const pDeviceGlobal,
             IN CHub * const pAttachedHub=NULL,const UCHAR uAttachedPort=0);

    virtual ~CDevice();
    virtual DWORD EnterOperationalState( IN CPipeAbs* const pEndpoint0Pipe ) = 0;

    virtual HCD_REQUEST_STATUS OpenPipe( IN const UINT address,
                                         IN LPCUSB_ENDPOINT_DESCRIPTOR const lpEndpointDescriptor,
                                         OUT LPUINT const lpPipeIndex,
                                         OUT LPVOID* const plpCPipe = NULL ) = 0;

    virtual HCD_REQUEST_STATUS ClosePipe( IN const UINT address,
                                          IN const UINT pipeIndex ) = 0;

    virtual HCD_REQUEST_STATUS IssueTransfer( ISSUE_TRANSFER_PARAMS* pITP ) = 0;

    virtual HCD_REQUEST_STATUS AbortTransfer(
                                    IN const UINT address,
                                    IN const UINT pipeIndex,
                                    IN LPTRANSFER_NOTIFY_ROUTINE const lpCancelAddress,
                                    IN LPVOID const lpvNotifyParameter,
                                    IN LPCVOID const lpvCancelId ) = 0;


    virtual HCD_REQUEST_STATUS IsPipeHalted( IN const UINT address,
                                             IN const UINT pipeIndex,
                                             OUT LPBOOL const lpbHalted ) = 0;

    virtual HCD_REQUEST_STATUS ResetPipe( IN const UINT address,
                                          IN const UINT pipeIndex ) = 0;

    virtual HCD_REQUEST_STATUS DisableDevice( IN const UINT address, IN const BOOL fReset );
    virtual HCD_REQUEST_STATUS SuspendResume( IN const UINT address,IN const BOOL fSuspend ) ;
    virtual BOOL ResumeNotification() { return FALSE; };
    virtual BOOL NotifyOnSuspendedResumed(BOOL /*fResumed*/) { return FALSE; };

    virtual void HandleDetach( void ) = 0;

    virtual CHub * GetUSB2TT(UCHAR& pTTAddr, UCHAR& pTTPort, PVOID& ttContext);

    LPCTSTR GetControllerName( void ) const { return m_pDeviceGlobal->GetControllerName(); }

#ifdef DEBUG
    void DumpDeviceDescriptor( IN const PUSB_DEVICE_DESCRIPTOR pDescriptor )const;
    void DumpConfigDescriptor( IN const PUSB_CONFIGURATION_DESCRIPTOR pDescriptor )const;
    void DumpInterfaceDescriptor( IN const PUSB_INTERFACE_DESCRIPTOR pDescriptor )const;
    void DumpEndpointDescriptor( IN const PUSB_ENDPOINT_DESCRIPTOR pDescriptor )const;
    void DumpExtendedBytes( IN BYTE const*const pByteArray, IN const DWORD dwSize )const;
#endif // DEBUG

    // ****************************************************
    // Public Variables for CDevice
    // ****************************************************

private:
    // ****************************************************
    // Private Functions for CDevice
    // ****************************************************

public:
    // utility functions for managing NON_CONST_USB_CONFIGURATION structures
    BOOL CreateUsbConfigurationStructure( IN NON_CONST_USB_CONFIGURATION& rConfig, __in_bcount(dataBufferLen) const PUCHAR pDataBuffer, IN const UINT dataBufferLen )const;
    void DeleteUsbConfigurationStructure( IN NON_CONST_USB_CONFIGURATION& rConfig ) const;
    // ****************************************************
    // Private Variables for CDevice
    // ****************************************************

protected:
    // ****************************************************
    // Protected Functions for CDevice
    // ****************************************************
    static DWORD CALLBACK TransferDoneCallbackSetEvent( PVOID context );
    BOOL AllocatePipeArray( void );

#ifdef DEBUG
    virtual const TCHAR* GetDeviceType( void ) const = 0;
#endif // DEBUG
    CDevice&operator=(CDevice&);

    // ****************************************************
    // Protected Variables for CDevice
    // ****************************************************
    CHub * const                m_pAttachedHub;     // Attached Hub.
    const UCHAR                 m_sAttachedPort;    // Port number of this hub.
    CRITICAL_SECTION            m_csDeviceLock;     // critical section for device

    const UCHAR                 m_address;          // address/deviceIndex of this device
    USB_DEVICE_INFO             m_deviceInfo;       // holds device's USB descriptors
    const BOOL                  m_fIsLowSpeed;      // indicates if device is low speed
    const BOOL                  m_fIsHighSpeed;     // indicates if device is high speed.
    const UCHAR                 m_tierNumber;       // indicates tier # of device (i.e. how far
                                                    // it is from the root hub) See the USB spec
                                                    // 1.1, section 4.1.1
    CDeviceGlobal * const       m_pDeviceGlobal;
    UCHAR                       m_maxNumPipes;      // size of m_ppCPipe array
    union {
        CPipeAbs*               m_pCtrlPipe0;       // used only during ATTACH
        BOOL                    m_fIsSuspend;       // Is this device suspend or not.
    };
    volatile DWORD              m_dwDevState;       // progress during attach - 0 is fully operational
                                                    // used to sync ATTACH and DETACH threads

#define ENDPOINT0_CONTROL_PIPE          UCHAR(0)    // control pipe is at m_ppCPipe[ 0 ]
#define STATUS_CHANGE_INTERRUPT_PIPE    UCHAR(1)    // for hubs, status change pipe is m_ppCPipe[ 1 ]
    CPipeAbs**                  m_ppCPipe;          // array of pipes for this device
public:
    BOOL ReserveAddress( OUT UCHAR& rAddress ) { return m_pDeviceGlobal->ReserveAddress(rAddress); };
    void  FreeAddress( IN const UCHAR address ) { m_pDeviceGlobal->FreeAddress(address); };
    UCHAR GetDeviceAddress() { return m_address; };

};

// this enum is used in AttachNewDevice
enum DEVICE_CONFIG_STATUS
{
    //
    // KEEP THIS ARRAY IN SYNC WITH cszCfgStateStrings array below!
    //

    // these steps are common for both hubs and functions
    DEVICE_CONFIG_STATUS_OPENING_ENDPOINT0_PIPE,
    DEVICE_CONFIG_STATUS_USING_ADDRESS0,
    DEVICE_CONFIG_STATUS_RESET_AND_ENABLE_PORT,
    DEVICE_CONFIG_STATUS_SCHEDULING_GET_DEVICE_DESCRIPTOR_TEST,
    DEVICE_CONFIG_STATUS_SCHEDULING_SET_ADDRESS,
    DEVICE_CONFIG_STATUS_LEAVE_ADDRESS0,
    DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_DEVICE_DESCRIPTOR,
    DEVICE_CONFIG_STATUS_SCHEDULING_GET_DEVICE_DESCRIPTOR,
    DEVICE_CONFIG_STATUS_SETUP_CONFIGURATION_DESCRIPTOR_ARRAY,
    DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_CONFIG_DESCRIPTOR,
    DEVICE_CONFIG_STATUS_SCHEDULING_GET_CONFIG_DESCRIPTOR,
    DEVICE_CONFIG_STATUS_DETERMINE_CONFIG_TO_CHOOSE,
    DEVICE_CONFIG_STATUS_SCHEDULING_SET_CONFIG,
    // if ( device is a hub ) {
    DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_HUB_DESCRIPTOR,
    DEVICE_CONFIG_STATUS_SCHEDULING_GET_HUB_DESCRIPTOR,
    DEVICE_CONFIG_STATUS_CREATE_NEW_HUB,
    // } else {
    DEVICE_CONFIG_STATUS_CREATE_NEW_FUNCTION,
    // }
    DEVICE_CONFIG_STATUS_INSERT_NEW_DEVICE_INTO_UPSTREAM_HUB_PORT_ARRAY,
    DEVICE_CONFIG_STATUS_SIGNAL_NEW_DEVICE_ENTER_OPERATIONAL_STATE,
    DEVICE_CONFIG_STATUS_FAILED,
    DEVICE_CONFIG_STATUS_DONE,

    DEVICE_CONFIG_STATUS_INVALID // must be last item in list
};
#ifdef DEBUG
__declspec(selectany) extern const TCHAR *cszCfgStateStrings[] =
{
    //
    // KEEP THIS ARRAY IN SYNC WITH DEVICE_CONFIG_STATUS enum above!
    //
    TEXT("DEVICE_CONFIG_STATUS_OPENING_ENDPOINT0_PIPE"),
    TEXT("DEVICE_CONFIG_STATUS_USING_ADDRESS0"),
    TEXT("DEVICE_CONFIG_STATUS_RESET_AND_ENABLE_PORT"),
    TEXT("DEVICE_CONFIG_STATUS_SCHEDULING_GET_DEVICE_DESCRIPTOR_TEST"),
    TEXT("DEVICE_CONFIG_STATUS_SCHEDULING_SET_ADDRESS"),
    TEXT("DEVICE_CONFIG_STATUS_LEAVE_ADDRESS0"),
    TEXT("DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_DEVICE_DESCRIPTOR"),
    TEXT("DEVICE_CONFIG_STATUS_SCHEDULING_GET_DEVICE_DESCRIPTOR"),
    TEXT("DEVICE_CONFIG_STATUS_SETUP_CONFIGURATION_DESCRIPTOR_ARRAY"),
    TEXT("DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_CONFIG_DESCRIPTOR"),
    TEXT("DEVICE_CONFIG_STATUS_SCHEDULING_GET_CONFIG_DESCRIPTOR"),
    TEXT("DEVICE_CONFIG_STATUS_DETERMINE_CONFIG_TO_CHOOSE"),
    TEXT("DEVICE_CONFIG_STATUS_SCHEDULING_SET_CONFIG"),
    // if ( device is a hub ) {
    TEXT("DEVICE_CONFIG_STATUS_SCHEDULING_GET_INITIAL_HUB_DESCRIPTOR"),
    TEXT("DEVICE_CONFIG_STATUS_SCHEDULING_GET_HUB_DESCRIPTOR"),
    TEXT("DEVICE_CONFIG_STATUS_CREATE_NEW_HUB"),
    // } else {
    TEXT("DEVICE_CONFIG_STATUS_CREATE_NEW_FUNCTION"),
    // }
    TEXT("DEVICE_CONFIG_STATUS_INSERT_NEW_DEVICE_INTO_UPSTREAM_HUB_PORT_ARRAY"),
    TEXT("DEVICE_CONFIG_STATUS_SIGNAL_NEW_DEVICE_ENTER_OPERATIONAL_STATE"),
    TEXT("DEVICE_CONFIG_STATUS_FAILED"),
    TEXT("DEVICE_CONFIG_STATUS_DONE"),

    TEXT("DEVICE_CONFIG_STATUS_INVALID")
};
#define STATUS_TO_STRING(status)  (( (status) < DEVICE_CONFIG_STATUS_INVALID) ? \
                           cszCfgStateStrings[ (status) ] : TEXT("Invalid"))
#endif // DEBUG

// abstract base class for hubs
class CHub : public CDevice
{
    friend class CDevice;
public:
    // ****************************************************
    // Public Functions for CHub
    // ****************************************************

    CHub( IN const UCHAR address,
          IN const USB_DEVICE_INFO& rDeviceInfo,
          IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
          IN const UCHAR tierNumber,
          IN const USB_HUB_DESCRIPTOR& rUsbHubDescriptor,
          IN CHcd * const pCHcd,
          IN CHub * const pAttachedHub=NULL,const UCHAR uAttachedPort=0);

    virtual ~CHub();

    void HandleDetach( void );

    HCD_REQUEST_STATUS OpenPipe( IN const UINT address,
                                 IN LPCUSB_ENDPOINT_DESCRIPTOR const lpEndpointDescriptor,
                                 OUT LPUINT const lpPipeIndex,
                                 OUT LPVOID* const plpCPipe = NULL );

    HCD_REQUEST_STATUS ClosePipe( IN const UINT address,
                                  IN const UINT pipeIndex );

    HCD_REQUEST_STATUS IssueTransfer( ISSUE_TRANSFER_PARAMS* pITP );

    HCD_REQUEST_STATUS AbortTransfer(
                            IN const UINT address,
                            IN const UINT pipeIndex,
                            IN LPTRANSFER_NOTIFY_ROUTINE const lpCancelAddress,
                            IN LPVOID const lpvNotifyParameter,
                            IN LPCVOID const lpvCancelId );

    HCD_REQUEST_STATUS IsPipeHalted( IN const UINT address,
                                     IN const UINT pipeIndex,
                                     OUT LPBOOL const lpbHalted );

    HCD_REQUEST_STATUS ResetPipe( IN const UINT address,
                                  IN const UINT pipeIndex );

    virtual BOOL DisableOffStreamDevice( IN const UINT address, IN const BOOL fReset );
    virtual BOOL SuspendResumeOffStreamDevice( IN const UINT address, IN const BOOL fSuspend );

    virtual HCD_REQUEST_STATUS DisableDevice( IN const UINT address, IN const BOOL fReset );
    virtual HCD_REQUEST_STATUS SuspendResume( IN const UINT address,IN const BOOL fSuspend ) ;

    // Notification when this hub is resumed.
    virtual BOOL ResumeNotification() {
        DEBUGMSG( ZONE_ATTACH, (TEXT("CHub(%s tier %d):: ResumeNotification(%d) !\n"), GetDeviceType(), m_tierNumber,m_address) );
        m_fIsSuspend = FALSE;
        return SetEvent(m_hHubSuspendBlockEvent);
    };
    virtual BOOL NotifyOnSuspendedResumed(BOOL fResumed) ;

#ifdef DEBUG
    static void DumpHubDescriptor( IN const PUSB_HUB_DESCRIPTOR pDescriptor );
#endif // DEBUG

    // ****************************************************
    // Public Variables for CHub
    // ****************************************************

private:
    // ****************************************************
    // Private Functions for CHub
    // ****************************************************

    DWORD HubStatusChangeThread( void );

protected:
    // ****************************************************
    // Protected Functions for CHub
    // ****************************************************

#ifdef USB_IF_ELECTRICAL_TEST_MODE
    HCD_COMPLIANCE_TEST_STATUS CheckUsbCompliance( IN CPipeAbs * const pControlPipe,
                                                   IN const UCHAR address,
                                                   IN const UCHAR port,
                                                   IN USHORT usbPid );
#endif

    static DWORD CALLBACK  HubStatusChangeThreadStub( IN PVOID context );

    void AttachDevice( IN const UCHAR port,
                       IN const BOOL fIsLowSpeed,
                       IN const BOOL fIsHighSpeed,
                       IN const BOOL fSyncronously = FALSE );

    BOOL GetDescriptor( IN CPipeAbs* const pControlPipe,
                        IN const UCHAR address,
                        IN const UCHAR descriptorType,
                        IN const UCHAR descriptorIndex,
                        IN const USHORT wDescriptorSize,
                        OUT PVOID pBuffer );

    BOOL ResetTT( IN const UCHAR port );

    virtual BOOL PowerAllHubPorts( void ) = 0;

    virtual BOOL WaitForPortStatusChange( OUT UCHAR& rPort,
                                          OUT USB_HUB_AND_PORT_STATUS& rStatus ) = 0;

    virtual BOOL  SetOrClearFeature( IN const WORD port,
                                     IN const UCHAR setOrClearFeature,
                                     IN const USHORT feature ) = 0;

    virtual BOOL  SetOrClearRemoteWakup(BOOL bSet) = 0;

    virtual BOOL GetStatus( IN const UCHAR port,
                            OUT USB_HUB_AND_PORT_STATUS& rStatus ) = 0;

    virtual BOOL ResetAndEnablePort( IN const UCHAR port ) = 0;

    virtual void DisablePort( IN const UCHAR port ) = 0;


    static DWORD CALLBACK DetachDownstreamDeviceThreadStub( IN PVOID context );
    DWORD CALLBACK DetachDownstreamDeviceThread( CDevice* pDevToDetach );

    static DWORD CALLBACK AttachDownstreamDeviceThreadStub( IN PVOID context );
    DWORD CALLBACK AttachDownstreamDeviceThread(  CDevice* pDevToAttach );

    void DetachDevice( IN const UCHAR port, IN const BOOL fSyncronously = FALSE );

    BOOL AllocateDeviceArray( void );
    CHub&operator=(CHub&){ASSERT(FALSE);}

    // ****************************************************
    // Protected Variables for CHub
    // ****************************************************
    USB_HUB_DESCRIPTOR          m_usbHubDescriptor; // the hub's USB descriptor
    CDevice**                   m_ppCDeviceOnPort;  // array of pointers to the devices on this hub's ports
    PVOID *                     m_pAddedTT;
    BOOL                        m_fHubThreadClosing;     // indicates to threads that this device is being closed

    HANDLE                      m_hHubStatusChangeEvent; // indicates status change on one of the hub's ports
    HANDLE                      m_hHubStatusChangeThread; // thread for checking port status change
    HANDLE                      m_hHubSuspendBlockEvent;

    CHcd * const               m_pCHcd;
};

class CRootHub : public CHub
{
public:
    // ****************************************************
    // Public Functions for CRootHub
    // ****************************************************
    CRootHub( IN const USB_DEVICE_INFO& rDeviceInfo,
              IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
              IN const USB_HUB_DESCRIPTOR& rUsbHubDescriptor,
              IN CHcd * const pCHcd );

    ~CRootHub();

    DWORD EnterOperationalState( IN CPipeAbs* const pEndpoint0Pipe );

    // ****************************************************
    // Public Variables for CRootHub
    // ****************************************************

private:

    BOOL  SetOrClearFeature( IN const WORD port,
                             IN const UCHAR setOrClearFeature,
                             IN const USHORT feature );

    virtual BOOL  SetOrClearRemoteWakup(BOOL bSet);

    BOOL  GetStatus( IN const UCHAR port,
                     OUT USB_HUB_AND_PORT_STATUS& rStatus );

#ifdef DEBUG
    const TCHAR* GetDeviceType( void ) const
    {
        static const TCHAR* cszDeviceType = TEXT("Root");
        return cszDeviceType;
    }
#endif // DEBUG

    BOOL PowerAllHubPorts( void );

    BOOL WaitForPortStatusChange( OUT UCHAR& rPort,
                                  OUT USB_HUB_AND_PORT_STATUS& rStatus );

    BOOL ResetAndEnablePort( IN const UCHAR port );

    void DisablePort( IN const UCHAR port );
    CRootHub&operator=(CRootHub&){ASSERT(FALSE);}
};

class CExternalHub : public CHub
{
public:
    // ****************************************************
    // Public Functions for CExternalHub
    // ****************************************************
    CExternalHub( IN const UCHAR address,
                  IN const USB_DEVICE_INFO& rDeviceInfo,
                  IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                  IN const UCHAR tierNumber,
                  IN const USB_HUB_DESCRIPTOR& rUsbHubDescriptor,
                  IN CHcd * const pCHcd,
                  IN CHub * const pAttachedHub,const UCHAR uAttachedPort);

    ~CExternalHub();

    DWORD EnterOperationalState( IN CPipeAbs* const pEndpoint0Pipe );

    // ****************************************************
    // Public Variables for CExternalHub
    // ****************************************************

private:
#ifdef DEBUG
    const TCHAR* GetDeviceType( void ) const
    {
        static const TCHAR* cszDeviceType = TEXT("External");
        return cszDeviceType;
    }
#endif // DEBUG

    BOOL  PowerAllHubPorts( void );

    BOOL  GetStatusChangeBitmap( OUT DWORD& rdwHubBitmap );

    BOOL  WaitForPortStatusChange( OUT UCHAR& rPort,
                                   OUT USB_HUB_AND_PORT_STATUS& rStatus );

    BOOL  ResetAndEnablePort( IN const UCHAR port );

    void  DisablePort( IN const UCHAR port );

    BOOL  SetOrClearFeature( IN const WORD port,
                             IN const UCHAR setOrClearFeature,
                             IN const USHORT feature );

    virtual BOOL  SetOrClearRemoteWakup(BOOL bSet);

    BOOL  GetStatus( IN const UCHAR port,
                     OUT USB_HUB_AND_PORT_STATUS& rStatus );
    CExternalHub&operator=(CExternalHub&){ASSERT(FALSE);}

    // ****************************************************
    // Private Variables for CExternalHub
    // ****************************************************

};

// class for USB functions (i.e. mice, keyboards, etc)
class CFunction : public CDevice
{
    friend class CHub;
public:
    // ****************************************************
    // Public Functions for CFunction
    // ****************************************************

    CFunction( IN const UCHAR address,
               IN const USB_DEVICE_INFO& rUhcaddress,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR tierNumber ,
               IN CHcd * const pCHcd ,
               IN CHub * const pAttachedHub,const UCHAR uAttachedPort);

    ~CFunction();

    DWORD EnterOperationalState( IN CPipeAbs* const pEndpoint0Pipe );

    HCD_REQUEST_STATUS OpenPipe( IN const UINT address,
                                 IN LPCUSB_ENDPOINT_DESCRIPTOR const lpEndpointDescriptor,
                                 OUT LPUINT const lpPipeIndex,
                                 OUT LPVOID* const plpCPipe = NULL );

    HCD_REQUEST_STATUS ClosePipe( IN const UINT address,
                                  IN const UINT pipeIndex );

    HCD_REQUEST_STATUS IssueTransfer( ISSUE_TRANSFER_PARAMS* pITP );

    HCD_REQUEST_STATUS AbortTransfer(
                            IN const UINT address,
                            IN const UINT pipeIndex,
                            IN LPTRANSFER_NOTIFY_ROUTINE const lpCancelAddress,
                            IN LPVOID const lpvNotifyParameter,
                            IN LPCVOID const lpvCancelId );

    HCD_REQUEST_STATUS IsPipeHalted( IN const UINT address,
                                     IN const UINT pipeIndex,
                                     OUT LPBOOL const lpbHalted );

    HCD_REQUEST_STATUS ResetPipe( IN const UINT address,
                                  IN const UINT pipeIndex );

    virtual BOOL NotifyOnSuspendedResumed(BOOL fResumed);
    // ****************************************************
    // Public Variables for CFunction
    // ****************************************************

private:

    void HandleDetach( void );

    BOOL  SetOrClearFeature( IN const UCHAR recipient,
                             IN const WORD wIndex,
                             IN const UCHAR setOrClearFeature,
                             IN const USHORT feature );

#ifdef DEBUG
    const TCHAR* GetDeviceType( void ) const
    {
        static const TCHAR* cszDeviceType = TEXT("Function");
        return cszDeviceType;
    }
#endif // DEBUG
    CFunction&operator=(CFunction&){ASSERT(FALSE);}

    // ****************************************************
    // Private Variables for CFunction
    // ****************************************************
    PVOID                       m_lpvDetachId;
    HANDLE                      m_hFunctionFeatureEvent; // for blocking on implicit transfers
    CHcd * const                m_pCHcd;

};

// bits for <CDevice::m_dwDevState>

#define FXN_DEV_STATE_OPERATIONAL      0

#define FXN_DEV_STATE_FUNCTION      0x01
#define FXN_DEV_STATE_UNDETERMINED  0x02
#define FXN_DEV_STATE_ATTACHMENT    0x04
#define FXN_DEV_STATE_ENABFAILED    0x10
#define FXN_DEV_STATE_DETACHMENT    0x40
#define FXN_DEV_STATE_DETACHUSBD    0x80

#define FXN_DEV_STATE_PROGR_BITS    0x0F
#define FXN_DEV_STATE_DETACH_BITS   0xF0

#endif // __CDEVICE_HPP__

