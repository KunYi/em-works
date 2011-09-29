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
#include <windows.h>
#include <windows.h>
#include <types.h>
#include <ceddk.h>

#include <ddkreg.h>
#include <serhw.h>
#include <hw16550.h>
#include <Serdbg.h>
#include "vr4131uart.h"

//------------------------------------------------------------------------------

BOOL CVR4131Pdd16550::SetBaudRate(ULONG baudRate, BOOL irModule)
{
    ULONG divisor;

    if (baudRate >= 50 || baudRate <= 1152000) {
        divisor = (1152000 + baudRate/2 - 1)/baudRate;
        m_HardwareLock.Lock();
        InterruptMask(m_dwSysIntr,TRUE);
        __try {
            m_pReg16550->Write_BaudRate((UINT16)divisor);
        }__except( EXCEPTION_EXECUTE_HANDLER ) {
        };
        InterruptMask(m_dwSysIntr,FALSE);
        m_HardwareLock.Unlock();      
        return TRUE;
    }
    else
        return FALSE;
}

//------------------------------------------------------------------------------

CVR4131Pdd16550::CVR4131Pdd16550(
    LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj
) : CPdd16550(lpActivePath, pMdd, pHwObj)
{
}

//------------------------------------------------------------------------------

CSerialPDD * CreateSerialObject(
    LPTSTR lpActivePath, PVOID pMdd,PHWOBJ pHwObj, DWORD DeviceArrayIndex
) {
    CSerialPDD * pSerialPDD = NULL;

    pSerialPDD = new CVR4131Pdd16550(lpActivePath, pMdd, pHwObj);
    if (pSerialPDD != NULL && !pSerialPDD->Init()) {
        delete pSerialPDD;
        pSerialPDD = NULL;
    }
    return pSerialPDD;
}

//------------------------------------------------------------------------------

void DeleteSerialObject(CSerialPDD * pSerialPDD)
{
    if (pSerialPDD != NULL) delete pSerialPDD;
}

//------------------------------------------------------------------------------
