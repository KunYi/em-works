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
//  File:  vr4131_sdramu.h
//
//  This header file defines SDRAM unit register layout and associated
//  constants and types.
//
//  Unit base address is defined in vr4131_base_regs.h header file.
//
#ifndef __VR4131_SDRAMU_H
#define __VR4131_SDRAMU_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

#endif // __MIPS_ASSEMBLER

//------------------------------------------------------------------------------

#define VR4131_REG_OA_SDRAMMODE     0x0000
#define VR4131_REG_OA_SDRAMCNT      0x0002
#define VR4131_REG_OA_BCURFCNT      0x0004
#define VR4131_REG_OA_RAMSIZE       0x0008

//------------------------------------------------------------------------------

#endif
