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
//  File:  sh4_cpg.h
//
//  This header file defines registers layout and
//  associated constants and types.
//
//  Note:  Base address is defined in sh4_base_regs.h header file.
//
#ifndef __SH4_CPG_H
#define __SH4_CPG_H

//------------------------------------------------------------------------------

typedef struct {
    UINT16  FRQCR;
    UINT16  UNUSED2[1];
    UINT8   STBCR;
    UINT8   UNUSED5[3];
    union   WTCNT 
    {
        UINT16  WTCNT_WRITE;
        UINT8   WTCNT_READ[2];
    };
    UINT16  UNUSEDA[1];
    union   WTCSR 
    {
        UINT16  WTCSR_WRITE;
        UINT8   WTCSR_READ[2];
    };
    UINT16  UNUSEDE[1];
    UINT8   STBCR2;
    UINT8   UNUSED11[3];
} SH4_CPG_REGS, *PSH4_CPG_REGS;

//------------------------------------------------------------------------------

#endif
