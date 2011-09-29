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
//  File:  vr4131_icu.h
//
//  This header file defines Interrupt Control Unit register layout
//  and associated constants and types.
//
//  Unit base address is defined in vr4131_base_regs.h header file.
//
#ifndef __VR4131_ICU_H
#define __VR4131_ICU_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

typedef struct {
    UINT16 SYSINT1;
    UINT16 UNUSED1[3];
    UINT16 GIUINTL;
    UINT16 DSIUINT;
    UINT16 MSYSINT1;
    UINT16 UNUSED2[3];
    UINT16 MGIUINTL;
    UINT16 MDSIUINT;
    UINT16 NMI;
    UINT16 SOFTINT;
    UINT16 UNUSED3[2];
    UINT16 SYSINT2;
    UINT16 GIUINTH;
    UINT16 FIRINT;
    UINT16 MSYSINT2;
    UINT16 MGIUINTH;
    UINT16 MFIRINT;
    UINT16 PCIINT;
    UINT16 SCUINT;
    UINT16 CSIINT;
    UINT16 MPCIINT;
    UINT16 MSCUINT;
    UINT16 MCSIINT;
    UINT16 BCUINT;
    UINT16 MBCUINT;
} VR4131_ICU_REGS;

#endif // __MIPS_ASSEMBLER

//------------------------------------------------------------------------------

#define ICU_NMIORINT            (1 << 0)

#define ICU_INTDSIU             (1 << 11)

//------------------------------------------------------------------------------

#endif
