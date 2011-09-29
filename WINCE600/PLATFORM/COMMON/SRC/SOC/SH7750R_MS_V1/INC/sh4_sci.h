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
//  File:  sh4_sci.h
//
//  This header file defines Serial Communication Interface registers layout and
//  associated constants and types.
//
//  Note:  Base address is defined in sh4_base_regs.h header file.
//
#ifndef __SH4_SCI_H
#define __SH4_SCI_H

//------------------------------------------------------------------------------

typedef struct {
    UINT8   SCSMR1;
    UINT8   UNUSED1[3];
    UINT8   SCBRR1;
    UINT8   UNUSED5[3];
    UINT8   SCSCR1;
    UINT8   UNUSED9[3];
    UINT8   SCTDR1;
    UINT8   UNUSEDD[3];
    UINT8   SCSSR1;
    UINT8   UNUSED11[3];
    UINT8   SCRDR1;
    UINT8   UNUSED15[3];
    UINT8   SCSCMR1;
    UINT8   UNUSED19[3];
    UINT8   SCSPTR1;
    UINT8   UNUSED1D[3];
} SH4_SCI_REGS, *PSH4_SCI_REGS;

//------------------------------------------------------------------------------

#define SCI_SCSCR1_TIE      0x80
#define SCI_SCSCR1_RIE      0x40
#define SCI_SCSCR1_MPIE     0x08
#define SCI_SCSCR1_TEIE     0x04

#define SCI_SCSPTR1_EIE     0x80

//------------------------------------------------------------------------------

#endif
