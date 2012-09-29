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

#ifndef _TRANSFER_H_
#define _TRANSFER_H_

#include <usbfn.h>

typedef class CPipeBase *PCPipeBase;
typedef struct UFN_MDD_CONTEXT *PUFN_MDD_CONTEXT;


#define UFN_TRANSFER_SIG        'TnfU' // "UfnT" signature
#define UFN_FREE_TRANSFER_SIG   'TdaB' // "BadT" signature


typedef class CUfnMddTransfer {
public:
    CUfnMddTransfer() {
        ClearMembers();
        m_dwSig = UFN_TRANSFER_SIG;
        m_lRefCnt = 0;
    }

    ~CUfnMddTransfer() {
        DEBUGCHK( m_dwSig == UFN_TRANSFER_SIG || 
                m_dwSig == UFN_FREE_TRANSFER_SIG );
        m_dwSig = GARBAGE_DWORD;
    }

    VOID MarkAsFree() {
        DEBUGCHK(m_dwSig == UFN_TRANSFER_SIG);
        DEBUGCHK(m_fInPdd == FALSE);
        DEBUGCHK(m_lRefCnt == 0);
        ClearMembers();
        m_dwSig = UFN_FREE_TRANSFER_SIG;
    }

    VOID MarkAsValid() {
        DEBUGCHK(m_dwSig == UFN_FREE_TRANSFER_SIG);
        m_dwSig = UFN_TRANSFER_SIG;
        m_lRefCnt = 0;
    }
    
    VOID AddRef();
    VOID Release();

    VOID Init(DWORD dwFlags, PVOID pvBuffer, DWORD dwBufferPhysicalAddress, 
        DWORD cbBuffer, PVOID pvPddTransferInfo, 
        LPTRANSFER_NOTIFY_ROUTINE lpNotifyRoutine, PVOID pvNotifyContext, 
        PCPipeBase pPipe);

    PCPipeBase GetPipe() { return m_pPipe; }

    BOOL IsInPdd() { return m_fInPdd; }
    VOID EnteringPdd() {
        DEBUGCHK(m_fInPdd == FALSE);
        AddRef();
        m_fInPdd = TRUE;
    }
    
    VOID LeavingPdd() {
        DEBUGCHK(m_fInPdd == TRUE);
        m_fInPdd = FALSE;
        Release();
    }

    VOID CallCompletionRoutine();

    DWORD GetDwUsbError() const { return m_PddTransfer.dwUsbError; }
    DWORD GetCbBuffer() const { return m_PddTransfer.cbBuffer; }
    PVOID GetPvBuffer() const { return m_PddTransfer.pvBuffer; }
    DWORD GetDwFlags() const { return m_PddTransfer.dwFlags; }
    DWORD GetCbTransferred() const { return m_PddTransfer.cbTransferred; }
    
    PSTransfer GetPddTransfer() { return &m_PddTransfer; }

    VOID SetDwUsbError(DWORD dwUsbError) {
        DEBUGCHK(!IsComplete());
        m_PddTransfer.dwUsbError = dwUsbError;
    }
    
    BOOL IsComplete() { return (GetDwUsbError() != UFN_NOT_COMPLETE_ERROR); }

#ifdef DEBUG
    VOID Validate() {
        DEBUGCHK(m_dwSig == UFN_TRANSFER_SIG);
        DEBUGCHK(m_pPipe);
        DEBUGCHK(!GetCbBuffer() || GetPvBuffer());
    }
#else
    VOID Validate() {}
#endif

    static CUfnMddTransfer* ConvertFromPddTransfer(PSTransfer pPddTransfer);

    static DWORD ReferenceTransferHandle(CUfnMddTransfer *pTransfer);

private:
#ifdef DEBUG
    VOID ClearMembers() {
        memset(this, GARBAGE_DWORD, sizeof(*this));
    }
#else
    VOID ClearMembers() {}
#endif

    DWORD                       m_dwSig;
    LONG                        m_lRefCnt;
    PCPipeBase                  m_pPipe;
    BOOL                        m_fInPdd;

    LPTRANSFER_NOTIFY_ROUTINE   m_lpNotifyRoutine;
    PVOID                       m_pvNotifyContext;

    STransfer                   m_PddTransfer;
} *PCUfnMddTransfer;

#endif // _TRANSFER_H_

