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
//     CPipe.cpp
// Abstract:  
//     Implements the Pipe class for managing open pipes for UHCI
//
//                             CPipe (ADT)
//                           /             \
//                  CQueuedPipe (ADT)       CIsochronousPipe
//                /         |       \ 
//              /           |         \
//   CControlPipe    CInterruptPipe    CBulkPipe
// 
// 
// Notes: 
// 
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include "trans.h"
#include "Cpipe.h"
#pragma warning(pop)

#include "chw.h"
#include "CEhcd.h"

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

#define RETAIL_LOG 0
#define CRITICAL_LOG 0
#define UFI_LOG 0
#define IRAM_MEM_LOG 0
// ******************************************************************
// Scope: public 
CPipe::CPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
              IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
              IN const UCHAR bDeviceAddress,
              IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
              IN CEhcd *const pCEhcd)
//
// Purpose: constructor for CPipe
//
// Parameters: lpEndpointDescriptor - pointer to endpoint descriptor for
//                                    this pipe (assumed non-NULL)
//
//             fIsLowSpeed - indicates if this pipe is low speed
//
// Returns: Nothing.
//
// Notes: Most of the work associated with setting up the pipe
//        should be done via OpenPipe. The constructor actually
//        does very minimal work.
//
//        Do not modify static variables here!!!!!!!!!!!
// ******************************************************************
: CPipeAbs(lpEndpointDescriptor->bEndpointAddress )
, m_usbEndpointDescriptor( *lpEndpointDescriptor )
, m_bDeviceAddress(bDeviceAddress)
, m_pCEhcd(pCEhcd)
, m_fIsLowSpeed( !!fIsLowSpeed ) // want to ensure m_fIsLowSpeed is 0 or 1
, m_fIsHighSpeed( !!fIsHighSpeed)
, m_fIsHalted( FALSE )
, m_bHubAddress (bHubAddress)
, m_bHubPort (bHubPort)
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CPipe::CPipe\n")) );
    // CPipe::Initialize should already have been called by now
    // to set up the schedule and init static variables
    //DEBUGCHK( pUHCIFrame->m_debug_fInitializeAlreadyCalled );

    InitializeCriticalSection( &m_csPipeLock );
    m_fIsHalted = FALSE;
    // Assume it is Async. If it is not It should be ovewrited.
    m_bFrameSMask =  0;
    m_bFrameCMask =  0;

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CPipe::CPipe\n")) );
}

// ******************************************************************
// Scope: public virtual 
CPipe::~CPipe( )
//
// Purpose: Destructor for CPipe
//
// Parameters: None
//
// Returns: Nothing.
//
// Notes:   Most of the work associated with destroying the Pipe
//          should be done via ClosePipe
//
//          Do not delete static variables here!!!!!!!!!!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CPipe::~CPipe\n")) );
    // transfers should be aborted or closed before deleting object
    DeleteCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CPipe::~CPipe\n")) );
}
CPhysMem *CPipe::GetCPhysMem()
{
     return m_pCEhcd->GetPhysMem();
}
#ifdef IRAM_PATCH
CPhysMem *CPipe::GetIRamElePhysMem()
{
     return m_pCEhcd->GetIRamEleMem();
}
CPhysMem *CPipe::GetIRamDataPhysMem()
{
    return m_pCEhcd->GetIRamDataMem();
}
#endif
// ******************************************************************
// Scope: public
HCD_REQUEST_STATUS CPipe::IsPipeHalted( OUT LPBOOL const lpbHalted )
//
// Purpose: Return whether or not this pipe is halted (stalled)
//
// Parameters: lpbHalted - pointer to BOOL which receives
//                         TRUE if pipe halted, else FALSE
//
// Returns: requestOK 
//
// Notes:  Caller should check for lpbHalted to be non-NULL
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CPipe(%s)::IsPipeHalted\n"), GetPipeType()) );

    DEBUGCHK( lpbHalted ); // should be checked by CUhcd

    EnterCriticalSection( &m_csPipeLock );
    if (lpbHalted)
        *lpbHalted = m_fIsHalted;
    LeaveCriticalSection( &m_csPipeLock );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CPipe(%s)::IsPipeHalted, *lpbHalted = %d, returning HCD_REQUEST_STATUS %d\n"), GetPipeType(), *lpbHalted, requestOK) );
    return requestOK;
}
// ******************************************************************
// Scope: public
void CPipe::ClearHaltedFlag( void )
//
// Purpose: Clears the pipe is halted flag
//
// Parameters: None
//
// Returns: Nothing 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CPipe(%s)::ClearHaltedFlag\n"), GetPipeType() ) );

    EnterCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_WARNING && !m_fIsHalted, (TEXT("CPipe(%s)::ClearHaltedFlag - warning! Called on non-stalled pipe\n"), GetPipeType()) );
    m_fIsHalted = FALSE;
    LeaveCriticalSection( &m_csPipeLock );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CPipe(%s)::ClearHaltedFlag\n"), GetPipeType()) );
}

// ******************************************************************               
// Scope: public
CQueuedPipe::CQueuedPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
                 IN CEhcd *const pCEhcd)
//
// Purpose: Constructor for CQueuedPipe
//
// Parameters: See CPipe::CPipe
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
: CPipe( lpEndpointDescriptor, fIsLowSpeed,fIsHighSpeed, bDeviceAddress,bHubAddress,bHubPort, pCEhcd )   // constructor for base class
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CQueuedPipe::CQueuedPipe\n")) );
    m_pPipeQHead = NULL;
    m_pUnQueuedTransfer=NULL;      // ptr to last transfer in queue
    m_pQueuedTransfer=NULL;
#ifdef SYNC_CQP
    m_transDoneEvent=CreateEvent(NULL, FALSE, TRUE, NULL);
#endif
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CQueuedPipe::CQueuedPipe\n")) );
}

// ******************************************************************               
// Scope: public virtual
CQueuedPipe::~CQueuedPipe( )
//
// Purpose: Destructor for CQueuedPipe
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CQueuedPipe::~CQueuedPipe\n")) );
    // queue should be freed via ClosePipe before calling destructor
    EnterCriticalSection( &m_csPipeLock );
    ASSERT(m_pPipeQHead == NULL);
    ASSERT(m_pUnQueuedTransfer==NULL);      // ptr to last transfer in queue
    ASSERT(m_pQueuedTransfer==NULL);
#ifdef SYNC_CQP
    CloseHandle(m_transDoneEvent);
#endif
    LeaveCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CQueuedPipe::~CQueuedPipe\n")) );
}
// ******************************************************************               
// Scope: public (implements CPipe::AbortTransfer = 0)
HCD_REQUEST_STATUS CQueuedPipe::AbortTransfer( 
                                IN const LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                                IN const LPVOID lpvNotifyParameter,
                                IN LPCVOID lpvCancelId )
//
// Purpose: Abort any transfer on this pipe if its cancel ID matches
//          that which is passed in.
//
// Parameters: lpCancelAddress - routine to callback after aborting transfer
//
//             lpvNotifyParameter - parameter for lpCancelAddress callback
//
//             lpvCancelId - identifier for transfer to abort
//
// Returns: requestOK if transfer aborted
//          requestFailed if lpvCancelId doesn't match currently executing
//                 transfer, or if there is no transfer in progress
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("+CQueuedPipe(%s)::AbortTransfer - lpvCancelId = 0x%x\n"), GetPipeType(), lpvCancelId) );
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"Abort c%x\r\n", lpvCancelId));
#endif

    HCD_REQUEST_STATUS status = requestFailed;

    EnterCriticalSection( &m_csPipeLock );
    CQTransfer * pCurTransfer = m_pUnQueuedTransfer;
    CQTransfer * pPrevTransfer= NULL;
    while (pCurTransfer && pCurTransfer->GetSTransfer().lpvCancelId !=  lpvCancelId ) {
        pPrevTransfer = pCurTransfer;
        pCurTransfer= ( CQTransfer *)pCurTransfer->GetNextTransfer();
        
    };
    if (pCurTransfer) { // Found Transfer that not queue yet.
        pCurTransfer->AbortTransfer();
        if (pPrevTransfer)
            pPrevTransfer->SetNextTransfer(pCurTransfer->GetNextTransfer());
        else
            m_pUnQueuedTransfer = ( CQTransfer *)pCurTransfer->GetNextTransfer();
    }
    else {
        if (m_pQueuedTransfer!=NULL &&  m_pQueuedTransfer->GetSTransfer().lpvCancelId ==  lpvCancelId ) {
            // This is one in the schdeule
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"cancel the one in schedule\r\n"));
#endif
            RemoveQHeadFromQueue();;
            m_pQueuedTransfer->AbortTransfer();           
            GetQHead()->InvalidNextTD(); 
#ifdef IRAM_PATCH
            GetQHead()->FreeIRamSpace();
#endif
            Sleep(2);// this sleep is for Interrupt Pipe;
            pCurTransfer = m_pQueuedTransfer;
            m_pQueuedTransfer = NULL;
            InsertQHeadToQueue() ;
        }
        else 
            ASSERT(FALSE);        
    };
    if (pCurTransfer) {
        pCurTransfer->DoneTransfer();
        if ( lpCancelAddress ) {
            __try { // calling the Cancel function
                ( *lpCancelAddress )( lpvNotifyParameter );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                  DEBUGMSG( ZONE_ERROR, (TEXT("CQueuedPipe::AbortTransfer - exception executing cancellation callback function\n")) );
            }
        }
        status = requestOK;
        delete pCurTransfer;
#ifdef IRAM_PATCH
        if ((m_pQueuedTransfer == NULL) && (m_pUnQueuedTransfer != NULL)) {
#else
        if (m_pQueuedTransfer == NULL ) { // This queue is no longer active. re-activate it.
#endif
            ScheduleTransfer();
        }
    }
    LeaveCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_TRANSFER, (TEXT("-CQueuedPipe(%s)::AbortTransfer - lpvCancelId = 0x%x, returning HCD_REQUEST_STATUS %d\n"), GetPipeType(), lpvCancelId, status) );
    return status;
}
// ******************************************************************
// Scope: public
void CQueuedPipe::ClearHaltedFlag( void )
//
// Purpose: Clears the pipe is halted flag
//
// Parameters: None
//
// Returns: Nothing 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CQueuedPipe(%s)::ClearHaltedFlag\n"), GetPipeType() ) );

    EnterCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_WARNING && !m_fIsHalted, (TEXT("CQueuedPipe(%s)::ClearHaltedFlag - warning! Called on non-stalled pipe\n"), GetPipeType()) );
    m_fIsHalted = FALSE;
    m_pPipeQHead->ResetOverlayDataToggle();
    LeaveCriticalSection( &m_csPipeLock );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CQueuedPipe(%s)::ClearHaltedFlag\n"), GetPipeType()) );
}
// ******************************************************************               
// Scope: private
void  CQueuedPipe::AbortQueue( void ) 
//
// Purpose: Abort the current transfer (i.e., queue of TDs).
//
// Parameters: pQH - pointer to Queue Head for transfer to abort
//
// Returns: Nothing
//
// Notes: not used for OHCI
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("+CQueuedPipe(%s)::AbortQueue \n"), GetPipeType()) );
    EnterCriticalSection( &m_csPipeLock );
    if ( m_pUnQueuedTransfer) {      // ptr to last transfer in queue
        while ( m_pUnQueuedTransfer) {
            m_pUnQueuedTransfer ->AbortTransfer();
            m_pUnQueuedTransfer ->DoneTransfer();
            CQTransfer * pCurTransfer= (CQTransfer *) m_pUnQueuedTransfer->GetNextTransfer();
            delete m_pUnQueuedTransfer;
            m_pUnQueuedTransfer = pCurTransfer;
        }       
    }
    ASSERT( m_pUnQueuedTransfer == NULL);
    if (m_pQueuedTransfer) {
        RemoveQHeadFromQueue();
        //m_pQueuedTransfer;
        m_pQueuedTransfer ->AbortTransfer();
        GetQHead()->InvalidNextTD();  
        m_pCEhcd->AsyncBell();// Ask HC update internal structure.
        Sleep(2);// this sleep is for Interrupt Pipe;
        m_pQueuedTransfer->DoneTransfer();
        //m_pQueuedTransfer =  NULL;
        delete m_pQueuedTransfer;
        m_pQueuedTransfer = NULL;
        InsertQHeadToQueue() ;
    }
    ASSERT(m_pQueuedTransfer == NULL);

    LeaveCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_TRANSFER, (TEXT("-CQueuedPipe(%s)::AbortQueue - %d\n"), GetPipeType()) );
}

// ******************************************************************               
// Scope: public 
HCD_REQUEST_STATUS  CQueuedPipe::IssueTransfer( 
                                    IN const UCHAR address,
                                    IN LPTRANSFER_NOTIFY_ROUTINE const lpStartAddress,
                                    IN LPVOID const lpvNotifyParameter,
                                    IN const DWORD dwFlags,
                                    IN LPCVOID const lpvControlHeader,
                                    IN const DWORD dwStartingFrame,
                                    IN const DWORD dwFrames,
                                    IN LPCDWORD const aLengths,
                                    IN const DWORD dwBufferSize,     
                                    IN_OUT LPVOID const lpvClientBuffer,
                                    IN const ULONG paBuffer,
                                    IN LPCVOID const lpvCancelId,
                                    OUT LPDWORD const adwIsochErrors,
                                    OUT LPDWORD const adwIsochLengths,
                                    OUT LPBOOL const lpfComplete,
                                    OUT LPDWORD const lpdwBytesTransferred,
                                    OUT LPDWORD const lpdwError )
//
// Purpose: Issue a Transfer on this pipe
//
// Parameters: address - USB address to send transfer to
//
//             OTHER PARAMS - see comment in CUhcd::IssueTransfer
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes:   
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("+CPipe(%s)::IssueTransfer, address = %d\n"), GetPipeType(), address) );
#ifdef SYNC_CQP
    WaitForSingleObject(m_transDoneEvent, INFINITE);
    RETAILMSG(CRITICAL_LOG, (L"issue next CQT\r\n"));
#endif

#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"+P(%s)::IT c%x\n", GetPipeType(), lpvCancelId));
    RETAILMSG(CRITICAL_LOG, (L"\r\nCQP: s %d, v %x, p %x, c %x\r\n", dwBufferSize, lpvClientBuffer, paBuffer, lpvCancelId));

    if ((lpvClientBuffer!=NULL) && (dwBufferSize==31))
    {
        if (((UCHAR*)lpvClientBuffer)[15]==0x2A)
        {
            RETAILMSG(UFI_LOG, (L"w %d sectors\r\n", (((UCHAR*)lpvClientBuffer)[22]*256 + ((UCHAR*)lpvClientBuffer)[23])));
        }
        else if (((UCHAR*)lpvClientBuffer)[15]==0x28)
        {
            RETAILMSG(UFI_LOG, (L"r %d sectors\r\n", (((UCHAR*)lpvClientBuffer)[22]*256 + ((UCHAR*)lpvClientBuffer)[23])));
        }
        else
        {
            RETAILMSG(UFI_LOG, (L"UFI %x\r\n",(((UCHAR*)lpvClientBuffer)[15])));
        }
    }
    //if (dwBufferSize == 31)
    //{
    //    int i;
    //    PUCHAR pBuf = (PUCHAR)lpvClientBuffer;
    //    for (i=0; i!=31; i++)
    //    {
    //        RETAILMSG(1, (L"%2x\t", pBuf[i]));
    //        if (((i+1)==31)||((i+1)%8==0)) RETAILMSG(1, (L"\r\n"));
    //    }
    //}
#endif
    STransfer sTransfer = {
    // These are the IssueTransfer parameters
        lpStartAddress,lpvNotifyParameter, dwFlags,lpvControlHeader, dwStartingFrame,dwFrames,
        aLengths,dwBufferSize,lpvClientBuffer,paBuffer,lpvCancelId,adwIsochErrors, adwIsochLengths,
        lpfComplete,lpdwBytesTransferred,lpdwError};
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"STransfer len %d\r\n", dwBufferSize));
#endif
    HCD_REQUEST_STATUS  status = requestFailed;
    if (AreTransferParametersValid(&sTransfer) && GetQHead() && m_bDeviceAddress == address ) {
        EnterCriticalSection( &m_csPipeLock );
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
        __try { // initializing transfer status parameters
            *sTransfer.lpfComplete = FALSE;
            *sTransfer.lpdwBytesTransferred = 0;
            *sTransfer.lpdwError = USB_NOT_COMPLETE_ERROR;
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"new CQ"));
#endif
//#ifdef IRAM_PATCH
//            CQTransfer * pTransfer = new CQTransfer(this,m_pCEhcd->GetPhysMem(), m_pCEhcd->GetIRamEleMem(), m_pCEhcd->GetIRamDataMem(), &sTransfer);
//#else
            CQTransfer * pTransfer = new CQTransfer(this,m_pCEhcd->GetPhysMem(),&sTransfer);
//#endif
            if (pTransfer && pTransfer->Init()) {
                CQTransfer * pCur = m_pUnQueuedTransfer;
                if (pCur) {
                    while (pCur->GetNextTransfer()!=NULL)
                         pCur = (CQTransfer * )pCur->GetNextTransfer();
                    pCur->SetNextTransfer( pTransfer);
                }
                else
                    m_pUnQueuedTransfer=pTransfer;
                status=requestOK ;
            }
            else
                if (pTransfer) { // We return fails here so do not need callback;
                    pTransfer->DoNotCallBack() ;
                    delete pTransfer;    
                }
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
        }
#pragma prefast(pop)
        LeaveCriticalSection( &m_csPipeLock );
#ifdef IRAM_PATCH
        if ((m_pQueuedTransfer==NULL) && (m_pUnQueuedTransfer!=NULL))
            ScheduleTransfer();
#else
        ScheduleTransfer();
#endif
    }
    else
        ASSERT(FALSE);
    DEBUGMSG( ZONE_TRANSFER, (TEXT("-CPipe(%s)::IssueTransfer - address = %d, returing HCD_REQUEST_STATUS %d\n"), GetPipeType(), address, status) );
    return status;
}

//******************************************************************
void CQueuedPipe::StopTransfers(void)
// 
// Purpose:  Stop the transfer
// 
// Returns: Nothing
//
//*********************************************************************
{
    DEBUGMSG(ZONE_PIPE, (TEXT("CQueuePipe::StopTransfers\r\n")));    
#ifdef EHCI_PROBE
    RETAILMSG(CRITICAL_LOG, (L"Abort Queue\r\n"));
#endif
    if ( m_pPipeQHead) 
        AbortQueue();
}

// ******************************************************************               
// Scope: public 
HCD_REQUEST_STATUS   CQueuedPipe::ScheduleTransfer( void )
//
// Purpose: Schedule a Transfer on this pipe
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes:   
// ******************************************************************
{
    HCD_REQUEST_STATUS  status = requestFailed;
    EnterCriticalSection( &m_csPipeLock );
#ifdef EHCI_PROBE
    RETAILMSG(CRITICAL_LOG, (L"ST! Q %x UQ %x QH %x\r\n", m_pQueuedTransfer, m_pUnQueuedTransfer, m_pPipeQHead));
#endif
    if ( m_pQueuedTransfer == NULL && m_pUnQueuedTransfer!=NULL  && 
            m_pPipeQHead && m_pPipeQHead->IsActive()==FALSE) { // We need cqueue new Transfer.
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"Get a List\r\n"));
#endif
        CQTransfer * pCurTransfer = m_pUnQueuedTransfer;
        ASSERT(pCurTransfer!=NULL);
        m_pUnQueuedTransfer = (CQTransfer * )pCurTransfer->GetNextTransfer();
        pCurTransfer->SetNextTransfer(NULL);

        if (GetQHead()->QueueTD(pCurTransfer->GetCQTDList())==TRUE) {
#ifdef EHCI_PROBE
            RETAILMSG(CRITICAL_LOG, (L"Link a List to %x\r\n", GetQHead()));
#endif
            m_pQueuedTransfer = pCurTransfer;
            ASSERT(pCurTransfer->GetCQTDList()->GetTransfer()== pCurTransfer);
            status=requestOK ;
#ifdef IRAM_PATCH
            /*
             * In IRAM design, QueueTD only links TD list to QH virtural pointer, the
             * actural transfer not begins yet.
             *
             * We need to call ScheduleSubTransfer to do actural transfers
             * 
             * ScheduleSubTransfer select the first element in virtural Link List, make
             * a clone of it in TD area, but modification should be done to its BP
             * pointer. Direct to specific IRAM area. If it is OUT transfer, we also need
             * to memcopy the buffer content from DRAM to SRAM. Its next pointer should 
             * point to NULL
             * 
             * After all these perparation is done. We Link this IRAM-TD to QH, this begin
             * the actural transfer
             */
            //RETAILMSG(1, (L"Set QH bufsize as %d\r\n", GetQHead()->GetBufSize()));
            GetQHead()->SetBufSize(pCurTransfer->GetBufSize());
            ScheduleSubTransfer();
#endif
        }
        else {
            ASSERT(FALSE);
            // Can not Queue. 
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"Do we hit here?\r\n"));
#endif
            m_fIsHalted = TRUE;
            m_pQueuedTransfer = pCurTransfer;
            m_pQueuedTransfer ->AbortTransfer();
            CheckForDoneTransfers();
        }
    }
    else 
        DEBUGMSG( ZONE_TRANSFER, (TEXT("-CPipe(%s)::ScheduleTransfer - Schedule called during QHead Busy or nothing to schedule! \n"), GetPipeType()) );
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"ST-\r\n"));
#endif
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}

#ifdef IRAM_PATCH
// ******************************************************************               
// Scope: public 
void CQueuedPipe::ScheduleSubTransfer(void)
//
// Purpose: Schedule a real Transfer on this pipe
//
// Returns: none
//
// Notes: every time before this is called, any concerning
//        bit is checked to make sure the "Link" Operation
//        is successful, so no return need
// ******************************************************************
{
    GetQHead()->QueueNextSubTD();
}
#endif
// ******************************************************************               
// Scope: protected (Implements CPipe::CheckForDoneTransfers = 0)
BOOL CQueuedPipe::CheckForDoneTransfers( void )
//
// Purpose: Check if the transfer on this pipe is finished, and 
//          take the appropriate actions - i.e. remove the transfer
//          data structures and call any notify routines
//
// Parameters: None
//
// Returns: TRUE if transfer is done, else FALSE
//
// Notes:
// ******************************************************************
{
#ifdef IRAM_PATCH
    /*
     * BookMarks : TransferDone
     */
    /*
     * Oirginally, this function checks if all element in TD list
     * are finished. If so, call "DoneTransfer", else do nothing
     *
     * in IRAM patch impelmentation. Some modification should be done
     *     1. First check if the current iRAM TD is finished
     *     2. If not do nothing, else
     *     3. Post processing for this single iRAM TD
     *         3.1 software back-overlay
     *         3.2 data buffer memcopy for IN transfer
     *         3.3 free the iRAM TD memory
     *     4. Look if this TD correspond to the last TD in TD list
     *        yes -> 5, no -> 6
     *     5. DoneTransfer, similar algorithm as in original function
     *     6. advance virturl Link list, ScheduleSubTransfer
     */
    BOOL bReturn = FALSE;
#ifdef SYNC_CQP
    BOOL bSetDoneEvent = FALSE;
#endif
    RETAILMSG(RETAIL_LOG, (L"TDone+ q %x unq %x qh %x\r\n", m_pQueuedTransfer, m_pUnQueuedTransfer, m_pPipeQHead));
    RETAILMSG(RETAIL_LOG, (L"e CS %x\r\n", &m_csPipeLock));
    EnterCriticalSection( &m_csPipeLock );
    RETAILMSG(RETAIL_LOG, (L"OK\r\n"));

    if (m_pQueuedTransfer!=NULL && m_pPipeQHead!=NULL) 
    {
        CQTD* pCurTD = m_pPipeQHead->GetCurrentTD();
#ifdef EHCI_PROBE
        //RETAILMSG(1, (L"pCurTD is %x, its start is %x, len is %d\r\n", pCurTD, pCurTD->StartOfBuffer(), pCurTD->BufLength()));
        RETAILMSG(CRITICAL_LOG, (L"IDone?\r\n"));
#endif
        if (m_pPipeQHead->IsIRamTdDone())
        {
#ifdef EHCI_PROBE
            RETAILMSG(CRITICAL_LOG, (L"Overlay\r\n"));
#endif
            //memcpy(m_pPipeQHead->GetIRamTD(), m_pPipeQHead->GetCurrentTD(), sizeof(CQTD));
            pCurTD->Overlay(m_pPipeQHead->GetIRamTD());
#ifdef EHCI_PROBE
            //RETAILMSG(1, (L"Done\r\n"));
#endif
            if (!m_pPipeQHead->GetCurrentTD()->IsOutTransfer())
            {
#ifdef IRAM_PATCH
                CQTD* pCurTD = m_pPipeQHead->GetCurrentTD();
                RETAILMSG(CRITICAL_LOG, (L"Mcpy IN f %x t %x, size %d, %x...\r\n", 
                            m_pPipeQHead->StartOfIRamBuffer(), pCurTD->StartOfBuffer(), pCurTD->BufLength(), *m_pPipeQHead->StartOfIRamBuffer()));
#endif
                RETAILMSG(IRAM_MEM_LOG, (L"mc %d fr %x\r\n", pCurTD->BufLength(), m_pPipeQHead->StartOfIRamBuffer()));
                memcpy(m_pPipeQHead->GetCurrentTD()->StartOfBuffer(),
                        m_pPipeQHead->StartOfIRamBuffer(), 
                        pCurTD->BufLength());
            }
            //TODO : Free IRAM space
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"FIRam\r\n"));
#endif
            m_pPipeQHead->InvalidNextTD();
            m_pPipeQHead->FreeIRamSpace();

            RETAILMSG(IRAM_MEM_LOG, (L"F ITD, E&D\r\n"));
#if (IRAM_MEM_LOG == 1)
            GetIRamElePhysMem()->ShowMemInfo();
            GetIRamDataPhysMem()->ShowMemInfo();
#endif

            bReturn = TRUE;
            //4. 
            if (!m_pPipeQHead->AdvanceTD())
            {
                // means we have finished the last TD
                CQTransfer * pCurTransfer = m_pQueuedTransfer;
                if (pCurTransfer->IsTransferDone() == TRUE) 
                {  
                    // means all TD in current CQTransfer has active = 0
                    RETAILMSG(CRITICAL_LOG, (L"CQT Done\r\n"));
                    ASSERT(m_pPipeQHead->IsActive() == FALSE) ;// Pipe Stopped.
                    m_fIsHalted = (pCurTransfer->DoneTransfer()!=TRUE);
                    // Put it into done Queue.
                    delete pCurTransfer;
                    if (m_fIsHalted)
                        m_pPipeQHead->ResetOverlayDataToggle();
                    // Excute Next one if there is any.
                    m_pQueuedTransfer =NULL;
                    //m_pPipeQHead->InvalidNextTD();
#ifdef SYNC_CQP
                    bSetDoneEvent = TRUE;
#endif
                }
            }
            else
            {
                ScheduleSubTransfer();
#ifdef SYNC_CQP
                bSetDoneEvent = FALSE;
#endif
            }
        }
    }

    if ((m_pQueuedTransfer==NULL)&&(m_pUnQueuedTransfer!=NULL))
    {
        RETAILMSG(RETAIL_LOG, (L"IST-ST\r\n"));
        ScheduleTransfer();
    }
    LeaveCriticalSection( &m_csPipeLock );
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"Leave %x\r\n", &m_csPipeLock));
    RETAILMSG(RETAIL_LOG, (L"TDONE-\r\n"));
#endif
#ifdef SYNC_CQP
    if (bSetDoneEvent)
    {
        SetEvent(m_transDoneEvent);
    }
#endif
    return bReturn;   
#else
    BOOL bReturn = FALSE;
    EnterCriticalSection( &m_csPipeLock );
    
    if (m_pQueuedTransfer!=NULL && m_pPipeQHead!=NULL) {
        // Check All the transfer done or not.
        CQTransfer * pCurTransfer = m_pQueuedTransfer;
        if (pCurTransfer->IsTransferDone() == TRUE) {  // means all TD in current CQTransfer has active = 0
            ASSERT(m_pPipeQHead->IsActive() == FALSE) ;// Pipe Stopped.
            m_fIsHalted = (pCurTransfer->DoneTransfer()!=TRUE);
            // Put it into done Queue.
            delete pCurTransfer;
            if (m_fIsHalted)
                m_pPipeQHead->ResetOverlayDataToggle();
            // Excute Next one if there is any.
            m_pQueuedTransfer =NULL;
            m_pPipeQHead->InvalidNextTD();
            bReturn = TRUE;
        }
    }
    if (m_pQueuedTransfer==NULL)
    if (m_pQueuedTransfer==NULL)
        ScheduleTransfer();
    LeaveCriticalSection( &m_csPipeLock );
    return bReturn;   
#endif
}

// ******************************************************************               
// Scope: public
CBulkPipe::CBulkPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
                 IN CEhcd *const pCEhcd)
//
// Purpose: Constructor for CBulkPipe
//
// Parameters: See CQueuedPipe::CQueuedPipe
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
: CQueuedPipe(lpEndpointDescriptor,fIsLowSpeed, fIsHighSpeed,bDeviceAddress, bHubAddress, bHubPort,  pCEhcd ) // constructor for base class
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CBulkPipe::CBulkPipe\n")) );
    DEBUGCHK( m_usbEndpointDescriptor.bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
              m_usbEndpointDescriptor.bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) &&
              (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_BULK );

    DEBUGCHK( !fIsLowSpeed ); // bulk pipe must be high speed

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CBulkPipe::CBulkPipe\n")) );
}

// ******************************************************************               
// Scope: public
CBulkPipe::~CBulkPipe( )
//
// Purpose: Destructor for CBulkPipe
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CBulkPipe::~CBulkPipe\n")) );
    ClosePipe();
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CBulkPipe::~CBulkPipe\n")) );
}

//********************************************************************
BOOL CBulkPipe::RemoveQHeadFromQueue() 
//
// Purpose: Bulk Pipe: Remove the queue head from the Asynch Queue list
//
// Parameters: None
//
// Returns: TRUE - success, FALSE  - failure
//
// ******************************************************************
{
    ASSERT(m_pPipeQHead);
    return (m_pCEhcd->AsyncDequeueQH( m_pPipeQHead )!=NULL);
}

//*******************************************************************
BOOL CBulkPipe::InsertQHeadToQueue() 
//
// Purpose: Bulk Pipe: Insert a queue head into the Asynch Queue list
//
// Parameters: None
//
// Returns: TRUE - success, FALSE  - failure
//
// ******************************************************************
{
    ASSERT(m_pPipeQHead);
    return (m_pCEhcd->AsyncQueueQH( m_pPipeQHead )!=NULL);
}
// ******************************************************************               
// Scope: public (Implements CPipe::OpenPipe = 0)
HCD_REQUEST_STATUS CBulkPipe::OpenPipe( void )
//
// Purpose: Create the data structures necessary to conduct
//          transfers on this pipe
//
// Parameters: None
//
// Returns: requestOK - if pipe opened
//
//          requestFailed - if pipe was not opened
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("+CBulkPipe::OpenPipe\n") ) );

    HCD_REQUEST_STATUS status = requestFailed;
    m_pUnQueuedTransfer=NULL;      // ptr to last transfer in queue
    m_pQueuedTransfer=NULL;
    PREFAST_DEBUGCHK( m_pCEhcd!=NULL );
    EnterCriticalSection( &m_csPipeLock );

    // if this fails, someone is trying to open
    // an already opened pipe
    DEBUGCHK( m_pPipeQHead  == NULL );
    ASSERT(m_pCEhcd !=NULL);
    // if this fails, we have a low speed Bulk device
    // which is not allowed by the UHCI spec (sec 1.3)
    DEBUGCHK( !m_fIsLowSpeed );
    // Freescale iMX31 specific
 
    if (m_pPipeQHead == NULL ) {
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"new QH @ %x of %x\r\n", m_pCEhcd->GetPhysMem(), m_pCEhcd));
#endif
#ifdef IRAM_PATCH
        RETAILMSG(CRITICAL_LOG, (L"new IQH\r\n"));
#ifdef IRAM_PATCH_EXTEND
        m_pPipeQHead = new(m_pCEhcd->GetIRamEleMem()) CQH (this, m_pCEhcd->GetPhysMem(), m_pCEhcd->GetIRamEleMem(), m_pCEhcd->GetIRamDataMem(), TRUE);
        //RETAILMSG(1, (L"new Bulk QH\r\n"));
        //m_pCEhcd->GetIRamEleMem()->ShowMemInfo();
#else
        m_pPipeQHead = new(m_pCEhcd->GetIRamEleMem()) CQH (this, m_pCEhcd->GetIRamEleMem(), m_pCEhcd->GetIRamDataMem(), TRUE);
#endif
        RETAILMSG(IRAM_MEM_LOG, (L"Bulk Open Pipe, new QH Ele Ram\r\n"));
#if (IRAM_MEM_LOG == 1)
        m_pCEhcd->GetIRamEleMem()->ShowMemInfo();
#endif
#else
        m_pPipeQHead = new( m_pCEhcd->GetPhysMem()) CQH (this);
#endif
#ifdef EHCI_PROBE
        RETAILMSG(CRITICAL_LOG, (L"BULK QH %x\r\n", m_pPipeQHead));
#endif
        if (m_pPipeQHead) {
            m_pPipeQHead->SetDTC(FALSE); // Auto Data Toggle for Bulk.
            if (m_pCEhcd->AsyncQueueQH( m_pPipeQHead )) {
                status = requestOK;
            }
            else {
                ASSERT(FALSE);
                delete m_pPipeQHead;
                m_pPipeQHead=NULL;
            }
        }
        else {
            ASSERT(FALSE);
        }
    }
    LeaveCriticalSection( &m_csPipeLock );
    ASSERT(m_pPipeQHead  != NULL);
    if (status == requestOK) {
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
        BOOL bReturn = m_pCEhcd->AddToBusyPipeList(this, FALSE);
#else
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"Add Bulk to Busy\r\n"));
#endif
        m_pCEhcd->AddToBusyPipeList(this, FALSE);
#endif
        ASSERT(bReturn == TRUE);
    }
    DEBUGMSG( ZONE_PIPE, (TEXT("-CBulkPipe::OpenPipe, returning HCD_REQUEST_STATUS %d\n"), status) );
    return status;
}
// ******************************************************************               
// Scope: public (Implements CPipe::ClosePipe = 0)
HCD_REQUEST_STATUS CBulkPipe::ClosePipe( void )
//
// Purpose: Abort any transfers associated with this pipe, and
//          remove its data structures from the schedule
//
// Parameters: None
//
// Returns: requestOK
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("+CBulkPipe(%s)::ClosePipe\n"), GetPipeType() ) );
    HCD_REQUEST_STATUS status = requestFailed;
    m_pCEhcd->RemoveFromBusyPipeList(this );
    EnterCriticalSection( &m_csPipeLock );
    if ( m_pPipeQHead) {
#ifdef IRAM_PATCH
        m_pPipeQHead->FreeIRamSpace();
        RETAILMSG(IRAM_MEM_LOG, (L"Bulk Close Pipe, Data RAM\r\n"));
#if (IRAM_MEM_LOG == 1)
        m_pCEhcd->GetIRamDataMem()->ShowMemInfo();
#endif
#endif
        AbortQueue();
        m_pCEhcd->AsyncDequeueQH( m_pPipeQHead );
        DWORD dwPhysMem = m_pPipeQHead->GetPhysAddr();
#ifdef EHCI_PROBE
        RETAILMSG(CRITICAL_LOG, (L"free bulk qh @ %x\r\n", dwPhysMem));
#endif
        m_pPipeQHead->~CQH();
#ifdef IRAM_PATCH
        m_pCEhcd->GetIRamEleMem()->FreeMemory((PBYTE)m_pPipeQHead, dwPhysMem , CPHYSMEM_FLAG_HIGHPRIORITY |CPHYSMEM_FLAG_NOBLOCK);
        RETAILMSG(IRAM_MEM_LOG, (L"Bulk Close Pipe, Ele RAM\r\n"));
#if (IRAM_MEM_LOG == 1)
        m_pCEhcd->GetIRamEleMem()->ShowMemInfo();
#endif
#else
        m_pCEhcd->GetPhysMem()->FreeMemory((PBYTE)m_pPipeQHead, dwPhysMem , CPHYSMEM_FLAG_HIGHPRIORITY |CPHYSMEM_FLAG_NOBLOCK);
#endif
        m_pPipeQHead = NULL;
        status = requestOK;
    }
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}
// ******************************************************************               
// Scope: private (Implements CPipe::AreTransferParametersValid = 0)
BOOL CBulkPipe::AreTransferParametersValid( const STransfer *pTransfer ) const 
//
// Purpose: Check whether this class' transfer parameters are valid.
//          This includes checking m_transfer, m_pPipeQH, etc
//
// Parameters: None (all parameters are vars of class)
//
// Returns: TRUE if parameters valid, else FALSE
//
// Notes: Assumes m_csPipeLock already held
// ******************************************************************
{
    if (pTransfer == NULL) {
        ASSERT(FALSE);
        return FALSE;
    }
        
    
    //DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("+CBulkPipe::AreTransferParametersValid\n")) );

    // these parameters aren't used by CBulkPipe, so if they are non NULL,
    // it doesn't present a serious problem. But, they shouldn't have been
    // passed in as non-NULL by the calling driver.
    DEBUGCHK( pTransfer->adwIsochErrors == NULL && // ISOCH
              pTransfer->adwIsochLengths == NULL && // ISOCH
              pTransfer->aLengths == NULL && // ISOCH
              pTransfer->lpvControlHeader == NULL ); // CONTROL
    // this is also not a serious problem, but shouldn't happen in normal
    // circumstances. It would indicate a logic error in the calling driver.
    DEBUGCHK( !(pTransfer->lpStartAddress == NULL && pTransfer->lpvNotifyParameter != NULL) );
    // DWORD                     pTransfer->dwStartingFrame (ignored - ISOCH)
    // DWORD                     pTransfer->dwFrames (ignored - ISOCH)

    BOOL fValid = ( m_pPipeQHead!=NULL &&
                    (pTransfer->lpvBuffer != NULL || pTransfer->dwBufferSize == 0) &&
                    // paClientBuffer could be 0 or !0
                    m_bDeviceAddress > 0 && m_bDeviceAddress <= USB_MAX_ADDRESS &&
                    pTransfer->lpfComplete != NULL &&
                    pTransfer->lpdwBytesTransferred != NULL &&
                    pTransfer->lpdwError != NULL );

    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE && !fValid, (TEXT("!CBulkPipe::AreTransferParametersValid, returning BOOL %d\n"), fValid) );
    ASSERT(fValid);
    return fValid;
}

// ******************************************************************               
// Scope: public
CControlPipe::CControlPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
                 IN CEhcd *const pCEhcd)
//
// Purpose: Constructor for CControlPipe
//
// Parameters: See CQueuedPipe::CQueuedPipe
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
: CQueuedPipe( lpEndpointDescriptor, fIsLowSpeed, fIsHighSpeed, bDeviceAddress,bHubAddress, bHubPort, pCEhcd ) // constructor for base class
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CControlPipe::CControlPipe\n")) );
    DEBUGCHK( m_usbEndpointDescriptor.bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
              m_usbEndpointDescriptor.bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) &&
              (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_CONTROL );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CControlPipe::CControlPipe\n")) );
}

// ******************************************************************               
// Scope: public
CControlPipe::~CControlPipe( )
//
// Purpose: Destructor for CControlPipe
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CControlPipe::~CControlPipe\n")) );
    ClosePipe();
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CControlPipe::~CControlPipe\n")) );
}

// ******************************************************************               
// Scope: public (Implements CPipe::OpenPipe = 0)
HCD_REQUEST_STATUS CControlPipe::OpenPipe( void )
//
// Purpose: Create the data structures necessary to conduct
//          transfers on this pipe
//
// Parameters: None
//
// Returns: requestOK - if pipe opened
//
//          requestFailed - if pipe was not opened
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("+CControlPipe::OpenPipe\n") ) );
    HCD_REQUEST_STATUS status = requestFailed;
    m_pUnQueuedTransfer=NULL;      // ptr to last transfer in queue
    m_pQueuedTransfer=NULL;
    PREFAST_DEBUGCHK( m_pCEhcd!=NULL );
    EnterCriticalSection( &m_csPipeLock );
    // if this fails, someone is trying to open
    // an already opened pipe
    DEBUGCHK( m_pPipeQHead  == NULL );
    ASSERT(m_pCEhcd !=NULL);

 
    // if this fails, we have a low speed Bulk device
    // which is not allowed by the UHCI spec (sec 1.3)
    if (m_pPipeQHead == NULL ) {
#ifdef IRAM_PATCH
#ifdef IRAM_PATCH_EXTEND
        m_pPipeQHead = new(m_pCEhcd->GetIRamEleMem()) CQH (this, m_pCEhcd->GetPhysMem(), m_pCEhcd->GetIRamEleMem(), m_pCEhcd->GetIRamDataMem(), TRUE);
        //RETAILMSG(1, (L"New Control QH\r\n"));
        //m_pCEhcd->GetIRamEleMem()->ShowMemInfo();
#else
        m_pPipeQHead = new(m_pCEhcd->GetIRamEleMem()) CQH (this, m_pCEhcd->GetIRamEleMem(), m_pCEhcd->GetIRamDataMem(), TRUE);
#endif
        RETAILMSG(IRAM_MEM_LOG, (L"Control Open Pipe, Ele Ram\r\n"));
#if (IRAM_MEM_LOG == 1)
        m_pPipeQHead->Dump();
        m_pCEhcd->GetIRamEleMem()->ShowMemInfo();
#endif
#else
        m_pPipeQHead = new(m_pCEhcd->GetPhysMem()) CQH (this);
#endif
#ifdef EHCI_PROBE
        RETAILMSG(CRITICAL_LOG, (L"Control QH %x\r\n", m_pPipeQHead));
        //RETAILMSG(CRITICAL_LOG, (L"has phyiscal %x\r\n", m_pPipeQHead->GetPhysMem()));
#endif
        if (m_pPipeQHead ) {
            m_pPipeQHead->SetDTC(TRUE); // Self Data Toggle for Control 
            if (!m_fIsHighSpeed)
                m_pPipeQHead->SetControlEnpt(TRUE) ; 
            if (m_pCEhcd->AsyncQueueQH( m_pPipeQHead ) ) {
                status = requestOK;
            }
            else {
                delete m_pPipeQHead;
                m_pPipeQHead=NULL;
                ASSERT(FALSE);
            }
                
        }
        else {
            ASSERT(FALSE);
        }
    }
    RETAILMSG(IRAM_MEM_LOG, (L"Post QH is\r\n"));
#if (IRAM_MEM_LOG == 1)
    m_pPipeQHead->Dump();
#endif
    LeaveCriticalSection( &m_csPipeLock );

    if (status == requestOK) {
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
        BOOL bReturn = m_pCEhcd->AddToBusyPipeList(this, FALSE);
#else
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"Add control to Busy\r\n"));
#endif
        m_pCEhcd->AddToBusyPipeList(this, FALSE);
#endif
        ASSERT(bReturn == TRUE);
    }
    DEBUGMSG( ZONE_PIPE, (TEXT("-CControlPipe::OpenPipe\n") ) );
    return status;
}
// ******************************************************************               
// Scope: public (Implements CPipe::ClosePipe = 0)
HCD_REQUEST_STATUS CControlPipe::ClosePipe( void )
//
// Purpose: Abort any transfers associated with this pipe, and
//          remove its data structures from the schedule
//
// Parameters: None
//
// Returns: requestOK
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("+CBulkPipe(%s)::ClosePipe\n"), GetPipeType() ) );
    HCD_REQUEST_STATUS status = requestFailed;
    m_pCEhcd->RemoveFromBusyPipeList(this );
    EnterCriticalSection( &m_csPipeLock );
    if ( m_pPipeQHead) {
#ifdef IRAM_PATCH
        m_pPipeQHead->FreeIRamSpace();
        RETAILMSG(IRAM_MEM_LOG, (L"Control Close Pipe, Data RAM\r\n"));
#if (IRAM_MEM_LOG == 1)
        m_pCEhcd->GetIRamDataMem()->ShowMemInfo();
#endif
#endif
        AbortQueue();
        m_pCEhcd->AsyncDequeueQH( m_pPipeQHead );
        //delete m_pPipeQHead;
        DWORD dwPhysMem = m_pPipeQHead->GetPhysAddr();
#ifdef EHCI_PROBE
        RETAILMSG(CRITICAL_LOG, (L"QH mem @ %x\r\n", dwPhysMem));
#endif
        m_pPipeQHead->~CQH();
#ifdef EHCI_PROBE
        RETAILMSG(CRITICAL_LOG, (L"Free Control memory here\r\n"));
#endif
#ifdef IRAM_PATCH
        m_pCEhcd->GetIRamEleMem()->FreeMemory((PBYTE)m_pPipeQHead, dwPhysMem ,CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);        
        RETAILMSG(IRAM_MEM_LOG, (L"Control Close Pipe, Ele Ram\r\n"));
#if (IRAM_MEM_LOG == 1)
        m_pCEhcd->GetIRamEleMem()->ShowMemInfo();
#endif
#else
        m_pCEhcd->GetPhysMem()->FreeMemory((PBYTE)m_pPipeQHead, dwPhysMem ,CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);        
#endif
        m_pPipeQHead = NULL;
        status = requestOK;
    }
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}

//*******************************************************************
BOOL CControlPipe::RemoveQHeadFromQueue() 
//
// Purpose: Control Pipe: Remove the queue head from the Asynch Queue list
//
// Parameters: None
//
// Returns: TRUE - success, FALSE  - failure
//
// ******************************************************************
{
    ASSERT(m_pPipeQHead);
    return( m_pCEhcd->AsyncDequeueQH( m_pPipeQHead )!=NULL);
}

//*******************************************************************
BOOL CControlPipe::InsertQHeadToQueue() 
//
// Purpose: Control Pipe: Insert the queue head from the Asynch Queue list
//
// Parameters: None
//
// Returns: TRUE - success, FALSE  - failure
//
// ******************************************************************
{
    ASSERT(m_pPipeQHead);
    return (m_pCEhcd->AsyncQueueQH( m_pPipeQHead )!=NULL);
}
// ******************************************************************               
// Scope: public 
HCD_REQUEST_STATUS  CControlPipe::IssueTransfer( 
                                    IN const UCHAR address,
                                    IN LPTRANSFER_NOTIFY_ROUTINE const lpStartAddress,
                                    IN LPVOID const lpvNotifyParameter,
                                    IN const DWORD dwFlags,
                                    IN LPCVOID const lpvControlHeader,
                                    IN const DWORD dwStartingFrame,
                                    IN const DWORD dwFrames,
                                    IN LPCDWORD const aLengths,
                                    IN const DWORD dwBufferSize,     
                                    IN_OUT LPVOID const lpvClientBuffer,
                                    IN const ULONG paBuffer,
                                    IN LPCVOID const lpvCancelId,
                                    OUT LPDWORD const adwIsochErrors,
                                    OUT LPDWORD const adwIsochLengths,
                                    OUT LPBOOL const lpfComplete,
                                    OUT LPDWORD const lpdwBytesTransferred,
                                    OUT LPDWORD const lpdwError )
//
// Purpose: Issue a Transfer on this pipe
//
// Parameters: address - USB address to send transfer to
//
//             OTHER PARAMS - see comment in CUhcd::IssueTransfer
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes:   
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("+CControlPipe::IssueTransfer, address = %d\n"), address) );
    if (m_bDeviceAddress ==0 && address !=0) { // Address Changed.
        if ( m_pQueuedTransfer == NULL &&  m_pPipeQHead && m_pPipeQHead->IsActive()==FALSE) { // We need cqueue new Transfer.
            m_bDeviceAddress = address;
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"%d SetDeviceAddress\r\n", __LINE__));
#endif
            m_pPipeQHead ->SetDeviceAddress(m_bDeviceAddress);
        }
        else {
            ASSERT(FALSE);
            return requestFailed;
        }
    }
    HCD_REQUEST_STATUS status = CQueuedPipe::IssueTransfer( address, lpStartAddress,lpvNotifyParameter,
            dwFlags,lpvControlHeader, dwStartingFrame, dwFrames, aLengths, dwBufferSize, lpvClientBuffer,
            paBuffer, lpvCancelId, adwIsochErrors, adwIsochLengths, lpfComplete, lpdwBytesTransferred, lpdwError );
    DEBUGMSG( ZONE_TRANSFER, (TEXT("-CControlPipe::::IssueTransfer - address = %d, returing HCD_REQUEST_STATUS %d\n"), address, status) );
    return status;
};
// ******************************************************************
// Scope: public
void CControlPipe::ChangeMaxPacketSize( IN const USHORT wMaxPacketSize )
//
// Purpose: Update the max packet size for this pipe. This should
//          ONLY be done for control endpoint 0 pipes. When the endpoint0
//          pipe is first opened, it has a max packet size of 
//          ENDPOINT_ZERO_MIN_MAXPACKET_SIZE. After reading the device's
//          descriptor, the device attach procedure can update the size.
//
// Parameters: wMaxPacketSize - new max packet size for this pipe
//
// Returns: Nothing
//
// Notes:   This function should only be called by the Hub AttachDevice
//          procedure
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CControlPipe::ChangeMaxPacketSize - new wMaxPacketSize = %d\n"), wMaxPacketSize) );

    EnterCriticalSection( &m_csPipeLock );

    // this pipe should be for endpoint 0, control pipe
    DEBUGCHK( (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_CONTROL &&
              (m_usbEndpointDescriptor.bEndpointAddress & TD_ENDPOINT_MASK) == 0 );
    // update should only be called if the old address was ENDPOINT_ZERO_MIN_MAXPACKET_SIZE
    DEBUGCHK( m_usbEndpointDescriptor.wMaxPacketSize == ENDPOINT_ZERO_MIN_MAXPACKET_SIZE );
    // this function should only be called if we are increasing the max packet size.
    // in addition, the USB spec 1.0 section 9.6.1 states only the following
    // wMaxPacketSize are allowed for endpoint 0
    DEBUGCHK( wMaxPacketSize > ENDPOINT_ZERO_MIN_MAXPACKET_SIZE &&
              (wMaxPacketSize == 16 ||
               wMaxPacketSize == 32 ||
               wMaxPacketSize == 64) );
    
    m_usbEndpointDescriptor.wMaxPacketSize = wMaxPacketSize;
    if ( m_pQueuedTransfer == NULL &&  m_pPipeQHead && m_pPipeQHead->IsActive()==FALSE) { // We need cqueue new Transfer.
        m_pPipeQHead ->SetMaxPacketLength(wMaxPacketSize);
    }
    else {
        ASSERT(FALSE);
    } 

    LeaveCriticalSection( &m_csPipeLock );

    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CControlPipe::ChangeMaxPacketSize - new wMaxPacketSize = %d\n"), wMaxPacketSize) );
}
// ******************************************************************               
// Scope: private (Implements CPipe::AreTransferParametersValid = 0)
BOOL CControlPipe::AreTransferParametersValid( const STransfer *pTransfer ) const 
//
// Purpose: Check whether this class' transfer parameters are valid.
//          This includes checking m_transfer, m_pPipeQH, etc
//
// Parameters: None (all parameters are vars of class)
//
// Returns: TRUE if parameters valid, else FALSE
//
// Notes: Assumes m_csPipeLock already held
// ******************************************************************
{
    if (pTransfer == NULL)
        return FALSE;    
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("+CControlPipe::AreTransferParametersValid\n")) );

    // these parameters aren't used by CControlPipe, so if they are non NULL,
    // it doesn't present a serious problem. But, they shouldn't have been
    // passed in as non-NULL by the calling driver.
    DEBUGCHK( pTransfer->adwIsochErrors == NULL && // ISOCH
              pTransfer->adwIsochLengths == NULL && // ISOCH
              pTransfer->aLengths == NULL ); // ISOCH
    // this is also not a serious problem, but shouldn't happen in normal
    // circumstances. It would indicate a logic error in the calling driver.
    DEBUGCHK( !(pTransfer->lpStartAddress == NULL && pTransfer->lpvNotifyParameter != NULL) );
    // DWORD                     pTransfer->dwStartingFrame; (ignored - ISOCH)
    // DWORD                     pTransfer->dwFrames; (ignored - ISOCH)

    BOOL fValid = ( m_pPipeQHead != NULL &&
                    m_bDeviceAddress <= USB_MAX_ADDRESS &&
                    pTransfer->lpvControlHeader != NULL &&
                    pTransfer->lpfComplete != NULL &&
                    pTransfer->lpdwBytesTransferred != NULL &&
                    pTransfer->lpdwError != NULL );
    if ( fValid ) {
        if ( pTransfer->dwFlags & USB_IN_TRANSFER ) {
            fValid = (pTransfer->lpvBuffer != NULL &&
                      // paClientBuffer could be 0 or !0
                      pTransfer->dwBufferSize > 0);
        } else {
            fValid = ( (pTransfer->lpvBuffer == NULL && 
                        pTransfer->paBuffer == 0 &&
                        pTransfer->dwBufferSize == 0) ||
                       (pTransfer->lpvBuffer != NULL &&
                        // paClientBuffer could be 0 or !0
                        pTransfer->dwBufferSize > 0) );
        }
    }

    ASSERT(fValid);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("-CControlPipe::AreTransferParametersValid, returning BOOL %d\n"), fValid) );
    return fValid;
}
// ******************************************************************               
// Scope: public
CInterruptPipe::CInterruptPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
                 IN CEhcd *const pCEhcd)
//
// Purpose: Constructor for CInterruptPipe
//
// Parameters: See CQueuedPipe::CQueuedPipe
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
: CQueuedPipe( lpEndpointDescriptor, fIsLowSpeed, fIsHighSpeed, bDeviceAddress, bHubAddress,bHubPort, pCEhcd ) // constructor for base class
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CInterruptPipe::CInterruptPipe\n")) );
    DEBUGCHK( m_usbEndpointDescriptor.bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
              m_usbEndpointDescriptor.bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) &&
              (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_INTERRUPT );
    //TODO:I need Get S-MASK and C-MASK.

    memset(&m_EndptBuget,0,sizeof(m_EndptBuget));
    m_EndptBuget.max_packet= lpEndpointDescriptor->wMaxPacketSize & 0x7ff;
    BYTE bInterval=lpEndpointDescriptor->bInterval;

    if (bInterval==0)
        bInterval=1;
        
    if (fIsHighSpeed) { // Table 9-13
        // Max can only be 1-16     
        if (bInterval > 16)
        {
            m_bSuccess = FALSE;
            goto CInterruptPipe_Exit;
        }
        m_EndptBuget.max_packet *=(((lpEndpointDescriptor->wMaxPacketSize >>11) & 3)+1);
        m_EndptBuget.period = (1<< (bInterval-1));
    }
    else {
        m_EndptBuget.period = bInterval;
        for (UCHAR uBit=0x80;uBit!=0;uBit>>=1) {
            if ((m_EndptBuget.period & uBit)!=0) {
                m_EndptBuget.period = uBit;
                break;
            }
        }
    }
    ASSERT(m_EndptBuget.period!=0);
    m_EndptBuget.ep_type = interrupt ;
    m_EndptBuget.type= lpEndpointDescriptor->bDescriptorType;
    m_EndptBuget.direction =  (USB_ENDPOINT_DIRECTION_OUT(lpEndpointDescriptor->bEndpointAddress)?OUTDIR:INDIR);
    m_EndptBuget.speed=(fIsHighSpeed?HSSPEED:(fIsLowSpeed?LSSPEED:FSSPEED));

    m_bSuccess= pCEhcd->AllocUsb2BusTime(bHubAddress,bHubPort,&m_EndptBuget);
    ASSERT(m_bSuccess);
    if (m_bSuccess ) {
        if (fIsHighSpeed) { // Update SMask and CMask for Slit Interrupt Endpoitt
            m_bFrameSMask = 0xff;
            m_bFrameCMask = 0;
        }
        else {
            m_bFrameSMask=pCEhcd->GetSMASK(&m_EndptBuget);
            m_bFrameCMask=pCEhcd->GetCMASK(&m_EndptBuget);
        }
    }
    else {
        ASSERT(FALSE);
    }

CInterruptPipe_Exit:
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CInterruptPipe::CInterruptPipe\n")) );
}

// ******************************************************************               
// Scope: public
CInterruptPipe::~CInterruptPipe( )
//
// Purpose: Destructor for CInterruptPipe
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CInterruptPipe::~CInterruptPipe\n")) );
    if (m_bSuccess)
        m_pCEhcd->FreeUsb2BusTime( m_bHubAddress, m_bHubPort,&m_EndptBuget);
    ClosePipe();
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CInterruptPipe::~CInterruptPipe\n")) );
}
// ******************************************************************               
// Scope: public (Implements CPipe::OpenPipe = 0)
HCD_REQUEST_STATUS CInterruptPipe::OpenPipe( void )
//
// Purpose: Create the data structures necessary to conduct
//          transfers on this pipe
//
// Parameters: None
//
// Returns: requestOK - if pipe opened
//
//          requestFailed - if pipe was not opened
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("+CInterruptPipe::OpenPipe\n") ) );

    HCD_REQUEST_STATUS status = requestFailed;
    m_pUnQueuedTransfer=NULL;      // ptr to last transfer in queue
    m_pQueuedTransfer=NULL;
    if (!m_bSuccess)
        return status;
    EnterCriticalSection( &m_csPipeLock );

    // if this fails, someone is trying to open
    // an already opened pipe
        // Freescale iMX31 specific: check the portsc before.
 
    DEBUGCHK(m_pPipeQHead == NULL );
    if ( m_pPipeQHead == NULL )
#ifdef IRAM_PATCH
    {
#ifdef IRAM_PATCH_EXTEND
        m_pPipeQHead = new(m_pCEhcd->GetIRamEleMem()) CQH (this, m_pCEhcd->GetPhysMem(), m_pCEhcd->GetIRamEleMem(), m_pCEhcd->GetIRamDataMem(), TRUE);
        //RETAILMSG(1, (L"new Interrupt QH\r\n"));
        //m_pCEhcd->GetIRamEleMem()->ShowMemInfo();
#else
        m_pPipeQHead = new(m_pCEhcd->GetIRamEleMem()) CQH (this, m_pCEhcd->GetIRamEleMem(), m_pCEhcd->GetIRamDataMem(), TRUE);
#endif
    }
#else
        m_pPipeQHead =  new(m_pCEhcd->GetPhysMem()) CQH(this);
#endif
    if (m_pPipeQHead) {
        m_pPipeQHead->SetDTC(FALSE); // Auto Data Toggle for Interrupt.
        m_pPipeQHead->SetReLoad(0xf);
        if (!m_fIsHighSpeed)
            m_pPipeQHead->SetINT(FALSE);
        m_pCEhcd->PeriodQeueuQHead(m_pPipeQHead,(UCHAR)(m_EndptBuget.actual_period>=0x100?0xff:m_EndptBuget.actual_period),
            m_fIsHighSpeed?(m_EndptBuget.start_frame*MICROFRAMES_PER_FRAME+ m_EndptBuget.start_microframe):m_EndptBuget.start_frame,
            m_fIsHighSpeed);
        // Interrupt QHs are a bit complicated. See comment
        // in Initialize() routine as well.
        //
        
        status = requestOK;
    }
    LeaveCriticalSection( &m_csPipeLock );

    if (status == requestOK) {
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
        BOOL bReturn = m_pCEhcd->AddToBusyPipeList(this, FALSE);
#else
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"Add Int to Busy\r\n"));
#endif
        m_pCEhcd->AddToBusyPipeList(this, FALSE);
#endif
        ASSERT(bReturn == TRUE);
    }
    DEBUGMSG( ZONE_PIPE, (TEXT("-CInterruptPipe::OpenPipe, returning HCD_REQUEST_STATUS %d\n"), status) );
    return status;
}
// ******************************************************************               
// Scope: public (Implements CPipe::ClosePipe = 0)
HCD_REQUEST_STATUS CInterruptPipe::ClosePipe( void )
//
// Purpose: Abort any transfers associated with this pipe, and
//          remove its data structures from the schedule
//
// Parameters: None
//
// Returns: requestOK
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("+CBulkPipe(%s)::ClosePipe\n"), GetPipeType() ) );
    HCD_REQUEST_STATUS status = requestFailed;
    m_pCEhcd->RemoveFromBusyPipeList(this );
    EnterCriticalSection( &m_csPipeLock );
    if ( m_pPipeQHead) {
#ifdef IRAM_PATCH
        // Without this section of code, Mouse Plug / Unplug will cause memory overflow in IRAM
        m_pPipeQHead->FreeIRamSpace();
        RETAILMSG(IRAM_MEM_LOG, (L"Interrupt Close Pipe, Data RAM\r\n"));
#if (IRAM_MEM_LOG == 1)
        m_pCEhcd->GetIRamDataMem()->ShowMemInfo();
#endif
#endif
        AbortQueue( );
        m_pCEhcd->PeriodDeQueueuQHead( m_pPipeQHead );
        //delete m_pPipeQHead;
        DWORD dwPhysAddr = m_pPipeQHead->GetPhysAddr();
        m_pPipeQHead->~CQH();
#ifdef IRAM_PATCH
        m_pCEhcd->GetIRamEleMem()->FreeMemory((PBYTE)m_pPipeQHead, dwPhysAddr , CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
#else
        m_pCEhcd->GetPhysMem()->FreeMemory((PBYTE)m_pPipeQHead, dwPhysAddr , CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK);
#endif
        m_pPipeQHead = NULL;
        status = requestOK;
    }
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}
// ******************************************************************               
// Scope: private (Implements CPipe::AreTransferParametersValid = 0)
BOOL CInterruptPipe::AreTransferParametersValid( const STransfer *pTransfer ) const
//
// Purpose: Check whether this class' transfer parameters are valid.
//          This includes checking m_transfer, m_pPipeQH, etc
//
// Parameters: None (all parameters are vars of class)
//
// Returns: TRUE if parameters valid, else FALSE
//
// Notes: Assumes m_csPipeLock already held
// ******************************************************************
{
    if (pTransfer == NULL)
        return FALSE;
    
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("+CInterruptPipe::AreTransferParametersValid\n")) );

    // these parameters aren't used by CInterruptPipe, so if they are non NULL,
    // it doesn't present a serious problem. But, they shouldn't have been
    // passed in as non-NULL by the calling driver.
    DEBUGCHK( pTransfer->adwIsochErrors == NULL && // ISOCH
              pTransfer->adwIsochLengths == NULL && // ISOCH
              pTransfer->aLengths == NULL && // ISOCH
              pTransfer->lpvControlHeader == NULL ); // CONTROL
    // this is also not a serious problem, but shouldn't happen in normal
    // circumstances. It would indicate a logic error in the calling driver.
    DEBUGCHK( !(pTransfer->lpStartAddress  == NULL && pTransfer->lpvNotifyParameter  != NULL) );
    // DWORD                     pTransfer->dwStartingFrame (ignored - ISOCH)
    // DWORD                     pTransfer->dwFrames (ignored - ISOCH)

    BOOL fValid = (  m_pPipeQHead!= NULL &&
                    m_bDeviceAddress > 0 && m_bDeviceAddress <= USB_MAX_ADDRESS &&
                    (pTransfer->lpvBuffer != NULL || pTransfer->dwBufferSize == 0) &&
                    // paClientBuffer could be 0 or !0
                    pTransfer->lpfComplete != NULL &&
                    pTransfer->lpdwBytesTransferred != NULL &&
                    pTransfer->lpdwError != NULL );

    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("-CInterruptPipe::AreTransferParametersValid, returning BOOL %d\n"), fValid) );
    return fValid;
}

#define NUM_OF_PRE_ALLOCATED_TD 0x100
// ******************************************************************               
// Scope: public
CIsochronousPipe::CIsochronousPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
                 IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
                 IN const UCHAR bDeviceAddress,
                 IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
                 IN CEhcd *const pCEhcd )
//
// Purpose: Constructor for CIsochronousPipe
//
// Parameters: See CPipe::CPipe
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
: CPipe(lpEndpointDescriptor, fIsLowSpeed,fIsHighSpeed, bDeviceAddress,bHubAddress,bHubPort, pCEhcd )   // constructor for base class
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CIsochronousPipe::CIsochronousPipe\n")) );
    DEBUGCHK( m_usbEndpointDescriptor.bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
              m_usbEndpointDescriptor.bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) &&
              (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_ISOCHRONOUS );
    
    m_pQueuedTransfer = NULL;
    memset(&m_EndptBuget,0,sizeof(m_EndptBuget));
    m_EndptBuget.max_packet= lpEndpointDescriptor->wMaxPacketSize & 0x7ff;
    BYTE bInterval = lpEndpointDescriptor->bInterval;
    if ( bInterval==0)
         bInterval=1;
    
    m_pArrayOfCITD = NULL;
    m_pArrayOfCSITD = NULL;
    m_dwNumOfTDAvailable=0;
    
    if (fIsHighSpeed) { // Table 9-13
        m_pArrayOfCITD = (CITD **) new PVOID[NUM_OF_PRE_ALLOCATED_TD];
        if (m_pArrayOfCITD) {
            memset(m_pArrayOfCITD, 0, sizeof (CITD *) * NUM_OF_PRE_ALLOCATED_TD) ;
            for (m_dwNumOfTDAvailable=0;m_dwNumOfTDAvailable<NUM_OF_PRE_ALLOCATED_TD;m_dwNumOfTDAvailable++) {
                if ( (*(m_pArrayOfCITD + m_dwNumOfTDAvailable) = new (m_pCEhcd->GetPhysMem()) CITD(NULL)) == NULL)
                    break;                
            }
        }
        ASSERT(m_pArrayOfCITD);
        ASSERT(m_dwNumOfTDAvailable == NUM_OF_PRE_ALLOCATED_TD);
        
        m_EndptBuget.max_packet *=(((lpEndpointDescriptor->wMaxPacketSize >>11) & 3)+1);
        m_EndptBuget.period = (1<< ( bInterval-1));
        
        if (m_EndptBuget.period<= MAX_TRNAS_PER_ITD ) {
            m_dwMaxTransPerItd = MAX_TRNAS_PER_ITD / m_EndptBuget.period;
            m_dwTDInterval = 1;
        }
        else {
            m_dwMaxTransPerItd = 1;
            m_dwTDInterval = m_EndptBuget.period / MAX_TRNAS_PER_ITD ;
        }
            
        DEBUGMSG(ZONE_INIT, (TEXT("CIsochronousPipe::CIsochronousPipe: m_dwMaxTransPerItd = %d \r\n"), m_dwMaxTransPerItd));

    }
    else {
        m_pArrayOfCSITD = (CSITD **) new PVOID[NUM_OF_PRE_ALLOCATED_TD];
        if (m_pArrayOfCSITD) {
            memset(m_pArrayOfCSITD, 0, sizeof (CSITD *) * NUM_OF_PRE_ALLOCATED_TD) ;
            for (m_dwNumOfTDAvailable=0;m_dwNumOfTDAvailable<NUM_OF_PRE_ALLOCATED_TD;m_dwNumOfTDAvailable++) {
                if ( (*(m_pArrayOfCSITD + m_dwNumOfTDAvailable) = new (m_pCEhcd->GetPhysMem()) CSITD (NULL,NULL)) == NULL)
                    break;                
            }
        }
        ASSERT(m_pArrayOfCSITD);
        ASSERT(m_dwNumOfTDAvailable == NUM_OF_PRE_ALLOCATED_TD);
        
        m_EndptBuget.period = (1<< ( bInterval-1));
        m_dwMaxTransPerItd = 1;
        m_dwTDInterval = 1;
    }
    m_EndptBuget.ep_type = isoch ;
    m_EndptBuget.type= lpEndpointDescriptor->bDescriptorType;
    m_EndptBuget.direction =(USB_ENDPOINT_DIRECTION_OUT(lpEndpointDescriptor->bEndpointAddress)?OUTDIR:INDIR);
    m_EndptBuget.speed=(fIsHighSpeed?HSSPEED:(fIsLowSpeed?LSSPEED:FSSPEED));

    m_bSuccess=pCEhcd->AllocUsb2BusTime(bHubAddress,bHubPort,&m_EndptBuget);
    ASSERT(m_bSuccess);
    if (m_bSuccess ) {
        
        if (fIsHighSpeed) { // Update SMask and CMask for Slit Interrupt Endpoitt
            m_bFrameSMask=pCEhcd->GetSMASK(&m_EndptBuget);
            m_bFrameCMask=0;
        }
        else {
            m_bFrameSMask=pCEhcd->GetSMASK(&m_EndptBuget);
            m_bFrameCMask=pCEhcd->GetCMASK(&m_EndptBuget);
        }
    }
    else {
        ASSERT(FALSE);
    }
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CIsochronousPipe::CIsochronousPipe\n")) );
}

// ******************************************************************               
// Scope: public
CIsochronousPipe::~CIsochronousPipe( )
//
// Purpose: Destructor for CIsochronousPipe
//
// Parameters: None
//
// Returns: Nothing
//
// Notes: Do not modify static variables here!!
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("+CIsochronousPipe::~CIsochronousPipe\n")) );
    // m_pWakeupTD should have been freed by the time we get here
    if (m_bSuccess)
        m_pCEhcd->FreeUsb2BusTime( m_bHubAddress, m_bHubPort,&m_EndptBuget);
    ClosePipe();
    if (m_pArrayOfCITD) {
        for (m_dwNumOfTDAvailable=0;m_dwNumOfTDAvailable<NUM_OF_PRE_ALLOCATED_TD;m_dwNumOfTDAvailable++) {
            if ( *(m_pArrayOfCITD + m_dwNumOfTDAvailable) !=NULL) {
                 (*(m_pArrayOfCITD + m_dwNumOfTDAvailable))->~CITD();
                 m_pCEhcd->GetPhysMem()->FreeMemory((PBYTE)*(m_pArrayOfCITD + m_dwNumOfTDAvailable),
                    m_pCEhcd->GetPhysMem()->VaToPa((PBYTE)*(m_pArrayOfCITD + m_dwNumOfTDAvailable)),
                    CPHYSMEM_FLAG_HIGHPRIORITY |CPHYSMEM_FLAG_NOBLOCK );                                
            }
        }
        delete (m_pArrayOfCITD);
    }
    if (m_pArrayOfCSITD) {
        for (m_dwNumOfTDAvailable=0;m_dwNumOfTDAvailable<NUM_OF_PRE_ALLOCATED_TD;m_dwNumOfTDAvailable++) {
            if ( *(m_pArrayOfCSITD + m_dwNumOfTDAvailable) != NULL) {
                 (*(m_pArrayOfCSITD + m_dwNumOfTDAvailable))->~CSITD();
                 m_pCEhcd->GetPhysMem()->FreeMemory((PBYTE)*(m_pArrayOfCSITD + m_dwNumOfTDAvailable),
                    m_pCEhcd->GetPhysMem()->VaToPa((PBYTE)*(m_pArrayOfCSITD + m_dwNumOfTDAvailable)),
                    CPHYSMEM_FLAG_HIGHPRIORITY |CPHYSMEM_FLAG_NOBLOCK );                                
            }
        }
        delete (m_pArrayOfCSITD);
    }
    
    DEBUGMSG( ZONE_PIPE && ZONE_VERBOSE, (TEXT("-CIsochronousPipe::~CIsochronousPipe\n")) );
}

//******************************************************************************
CITD *  CIsochronousPipe::AllocateCITD( CITransfer *  pTransfer)
//
// Purpose: Isochronous Pipe: Allocate the Isochronous High Speed Transfer Descriptor
//
// Parameters: pTransfer - Pointer to CITransfer object containing all transfer information
//
// Returns: Pointer to CITD object, NULL - fail
//
// ******************************************************************
{
    EnterCriticalSection( &m_csPipeLock );
    ASSERT(m_pArrayOfCITD!=NULL) ;
    CITD * pReturn = NULL;
    if (m_pArrayOfCITD!=NULL ) {
        ASSERT(m_dwNumOfTDAvailable <= NUM_OF_PRE_ALLOCATED_TD);
        for (DWORD dwIndex=m_dwNumOfTDAvailable;dwIndex!=0;dwIndex--) {
            if ((pReturn = *(m_pArrayOfCITD + dwIndex -1))!=NULL) {
                m_dwNumOfTDAvailable = dwIndex-1;
                *(m_pArrayOfCITD + m_dwNumOfTDAvailable) = NULL;
                pReturn->ReInit(pTransfer);
                break;
            }
        }
    }
    LeaveCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_WARNING && (pReturn==NULL) , (TEXT("CIsochronousPipe::AllocateCITD: return NULL, run out of pre-allocated ITD\r\n")) );
    return pReturn;
}

//********************************************************************************
CSITD * CIsochronousPipe::AllocateCSITD( CSITransfer * pTransfer,CSITD * pPrev)
//
// Purpose: Isochronous Pipe: Allocate the Spilt Isochronous (Full/Low Speed) Transfer Descriptor
//
// Parameters: pTransfer - Pointer to CSITransfer object containing all transfer information
//             pPrev - Pointer to previous CSITD
//
// Returns: Pointer to CITD object, NULL - fail
//
// ******************************************************************

{
    EnterCriticalSection( &m_csPipeLock );
    ASSERT(m_pArrayOfCSITD!=NULL) ;
    CSITD * pReturn = NULL;
    if (m_pArrayOfCSITD!=NULL ) {
        ASSERT(m_dwNumOfTDAvailable <= NUM_OF_PRE_ALLOCATED_TD);
        for (DWORD dwIndex=m_dwNumOfTDAvailable;dwIndex!=0;dwIndex--) {
            if ((pReturn = *(m_pArrayOfCSITD + dwIndex -1))!=NULL) {
                m_dwNumOfTDAvailable = dwIndex -1;
                *(m_pArrayOfCSITD + m_dwNumOfTDAvailable) = NULL;
                pReturn->ReInit(pTransfer,pPrev);
                break;
            }
        }
    }
    LeaveCriticalSection( &m_csPipeLock );
    DEBUGMSG( ZONE_WARNING && (pReturn==NULL) , (TEXT("CIsochronousPipe::AllocateCSITD: return NULL, run out of pre-allocated CITD\r\n")) );
    return pReturn;

}
//********************************************************************************
void    CIsochronousPipe::FreeCITD(CITD *  pITD)
//
// Purpose: Isochronous Pipe: Release and free the CITD being allocated
//
// Parameters: pITD - pointer to isochronous transfer descriptor
//
// Returns: Nothing
//
// ******************************************************************
{
    EnterCriticalSection( &m_csPipeLock );
    ASSERT(m_pArrayOfCITD!=NULL);
    ASSERT(m_dwNumOfTDAvailable< NUM_OF_PRE_ALLOCATED_TD);
    if (m_pArrayOfCITD && pITD && m_dwNumOfTDAvailable< NUM_OF_PRE_ALLOCATED_TD) {
        ASSERT(*(m_pArrayOfCITD+m_dwNumOfTDAvailable)==NULL);
        pITD->~CITD();
        *(m_pArrayOfCITD+m_dwNumOfTDAvailable)= pITD;
        m_dwNumOfTDAvailable ++;
    }
    LeaveCriticalSection( &m_csPipeLock );
}

//******************************************************************
void    CIsochronousPipe::FreeCSITD(CSITD * pSITD)
//
// Purpose: Isochronous Pipe: Release and free the CSITD being allocated
//
// Parameters: pSITD - pointer to spilt isochronous transfer descriptor
//
// Returns: Nothing
//
// ******************************************************************
{
    EnterCriticalSection( &m_csPipeLock );
    ASSERT(m_pArrayOfCSITD );
    ASSERT(m_dwNumOfTDAvailable< NUM_OF_PRE_ALLOCATED_TD);
    if (m_pArrayOfCSITD && pSITD && m_dwNumOfTDAvailable< NUM_OF_PRE_ALLOCATED_TD ) {
        ASSERT(*(m_pArrayOfCSITD+m_dwNumOfTDAvailable)==NULL);
        pSITD->~CSITD();
        *(m_pArrayOfCSITD+m_dwNumOfTDAvailable)= pSITD;
        m_dwNumOfTDAvailable ++;
    }
    LeaveCriticalSection( &m_csPipeLock );
}

// ******************************************************************               
// Scope: public (Implements CPipe::OpenPipe = 0)
HCD_REQUEST_STATUS CIsochronousPipe::OpenPipe( void )
//
// Purpose: Inserting the necessary (empty) items into the
//          schedule to permit future transfers
//
// Parameters: None
//
// Returns: requestOK if pipe opened successfuly
//          requestFailed if pipe not opened
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("+CIsochronousPipe::OpenPipe\n")) );

    HCD_REQUEST_STATUS status = requestFailed;
    m_pQueuedTransfer=NULL;

    EnterCriticalSection( &m_csPipeLock );

    DEBUGCHK( m_usbEndpointDescriptor.bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE &&
              m_usbEndpointDescriptor.bLength >= sizeof( USB_ENDPOINT_DESCRIPTOR ) &&
              (m_usbEndpointDescriptor.bmAttributes & USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_ISOCHRONOUS );

    m_dwLastValidFrame = 0;
    m_pCEhcd->GetFrameNumber(&m_dwLastValidFrame);
    
    // if this fails, someone is trying to open an already opened pipe
    if ( m_pQueuedTransfer == NULL && m_bSuccess == TRUE) {
        status = requestOK;
    }
    else
        ASSERT(FALSE);
    LeaveCriticalSection( &m_csPipeLock );
    if (status == requestOK ) {
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
        BOOL bReturn = m_pCEhcd->AddToBusyPipeList(this, TRUE);
#else
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"add ISO to Busy\r\n"));
#endif
        m_pCEhcd->AddToBusyPipeList(this, TRUE);
#endif
        ASSERT(bReturn == TRUE);
    }
    DEBUGMSG( ZONE_PIPE, (TEXT("-CIsochronousPipe::OpenPipe, returning HCD_REQUEST_STATUS %d\n"), status ) );
    return status;
}
// ******************************************************************               
// Scope: public (Implements CPipe::ClosePipe = 0)
HCD_REQUEST_STATUS CIsochronousPipe::ClosePipe( void )
//
// Purpose: Abort any transfers associated with this pipe, and
//          remove its data structures from the schedule
//
// Parameters: None
//
// Returns: requestOK
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_PIPE, (TEXT("+CIsochronousPipe::ClosePipe\n")) );

    m_pCEhcd->RemoveFromBusyPipeList( this );
    EnterCriticalSection( &m_csPipeLock );
    CIsochTransfer *  pCurTransfer = m_pQueuedTransfer;
    m_pQueuedTransfer = NULL;
    while ( pCurTransfer ) {
         pCurTransfer->AbortTransfer();
         CIsochTransfer *  pNext = (CIsochTransfer *)pCurTransfer ->GetNextTransfer();
         delete pCurTransfer;
         pCurTransfer = pNext;
    }
    LeaveCriticalSection( &m_csPipeLock );

    DEBUGMSG( ZONE_PIPE, (TEXT("-CIsochronousPipe::ClosePipe\n")) );
    return requestOK;
}
// ******************************************************************               
// Scope: public (Implements CPipe::AbortTransfer = 0)
HCD_REQUEST_STATUS CIsochronousPipe::AbortTransfer( 
                                    IN const LPTRANSFER_NOTIFY_ROUTINE lpCancelAddress,
                                    IN const LPVOID lpvNotifyParameter,
                                    IN LPCVOID lpvCancelId )
//
// Purpose: Abort any transfer on this pipe if its cancel ID matches
//          that which is passed in.
//
// Parameters: lpCancelAddress - routine to callback after aborting transfer
//
//             lpvNotifyParameter - parameter for lpCancelAddress callback
//
//             lpvCancelId - identifier for transfer to abort
//
// Returns: requestOK if transfer aborted
//          requestFailed if lpvCancelId doesn't match currently executing
//                 transfer, or if there is no transfer in progress
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("+CIsochronousPipe::AbortTransfer\n")));

    HCD_REQUEST_STATUS status = requestFailed;

    EnterCriticalSection( &m_csPipeLock );
    // Find this transfer.
    if (m_pQueuedTransfer!=NULL) {
        CIsochTransfer *  pCur=m_pQueuedTransfer;
        CIsochTransfer *  pPrev=NULL;
        while (pCur!=NULL && (pCur ->GetSTransfer()).lpvCancelId != lpvCancelId) {
            pPrev = pCur;
            pCur =(CIsochTransfer *  )pCur ->GetNextTransfer();
        };
        if (pCur!=NULL) { // We found it
            if (pPrev!=NULL) 
                pPrev->SetNextTransfer(pCur->GetNextTransfer());
            else // It is Locate at header
                m_pQueuedTransfer = (CIsochTransfer * )m_pQueuedTransfer->GetNextTransfer();
            pCur->AbortTransfer();
            // Do not need call DoneTransfer here because AboutTransfer Called DoneTransfer already.
            delete pCur;  
            if ( lpCancelAddress ) {
                __try { // calling the Cancel function
                    ( *lpCancelAddress )( lpvNotifyParameter );
                } __except( EXCEPTION_EXECUTE_HANDLER ) {
                      DEBUGMSG( ZONE_ERROR, (TEXT("CIsochronousPipe::AbortTransfer - exception executing cancellation callback function\n")) );
                }
            }
            status=requestOK;
        }
    }
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}
// ******************************************************************               
// Scope: public 
HCD_REQUEST_STATUS  CIsochronousPipe::IssueTransfer( 
                                    IN const UCHAR address,
                                    IN LPTRANSFER_NOTIFY_ROUTINE const lpStartAddress,
                                    IN LPVOID const lpvNotifyParameter,
                                    IN const DWORD dwFlags,
                                    IN LPCVOID const lpvControlHeader,
                                    IN const DWORD dwStartingFrame,
                                    IN const DWORD dwFrames,
                                    IN LPCDWORD const aLengths,
                                    IN const DWORD dwBufferSize,     
                                    IN_OUT LPVOID const lpvClientBuffer,
                                    IN const ULONG paBuffer,
                                    IN LPCVOID const lpvCancelId,
                                    OUT LPDWORD const adwIsochErrors,
                                    OUT LPDWORD const adwIsochLengths,
                                    OUT LPBOOL const lpfComplete,
                                    OUT LPDWORD const lpdwBytesTransferred,
                                    OUT LPDWORD const lpdwError )
//
// Purpose: Issue a Transfer on this pipe
//
// Parameters: address - USB address to send transfer to
//
//             OTHER PARAMS - see comment in CUhcd::IssueTransfer
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes:   
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("+CPipe(%s)::IssueTransfer, address = %d\n"), GetPipeType(), address) );
    
    DWORD dwEarliestFrame=0;
    m_pCEhcd->GetFrameNumber(&dwEarliestFrame);
    dwEarliestFrame = max(m_dwLastValidFrame,dwEarliestFrame+MIN_ADVANCED_FRAME);
    DWORD dwTransferStartFrame = dwStartingFrame;
    if ( (dwFlags & USB_START_ISOCH_ASAP)!=0) { // If ASAP, Overwrite the dwStartingFrame.
        dwTransferStartFrame = dwEarliestFrame;
    }
    STransfer sTransfer = {
    // These are the IssueTransfer parameters
        lpStartAddress,lpvNotifyParameter, dwFlags,lpvControlHeader,dwTransferStartFrame,dwFrames,
        aLengths,dwBufferSize,lpvClientBuffer,paBuffer,lpvCancelId,adwIsochErrors, adwIsochLengths,
        lpfComplete,lpdwBytesTransferred,lpdwError};
    
    HCD_REQUEST_STATUS  status = requestFailed;
    if ( dwTransferStartFrame < dwEarliestFrame ) {
        SetLastError(ERROR_CAN_NOT_COMPLETE);
        DEBUGMSG( ZONE_TRANSFER|ZONE_WARNING,
                  (TEXT("!CIsochronousPipe::IssueTransfer - cannot meet the schedule")
                   TEXT(" (reqFrame=%08x, curFrame=%08x\n"),
                   dwTransferStartFrame,dwEarliestFrame) );
    }
    else {
        sTransfer.dwStartingFrame = dwTransferStartFrame;
        if (AreTransferParametersValid(&sTransfer) && m_bDeviceAddress == address) {
            EnterCriticalSection( &m_csPipeLock );
#pragma prefast(disable: 322, "Recover gracefully from hardware failure")
            __try { // initializing transfer status parameters
                *(sTransfer.lpfComplete) = FALSE;
                *(sTransfer.lpdwBytesTransferred) = 0;
                *(sTransfer.lpdwError) = USB_NOT_COMPLETE_ERROR;
                
                CIsochTransfer * pTransfer;
                if (m_fIsHighSpeed )
                    pTransfer = new CITransfer(this,m_pCEhcd,&sTransfer);
                else
                    pTransfer = new CSITransfer(this,m_pCEhcd,&sTransfer);

                if (pTransfer && pTransfer->Init()) {
                    CTransfer * pCur = m_pQueuedTransfer;
                    if (pCur) {
                        while (pCur->GetNextTransfer()!=NULL)
                             pCur = (CIsochTransfer * )pCur->GetNextTransfer();
                        
                        pCur->SetNextTransfer( pTransfer);
                    }
                    else
                         m_pQueuedTransfer=pTransfer;
                    // Update Start Frame again.
                    dwEarliestFrame=0; 
                    m_pCEhcd->GetFrameNumber(&dwEarliestFrame);
                    dwTransferStartFrame = max(dwEarliestFrame+MIN_ADVANCED_FRAME,pTransfer->GetStartFrame());
                    pTransfer->SetStartFrame(dwTransferStartFrame);
                    
                    DWORD dwNumOfFrame = (m_fIsHighSpeed?((dwFrames + m_dwMaxTransPerItd - 1)/m_dwMaxTransPerItd): dwFrames);
                    m_dwLastValidFrame = dwTransferStartFrame + dwNumOfFrame;
                    ScheduleTransfer( );
                    status=requestOK ;
                }
                else
                    if (pTransfer) { // We return fails, so do not need callback.
                        pTransfer->DoNotCallBack() ;
                        delete pTransfer;    
                    }
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
            }
#pragma prefast(pop)
            LeaveCriticalSection( &m_csPipeLock );
        }
    }
    DEBUGMSG( ZONE_TRANSFER, (TEXT("-CPipe(%s)::IssueTransfer - address = %d, returing HCD_REQUEST_STATUS %d\n"), GetPipeType(), address, status) );
    return status;
}

// ******************************************************************               
// Scope: private (Implements CPipe::ScheduleTransfer = 0)
HCD_REQUEST_STATUS CIsochronousPipe::ScheduleTransfer( void ) 
//
// Purpose: Schedule a USB Transfer on this pipe
//
// Parameters: None (all parameters are in m_transfer)
//
// Returns: requestOK if transfer issued ok, else requestFailed
//
// Notes: 
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("+CIsochronousPipe::ScheduleTransfer\n")) );

    HCD_REQUEST_STATUS status = requestOK;
    EnterCriticalSection( &m_csPipeLock );
    DWORD dwFrame=0;
    m_pCEhcd->GetFrameNumber(&dwFrame);
    
    CIsochTransfer *  pCur= m_pQueuedTransfer;
    while (pCur!=NULL ) { 
        pCur->ScheduleTD(dwFrame,0);
        pCur = (CIsochTransfer *)pCur->GetNextTransfer();            
    }
    LeaveCriticalSection( &m_csPipeLock );
    return status;
}
// ******************************************************************               
// Scope: private (Implements CPipe::CheckForDoneTransfers = 0)
BOOL CIsochronousPipe::CheckForDoneTransfers( void )
//
// Purpose: Check if the transfer on this pipe is finished, and 
//          take the appropriate actions - i.e. remove the transfer
//          data structures and call any notify routines
//
// Parameters: None
//
// Returns: TRUE if this pipe is no longer busy; FALSE if there are still
//          some pending transfers.
//
// Notes:
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER, (TEXT("+CIsochronousPipe::CheckForDoneTransfers\n")) );

    BOOL fTransferDone = FALSE;
    EnterCriticalSection( &m_csPipeLock );
    if (m_pQueuedTransfer!=NULL) {
        DWORD dwFrame=0;
        m_pCEhcd->GetFrameNumber(&dwFrame);
        CIsochTransfer *  pCur=m_pQueuedTransfer;
        CIsochTransfer *  pPrev=NULL;
        while (pCur!=NULL ) { 
            if (pCur->IsTransferDone(dwFrame,0)) { //  Transfer Done.
                pCur->DoneTransfer(dwFrame,0);
                // Delete this Transfer 
                CIsochTransfer *  pNext = (CIsochTransfer * )pCur->GetNextTransfer();
                if (pPrev!=NULL)
                    pPrev->SetNextTransfer( pNext);
                else
                    m_pQueuedTransfer = pNext;
                delete pCur;
                pCur = pNext;
            }
            else {
                pPrev = pCur;
                pCur = (CIsochTransfer * )pCur ->GetNextTransfer();
            }
        };
    }
    LeaveCriticalSection( &m_csPipeLock );
    ScheduleTransfer() ;
    DEBUGMSG( ZONE_TRANSFER, (TEXT("-CIsochronousPipe::CheckForDoneTransfers, returning BOOL %d\n"), fTransferDone) );
    return fTransferDone;

};
// ******************************************************************               
// Scope: private (Implements CPipe::AreTransferParametersValid = 0)
BOOL CIsochronousPipe::AreTransferParametersValid( const STransfer *pTransfer ) const
//
// Purpose: Check whether this class' transfer parameters, stored in
//          m_transfer, are valid.
//
// Parameters: None (all parameters are vars of class)
//
// Returns: TRUE if parameters valid, else FALSE
//
// Notes: Assumes m_csPipeLock already held
// ******************************************************************
{
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("+CIsochronousPipe::AreTransferParametersValid\n")) );

    if (pTransfer == NULL) {
        ASSERT(FALSE);
        return FALSE;
    }

    // these parameters aren't used by CIsochronousPipe, so if they are non NULL,
    // it doesn't present a serious problem. But, they shouldn't have been
    // passed in as non-NULL by the calling driver.
    DEBUGCHK( pTransfer->lpvControlHeader == NULL ); // CONTROL
    // this is also not a serious problem, but shouldn't happen in normal
    // circumstances. It would indicate a logic error in the calling driver.
    DEBUGCHK( !(pTransfer->lpStartAddress == NULL && pTransfer->lpvNotifyParameter != NULL) );

    BOOL fValid = ( 
                    m_bDeviceAddress > 0 && m_bDeviceAddress <= USB_MAX_ADDRESS &&
                    pTransfer->dwStartingFrame >= m_dwLastValidFrame &&
                    pTransfer->lpvBuffer != NULL &&
                    // paClientBuffer could be 0 or !0
                    pTransfer->dwBufferSize > 0 &&
                    pTransfer->adwIsochErrors != NULL &&
                    pTransfer->adwIsochLengths != NULL &&
                    pTransfer->aLengths != NULL &&
                    pTransfer->dwFrames > 0 &&
                    pTransfer->lpfComplete != NULL &&
                    pTransfer->lpdwBytesTransferred != NULL &&
                    pTransfer->lpdwError != NULL );

    if ( fValid ) {
        __try {
            DWORD dwTotalData = 0;
            for ( DWORD frame = 0; frame < pTransfer->dwFrames; frame++ ) {
                if ( pTransfer->aLengths[ frame ] == 0 || 
                     pTransfer->aLengths[ frame ] > m_EndptBuget.max_packet ) {
                    fValid = FALSE;
                    break;
                }
                dwTotalData += pTransfer->aLengths[ frame ];
            }
            fValid = ( fValid &&
                       dwTotalData == pTransfer->dwBufferSize );
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            fValid = FALSE;
        }
    }
    ASSERT(fValid);
    DEBUGMSG( ZONE_TRANSFER && ZONE_VERBOSE, (TEXT("-CIsochronousPipe::AreTransferParametersValid, returning BOOL %d\n"), fValid) );
    return fValid;
}

//************************************************************************************
CPipeAbs * CreateBulkPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
               IN CHcd * const pChcd)
//
//  Purpose: Function to create the Bulk Pipe
//  
//  Parameters:
//
//  Returns: Pointer to CBulkPipe object
//
//**************************************************************************************
{ 
    return new CBulkPipe(lpEndpointDescriptor,fIsLowSpeed,fIsHighSpeed,bDeviceAddress,bHubAddress,bHubPort,(CEhcd * const)pChcd);
}

//**************************************************************************************
CPipeAbs * CreateControlPipe(IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
               IN CHcd * const pChcd)
//
//  Purpose: Function to create the Control Pipe
//  
//  Parameters:
//
//  Returns: Pointer to CControlPipe object
//
//**************************************************************************************
{ 
    return new CControlPipe(lpEndpointDescriptor,fIsLowSpeed,fIsHighSpeed,bDeviceAddress,bHubAddress,bHubPort,(CEhcd * const)pChcd);
}

//**************************************************************************************
CPipeAbs * CreateInterruptPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
               IN CHcd * const pChcd)
//
//  Purpose: Function to create the Interrupt Pipe
//  
//  Parameters:
//
//  Returns: Pointer to CInterruptPipe object
//
//**************************************************************************************
{ 
    return new CInterruptPipe(lpEndpointDescriptor,fIsLowSpeed,fIsHighSpeed,bDeviceAddress,bHubAddress,bHubPort,(CEhcd * const)pChcd);
}

//***************************************************************************************
CPipeAbs * CreateIsochronousPipe( IN const LPCUSB_ENDPOINT_DESCRIPTOR lpEndpointDescriptor,
               IN const BOOL fIsLowSpeed,IN const BOOL fIsHighSpeed,
               IN const UCHAR bDeviceAddress,
               IN const UCHAR bHubAddress,IN const UCHAR bHubPort,
               IN CHcd * const pChcd)
//
//  Purpose: Function to create the Isochronous Pipe
//  
//  Parameters:
//
//  Returns: Pointer to CIsochronousPipe object
//
//**************************************************************************************
{ 
    return new CIsochronousPipe(lpEndpointDescriptor,fIsLowSpeed,fIsHighSpeed,bDeviceAddress,bHubAddress,bHubPort,(CEhcd * const)pChcd);
}



