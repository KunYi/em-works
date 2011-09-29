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
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------
// Copyright (C) 2005-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------
// 
// Module Name:  
//     chw.h
// 
// Abstract: Provides interface to UHCI host controller
// 
// Notes: 
//
//------------------------------------------------------------------------------
#ifndef __CHW_H__
#define __CHW_H__
#include <usb200.h>
#include <sync.hpp>
#ifdef QFE_MERGE /*070630*/ /*CE5QFE*/
#include <CRegEdit.h>
#endif
#include <hcd.hpp>
#include "cpipe.h"
//#include <csp.h>

// Remove-W4: Warning C4512 workaround
#pragma warning(push)
#pragma warning(disable: 4512)

#ifdef QFE_MERGE /*070630*/ /*CE5QFE*/
#define EHCI_REG_IntThreshCtrl TEXT("IntThreshCtrl")
#define EHCI_REG_IntThreshCtrl_DEFAULT 8
#endif

class CHW;
class CEhcd;

typedef struct _PERIOD_TABLE {
    UCHAR Period;
    UCHAR qhIdx;
    UCHAR InterruptScheduleMask;
} PERIOD_TABLE, *PPERIOD_TABLE;
//-----------------------------------Dummy Queue Head for static QHEad ---------------
class CDummyPipe : public CPipe
{

public:
    // ****************************************************
    // Public Functions for CQueuedPipe
    // ****************************************************
    CDummyPipe(IN CPhysMem * const pCPhysMem);
    virtual ~CDummyPipe() {;};

//    inline const int GetTdSize( void ) const { return sizeof(TD); };

    HCD_REQUEST_STATUS  IssueTransfer( 
                                IN const UCHAR /*address*/,
                                IN LPTRANSFER_NOTIFY_ROUTINE const /*lpfnCallback*/,
                                IN LPVOID const /*lpvCallbackParameter*/,
                                IN const DWORD /*dwFlags*/,
                                IN LPCVOID const /*lpvControlHeader*/,
                                IN const DWORD /*dwStartingFrame*/,
                                IN const DWORD /*dwFrames*/,
                                IN LPCDWORD const /*aLengths*/,
                                IN const DWORD /*dwBufferSize*/,     
                                IN_OUT LPVOID const /*lpvBuffer*/,
                                IN const ULONG /*paBuffer*/,
                                IN LPCVOID const /*lpvCancelId*/,
                                OUT LPDWORD const /*adwIsochErrors*/,
                                OUT LPDWORD const /*adwIsochLengths*/,
                                OUT LPBOOL const /*lpfComplete*/,
                                OUT LPDWORD const /*lpdwBytesTransferred*/,
                                OUT LPDWORD const /*lpdwError*/ )  
        { return requestFailed;};

    virtual HCD_REQUEST_STATUS  OpenPipe( void )
        { return requestFailed;};

    virtual HCD_REQUEST_STATUS  ClosePipe( void ) 
        { return requestFailed;};

    virtual HCD_REQUEST_STATUS IsPipeHalted( OUT LPBOOL const /*lpbHalted*/ )
        {   
            ASSERT(FALSE);
            return requestFailed;
        };

    virtual void ClearHaltedFlag( void ) {;};
    void StopTransfers(void) { return;};    
    
    HCD_REQUEST_STATUS AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE /*lpCancelAddress*/,
                                IN const LPVOID /*lpvNotifyParameter*/,
                                IN LPCVOID /*lpvCancelId*/ )
        {return requestFailed;};

    // ****************************************************
    // Public Variables for CQueuedPipe
    // ****************************************************
    virtual CPhysMem * GetCPhysMem() {return m_pCPhysMem;};

private:
    // ****************************************************
    // Private Functions for CQueuedPipe
    // ****************************************************
    void  AbortQueue( void ) { ; };
    HCD_REQUEST_STATUS  ScheduleTransfer( void ) { return requestFailed;};

    // ****************************************************
    // Private Variables for CQueuedPipe
    // ****************************************************
    IN CPhysMem * const m_pCPhysMem;
protected:
    // ****************************************************
    // Protected Functions for CQueuedPipe
    // ****************************************************
//#ifdef DEBUG
    const TCHAR*  GetPipeType( void ) const
    {
        static const TCHAR* cszPipeType = TEXT("Dummy");
        return cszPipeType;
    }
//#endif // DEBUG

    virtual BOOL    AreTransferParametersValid( const STransfer * /*pTransfer = NULL*/ )  const { return FALSE;};

    BOOL    CheckForDoneTransfers( void ) { return FALSE; };

};

class CPeriodicMgr : public LockObject {
public:
    CPeriodicMgr(IN CPhysMem * const pCPhysMem, DWORD dwFlameSize);
    ~CPeriodicMgr();
    BOOL Init();
    void DeInit() ;
    DWORD GetFrameSize() { return m_dwFrameSize; };
private:
    CPhysMem * const m_pCPhysMem;
    //Frame;
    CDummyPipe * const m_pCDumpPipe;
public:    
    DWORD GetFrameListPhysAddr() { return m_pFramePhysAddr; };
private:
    const DWORD m_dwFrameSize;
    // Isoch Periodic List.
    DWORD   m_pFramePhysAddr;
    DWORD   m_dwFrameMask;
    volatile DWORD * m_pFrameList; // point to dword (physical address)
    // Periodic For Interrupt.
#define PERIOD_TABLE_SIZE 32
    CQH *   m_pStaticQHArray[2*PERIOD_TABLE_SIZE];
    PBYTE   m_pStaticQH;
    // Interrupt Endpoint Span
public:
    // ITD Service.
    BOOL QueueITD(CITD * piTD,DWORD FrameIndex);
    BOOL QueueSITD(CSITD * psiTD,DWORD FrameIndex);
    BOOL DeQueueTD(DWORD dwPhysAddr,DWORD FrameIndex);
    // Pseriodic Qhead Service
    CQH * QueueQHead(CQH * pQh,UCHAR uInterval,UCHAR offset,BOOL bHighSpeed);
    BOOL DequeueQHead( CQH * pQh);
private:
    static PERIOD_TABLE periodTable[64];
    
};

class CAsyncMgr: public LockObject {
public:
    CAsyncMgr(IN CPhysMem * const pCPhysMem);
    ~CAsyncMgr();
    BOOL Init();
    void DeInit() ;
private:
    CPhysMem * const m_pCPhysMem;
    //Frame;
    CDummyPipe * const m_pCDumpPipe;
    CQH * m_pStaticQHead;
public:   
    DWORD GetPhysAddr() { return (m_pStaticQHead?m_pStaticQHead->GetPhysAddr():0); };
public:
    // Service.
    CQH *  QueueQH(CQH * pQHead);    
    BOOL DequeueQHead( CQH * pQh);
};
typedef struct _PIPE_LIST_ELEMENT {
    CPipe*                      pPipe;
    struct _PIPE_LIST_ELEMENT * pNext;
} PIPE_LIST_ELEMENT, *PPIPE_LIST_ELEMENT;

class CBusyPipeList : public LockObject {
public:
    CBusyPipeList(DWORD dwFrameSize) { m_FrameListSize=dwFrameSize;};
    ~CBusyPipeList() {DeInit();};
    BOOL Init();
    void DeInit();
    BOOL AddToBusyPipeList( IN CPipe * const pPipe, IN const BOOL fHighPriority );
    void RemoveFromBusyPipeList( IN CPipe * const pPipe );
    void SignalCheckForDoneTransfers( void );
private:
    // ****************************************************
    // Private Functions for CPipe
    // ****************************************************
    static ULONG CALLBACK CheckForDoneTransfersThreadStub( IN PVOID pContext);
    ULONG CheckForDoneTransfersThread();
private:
    DWORD   m_FrameListSize ;
    // ****************************************************
    // Private Variables for CPipe
    // ****************************************************
    // CheckForDoneTransfersThread related variables
    BOOL             m_fCheckTransferThreadClosing; // signals CheckForDoneTransfersThread to exit
    HANDLE           m_hCheckForDoneTransfersEvent; // event for CheckForDoneTransfersThread
    HANDLE           m_hCheckForDoneTransfersThread; // thread for handling done transfers
    PPIPE_LIST_ELEMENT m_pBusyPipeList;
#ifdef DEBUG
    int              m_debug_numItemsOnBusyPipeList;
#endif // DEBUG    
};

// this class is an encapsulation of UHCI hardware registers.
class CHW : public CHcd {
public:
    // ****************************************************
    // public Functions
    // ****************************************************

    // 
    // Hardware Init/Deinit routines
    //
#ifdef QFE_MERGE /*070630*/ /*CE5QFE*/
    CHW( IN const REGISTER portBase,
                              IN const DWORD dwOffset,
                              IN const DWORD dwSysIntr,
                              IN CPhysMem * const pCPhysMem,
                              //IN CUhcd * const pHcd,
                              IN LPVOID pvUhcdPddObject,
                              IN LPCTSTR lpDeviceRegistry);
#else
    CHW( IN const REGISTER portBase,
                              IN const DWORD dwSysIntr,
                              IN CPhysMem * const pCPhysMem,
                              //IN CUhcd * const pHcd,
                              IN LPVOID pvUhcdPddObject );
#endif
    ~CHW(); 
    virtual BOOL    Initialize();
    virtual void    DeInitialize( void );
    virtual void    SignalCheckForDoneTransfers( void ) { 
            m_cBusyPipeList.SignalCheckForDoneTransfers();
        };
    
    void   EnterOperationalState(void);

    void   StopHostController(void);
    void   RunStopUSBController(BOOL fRun);

    void   EnterLowPowerMode(void);

    //
    // Functions to Query frame values
    //
    BOOL GetFrameNumber( OUT LPDWORD lpdwFrameNumber );

    BOOL GetFrameLength( OUT LPUSHORT lpuFrameLength );
    
    BOOL SetFrameLength( IN HANDLE hEvent,
                                IN USHORT uFrameLength );
    
    BOOL StopAdjustingFrame( void );

    BOOL WaitOneFrame( void );

    //
    // Root Hub Queries
    //
    BOOL DidPortStatusChange( IN const UCHAR port );

    BOOL GetPortStatus( IN const UCHAR port,
                               OUT USB_HUB_AND_PORT_STATUS& rStatus,
                               OUT PDWORD pdwForceDetach);

    BOOL RootHubFeature( IN const UCHAR port,
                                IN const UCHAR setOrClearFeature,
                                IN const USHORT feature );

    BOOL ResetAndEnablePort( IN const UCHAR port );

    void DisablePort( IN const UCHAR port );

    BOOL IsOTGHostDevice(void); // 0 - A device, 1 - B device

    virtual BOOL WaitForPortStatusChange (HANDLE m_hHubChanged);
    //
    // Miscellaneous bits
    //
    PULONG GetFrameListAddr( ) { return m_pFrameList; };
    // PowerCallback
    VOID PowerMgmtCallback( IN BOOL fOff );

    // Ringo SOC - non EHCI standard
    // 00 - Full Speed, 01 - Low Speed, 10 - High Speed, 11 - Unknown
    USHORT GetPortSpeed (IN const USHORT port);
    BOOL EnableAsyncSchedule();
    BOOL DisableAsyncSchedule();
    BOOL PowerOnAllPorts();
    BOOL ConfigureHS();
    void WriteAsyncListAddr(IN const DWORD addr);
    void StartHostController(void);
    DWORD GetForceReAttach() { return m_dwForceReAttach; };
    void SetForceReAttach(IN const DWORD dwForceReAttach) { m_dwForceReAttach = dwForceReAttach;};
    BOOL StopUSBClock(void);
    BOOL StartUSBClock(void);
    void WakeUpSysIntr(void);
    
#if 1
    // This function was provided for CDevice to access 
    // EHCI Registers
    PUCHAR GetBaseMem(void) {return (PUCHAR)(m_capBase - m_dwOffset);};
#endif


#ifdef QFE_MERGE /*070630*/ /*CE5QFE*/
    CRegistryEdit   m_deviceReg;
#endif
private:
    // ****************************************************
    // private Functions
    // ****************************************************
    
    static DWORD CALLBACK CeResumeThreadStub( IN PVOID context );
    DWORD CeResumeThread();
    static DWORD CALLBACK UsbInterruptThreadStub( IN PVOID context );
    DWORD UsbInterruptThread();

    static DWORD CALLBACK UsbAdjustFrameLengthThreadStub( IN PVOID context );
    DWORD UsbAdjustFrameLengthThread();

    void   UpdateFrameCounter( void );
    VOID    SuspendHostController();
    VOID    ResumeHostController();
        
#ifdef DEBUG
    // Query Host Controller for registers, and prints contents
    void DumpUSBCMD(void);
    void DumpUSBSTS(void);
    void DumpUSBINTR(void);
    void DumpFRNUM(void);
    void DumpFLBASEADD(void);
    void DumpSOFMOD(void);
    void DumpAllRegisters(void);
    void DumpPORTSC( IN const USHORT port );
#endif

    //
    // EHCI USB I/O registers (See UHCI spec, section 2)
    //
    
    // EHCI Spec - Section 2.3.1
    // USB Command Register (USBCMD)
    typedef struct {
        DWORD   RunStop:1; // Run/Stop
        DWORD   HCReset:1; //Controller Reset
        DWORD   FrameListSize:2;
        DWORD   PSchedEnable:1;
        DWORD   ASchedEnable:1;
        DWORD   IntOnAADoorbell:1;
        DWORD   LHCReset:1;
        DWORD   ASchedPMCount:2;
        DWORD   Reserved:1;
        DWORD   ASchedPMEnable:1;
        DWORD   Reserved2:4;
        DWORD   IntThreshCtrl:8;
        DWORD   Reserved3:8;
    } USBCMD_Bit;
    typedef union {
        volatile USBCMD_Bit bit;
        volatile DWORD ul;
    } USBCMD;

    inline USBCMD Read_USBCMD( void )
    {
        DEBUGCHK( m_portBase != 0 );
        USBCMD usbcmd;
        usbcmd.ul=READ_REGISTER_ULONG( ((PULONG) m_portBase) );
        return usbcmd;
    }
    inline void Write_USBCMD( IN const USBCMD data )
    {
        DEBUGCHK( m_portBase != 0 );
    #ifdef DEBUG // added this after accidentally writing to USBCMD instead of USBSTS
        if (data.bit.RunStop && data.bit.HCReset && data.bit.LHCReset) {
            DEBUGMSG( ZONE_WARNING, (TEXT("!!!Warning!!! Setting resume/suspend/reset bits of USBCMD\n")));
        }
    #endif // DEBUG
        WRITE_REGISTER_ULONG( ((PULONG)m_portBase), data.ul );
    }
    
    // EHCI Spec - Section 2.3.2
    // USB Status Register (USBSTS)
    typedef struct {
        DWORD   USBINT:1;
        DWORD   USBERRINT:1;
        DWORD   PortChanged:1;
        DWORD   FrameListRollover:1;
        DWORD   HSError:1;
        DWORD   ASAdvance:1;
        DWORD   Reserved:6;
        DWORD   HCHalted:1;
        DWORD   Reclamation:1;
        DWORD   PSStatus:1;
        DWORD   ASStatus:1;
        DWORD   Reserved2:16;
    } USBSTS_Bit;
    typedef union {
        volatile USBSTS_Bit  bit;
        volatile DWORD       ul;
    } USBSTS;
    inline USBSTS Read_USBSTS( void )
    {
        DEBUGCHK( m_portBase != 0 );
        USBSTS data;
        data.ul=READ_REGISTER_ULONG( (PULONG)(m_portBase + 0x4) );
    #ifdef DEBUG // added this after accidentally writing to USBCMD instead of USBSTS
        if (data.bit.USBERRINT && data.bit.HSError && data.bit.HCHalted) {
            DEBUGMSG( ZONE_WARNING, (TEXT("!!!Warning!!! status show error/halted bits of USBSTS\n")));
        }
    #endif // DEBUG
        return data;
    }
    inline void Write_USBSTS( IN const USBSTS data )
    {
        DEBUGCHK( m_portBase != 0 );
        WRITE_REGISTER_ULONG( (PULONG)(m_portBase + 0x4), data.ul );
    }
    inline void Clear_USBSTS( void )
    {
        USBSTS clearUSBSTS;
        clearUSBSTS.ul=(DWORD)-1;
        clearUSBSTS.bit.Reserved=0;
        clearUSBSTS.bit.Reserved2=0;
        // write to USBSTS will clear contents
        Write_USBSTS(clearUSBSTS );
    }

    // EHCI Spec - Section 2.3.3
    // USB Interrupt Enable Register (USBINTR)
    typedef struct {
        DWORD   USBINT:1;
        DWORD   USBERRINT:1;
        DWORD   PortChanged:1;
        DWORD   FrameListRollover:1;
        DWORD   HSError:1;
        DWORD   ASAdvance:1;
        DWORD   Reserved:26;
    } USBINTR_Bit;
    typedef union {
        volatile USBINTR_Bit bit;
        volatile DWORD       ul;
    } USBINTR;
    
    inline USBINTR Read_USBINTR( void )
    {
        DEBUGCHK( m_portBase != 0 );
        USBINTR usbintr;
        usbintr.ul=READ_REGISTER_ULONG( (PULONG)(m_portBase + 0x8) );
        return usbintr;
    }
    inline void Write_USBINTR( IN const USBINTR data )
    {
        DEBUGCHK( m_portBase != 0 );
        WRITE_REGISTER_ULONG( ((PULONG)(m_portBase + 0x8)), data.ul );
    }

    // EHCI Spec - Section 2.3.4
    typedef struct {
        DWORD microFlame:3;
        DWORD FrameIndex:11;
        DWORD Reserved:18;
    } FRINDEX_Bit;
    typedef union  {
        volatile FRINDEX_Bit bit;
        volatile DWORD       ul;
    } FRINDEX;
    inline FRINDEX Read_FRINDEX( void )
    {
        DEBUGCHK( m_portBase != 0 );
        FRINDEX frindex;
        frindex.ul=READ_REGISTER_ULONG( (PULONG)(m_portBase + 0xc) );
        return frindex;
    }
    inline void Write_FRINDEX( IN const FRINDEX data )
    {
        DEBUGCHK( m_portBase != 0 );
        WRITE_REGISTER_ULONG( (PULONG)(m_portBase + 0xc), data.ul );
    }
    
#define CTLDSSEGMENT        0x10
#define PERIODICLISTBASE    0x14
#define ASYNCLISTADDR       0x18
#define USBMODE             0x68
#define CONFIGFLAG          0x40
#define OTGSC_OFFSET        0x64
#define TXFILLTUNING_OFFSET 0x24


#define  EHCD_FLBASEADD_MASK 0xfffff000
    inline void Write_EHCIRegister(IN ULONG const Index, IN ULONG const data) 
    {
        DEBUGCHK( m_portBase != 0 );
        WRITE_REGISTER_ULONG( ((PULONG)(m_portBase + Index)), data );
    }
    inline ULONG Read_EHCIRegister( IN ULONG const Index )
    {
        DEBUGCHK( m_portBase != 0 );
        return READ_REGISTER_ULONG( ((PULONG)(m_portBase + Index)) );
    }
    // UHCI Spec - Section 2.1.7
    typedef struct {
        DWORD   ConnectStatus:1;
        DWORD   ConnectStatusChange:1;
        DWORD   Enabled:1;
        DWORD   EnableChange:1;
        DWORD   OverCurrentActive:1;
        DWORD   OverCurrentChange:1;
        DWORD   ForcePortResume:1;
        DWORD   Suspend:1;     
        DWORD   Reset:1;
        DWORD   Reserved:1;
        DWORD   LineStatus:2;
        DWORD   Power:1;
        DWORD   Owner:1;
        DWORD   Indicator:2;
        DWORD   TestControl:4;
        DWORD   WakeOnConnect:1;
        DWORD   WakeOnDisconnect:1;
        DWORD   WakeOnOverCurrent:1;
        // The following is for Freescale Ringo SOC
        DWORD   PHCD:1;
        DWORD   PFSC:1;
        DWORD   Reserved2:1;
        DWORD   PSPD:2;
        DWORD   PTW:1;
        DWORD   STS:1;
        DWORD   PTS:2;      
    }PORTSC_Bit;
    typedef union {
        volatile PORTSC_Bit  bit;
        volatile DWORD       ul;
    } PORTSC;
    inline PORTSC Read_PORTSC( IN const UINT port )
    {
        DEBUGCHK( m_portBase != 0 );
        // check that we're trying to read a valid port
        DEBUGCHK( port <= m_NumOfPort && port !=0 );
        // port #1 is at 0x10, port #2 is at 0x12
        PORTSC portsc;
        portsc.ul=READ_REGISTER_ULONG( (PULONG)(m_portBase + 0x44  + 4 * (port-1)) );
        return portsc;
    }
    inline void Write_PORTSC( IN const UINT port, IN const PORTSC data )
    {
        DEBUGCHK( m_portBase != 0 );
        // check that we're trying to read a valid port
        DEBUGCHK( port <= m_NumOfPort && port !=0 );
        // port #1 is at 0x10, port #2 is at 0x12.
        WRITE_REGISTER_ULONG( (PULONG)(m_portBase + 0x44  + 4 * (port-1)), data.ul );   
    }
    inline UCHAR Read_CapLength(void)
    {
        return READ_REGISTER_UCHAR( (PUCHAR) m_capBase);
    };
    inline USHORT Read_HCIVersion(void)
    {
        return READ_REGISTER_USHORT( (PUSHORT) (m_capBase+2));
    }
    typedef struct {
        DWORD   N_PORTS:4;
        DWORD   PPC:1;
        DWORD   Reserved:2;
        DWORD   PortRoutingRules:1;
        DWORD   N_PCC:4;
        DWORD   N_CC:4;
        DWORD   P_INDICATOR:1;
        DWORD   Reserved2:3;
        DWORD   DebugPortNumber:4;
        DWORD   Reserved3:8;
    } HCSPARAMS_Bit;
    typedef union {
        volatile HCSPARAMS_Bit   bit;
        volatile DWORD           ul;
    } HCSPARAMS;
    inline HCSPARAMS Read_HCSParams(void)
    {
        HCSPARAMS hcsparams;
        hcsparams.ul=READ_REGISTER_ULONG( (PULONG) (m_capBase+4));
        return hcsparams;
    };
    typedef struct {
        DWORD Addr_64Bit:1;
        DWORD Frame_Prog:1;
        DWORD Async_Park:1;
        DWORD Reserved1:1;
        DWORD Isoch_Sched_Threshold:4;
        DWORD EHCI_Ext_Cap_Pointer:8;
        DWORD Reserved2:16;
    } HCCP_CAP_Bit;
    typedef union {
        volatile HCCP_CAP_Bit   bit;
        volatile DWORD          ul;
    } HCCP_CAP;
    inline HCCP_CAP Read_HHCCP_CAP(void) {
        HCCP_CAP hcsparams;
        hcsparams.ul=READ_REGISTER_ULONG( (PULONG) (m_capBase+8));
        return hcsparams;
    }

    typedef struct {
        DWORD VD    : 1;
        DWORD VC    : 1;
        DWORD R1    : 1;
        DWORD OT    : 1;
        DWORD DP    : 1;
        DWORD IDPU  : 1;
        DWORD R2    : 2;
        DWORD ID    : 1;
        DWORD AVV   : 1;
        DWORD ASV   : 1;
        DWORD BSV   : 1;
        DWORD BSE   : 1;
        DWORD OnemsT: 1;
        DWORD DPS   : 1;
        DWORD R3    : 1;

        DWORD IDIS  : 1;
        DWORD AVVIS : 1;
        DWORD ASVIS : 1;
        DWORD BSVIS : 1;
        DWORD BSEIS : 1;
        DWORD Onemss: 1;
        DWORD DPIS  : 1;
        DWORD R4    : 1;
        DWORD IDIE  : 1;
        DWORD AVVIE : 1;
        DWORD ASVIE : 1;
        DWORD BSVIE : 1;
        DWORD BSEIE : 1;
        DWORD OnemsE: 1;
        DWORD DPIE  : 1;
        DWORD R5    : 1;
    } OTGSC_Bit;

    typedef union {
        volatile OTGSC_Bit  bit;
        volatile DWORD      ul;
    } OTGSC;

    inline OTGSC Read_OTGSC(void)
    {
        DEBUGCHK( m_portBase != 0 );
        OTGSC otgsc;
        otgsc.ul=READ_REGISTER_ULONG( (PULONG)(m_portBase + OTGSC_OFFSET) );
        return otgsc;
    }

    typedef struct {
        DWORD txschoh:8;
        DWORD txschealth:5;
        DWORD reserved:3;
        DWORD txfifothres:6;
        DWORD reserved2:10;
    } TXFILLTUNING_Bit;

    typedef union {
        volatile TXFILLTUNING_Bit   bit;
        volatile DWORD              ul;
    } TXFILLTUNING;

    inline TXFILLTUNING Read_TXFILLTUNING(void)
    {
        TXFILLTUNING txft;
        txft.ul = READ_REGISTER_ULONG( (PULONG)(m_portBase + TXFILLTUNING_OFFSET));
        return txft;
    }

    inline void Write_TXFILLTUNING(IN const TXFILLTUNING data)
    {
         WRITE_REGISTER_ULONG( (PULONG)(m_portBase + TXFILLTUNING_OFFSET), data.ul );   
    }

    //
    // ****************************************************
    // Private Variables
    // ****************************************************
    
    REGISTER    m_portBase;
    REGISTER    m_capBase;
    DWORD       m_dwOffset;
    DWORD       m_NumOfPort;

    CAsyncMgr   m_cAsyncMgr;
    CPeriodicMgr m_cPeriodicMgr;
    CBusyPipeList m_cBusyPipeList;
    // internal frame counter variables
    CRITICAL_SECTION m_csFrameCounter;
    DWORD   m_frameCounterHighPart;
    DWORD   m_frameCounterLowPart;
    DWORD   m_FrameListMask;
    // interrupt thread variables
    DWORD    m_dwSysIntr;
    HANDLE   m_hUsbInterruptEvent;
    HANDLE   m_hUsbHubChangeEvent;
    HANDLE   m_hUsbInterruptThread;
    BOOL     m_fUsbInterruptThreadClosing;

    // frame length adjustment variables
    // note - use LONG because we need to use InterlockedTestExchange
    LONG     m_fFrameLengthIsBeingAdjusted;
    LONG     m_fStopAdjustingFrameLength;
    HANDLE   m_hAdjustDoneCallbackEvent;
    USHORT   m_uNewFrameLength;
    PULONG   m_pFrameList;

    DWORD   m_dwCapability;
    BOOL    m_bDoResume;

    // Add for supporting OTG
    DWORD   m_dwOTGSupport;
    TCHAR   m_szOTGGroup[15];

    // Add for Ringo SOC specific
    BOOL    m_bUSBClockStop;
    BOOL    m_bUSBPanicMode;
    BOOL    m_bUSBPanicIntrMask;
    DWORD   m_dwForceReAttach; 
    /* 0 - no force reAttach is required
       1 - regular force reattach request and signal to do a deattach first.
       2 - regular force reattach and send signal to do attach back */
    BOOL    m_bUSBIdleSuspend;
public:
    DWORD   SetCapability(DWORD dwCap); 
    DWORD   GetCapability() { return m_dwCapability; };
private:
    // initialization parameters for the IST to support CE resume
    // (resume from fully unpowered controller).
    //CUhcd    *m_pHcd;
    CPhysMem *m_pMem;
    LPVOID    m_pPddContext;
    BOOL g_fPowerUpFlag ;
    BOOL g_fPowerResuming ;
    HANDLE    m_hAsyncDoorBell;
    LockObject m_DoorBellLock;
    BOOL   bInHost; 
public:
    BOOL GetPowerUpFlag() { return g_fPowerUpFlag; };
    BOOL SetPowerUpFlag(BOOL bFlag) { return (g_fPowerUpFlag=bFlag); };
    BOOL GetPowerResumingFlag() { return g_fPowerResuming ; };
    BOOL SetPowerResumingFlag(BOOL bFlag) { return (g_fPowerResuming=bFlag) ; };
    CPhysMem * GetPhysMem() { return m_pMem; };
    DWORD GetNumberOfPort() { return m_NumOfPort; };
    //Bridge To its Instance.
    BOOL AddToBusyPipeList( IN CPipe * const pPipe, IN const BOOL fHighPriority ) {  return m_cBusyPipeList.AddToBusyPipeList(pPipe,fHighPriority);};
    void RemoveFromBusyPipeList( IN CPipe * const pPipe ) { m_cBusyPipeList.RemoveFromBusyPipeList(pPipe); };
        
    CQH * PeriodQeueuQHead(CQH * pQh,UCHAR uInterval,UCHAR offset,BOOL bHighSpeed){ return m_cPeriodicMgr.QueueQHead(pQh,uInterval,offset,bHighSpeed);};
    BOOL PeriodDeQueueuQHead( CQH * pQh) { return m_cPeriodicMgr.DequeueQHead( pQh); }
    BOOL PeriodQueueITD(CITD * piTD,DWORD FrameIndex) ;//{ return  m_cPeriodicMgr.QueueITD(piTD,FrameIndex); };
    BOOL PeriodQueueSITD(CSITD * psiTD,DWORD FrameIndex);// { return  m_cPeriodicMgr.QueueSITD(psiTD,FrameIndex);};
    BOOL PeriodDeQueueTD(DWORD dwPhysAddr,DWORD FrameIndex) ;//{ return  m_cPeriodicMgr.DeQueueTD(dwPhysAddr, FrameIndex); };
    CPeriodicMgr& GetPeriodicMgr() { return m_cPeriodicMgr; };
    
    CQH *  AsyncQueueQH(CQH * pQHead) { return m_cAsyncMgr.QueueQH(pQHead); };
    BOOL  AsyncDequeueQH( CQH * pQh) ;
    CAsyncMgr& GetAsyncMgr() { return m_cAsyncMgr; };
    BOOL AsyncBell();
    void SignalHubStatusChange(void) { if (m_hUsbHubChangeEvent) SetEvent(m_hUsbHubChangeEvent);};
};

#pragma warning(pop)

#endif

