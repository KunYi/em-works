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

#ifndef _INTRLK_STACK_
#define _INTRLK_STACK_

#include <intrlk.h>


// A stack that uses Interlocked functions instead of critical sections
// for a performance improvement.
template <typename T, typename _Al = ce::allocator>
class interlocked_stack : public _Al
{
protected:
    struct _NODE : SINGLE_LIST_ENTRY
    {
        T value;
    };
    typedef _NODE *_PNODE;
    
public:
    typedef _Al     _MyBase;

    interlocked_stack() 
        : _MyBase() 
    {
        m_head.Next = NULL;
    }

    explicit interlocked_stack(const _MyBase& Alloc) 
        : _MyBase(Alloc) 
    {
        m_head.Next = NULL;
    }

    ~interlocked_stack() {}

    // Returns false if the node could not be allocated.
    bool push(const T &value) 
    {
        _PNODE pNode = (_PNODE) _MyBase::allocate(sizeof(_NODE));
        
        if (pNode) {
            pNode->value = value;
            InterlockedPushEntrySingleList(&m_head, pNode);
        }

        return (pNode != NULL);
    }

    // Returns false if the stack is empty.
    // Note that calls to this method must be serialized. If there 
    // are multiple threads calling this method, protect calls with
    // a critical section.
    bool pop(T &value) 
    {
        _PNODE pNode = (_PNODE) InterlockedPopEntrySingleList(&m_head);
        
        if (pNode) {
            value = pNode->value;
            _MyBase::deallocate(pNode, sizeof(_NODE));
        }
        
        return (pNode != NULL);
    }

protected:
    SINGLE_LIST_ENTRY m_head;   // List head
};

#endif // _INTRLK_STACK_

