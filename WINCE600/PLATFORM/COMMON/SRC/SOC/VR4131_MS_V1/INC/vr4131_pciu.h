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
//  File:  vr4131_pciu.h
//
//  This header file defines PCI control unit register layout and
//  associated constants and types.
//
//  Unit base address is defined in vr4131_base_regs.h header file.
//
#ifndef __VR4131_PCIU_H
#define __VR4131_PCIU_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

typedef struct {
    UINT32 PCIMMAW1;
    UINT32 PCIMMAW2;
    UINT32 PCITAW1;
    UINT32 PCITAW2;
    UINT32 PCIMIOAW;
    UINT32 PCICONFD;
    UINT32 PCICONFA;
    UINT32 PCIMAIL;
    UINT32 UNUSED1;
    UINT32 PCIBUSERRA;
    UINT32 PCIINT;
    UINT32 PCIEXACC;
    UINT32 PCIRECONT;
    UINT32 PCIEN;
    UINT32 PCICLKSEL;
    UINT32 PCITRDYV;
    UINT32 UNUSED2[8];
    UINT32 PCICLKRUN;
    UINT32 UNUSED3[39];

    UINT32 PCICFGID;
    UINT32 PCICFGCMDST;
    UINT32 PCICFGCLSREV;
    UINT32 PCICFGHEAD;
    UINT32 PCICFGMAIL;
    UINT32 PCICFGBASE1;
    UINT32 PCICFGBASE2;
    UINT32 UNUSED4[8];
    UINT32 PCICFGINT;
    UINT32 PCICFGARB;
} VR4131_PCIU_REGS;

#endif // __MIPS_ASSEMBLER

//------------------------------------------------------------------------------

#define PCIU_CONFIG_DONE        (1 << 2)

//------------------------------------------------------------------------------

#endif
