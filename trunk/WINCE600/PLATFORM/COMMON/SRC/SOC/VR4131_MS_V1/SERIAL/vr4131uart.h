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

//-----------------------------------------------------------------------------

#ifndef __VR4131UART_H
#define __VR4131UART_H

#include "pdd16550.h"

class CVR4131Pdd16550: public CPdd16550 {

public:
    virtual BOOL SetBaudRate(ULONG BaudRate, BOOL bIrModule);

    CVR4131Pdd16550(LPTSTR lpActivePath, PVOID pMdd, PHWOBJ pHwObj);
};

#endif

//-----------------------------------------------------------------------------
