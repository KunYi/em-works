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

#ifndef _XFERQUEUE_H_
#define _XFERQUEUE_H_

#include <usbfn.h>
#include <block_allocator.hxx>
#include <list.hxx>


// Access to this structure must be protected by a critical section.
template <class T>
class CTransferQueue {
public:
    CTransferQueue() : m_queue(*s_pListStructAllocator) {
    }

    ~CTransferQueue() {
        DEBUGCHK(IsEmpty());
    }

    BOOL IsEmpty() const {
        BOOL fRet = m_queue.empty();
        return fRet;
    }

    T* Front() const {
        DEBUGCHK(IsEmpty() == FALSE);
        return m_queue.front();
    }

    T* Back() const {
        DEBUGCHK(IsEmpty() == FALSE);
        return m_queue.back();
    }
    
    DWORD PushBack(T* pTransfer) {
        DWORD dwErr = ERROR_SUCCESS;
        
        if (!m_queue.push_back(pTransfer)) {
            dwErr = ERROR_GEN_FAILURE;
        }

        return dwErr;
    }

    VOID PopFront() {
        DEBUGCHK(IsEmpty() == FALSE);
        return m_queue.pop_front();
    }

    VOID PopBack() {
        DEBUGCHK(IsEmpty() == FALSE);
        return m_queue.pop_back();
    }

    static VOID InitializeClass(DWORD dwListSize) {
        DEBUGCHK(s_pListStructAllocator == NULL);
        s_pListStructAllocator = (ListStructAllocator*) &s_rgbBuffer;
        new(s_pListStructAllocator) ListStructAllocator(dwListSize);
    }

#ifdef DEBUG
    static BOOL InitializeClassCalled() {
        return (s_pListStructAllocator != NULL);
    }
#endif
    
    static VOID DeinitializeClass() {
        DEBUGCHK(InitializeClassCalled());
        s_pListStructAllocator->~ListStructAllocator();
        s_pListStructAllocator = NULL;
    }
    
    
private:
    typedef ce::fixed_block_allocator<> ListStructAllocator; 
    typedef ce::list<T*, ListStructAllocator> ActualTransferQueue;

    static BYTE s_rgbBuffer[sizeof(ListStructAllocator)];
    static ListStructAllocator *s_pListStructAllocator;
    
    
    ActualTransferQueue m_queue; // Queue of transfers
};


// Use this macro to declare the statics required by this class
#define CTransferQueue_DECLARE_STATICS(T) \
    __declspec(align(16)) BYTE CTransferQueue<T>::s_rgbBuffer[sizeof(CTransferQueue<T>::ListStructAllocator)]; \
    CTransferQueue<T>::ListStructAllocator *CTransferQueue<T>::s_pListStructAllocator = NULL;



#endif // _XFERQUEUE_H_

