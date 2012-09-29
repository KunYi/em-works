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

#include "ufnmdd.h"



VOID CUfnMddTransfer::AddRef() 
{
    SETFNAME();
    
    DEBUGCHK(m_lRefCnt < 1000); // Look for use of freed memory or loops
    LONG lNewRefCnt = InterlockedIncrement(&m_lRefCnt);
    DEBUGCHK(m_lRefCnt < 1000); // Look for use of freed memory or loops

    DEBUGMSG(ZONE_TRANSFER && ZONE_COMMENT, (_T("%s Referenced 0x%08x to %x\r\n"), 
        pszFname, this, lNewRefCnt));
}


VOID CUfnMddTransfer::Release() 
{
    SETFNAME();
    
    DEBUGCHK(m_lRefCnt != 0);
    DEBUGCHK(m_lRefCnt < 1000); // Look for use of freed memory or loops
    LONG lNewRefCnt = InterlockedDecrement(&m_lRefCnt);
    DEBUGCHK(lNewRefCnt != -1);

    DEBUGMSG(ZONE_TRANSFER && ZONE_COMMENT, (_T("%s Dereferenced 0x%08x to %x\r\n"), 
        pszFname, this, lNewRefCnt));

    if (lNewRefCnt == 0) {
        m_pPipe->FreeTransfer(this);
    }
}


VOID CUfnMddTransfer::Init(DWORD dwFlags, PVOID pvBuffer, 
    DWORD dwBufferPhysicalAddress, 
    DWORD cbBuffer, PVOID pvPddTransferInfo, 
    LPTRANSFER_NOTIFY_ROUTINE lpNotifyRoutine, PVOID pvNotifyContext, 
    PCPipeBase pPipe
    )
{
    DEBUGCHK(pPipe);
    DEBUGCHK(m_dwSig == UFN_TRANSFER_SIG);
    DEBUGCHK(m_lRefCnt == 0);

    m_pPipe = pPipe;
    m_fInPdd = FALSE;
    m_lpNotifyRoutine = lpNotifyRoutine;
    m_pvNotifyContext = pvNotifyContext;

    m_PddTransfer.dwFlags = dwFlags;
    m_PddTransfer.pvBuffer = pvBuffer;
    m_PddTransfer.dwBufferPhysicalAddress = dwBufferPhysicalAddress;
    m_PddTransfer.cbBuffer = cbBuffer;
    m_PddTransfer.cbTransferred = 0;
    m_PddTransfer.dwUsbError = UFN_NOT_COMPLETE_ERROR;

    m_PddTransfer.pvPddData = NULL;
    m_PddTransfer.pvPddTransferInfo = pvPddTransferInfo;

    Validate();
}


VOID CUfnMddTransfer::CallCompletionRoutine(
    )
{
    SETFNAME();
    
    Validate();

    if (m_lpNotifyRoutine) {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try {
            (*m_lpNotifyRoutine)(m_pvNotifyContext);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Exception!\r\n"), pszFname));
        }
    }
}



DWORD CUfnMddTransfer::ReferenceTransferHandle(
    CUfnMddTransfer *pTransfer
    )
{
    SETFNAME();
    
    DWORD dwRet = ERROR_SUCCESS;
    
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try {
        if (!pTransfer) {
            DEBUGMSG(ZONE_WARNING, (_T("%s Invalid transfer handle 0x%08x\r\n"),
                pszFname, pTransfer));
            dwRet = ERROR_INVALID_HANDLE;
        }
        else if (pTransfer->m_dwSig != UFN_TRANSFER_SIG) {
            DEBUGMSG(ZONE_WARNING, (_T("%s Invalid transfer handle 0x%08x (sig 0x%X)\r\n"),
                pszFname, pTransfer, pTransfer->m_dwSig));
            dwRet = ERROR_INVALID_HANDLE;
        }
        else {
            pTransfer->AddRef();
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Exception checking transfer handle 0x%08x\r\n"),
            pszFname, pTransfer));
        dwRet = ERROR_INVALID_HANDLE;
    }

    return dwRet;
}


CUfnMddTransfer* CUfnMddTransfer::ConvertFromPddTransfer(
    PSTransfer pPddTransfer
    )
{
    DWORD dwTransfer = ((DWORD) pPddTransfer) - 
        offsetof(CUfnMddTransfer, m_PddTransfer);
    return (PCUfnMddTransfer) dwTransfer;
}
        
