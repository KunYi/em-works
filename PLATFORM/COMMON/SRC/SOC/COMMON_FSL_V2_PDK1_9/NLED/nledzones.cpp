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
//------------------------------------------------------------------------------
//
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include <windows.h>

#ifdef DEBUG

#define DEBUGMASK(bit)      (1 << (bit))

#define MASK_ERROR          DEBUGMASK(0)
#define MASK_WARN           DEBUGMASK(1)
#define MASK_INIT           DEBUGMASK(2)
#define MASK_IOCTL          DEBUGMASK(3)
#define MASK_HARDWARE       DEBUGMASK(4)
#define MASK_PDD            DEBUGMASK(5)
#define MASK_FUNCTION       DEBUGMASK(6)

DBGPARAM dpCurSettings = {
    _T("NLED"), 
    {
        _T("Errors"), _T("Warnings"), _T("Init"), _T("Ioctl"), 
        _T("Hardware"), _T("PDD"), _T("Function"), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T(""),_T(""),_T(""),_T("") 
    },
    MASK_ERROR | MASK_WARN
}; 

#endif
