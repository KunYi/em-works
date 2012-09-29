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
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
//
//  Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
// Module Name:  
//     SdWroki.hpp
// Abstract:  
//     Async Work Item Queue process.
//
// 
// 
// Notes: 
// 
//
#pragma once
#include <Csync.h>
#include <CMthread.h>

#define MIN_WORK_ITEM 0x10

class CSDWorkItem : public CLockObject, public CMiniThread{
public:
    // post a message to the message queue
    BOOL PostEvent(SD_SLOT_EVENT sdEvent, DWORD dwWaitTick = INFINITE );
    // constructor
    CSDWorkItem(DWORD dwMaxItem = MIN_WORK_ITEM);
    // destructor
    ~CSDWorkItem();
    BOOL Init(DWORD dwCeThreadPriority);
    inline DWORD    IncIndex(DWORD dwIndex) {
        return (dwIndex < m_dwMaxOfSlotEvent-1? dwIndex+1: 0 );
    }
    inline BOOL     IsEmpty() {
        return (m_dwReadIndex== m_dwWriteIndex);
    }
    inline BOOL     IsFull() {
        return (IncIndex(m_dwWriteIndex) == m_dwReadIndex);
    };

protected:
    const DWORD     m_dwMaxOfSlotEvent;
    PSD_SLOT_EVENT  m_psdSlotEvent;
    HANDLE          m_hWakeupEvent;
    HANDLE          m_hEmptySlotSem;
    DWORD           m_dwReadIndex, m_dwWriteIndex;
private:
    virtual DWORD   ThreadRun() ;
    virtual VOID    SlotStatusChangeProcessing(SD_SLOT_EVENT sdEvent) = 0; 

};




