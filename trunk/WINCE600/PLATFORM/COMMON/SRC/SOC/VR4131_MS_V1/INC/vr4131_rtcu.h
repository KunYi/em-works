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
//  File:  vr4131_rtcu.h
//
//  This header file defines Realtime Clock Unit register layout and 
//  associated constants and types.
//
//  Unit base address is defined in vr4131_base_regs.h header file.
//
#ifndef __VR4131_RTCU_H
#define __VR4132_RTCU_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

typedef struct {
    UINT16 ETIMEL;
    UINT16 ETIMEM;
    UINT16 ETIMEH;
    UINT16 UNUSED1;
    UINT16 ECMPL;
    UINT16 ECMPM;
    UINT16 ECMPH;
    UINT16 UNUSED2;
    UINT16 RTCL1L;
    UINT16 RTCL1H;
    UINT16 RTCL1CNTL;
    UINT16 RTCL1CNTH;
    UINT16 RTCL2L;
    UINT16 RTCL2H;
    UINT16 RTCL2CNTL;
    UINT16 RTCL2CNTH;
    UINT16 TCLKL;
    UINT16 TCLKH;
    UINT16 TCLKCNTL;
    UINT16 TCLKCNTH;
    UINT16 UNUSED[11];
    UINT16 RTCINT;
} VR4131_RTCU_REGS;

#endif // __MIPS_ASSEMBLER

//------------------------------------------------------------------------------

#endif
