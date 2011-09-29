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
#pragma once

#include <sync.hxx>
#include <list.hxx>


// Keeps a list of context values generated from XXX_Open calls.
class COpenList : public ce::list<DWORD>, public ce::critical_section {
protected:
    typedef ce::list<DWORD>         _BaseList;
    typedef ce::critical_section    _BaseSync;

public:    
    COpenList() : _BaseList(), _BaseSync() {}
    ~COpenList() {
        DEBUGCHK(size() == 0);
    }

    // Call from XXX_Open. Add a context to the list. Returns false upon failure.
    bool AddContext(DWORD dwContext) {
        ce::gate<_BaseSync> gate(*this);
        return push_back(dwContext);
    }

    // Call from XXX_Close. Remove a context from the list. 
    // The context must be present in the list.
    void RemoveContext(DWORD dwContext) {
        ce::gate<_BaseSync> gate(*this);
        DEBUGCHK( std::find(begin(), end(), dwContext) != end() );
        remove(dwContext);
    }

    // Call from XXX_PreDeinit. Calls the provided PreClose routine for each
    // context in the list.
    template <class Fn>
    void PreDeinit(Fn PreClose) {
        ce::gate<_BaseSync> gate(*this);
        std::for_each(begin(), end(), PreClose);
    }

    // Call from XXX_Deinit. Calls the provided Close routine for each context
    // in the list. Note this this acts under the assumption that the provided
    // Close routine will actually remove the context from the list.
    template <class Fn>
    void Deinit(Fn Close) {
        // No real need to lock here, but it might help find problems.
        ce::gate<_BaseSync> gate(*this);
        while (!empty()) {
            Close(front());
        }
    }
};

