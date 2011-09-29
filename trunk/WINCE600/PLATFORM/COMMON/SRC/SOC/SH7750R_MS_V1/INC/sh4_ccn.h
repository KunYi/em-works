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
//  File:  sh4_ccn.h
//
//  This header file defines CCN controllers register layout and
//  associated constants and types.
//
//  Note:  Unit base addresses are defined in sh4_base_regs.h header file.
//
#ifndef __SH4_CCN_H
#define __SH4_CCN_H

//------------------------------------------------------------------------------

typedef struct {
    UINT32  PTEH;
    UINT32  PTEL;
    UINT32  TTB;
    UINT32  TEA;
    UINT32  MMUCR;
    UINT8   BASRA;
    UINT8   UNUSED15[3];
    UINT8   BASRB;
    UINT8   UNUSED19[3];
    UINT32  CCR;
    UINT32  TRA;
    UINT32  EXPEVT;
    UINT32  INTEVT;
    UINT32  PTEA;
    UINT32  QACR0;
    UINT32  QACR1;
} SH4_CCN_REGS, *PSH4_CCN_REGS;

//------------------------------------------------------------------------------

#endif
