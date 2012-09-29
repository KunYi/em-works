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

        ist.cpp

Abstract:

       IST framework.

--*/
#include <windows.h>
#include <types.h>

#include <nkintr.h>
#pragma warning(push)
#pragma warning(disable: 4100 4512 4245 6258 6262 6287)
#include "ist.hpp"
#pragma warning(pop)

CIST::CIST(LPCTSTR pszActiveKey, DWORD dwISTTimeout, DWORD dwTerminationTimeout)
: CMiniThread(0, TRUE)
, m_Reg(pszActiveKey)
, m_ISTTimeout(dwISTTimeout)
, m_TerminationTimeout(dwTerminationTimeout)
{
    m_fAllocatedSysIntr = FALSE;
    m_hIsrHandler = NULL;
    m_hISTEvent = NULL;
    m_fIntInitialized = FALSE;
    m_dwIrq = IRQ_UNSPECIFIED;
    m_dwSysIntr = (DWORD)SYSINTR_UNDEFINED;

    DDKISRINFO ddi;
    if (m_Reg.IsKeyOpened() && m_Reg.GetIsrInfo(&ddi) == ERROR_SUCCESS)
    {
        if (ddi.dwIrq != IRQ_UNSPECIFIED)
        {
            if (ddi.dwSysintr == SYSINTR_NOP)
            {
                // We need allocate SysIntr for this IRQ.
                BOOL RetVal = KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &ddi.dwIrq, sizeof(ddi.dwIrq), &ddi.dwSysintr, sizeof(ddi.dwSysintr), NULL);
                if (!RetVal || ddi.dwSysintr == SYSINTR_UNDEFINED)
                {
                    ddi.dwSysintr = SYSINTR_NOP;
                }
                else
                {
                    m_fAllocatedSysIntr = TRUE;
                }
            }
        }
        if (ddi.dwIrq != IRQ_UNSPECIFIED && ddi.dwSysintr != SYSINTR_NOP && ddi.szIsrDll[0] != 0 && ddi.szIsrHandler[0] != 0)
        {
            // Time for IISR
            m_hIsrHandler = LoadIntChainHandler(ddi.szIsrDll, ddi.szIsrHandler, (BYTE)ddi.dwIrq);
        }
        m_dwIrq = ddi.dwIrq;
        m_dwSysIntr = ddi.dwSysintr;
    }
}

BOOL CIST::Init()
{
    return (m_dwSysIntr != SYSINTR_NOP);
}

BOOL CIST::IntChainHandlerIoControl(DWORD dwIoContro, PVOID pInPtr, DWORD dwInSize, PVOID pOutPtr, DWORD dwOutSize, PDWORD pdwActualWrite)
{
    BOOL fReturn = (m_hIsrHandler?
        (::IntChainHandlerIoControl(m_hIsrHandler, dwIoContro, pInPtr, dwInSize, pOutPtr, dwOutSize, pdwActualWrite)) :
        TRUE);
    ASSERT(fReturn);
    return fReturn;
}

BOOL CIST::IntializeInterrupt(LPVOID pvData, DWORD cbData)
{
    if (m_dwSysIntr != SYSINTR_NOP && !m_hISTEvent && !m_fIntInitialized)
    {
        m_hISTEvent = CreateEvent(0, FALSE, FALSE, NULL);
        if (m_hISTEvent)
        {
            m_fIntInitialized = InterruptInitialize(m_dwSysIntr, m_hISTEvent, pvData, cbData);
        }
    }
    if (m_fIntInitialized)
    {
        ThreadStart();
    }
    return m_fIntInitialized;
}

CIST::~CIST()
{
    m_bTerminated = TRUE;
    ThreadStart();
    if (m_hISTEvent)
    {
        SetEvent(m_hISTEvent);
        ThreadTerminated(m_TerminationTimeout);
    };

    if (m_fIntInitialized)
    {
        InterruptDisable(m_dwSysIntr);
    }
    if (m_hISTEvent)
        CloseHandle(m_hISTEvent);

    if (m_hIsrHandler)
    {
        FreeIntChainHandler(m_hIsrHandler);
    }

    if (m_fAllocatedSysIntr && m_dwSysIntr != SYSINTR_NOP)
    {
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &m_dwSysIntr, sizeof(m_dwSysIntr), NULL, 0, NULL);
    }
}

DWORD CIST::ThreadRun()
{
    if (!m_bTerminated && m_dwSysIntr != SYSINTR_NOP && m_hISTEvent != NULL)
    {
        BOOL fRet = TRUE;
        while (!m_bTerminated && fRet)
        {
            switch (WaitForSingleObject(m_hISTEvent, m_ISTTimeout))  // m_ISTTimeout is "INFINITE"
            {
            case WAIT_OBJECT_0:
                fRet = ISTProcess();
                ASSERT(fRet);
                InterruptDone(m_dwSysIntr);
                break;
            case WAIT_TIMEOUT:
                fRet = ISTTimeout();
                ASSERT(fRet);
                break;
            default:
                ASSERT(FALSE);
                fRet = FALSE;
                break;
            }
        }
    }
    return 0;
}
