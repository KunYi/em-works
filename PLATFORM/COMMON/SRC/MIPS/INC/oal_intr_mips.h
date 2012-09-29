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
//  File:  oal_intr_mips.h
//
#ifndef __OAL_INTR_MIPS_H
#define __OAL_INTR_MIPS_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

// Interrupt handler function
UINT32 OALIntr0Handler();
UINT32 OALIntr1Handler();
UINT32 OALIntr2Handler();
UINT32 OALIntr3Handler();
UINT32 OALIntr4Handler();
UINT32 OALProfileIntrHandler();

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
