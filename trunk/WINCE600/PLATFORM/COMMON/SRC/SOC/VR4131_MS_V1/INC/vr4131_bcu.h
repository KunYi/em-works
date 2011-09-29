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
//  File:  vr4131_bcu.h
//
//  This header file defines bus control unit register layout and associated
//  constants and types.
//
//  Unit base address is defined in vr4131_base_regs.h header file.
//
#ifndef __VR4131_BCU_H
#define __VR4131_BCU_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

typedef volatile struct {
    UINT16 BCUCNT1;
    UINT16 UNUSED1;
    UINT16 BCUROMSIZE;
    UINT16 ROMSPEED;
    UINT16 IO0SPEED;
    UINT16 IO1SPEED;
    UINT16 BCUCNT3;
} VR4131_BCU_REGS;

#endif // __MIPS_ASSEMBLER

//------------------------------------------------------------------------------

#define VR4131_REG_OA_BCUCNT1       0x0000
#define VR4131_REG_OA_ROMSIZE       0x0004
#define VR4131_REG_OA_ROMSPEED      0x0006
#define VR4131_REG_OA_IO0SPEED      0x0008
#define VR4131_REG_OA_IO1SPEED      0x000A
#define VR4131_REG_OA_BCUCNT3       0x0016

//------------------------------------------------------------------------------

#define BCUCNT1_ROMWEN0             (1 << 4)
#define BCUCNT1_ROMWEN2             (1 << 6)

//------------------------------------------------------------------------------

#endif

