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
// Module Name:  
//     cpipe.h
// 
// Abstract: Implements class for managing open pipes for UHCI
//
//                             CPipe (ADT)
//                           /             \
//                  CQueuedPipe (ADT)       CIsochronousPipe
//                /         |       \ 
//              /           |         \
//   CControlPipe    CInterruptPipe    CBulkPipe
// 
// Notes: 
// 
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#ifndef __CPIPE_H_
#define __CPIPE_H_

#include <globals.hpp>
#include <pipeabs.hpp>
#include "ctd.h"
#include "usb2lib.h"

// Remove-W4: Warning C4512 workaround
#pragma warning(push)
#pragma warning(disable: 4512)

class CPhysMem;
class CEhcd;
typedef struct STRANSFER STransfer;
class CTransfer ;
class CIsochTransfer;
class CPipe : public CPipeAbs {
public:
    // ****************************************************
    // Public Functions for CPipe
    // ****************************************************

    CPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
           IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
           IN const UCHAR bDeviceAddress,
           IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
           IN CEhcd * const pCEhcd);

    virtual ~CPipe();

    virtual HCD_REQUEST_STATUS  OpenPipe( void ) = 0;

    virtual HCD_REQUEST_STATUS  ClosePipe( void ) = 0;

    virtual HCD_REQUEST_STATUS AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                                IN const LPVOID lpvNotifyParameter,
                                IN LPCVOID lpvCancelId ) = 0;

    HCD_REQUEST_STATUS IsPipeHalted( OUT LPBOOL const lpbHalted );
    UCHAR   GetSMask(){ return  m_bFrameSMask; };
    UCHAR   GetCMask() { return m_bFrameCMask; };
    BOOL    IsHighSpeed() { return m_fIsHighSpeed; };
    BOOL    IsLowSpeed() { return m_fIsLowSpeed; };
    virtual CPhysMem * GetCPhysMem();
    virtual HCD_REQUEST_STATUS  ScheduleTransfer( void ) = 0;
    virtual BOOL    CheckForDoneTransfers( void ) = 0;
//#ifdef DEBUG
    virtual const TCHAR*  GetPipeType( void ) const = 0;
//#endif // DEBUG


    virtual void ClearHaltedFlag( void );
    USB_ENDPOINT_DESCRIPTOR GetEndptDescriptor() { return m_usbEndpointDescriptor;};
    UCHAR GetDeviceAddress() { return m_bDeviceAddress; };
    virtual void StopTransfers(void)=0;  
    
    
    // ****************************************************
    // Public Variables for CPipe
    // ****************************************************
    UCHAR const m_bHubAddress;
    UCHAR const m_bHubPort;
    CEhcd * const m_pCEhcd;
private:
    // ****************************************************
    // Private Functions for CPipe
    // ****************************************************

protected:
    // ****************************************************
    // Protected Functions for CPipe
    // ****************************************************
    virtual BOOL    AreTransferParametersValid( const STransfer *pTransfer = NULL ) const = 0;

    
    // pipe specific variables
    UCHAR   m_bFrameSMask;
    UCHAR   m_bFrameCMask;
    CRITICAL_SECTION        m_csPipeLock;           // crit sec for this specific pipe's variables
    USB_ENDPOINT_DESCRIPTOR m_usbEndpointDescriptor; // descriptor for this pipe's endpoint
    UCHAR                   m_bDeviceAddress;       // Device Address that assigned by HCD.
    BOOL                    m_fIsLowSpeed;          // indicates speed of this pipe
    BOOL                    m_fIsHighSpeed;         // Indicates speed of this Pipe;
    BOOL                    m_fIsHalted;            // indicates pipe is halted
};
class CQueuedPipe : public CPipe
{

public:
    // ****************************************************
    // Public Functions for CQueuedPipe
    // ****************************************************
    CQueuedPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
                 IN CEhcd * const pCEhcd);
    virtual ~CQueuedPipe();

    inline const int GetTdSize( void ) const { return sizeof(CQTD); };

    HCD_REQUEST_STATUS  IssueTransfer( 
                                IN const UCHAR address,
                                IN LPTRANSFER_NOTIFY_ROUTINE const lpfnCallback,
                                IN LPVOID const lpvCallbackParameter,
                                IN const DWORD dwFlags,
                                IN LPCVOID const lpvControlHeader,
                                IN const DWORD dwStartingFrame,
                                IN const DWORD dwFrames,
                                IN LPCDWORD const aLengths,
                                IN const DWORD dwBufferSize,     
                                IN_OUT LPVOID const lpvBuffer,
                                IN const ULONG paBuffer,
                                IN LPCVOID const lpvCancelId,
                                OUT LPDWORD const adwIsochErrors,
                                OUT LPDWORD const adwIsochLengths,
                                OUT LPBOOL const lpfComplete,
                                OUT LPDWORD const lpdwBytesTransferred,
                                OUT LPDWORD const lpdwError ) ;

    HCD_REQUEST_STATUS AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                                IN const LPVOID lpvNotifyParameter,
                                IN LPCVOID lpvCancelId );
    CQH *  GetQHead() { return m_pPipeQHead; };
    HCD_REQUEST_STATUS  ScheduleTransfer( void );
    BOOL    CheckForDoneTransfers( void );
    virtual void ClearHaltedFlag( void );
    void StopTransfers(void);    
    // ****************************************************
    // Public Variables for CQueuedPipe
    // ****************************************************

protected:
    // ****************************************************
    // Private Functions for CQueuedPipe
    // ****************************************************
    void  AbortQueue( void ); 
    virtual BOOL RemoveQHeadFromQueue() = 0;
    virtual BOOL InsertQHeadToQueue() = 0 ;
    // ****************************************************
    // Private Variables for CQueuedPipe
    // ****************************************************

    // ****************************************************
    // Protected Functions for CQueuedPipe
    // ****************************************************
    CQH *   m_pPipeQHead;

    // ****************************************************
    // Protected Variables for CQueuedPipe
    // ****************************************************
    //BOOL         m_fIsReclamationPipe; // indicates if this pipe is participating in bandwidth reclamation
//    UCHAR        m_dataToggle;         // Transfer data toggle.
    // WARNING! These parameters are treated as a unit. They
    // can all be wiped out at once, for example when a 
    // transfer is aborted.
    CQTransfer *             m_pUnQueuedTransfer;      // ptr to last transfer in queue
    CQTransfer *             m_pQueuedTransfer;
};

class CBulkPipe : public CQueuedPipe
{
public:
    // ****************************************************
    // Public Functions for CBulkPipe
    // ****************************************************
    CBulkPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
               IN CEhcd * const pCEhcd);
    ~CBulkPipe();

    HCD_REQUEST_STATUS  OpenPipe( void );
    HCD_REQUEST_STATUS  ClosePipe( void );
    // ****************************************************
    // Public variables for CBulkPipe
    // ****************************************************
//#ifdef DEBUG  
    const TCHAR*  GetPipeType( void ) const
    {
        return TEXT("Bulk");
    }
//#endif // DEBUG
protected :
    virtual BOOL RemoveQHeadFromQueue();
    virtual BOOL InsertQHeadToQueue() ;
private:
    // ****************************************************
    // Private Functions for CBulkPipe
    // ****************************************************

    BOOL   AreTransferParametersValid( const STransfer *pTransfer = NULL ) const;

    // ****************************************************
    // Private variables for CBulkPipe
    // ****************************************************
};
class CControlPipe : public CQueuedPipe
{
public:
    // ****************************************************
    // Public Functions for CBulkPipe
    // ****************************************************
    CControlPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
               IN CEhcd * const pCEhcd);
    ~CControlPipe();
    void ChangeMaxPacketSize( IN const USHORT wMaxPacketSize );
    HCD_REQUEST_STATUS  OpenPipe( void );
    HCD_REQUEST_STATUS  ClosePipe( void );
    // ****************************************************
    // Public variables for CBulkPipe
    // ****************************************************
//#ifdef DEBUG
    const TCHAR*  GetPipeType( void ) const
    {
        return TEXT("Control");
    }
//#endif // DEBUG
    HCD_REQUEST_STATUS  IssueTransfer( 
                                IN const UCHAR address,
                                IN LPTRANSFER_NOTIFY_ROUTINE const lpfnCallback,
                                IN LPVOID const lpvCallbackParameter,
                                IN const DWORD dwFlags,
                                IN LPCVOID const lpvControlHeader,
                                IN const DWORD dwStartingFrame,
                                IN const DWORD dwFrames,
                                IN LPCDWORD const aLengths,
                                IN const DWORD dwBufferSize,     
                                IN_OUT LPVOID const lpvBuffer,
                                IN const ULONG paBuffer,
                                IN LPCVOID const lpvCancelId,
                                OUT LPDWORD const adwIsochErrors,
                                OUT LPDWORD const adwIsochLengths,
                                OUT LPBOOL const lpfComplete,
                                OUT LPDWORD const lpdwBytesTransferred,
                                OUT LPDWORD const lpdwError ) ;

protected :
    virtual BOOL RemoveQHeadFromQueue();
    virtual BOOL InsertQHeadToQueue() ;
private:
    // ****************************************************
    // Private Functions for CBulkPipe
    // ****************************************************

    BOOL   AreTransferParametersValid( const STransfer *pTransfer = NULL ) const;

    // ****************************************************
    // Private variables for CBulkPipe
    // ****************************************************
};
class CInterruptPipe : public CQueuedPipe
{
public:
    // ****************************************************
    // Public Functions for CInterruptPipe
    // ****************************************************
CInterruptPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
               IN CEhcd * const pCEhcd);
~CInterruptPipe();

    HCD_REQUEST_STATUS  OpenPipe( void );
    HCD_REQUEST_STATUS  ClosePipe( void );
//#ifdef DEBUG  
    const TCHAR*  GetPipeType( void ) const
    {
        static const TCHAR* cszPipeType = TEXT("Interrupt");
        return cszPipeType;
    }
//#endif // DEBUG

protected :
    virtual BOOL RemoveQHeadFromQueue() { return TRUE;}; // We do not need for Interrupt
    virtual BOOL InsertQHeadToQueue() {return TRUE;} ;

private:
    // ****************************************************
    // Private Functions for CInterruptPipe
    // ****************************************************


    void                UpdateInterruptQHTreeLoad( IN const UCHAR branch,
                                                   IN const int   deltaLoad );

    BOOL                AreTransferParametersValid( const STransfer *pTransfer = NULL ) const;


    HCD_REQUEST_STATUS  AddTransfer( CQTransfer *pTransfer );

    EndpointBuget m_EndptBuget;

    BOOL m_bSuccess;
    // ****************************************************
    // Private variables for CInterruptPipe
    // ****************************************************
};
#define MIN_ADVANCED_FRAME 6
class CIsochronousPipe : public CPipe
{
public:
    // ****************************************************
    // Public Functions for CIsochronousPipe
    // ****************************************************
    CIsochronousPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
               IN CEhcd *const pCEhcd);
    ~CIsochronousPipe();

    HCD_REQUEST_STATUS  OpenPipe( void );

    HCD_REQUEST_STATUS  ClosePipe( void );

    virtual HCD_REQUEST_STATUS  IssueTransfer( 
                                IN const UCHAR address,
                                IN LPTRANSFER_NOTIFY_ROUTINE const lpfnCallback,
                                IN LPVOID const lpvCallbackParameter,
                                IN const DWORD dwFlags,
                                IN LPCVOID const lpvControlHeader,
                                IN const DWORD dwStartingFrame,
                                IN const DWORD dwFrames,
                                IN LPCDWORD const aLengths,
                                IN const DWORD dwBufferSize,     
                                IN_OUT LPVOID const lpvBuffer,
                                IN const ULONG paBuffer,
                                IN LPCVOID const lpvCancelId,
                                OUT LPDWORD const adwIsochErrors,
                                OUT LPDWORD const adwIsochLengths,
                                OUT LPBOOL const lpfComplete,
                                OUT LPDWORD const lpdwBytesTransferred,
                                OUT LPDWORD const lpdwError );

    HCD_REQUEST_STATUS  AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                                IN const LPVOID lpvNotifyParameter,
                                IN LPCVOID lpvCancelId );

    // ****************************************************
    // Public variables for CIsochronousPipe
    // ****************************************************

    HCD_REQUEST_STATUS  ScheduleTransfer( void );
    BOOL                CheckForDoneTransfers( void );
    CITD *  AllocateCITD( CITransfer *  pTransfer);
    CSITD * AllocateCSITD( CSITransfer * pTransfer,CSITD * pPrev);
    void    FreeCITD(CITD *  pITD);
    void    FreeCSITD(CSITD * pSITD);
#ifdef QFE_MERGE /*070930*/ /*CE6QFE*/
    DWORD   GetMaxTransferPerItd() { return m_dwMaxTransPerItd; };
    DWORD   GetTDInteval() { return m_dwTDInterval; };
#endif
//#ifdef DEBUG

    const TCHAR*  GetPipeType( void ) const
    {
        static const TCHAR* cszPipeType = TEXT("Isochronous");
        return cszPipeType;
    }
//#endif // DEBUG
    void StopTransfers(void) { return;};    
private:
    // ****************************************************
    // Private Functions for CIsochronousPipe
    // ****************************************************

    BOOL                AreTransferParametersValid( const STransfer *pTransfer = NULL ) const;

    HCD_REQUEST_STATUS  AddTransfer( STransfer *pTransfer );


    CIsochTransfer *    m_pQueuedTransfer;
    DWORD           m_dwLastValidFrame;
    EndpointBuget   m_EndptBuget;
    BOOL            m_bSuccess;
    
    CITD **         m_pArrayOfCITD;
    CSITD **        m_pArrayOfCSITD;
    DWORD           m_dwNumOfTD;
    DWORD           m_dwNumOfTDAvailable;
#ifdef QFE_MERGE /*070930*/ /*CE6QFE*/
    DWORD           m_dwMaxTransPerItd;
    DWORD           m_dwTDInterval;
#endif

};

#pragma warning(pop)

#endif
