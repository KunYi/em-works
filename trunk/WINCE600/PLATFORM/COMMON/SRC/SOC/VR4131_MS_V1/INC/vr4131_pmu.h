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
//  File:  vr4131_pmu.h
//
//  This header file defines power management unit register layout and
//  associated constants and types.
//
//  Unit base address is defined in vr4131_base_regs.h header file.
//
#ifndef __VR4131_PMU_H
#define __VR4131_PMU_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

typedef struct {
    UINT16 PMUINT;
    UINT16 PMUCNT;
    UINT16 PMUIT2;
    UINT16 PMUCNT2;
    UINT16 PMUWAIT;
    UINT16 UNUSED1;
    UINT16 PMUTCLKDIV;
    UINT16 PMUINTRCLKDIV;
} VR4131_PMU_REGS;


#endif // __MIPS_ASSEMBLER

#define VR4131_REG_OA_PMUINT        0x0000
#define VR4131_REG_OA_PMUCNT        0x0002
#define VR4131_REG_OA_PMUIT2        0x0004
#define VR4131_REG_OA_PMUCNT2       0x0006
#define VR4131_REG_OA_PMUWAIT       0x0008
#define VR4131_REG_OA_PMUTCLKDIV    0x000C
#define VR4131_REG_OA_PMUINTRCLKDIV 0x000E

//------------------------------------------------------------------------------

#define PMU_SOFTRST                 (1 << 4)

//------------------------------------------------------------------------------

#endif
