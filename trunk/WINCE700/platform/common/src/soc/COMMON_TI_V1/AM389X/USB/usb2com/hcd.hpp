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
//     hcd.hpp
// 
// Abstract:  Chcd implements the abstract HCDI interface. It mostly
//            just passes requests on to other objects, which
//            do the real work.
//     
// Notes: 
//
#ifndef __HCD_HPP_
#define __HCD_HPP_

#include <cdevice.hpp>
#include <cphysmem.hpp>

class CHcd;
void InvalidateOpenedPipesCacheEntries(UINT iDevice, CHcd* pHcd);

class CHcd : public  LockObject, public CDeviceGlobal
{
public:
    // ****************************************************
    // Public Functions for CUhcd
    // ****************************************************
    CHcd( );
    // These functions are called by the HCDI interface
    virtual ~CHcd();
    virtual BOOL DeviceInitialize(void)=0;
    virtual void DeviceDeInitialize( void )=0;   
    
    virtual BOOL GetFrameNumber( OUT LPDWORD lpdwFrameNumber )=0;
    virtual BOOL GetFrameLength( OUT LPUSHORT lpuFrameLength )=0;
    virtual BOOL SetFrameLength( IN HANDLE hEvent,IN USHORT uFrameLength )=0;
    virtual BOOL StopAdjustingFrame( void )=0;
    BOOL OpenPipe( IN UINT address,
                   IN LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                   OUT LPUINT lpPipeIndex,
                   OUT LPVOID* const plpCPipe = NULL );
    
    BOOL ClosePipe( IN UINT address,
                    IN UINT pipeIndex );
    
    BOOL IssueTransfer( ISSUE_TRANSFER_PARAMS* pITP );
    
    BOOL AbortTransfer( IN UINT address,
                        IN UINT pipeIndex,
                        IN LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                        IN LPVOID lpvNotifyParameter,
                        IN LPCVOID lpvCancelId );

    BOOL IsPipeHalted( IN UINT address,
                       IN UINT pipeIndex,
                       OUT LPBOOL lpbHalted );

    BOOL ResetPipe( IN UINT address,
                    IN UINT pipeIndex );

#ifdef USB_IF_ELECTRICAL_TEST_MODE
    virtual HCD_COMPLIANCE_TEST_STATUS SetTestMode(IN UINT portNum, IN UINT mode)=0;
#endif //#ifdef USB_IF_ELECTRICAL_TEST_MODE
    
    virtual VOID PowerMgmtCallback( IN BOOL fOff )=0;
    CRootHub* GetRootHub() { return m_pCRootHub;};
    CRootHub* SetRootHub(CRootHub* pRootHub) ;
    virtual DWORD GetNumOfPorts() = 0;
    virtual BOOL DisableDevice( IN const UINT address, 
                                  IN const BOOL fReset ) ;
    
    virtual BOOL SuspendResume( IN const UINT address,
                                  IN const BOOL fSuspend );

    // Abstract for RootHub Function.
    virtual BOOL DidPortStatusChange( IN const UCHAR port )=0;
    virtual BOOL GetPortStatus( IN const UCHAR port,
                               OUT USB_HUB_AND_PORT_STATUS& rStatus )=0;
    virtual BOOL RootHubFeature( IN const UCHAR port,
                                IN const UCHAR setOrClearFeature,
                                IN const USHORT feature )=0;
    virtual BOOL ResetAndEnablePort( IN const UCHAR port )=0;
    virtual void DisablePort( IN const UCHAR port )=0;
    virtual BOOL WaitForPortStatusChange (HANDLE /*m_hHubChanged*/) { return FALSE; };

    virtual LPCTSTR GetControllerName( void ) const =0;
    virtual DWORD   SetCapability(DWORD dwCap)=0; 
    virtual DWORD   GetCapability()=0;
    virtual BOOL    SuspendHC() { return FALSE; }; // Default does not support it function.

    // ****************************************************
    // Public Variables for Chcd
    // ****************************************************
    CEDEVICE_POWER_STATE m_DevPwrState;

    BOOL  m_fDevicePowerDown;
    PVOID m_pOpenedPipesCache;
    DWORD m_dwOpenedPipeCacheSize;

protected:
    virtual BOOL ResumeNotification ()  {
        Lock();
        BOOL fReturn = FALSE;
        if (m_pCRootHub) {
            fReturn = m_pCRootHub->ResumeNotification();
            m_pCRootHub->NotifyOnSuspendedResumed(FALSE);
            m_pCRootHub->NotifyOnSuspendedResumed(TRUE);
        }
        Unlock();
        return fReturn;
    }
private:
    // ****************************************************
    // Private Functions for CUhcd
    // ****************************************************

    // ****************************************************
    // Private Variables for CUhcd
    // ****************************************************
    CRootHub*       m_pCRootHub;            // pointer to CRootHub object, which represents                                            
                                            // the built-in hardware USB ports
};


CHcd * CreateHCDObject(IN LPVOID pvUhcdPddObject,
                       IN CPhysMem * pCPhysMem,
                       IN LPCWSTR szDriverRegistryKey,
                       IN REGISTER portBase,
                       IN DWORD dwSysIntr);
    
extern "C"
{

static BOOL HcdGetFrameNumber(LPVOID lpvHcd, LPDWORD lpdwFrameNumber);
static BOOL HcdGetFrameLength(LPVOID lpvHcd, LPUSHORT lpuFrameLength);
static BOOL HcdSetFrameLength(LPVOID lpvHcd, HANDLE hEvent, USHORT uFrameLength);
static BOOL HcdStopAdjustingFrame(LPVOID lpvHcd);


static BOOL HcdOpenPipe(LPVOID lpvHcd, UINT iDevice,
                 LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 LPUINT lpiEndpointIndex);
static BOOL HcdClosePipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex);
static BOOL HcdResetPipe(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex);
static BOOL HcdIsPipeHalted(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
        LPBOOL lpbHalted);


static BOOL HcdIssueTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
                      LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                      LPVOID lpvNotifyParameter, DWORD dwFlags,
                      LPCVOID lpvControlHeader, DWORD dwStartingFrame,
                      DWORD dwFrames, LPCDWORD aLengths, DWORD dwBufferSize,
                      LPVOID lpvBuffer, ULONG paBuffer, LPCVOID lpvCancelId,
                      LPDWORD adwIsochErrors, LPDWORD adwIsochLengths,
                      LPBOOL lpfComplete, LPDWORD lpdwBytesTransfered,
                      LPDWORD lpdwError);

static BOOL HcdAbortTransfer(LPVOID lpvHcd, UINT iDevice, UINT iEndpointIndex,
                      LPTRANSFER_NOTIFY_ROUTINE lpStartAddress,
                      LPVOID lpvNotifyParameter, LPCVOID lpvCancelId);

}

#endif

