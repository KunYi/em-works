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
//  File:  sh4_ubc.h
//
//  This header file defines Bus State Control registers layout and
//  associated constants and types.
//
//  Note:  Base address is defined in sh4_base_regs.h header file.
//
#ifndef __SH4_UBC_H
#define __SH4_UBC_H

//------------------------------------------------------------------------------

typedef struct {
    UINT32  BARA;
    UINT8   BAMRA;
    UINT8   UNUSED5[3];
    UINT16  BBRA;
    UINT16  UNUSEDA[1];
    UINT32  BARB;
    UINT8   BAMRB;
    UINT8   UNUSED11[3];
    UINT16  BBRB;
    UINT16  UNUSED16[1];
    UINT32  BDRB;
    UINT32  BDMRB;
    UINT32  BRCR;
} SH4_UBC_REGS, *PSH4_UBC_REGS;

//------------------------------------------------------------------------------

#endif
