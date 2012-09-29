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

//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------


/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

        ist.hpp

Abstract:

       IST framework.

--*/


#pragma once
#include <cmthread.h>
#include <CRegEdit.h>

class CIST : public CMiniThread {
public:
    CIST(LPCTSTR pszActiveKey, DWORD dwISTTimeout = INFINITE, DWORD dwTerminationTimeout = 5000); // Take IST setting from registry.
    virtual ~CIST();
    virtual BOOL Init();
    virtual BOOL IntializeInterrupt(LPVOID pvData = NULL, DWORD cbData = 0);
    HANDLE GetIISRHandle() {return m_hIsrHandler;};
    DWORD GetIRQ() {return m_dwIrq;};
    DWORD GetSysIntr() {return m_dwSysIntr;};
    BOOL IsIISRLoaded() {return m_hIsrHandler != NULL;};
    HANDLE GetISTEvent() {return m_hISTEvent;};
    BOOL IntChainHandlerIoControl(DWORD dwIoContro, PVOID pInPtr, DWORD dwInSize, PVOID pOutPtr, DWORD dwOutSize, PDWORD pdwActualWrite);
protected:
    DWORD m_ISTTimeout;
    DWORD m_TerminationTimeout;
private:
    virtual DWORD ThreadRun();
    virtual BOOL ISTProcess() = 0;
    virtual BOOL ISTTimeout() {return TRUE;};
    CRegistryEdit m_Reg;

    BOOL m_fAllocatedSysIntr;
    HANDLE m_hIsrHandler;
    HANDLE m_hISTEvent;
    DWORD m_dwIrq;
    DWORD m_dwSysIntr;
    BOOL m_fIntInitialized;
};

