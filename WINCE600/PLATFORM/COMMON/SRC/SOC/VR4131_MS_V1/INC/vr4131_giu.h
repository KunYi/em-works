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
//  File:  vr4131_giu.h
//
//  This header file defines General-Purpose I/O Unit register layout and
//  associated constants and types.
//
//  Unit base address is defined in vr4131_base_regs.h header file.
//
#ifndef __VR4131_GIU_H
#define __VR4131_GIU_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

typedef struct {
    UINT16 GIUIOSELL;
    UINT16 GIUIOSELH;
    UINT16 GIUIODL;
    UINT16 GIUIODH;
    UINT16 GIUINTSTATL;
    UINT16 GIUINTSTATH;
    UINT16 GIUINTENL;
    UINT16 GIUINTENH;
    UINT16 GIUINTTYPL;
    UINT16 GIUINTTYPH;
    UINT16 GIUINTALSELL;
    UINT16 GIUINTALSELH;
    UINT16 GIUINTHTSELL;
    UINT16 GIUINTHTSELH;
    UINT16 GIUPODATEN;
    UINT16 GIUPODATL;
} VR4131_GIU_REGS;

#endif // __MIPS_ASSEMBLER

//------------------------------------------------------------------------------

#endif
