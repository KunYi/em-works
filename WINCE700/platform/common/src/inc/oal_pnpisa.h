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
//------------------------------------------------------------------------------
//
//  The OAL ISA module implements ISA bus support. The module must be
//  implemented on hardware which support ISA bus and it uses standard ISA
//  bus driver (which calls OAL/HAL to read/write ISA bus configuration space.
//  Module also implements simple ISA bus configration which is intented to
//  be used in boot loader/KITL implementation.
//
#ifndef __OAL_PNPISA_H
#define __OAL_PNPISA_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Function:  OALISACfgRead
//
UINT32
OALISACfgRead(
    UINT32 busId,
    UINT32 slot,
    UINT32 offset, 
    UINT32 size,
    __out_bcount(size) VOID *pData
    );

//------------------------------------------------------------------------------
//
//  Function:  OALISACfgWrite
//
UINT32
OALISACfgWrite(
    UINT32 busId, 
    UINT32 slot, 
    UINT32 offset, 
    UINT32 size,
    __in_bcount(size) VOID *pData
    );

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif // __OAL_PNPISA_H
