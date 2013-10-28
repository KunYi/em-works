// All rights reserved ADENEO EMBEDDED 2010
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
//     Sync.hpp
// 
// 
// Notes: 
// 
#ifndef __SYNC_HPP_
#define __SYNC_HPP_

    typedef enum e_CritSec_Status {
        CSS_SUCCESS, CSS_DESTROYED, CSS_TIMEOUT
    } CritSec_Status;

class CritSec_Ex  {
private:
        DWORD m_hOwnerThread;
        UINT m_cOwnerReferences;
        UINT m_cWaitingThreads;
        HANDLE m_hev;
        CRITICAL_SECTION m_cs;
        BOOL m_fClosing;
public:
    CritSec_Ex();
    ~CritSec_Ex();
    void Initialize( );
    void PrepareDeleteCritSec_Ex ();
    CritSec_Status EnterCritSec_Ex ( ULONG ulTimeout);
    void LeaveCritSec_Ex ();
};
class Countdown {
private:
        CRITICAL_SECTION cs;
        HANDLE hev;
        BOOL lock;
        DWORD count;
        void LockCountdown ();
public:
    Countdown ( DWORD);
    ~Countdown ();
    BOOL IncrCountdown ();
    void DecrCountdown ();
    void WaitForCountdown (BOOL keepLocked);
    void UnlockCountdown () {
        EnterCriticalSection(&cs);
        lock = FALSE;
        LeaveCriticalSection(&cs);
    }
} ;

class LockObject {
private:
    CRITICAL_SECTION m_CSection;
public:
    LockObject() { InitializeCriticalSection(&m_CSection); };
    ~LockObject() { DeleteCriticalSection( &m_CSection); };
    void Lock(void) { EnterCriticalSection(&m_CSection); };
    void Unlock(void) {LeaveCriticalSection( &m_CSection); };
};


#endif

