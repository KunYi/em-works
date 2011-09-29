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
//  File:  vr4131_cmu.h
//
//  This header file defines clock mask unit register layout and associated
//  constants and types.
//
//  Unit base address is defined in vr4131_base_regs.h header file.
//
#ifndef __VR4131_CMU_H
#define __VR4131_CMU_H

//------------------------------------------------------------------------------

#ifndef __MIPS_ASSEMBLER

typedef struct {
    UINT16 CLKMSK;
} VR4131_CMU_REGS;

#endif // __MIPS_ASSEMBLER

//------------------------------------------------------------------------------

#define CMU_CLKMSKPPCIU         (1 << 13)
#define CMU_CLKMSKDSIU          (1 << 11)
#define CMU_CLKMSKSSIU          (1 << 8)
#define CMU_CLKMSKSIU           (1 << 1)
#define CMU_CLKMSKPCIU          (1 << 7)

//------------------------------------------------------------------------------

#endif
